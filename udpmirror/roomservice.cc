#include "callerlist.h"
#include "common.h"
#include "udpsocket.h"
#include <condition_variable>
#include <queue>
#include <signal.h>
#include <string.h>
#include <thread>
#include <vector>

#include <curl/curl.h>

CURL* curl;

class latreport_t {
public:
  latreport_t() : src(0), dest(0), tmean(0), jitter(0){};
  latreport_t(stage_device_id_t src_, stage_device_id_t dest_, double tmean_,
              double jitter_)
      : src(src_), dest(dest_), tmean(tmean_), jitter(jitter_){};
  stage_device_id_t src;
  stage_device_id_t dest;
  double tmean;
  double jitter;
};

// period time of participant list announcement, in ping periods:
#define PARTICIPANTANNOUNCEPERIOD 20

static bool quit_app(false);

class udpreceiver_t : public endpoint_list_t {
public:
  udpreceiver_t(int portno, int prio);
  ~udpreceiver_t();
  int portno;
  void srv();
  void announce_new_connection(stage_device_id_t cid, const ep_desc_t& ep);
  void announce_connection_lost(stage_device_id_t cid);
  void announce_latency(stage_device_id_t cid, double lmin, double lmean,
                        double lmax, uint32_t received, uint32_t lost);
  void set_lobbyurl(const std::string& url) { lobbyurl = url; };
  void set_roomname(const std::string& name) { roomname = name; };

private:
  void announce_service();
  std::thread announce_thread;
  void ping_and_callerlist_service();
  std::thread logthread;
  void quitwatch();
  std::thread quitthread;
  const int prio;

  ovbox_udpsocket_t socket;
  bool runsession;
  struct sockaddr_in serv_addr;
  secret_t secret;
  std::string roomname;
  std::string lobbyurl;

  std::queue<latreport_t> latfifo;
  std::mutex latfifomtx;
};

udpreceiver_t::udpreceiver_t(int portno_, int prio)
    : portno(portno_), prio(prio), socket(secret), runsession(true),
      secret(1234), roomname("lakeview"), lobbyurl("http://localhost")
{
  endpoints.resize(255);
  portno = socket.bind(portno);
  logthread = std::thread(&udpreceiver_t::ping_and_callerlist_service, this);
  quitthread = std::thread(&udpreceiver_t::quitwatch, this);
  announce_thread = std::thread(&udpreceiver_t::announce_service, this);
}

udpreceiver_t::~udpreceiver_t()
{
  runsession = false;
  logthread.join();
  quitthread.join();
}

void udpreceiver_t::quitwatch()
{
  while(!quit_app)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  runsession = false;
  socket.close();
}

void udpreceiver_t::announce_new_connection(stage_device_id_t cid,
                                            const ep_desc_t& ep)
{
  log(portno,
      "new connection for " + std::to_string(cid) + " from " + ep2str(ep.ep) +
          " in " + ((ep.mode & B_PEER2PEER) ? "peer-to-peer" : "server") +
          "-mode" + ((ep.mode & B_DOWNMIXONLY) ? " downmixonly" : "") +
          ((ep.mode & B_DONOTSEND) ? " donotsend" : "") + " v" + ep.version);
}

void udpreceiver_t::announce_connection_lost(stage_device_id_t cid)
{
  log(portno, "connection for " + std::to_string(cid) + " lost.");
}

void udpreceiver_t::announce_latency(stage_device_id_t cid, double lmin,
                                     double lmean, double lmax,
                                     uint32_t received, uint32_t lost)
{
  if(lmean > 0) {
    {
      std::lock_guard<std::mutex> lk(latfifomtx);
      latfifo.push(latreport_t(cid, 200, lmean, lmax - lmean));
    }
    char ctmp[1024];
    sprintf(ctmp, "latency %d min=%1.2fms, mean=%1.2fms, max=%1.2fms", cid,
            lmin, lmean, lmax);
    log(portno, ctmp);
  }
}

