#include "callerlist.h"
#include "common.h"
#include "udpsocket.h"
#include <condition_variable>
#include <map>
#include <thread>
#include <lo/lo.h>
#include <lo/lo_cpp.h>

#define BUFSIZE 4096

class udpreceiver_t : public endpoint_list_t {
public:
  udpreceiver_t(const std::string& desthost, uint32_t destport,
                uint32_t recport, int32_t portoffset, int prio, secret_t secret,
                callerid_t callerid, bool peer2peer);
  ~udpreceiver_t();
  void run();
  void announce_new_connection(callerid_t cid, const endpoint_t& ep);
  void announce_connection_lost(callerid_t cid);
  void announce_latency(callerid_t cid, double lmin, double lmean, double lmax);
  void set_p2p( bool p ){peer2peer = p;};

private:
  void sendsrv();
  void recsrv();
  void pingservice();
  void handle_endpoint_list_update(callerid_t cid, const endpoint_t& ep);
  const int prio;
  secret_t secret;
  udpsocket_t remote_server;
  udpsocket_t local_server;
  uint32_t toport;
  uint32_t recport;
  int32_t portoffset;
  callerid_t callerid;
  bool runsession;
  std::thread sendthread;
  std::thread recthread;
  std::thread pingthread;
  bool peer2peer;
  lo::ServerThread lost;
  lo::Address ctlif;
};

udpreceiver_t::udpreceiver_t(const std::string& desthost, uint32_t destport,
                             uint32_t recport, int32_t portoffset, int prio,
                             secret_t secret, callerid_t callerid,
                             bool peer2peer_)
    : prio(prio), secret(secret), toport(destport), recport(recport),
      portoffset(portoffset), callerid(callerid), runsession(true),
      peer2peer(peer2peer_), lost(9876), ctlif("127.0.0.1", "9000")
{
  if(!lost.is_valid())
    throw ErrMsg("Unable to create OSC server at port 9876.");
  lost.add_method("/peer2peer", "i",
                  [this](lo_arg** argv, int) { this->set_p2p(argv[0]->i); });
  lost.start();
  remote_server.destination(desthost.c_str());
  local_server.destination("localhost");
  sendthread = std::thread(&udpreceiver_t::sendsrv, this);
  recthread = std::thread(&udpreceiver_t::recsrv, this);
  pingthread = std::thread(&udpreceiver_t::pingservice, this);
}

udpreceiver_t::~udpreceiver_t()
{
  runsession = false;
}

void udpreceiver_t::announce_new_connection(callerid_t cid,
                                            const endpoint_t& ep)
{
  log(recport,
      "new connection for " + std::to_string(cid) + " from " + ep2str(ep));
}

void udpreceiver_t::announce_connection_lost(callerid_t cid)
{
  log(recport, "connection for " + std::to_string(cid) + " lost.");
}

void udpreceiver_t::announce_latency(callerid_t cid, double lmin, double lmean,
                                     double lmax)
{
  char ctmp[1024];
  sprintf(ctmp, "latency %d min=%1.2fms, mean=%1.2fms, max=%1.2fms", cid, lmin,
          lmean, lmax);
  log(recport, ctmp);
  double data[4];
  data[0] = cid;
  data[1] = lmin;
  data[2] = lmean;
  data[3] = lmax;
  char buffer[BUFSIZE];
  size_t n = packmsg(buffer, BUFSIZE, secret, callerid, PORT_PEERLATREP,
                     (const char*)data, 4 * sizeof(double));
  remote_server.send(buffer, n, toport);
}

void udpreceiver_t::handle_endpoint_list_update(callerid_t cid,
                                                const endpoint_t& ep)
{
  std::cout << "received " << ep2str(ep) << " for cid " << (int)cid
            << std::endl;
}

// ping service
void udpreceiver_t::pingservice()
{
  char buffer[BUFSIZE];
  uint32_t ctlifcnt(10);
  while(runsession) {
    std::this_thread::sleep_for(std::chrono::milliseconds(PINGPERIODMS));
    for(auto ep : endpoints) {
      if(ep.timeout) {
        std::chrono::high_resolution_clock::time_point t1(
            std::chrono::high_resolution_clock::now());
        size_t n = packmsg(buffer, BUFSIZE, secret, callerid, PORT_PEERPING,
                           (const char*)(&t1), sizeof(t1));
        remote_server.send(buffer, n, ep.ep);
      }
    }
    if( !ctlifcnt ){
      ctlifcnt = 10;
      ctlif.send("/peer2peer","i",peer2peer);
    }
    --ctlifcnt;
  }
}