// this thread announces the room service to the lobby:
void udpreceiver_t::announce_service()
{
  // participand announcement counter:
  uint32_t cnt(0);
  char cpost[1024];
  while(runsession) {
    if(!cnt) {
      // if nobody is connected create a new pin:
      if(get_num_clients() == 0) {
        long int r(random());
        secret = r & 0xfffffff;
        socket.set_secret(secret);
      }
      // register at lobby:
      CURLcode res;
      sprintf(cpost, "?port=%d&name=%s&pin=%d", portno, roomname.c_str(),
              secret);
      std::string url(lobbyurl);
      url += cpost;
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_USERPWD, "room:room");
      curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
      curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
      res = curl_easy_perform(curl);
      if(res == 0)
        cnt = 3000;
      else
        cnt = 200;
    }
    --cnt;
    std::this_thread::sleep_for(std::chrono::milliseconds(PINGPERIODMS));
    while(!latfifo.empty()) {
      latreport_t lr(latfifo.front());
      latfifo.pop();
      // register at lobby:
      sprintf(cpost, "?latreport=%d&src=%d&dest=%d&lat=%1.1f&jit=%1.1f", portno,
              lr.src, lr.dest, lr.tmean, lr.jitter);
      std::string url(lobbyurl);
      url += cpost;
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_USERPWD, "room:room");
      curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
      curl_easy_perform(curl);
    }
  }
}

// this thread sends ping and participant list messages
void udpreceiver_t::ping_and_callerlist_service()
{
  char buffer[BUFSIZE];
  // participand announcement counter:
  uint32_t participantannouncementcnt(PARTICIPANTANNOUNCEPERIOD);
  while(runsession) {
    std::this_thread::sleep_for(std::chrono::milliseconds(PINGPERIODMS));
    // send ping message to all connected endpoints:
    for(stage_device_id_t cid = 0; cid != MAXEP; ++cid) {
      if(endpoints[cid].timeout) {
        // endpoint is connected
        socket.send_ping(cid, endpoints[cid].ep);
      }
    }
    if(!participantannouncementcnt) {
      // announcement of connected participants to all clients:
      participantannouncementcnt = PARTICIPANTANNOUNCEPERIOD;
      for(stage_device_id_t cid = 0; cid != MAXEP; ++cid) {
        if(endpoints[cid].timeout) {
          for(stage_device_id_t epl = 0; epl != MAXEP; ++epl) {
            if(endpoints[epl].timeout) {
              // endpoint is alive, send info of epl to cid:
              size_t n = packmsg(buffer, BUFSIZE, secret, epl, PORT_LISTCID,
                                 endpoints[epl].mode,
                                 (const char*)(&(endpoints[epl].ep)),
                                 sizeof(endpoints[epl].ep));
              socket.send(buffer, n, endpoints[cid].ep);
              n = packmsg(buffer, BUFSIZE, secret, epl, PORT_SETLOCALIP, 0,
                          (const char*)(&(endpoints[epl].localep)),
                          sizeof(endpoints[epl].localep));
              socket.send(buffer, n, endpoints[cid].ep);
            }
          }
        }
      }
    }
    --participantannouncementcnt;
  }
}

void udpreceiver_t::srv()
{
  set_thread_prio(prio);
  char buffer[BUFSIZE];
  log(portno, "Multiplex service started (version " OVBOXVERSION ")");
  endpoint_t sender_endpoint;
  stage_device_id_t rcallerid;
  port_t destport;
  while(runsession) {
    size_t n(BUFSIZE);
    size_t un(BUFSIZE);
    sequence_t seq(0);
    char* msg(socket.recv_sec_msg(buffer, n, un, rcallerid, destport, seq,
                                  sender_endpoint));
    if(msg) {
      // retransmit data:
      if(destport > MAXSPECIALPORT) {
        for(stage_device_id_t ep = 0; ep != MAXEP; ++ep) {
          if((ep != rcallerid) && (endpoints[ep].timeout > 0) &&
             (!(endpoints[ep].mode & B_DONOTSEND)) &&
             ((!(endpoints[ep].mode & B_PEER2PEER)) ||
              (!(endpoints[rcallerid].mode & B_PEER2PEER))) &&
             ((!(endpoints[ep].mode & B_DOWNMIXONLY)) ||
              (rcallerid == MAXEP - 1))) {
            socket.send(buffer, n, endpoints[ep].ep);
          }
        }
      } else {
        // this is a control message:
        switch(destport) {
        case PORT_SEQREP:
          if(un == sizeof(sequence_t) + sizeof(stage_device_id_t)) {
            stage_device_id_t sender_cid(*(sequence_t*)msg);
            sequence_t seq(*(sequence_t*)(&(msg[sizeof(stage_device_id_t)])));
            char ctmp[1024];
            sprintf(ctmp, "sequence error %d sender %d %d", rcallerid,
                    sender_cid, seq);
            log(portno, ctmp);
          }
          break;
        case PORT_PEERLATREP:
          if(un == 6 * sizeof(double)) {
            double* data((double*)msg);
            {
              std::lock_guard<std::mutex> lk(latfifomtx);
              latfifo.push(
                  latreport_t(rcallerid, data[0], data[2], data[3] - data[2]));
            }
            char ctmp[1024];
            sprintf(ctmp,
                    "peerlat %d-%g min=%1.2fms, mean=%1.2fms, max=%1.2fms",
                    rcallerid, data[0], data[1], data[2], data[3]);
            log(portno, ctmp);
            sprintf(ctmp, "packages %d-%g received=%g lost=%g (%1.2f%%)",
                    rcallerid, data[0], data[4], data[5],
                    100.0 * data[5] / (std::max(1.0, data[4] + data[5])));
            log(portno, ctmp);
          }
          break;
        case PORT_PONG: {
          double tms(get_pingtime(msg, un));
          if(tms > 0)
            cid_setpingtime(rcallerid, tms);
        } break;
        case PORT_SETLOCALIP:
          if(un == sizeof(endpoint_t)) {
            endpoint_t* localep((endpoint_t*)msg);
            cid_setlocalip(rcallerid, *localep);
          }
          break;
        case PORT_REGISTER:
          // in the register packet the sequence is used to transmit
          // peer2peer flag:
          std::string rver("---");
          if(un > 0) {
            msg[un - 1] = 0;
            rver = msg;
          }
          cid_register(rcallerid, sender_endpoint, seq, rver);
          break;
        }
      }
    }
  }
  log(portno, "Multiplex service stopped");
}

static void sighandler(int sig)
{
  quit_app = true;
}

int main(int argc, char** argv)
{
  std::chrono::high_resolution_clock::time_point start(
      std::chrono::high_resolution_clock::now());
  signal(SIGABRT, &sighandler);
  signal(SIGTERM, &sighandler);
  signal(SIGINT, &sighandler);
  try {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(!curl)
      throw ErrMsg("Unable to initialize curl.");
    int portno(0);
    int prio(55);
    std::string roomname;
    std::string lobby("http://box.orlandoviols.com");
    const char* options = "p:qr:hvn:l:";
    struct option long_options[] = {
        {"rtprio", 1, 0, 'r'},   {"quiet", 0, 0, 'q'}, {"port", 1, 0, 'p'},
        {"verbose", 0, 0, 'v'},  {"help", 0, 0, 'h'},  {"name", 1, 0, 'n'},
        {"lobbyurl", 1, 0, 'l'}, {0, 0, 0, 0}};
    int opt(0);
    int option_index(0);
    while((opt = getopt_long(argc, argv, options, long_options,
                             &option_index)) != -1) {
      switch(opt) {
      case 'h':
        app_usage("mplx_server", long_options, "");
        return 0;
      case 'q':
        verbose = 0;
        break;
      case 'p':
        portno = atoi(optarg);
        break;
      case 'v':
        verbose++;
        break;
      case 'r':
        prio = atoi(optarg);
        break;
      case 'n':
        roomname = optarg;
        break;
      case 'l':
        lobby = optarg;
        break;
      }
    }
    std::chrono::high_resolution_clock::time_point end(
        std::chrono::high_resolution_clock::now());
    unsigned int seed(
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
            .count());
    seed += portno;
    // initialize random generator:
    srandom(seed);
    udpreceiver_t rec(portno, prio);
    if(!roomname.empty())
      rec.set_roomname(roomname);
    if(!lobby.empty())
      rec.set_lobbyurl(lobby);
    rec.srv();
    curl_easy_cleanup(curl);
    curl_global_cleanup();
  }
  catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