// this thread receives messages from the server:
void udpreceiver_t::sendsrv()
{
  try {
    set_thread_prio(prio);
    char buffer[BUFSIZE];
    char msg[BUFSIZE];
    endpoint_t sender_endpoint;
    callerid_t rcallerid;
    secret_t ssecret;
    uint32_t destport;
    while(runsession) {
      ssize_t n = remote_server.recvfrom(buffer, BUFSIZE, sender_endpoint);
      size_t un =
          unpackmsg(buffer, n, ssecret, rcallerid, destport, msg, BUFSIZE);
      if(ssecret == secret) {
        // the first port numbers are reserved for the control infos:
        if(destport > MAXSPECIALPORT) {
          local_server.send(msg, un, destport + portoffset);
        } else {
          switch(destport) {
          case PORT_SRVPING:
            remote_server.send(buffer, n, toport);
            break;
          case PORT_LISTCID:
            if((un == sizeof(endpoint_t)) && (rcallerid != callerid))
              cid_isalive(rcallerid, *((endpoint_t*)msg));
            break;
          case PORT_PEERPING:
            if(un == sizeof(std::chrono::high_resolution_clock::time_point)) {
              std::chrono::high_resolution_clock::time_point t1(
                  *(std::chrono::high_resolution_clock::time_point*)msg);
              std::chrono::high_resolution_clock::time_point t2(
                  std::chrono::high_resolution_clock::now());
              std::chrono::duration<double> time_span =
                  std::chrono::duration_cast<std::chrono::duration<double>>(t2 -
                                                                            t1);
              double tms(1000.0 * time_span.count());
              cid_isalive(rcallerid, sender_endpoint, tms);
            }
            break;
          }
        }
      }
    }
  }
  catch(const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    runsession = false;
  }
}

// this thread receives local UDP messages and handles them:
void udpreceiver_t::recsrv()
{
  try {
    local_server.bind(recport);
    set_thread_prio(prio);
    char buffer[BUFSIZE];
    char msg[BUFSIZE];
    endpoint_t sender_endpoint;
    log(recport, "listening");
    while(runsession) {
      ssize_t n = local_server.recvfrom(buffer, BUFSIZE, sender_endpoint);
      size_t un = packmsg(msg, BUFSIZE, secret, callerid, recport, buffer, n);
      if(peer2peer) {
        for(auto ep : endpoints)
          if(ep.timeout)
            remote_server.send(msg, un, ep.ep);
      } else {
        remote_server.send(msg, un, toport);
      }
    }
  }
  catch(const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    runsession = false;
  }
}

void udpreceiver_t::run()
{
  char buffer[BUFSIZE];
  size_t msglen = packmsg(buffer, BUFSIZE, secret, callerid, 0, "", 0);
  while(runsession) {
    remote_server.send(buffer, msglen, toport);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

int main(int argc, char** argv)
{
  try {
    int destport(4464);
    int recport(9000);
    int portoffset(0);
    int prio(55);
    secret_t secret(1234);
    callerid_t callerid(0);
    bool peer2peer(false);
    std::string desthost("localhost");
    const char* options = "c:d:p:o:qr:hvl:2";
    struct option long_options[] = {
        {"callerid", 1, 0, 'c'},   {"dest", 1, 0, 'd'},
        {"rtprio", 1, 0, 'r'},     {"secret", 1, 0, 's'},
        {"quiet", 0, 0, 'q'},      {"port", 1, 0, 'p'},
        {"peer2peer", 1, 0, '2'},  {"portoffset", 1, 0, 'o'},
        {"listenport", 1, 0, 'l'}, {"verbose", 0, 0, 'v'},
        {"help", 0, 0, 'h'},       {0, 0, 0, 0}};
    int opt(0);
    int option_index(0);
    while((opt = getopt_long(argc, argv, options, long_options,
                             &option_index)) != -1) {
      switch(opt) {
      case 'h':
        app_usage("mplx_client", long_options, "");
        return 0;
      case 'q':
        verbose = 0;
        break;
      case 'd':
        desthost = optarg;
        break;
      case 'p':
        destport = atoi(optarg);
        break;
      case 'l':
        recport = atoi(optarg);
        break;
      case 'o':
        portoffset = atoi(optarg);
        break;
      case '2':
        peer2peer = true;
        break;
      case 'v':
        verbose++;
        break;
      case 'r':
        prio = atoi(optarg);
        break;
      case 'c':
        callerid = atoi(optarg);
        break;
      case 's':
        if(sizeof(long) == sizeof(secret))
          secret = atol(optarg);
        else if(sizeof(long) == sizeof(secret))
          secret = atoll(optarg);
        else
          secret = atoi(optarg);
        break;
      }
    }
    udpreceiver_t rec(desthost, destport, recport, portoffset, prio, secret,
                      callerid, peer2peer);
    rec.run();
  }
  catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
