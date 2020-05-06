#include "callerlist.h"
#include "common.h"
#include "udpsocket.h"
#include <condition_variable>
#include <signal.h>
#include <string.h>
#include <thread>
#include <vector>

// period time of participant list announcement, in ping periods:
#define PARTICIPANTANNOUNCEPERIOD 20

static bool quit_app(false);

class udpreceiver_t : public endpoint_list_t {
public:
  udpreceiver_t(int portno, int prio, secret_t secret);
  ~udpreceiver_t();
  const int portno;
  void srv();
  void announce_new_connection(callerid_t cid, const endpoint_t& ep);
  void announce_connection_lost(callerid_t cid);
  void announce_latency(callerid_t cid, double lmin, double lmean, double lmax);

private:
  void ping_and_callerlist_service();
  std::thread logthread;
  void quitwatch();
  std::thread quitthread;
  const int prio;

  ovbox_udpsocket_t socket;
  bool runsession;
  struct sockaddr_in serv_addr;
  secret_t secret;
};

udpreceiver_t::udpreceiver_t(int portno, int prio, secret_t secret)
    : portno(portno), prio(prio), socket(secret), runsession(true),
      secret(secret)
{
  endpoints.resize(255);
  socket.bind(portno);
  logthread = std::thread(&udpreceiver_t::ping_and_callerlist_service, this);
  quitthread = std::thread(&udpreceiver_t::quitwatch, this);
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

void udpreceiver_t::announce_new_connection(callerid_t cid,
                                            const endpoint_t& ep)
{
  log(portno,
      "new connection for " + std::to_string(cid) + " from " + ep2str(ep));
}

void udpreceiver_t::announce_connection_lost(callerid_t cid)
{
  log(portno, "connection for " + std::to_string(cid) + " lost.");
}

void udpreceiver_t::announce_latency(callerid_t cid, double lmin, double lmean,
                                     double lmax)
{
  char ctmp[1024];
  sprintf(ctmp, "latency %d min=%1.2fms, mean=%1.2fms, max=%1.2fms", cid, lmin,
          lmean, lmax);
  log(portno, ctmp);
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
    for(callerid_t cid = 0; cid != MAXEP; ++cid) {
      if(endpoints[cid].timeout) {
        // endpoint is connected
        socket.send_ping(cid, endpoints[cid].ep);
      }
    }
    if(!participantannouncementcnt) {
      // announcement of connected participants to all clients:
      participantannouncementcnt = PARTICIPANTANNOUNCEPERIOD;
      for(callerid_t cid = 0; cid != MAXEP; ++cid) {
        if(endpoints[cid].timeout) {
          for(callerid_t epl = 0; epl != MAXEP; ++epl) {
            if(endpoints[epl].timeout) {
              // endpoint is alive, send info of epl to cid:
              size_t n = packmsg(buffer, BUFSIZE, secret, epl, PORT_LISTCID,
                                 endpoints[epl].peer2peer,
                                 (const char*)(&(endpoints[epl].ep)),
                                 sizeof(endpoints[epl].ep));
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
  log(portno, "Multiplex service started");
  endpoint_t sender_endpoint;
  callerid_t rcallerid;
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
        for(callerid_t ep = 0; ep != MAXEP; ++ep) {
          if((ep != rcallerid) && (endpoints[ep].timeout > 0) &&
             ((!endpoints[ep].peer2peer) ||
              (!endpoints[rcallerid].peer2peer))) {
            socket.send(buffer, n, endpoints[ep].ep);
          }
        }
      } else {
        // this is a control message:
        switch(destport) {
        case PORT_SEQREP:
          if(un == sizeof(sequence_t) + sizeof(callerid_t)) {
            callerid_t sender_cid(*(sequence_t*)msg);
            sequence_t seq(*(sequence_t*)(&(msg[sizeof(callerid_t)])));
            char ctmp[1024];
            sprintf(ctmp, "sequence error %d sender %d %d", rcallerid,
                    sender_cid, seq);
            log(portno, ctmp);
          }
          break;
        case PORT_PEERLATREP:
          if(un == 4 * sizeof(double)) {
            double* data((double*)msg);
            char ctmp[1024];
            sprintf(ctmp,
                    "peerlat %d-%g min=%1.2fms, mean=%1.2fms, max=%1.2fms",
                    rcallerid, data[0], data[1], data[2], data[3]);
            log(portno, ctmp);
          }
          break;
        case PORT_PINGRESP: {
          double tms(get_pingtime(msg, un));
          if(tms > 0)
            cid_isalive(rcallerid, sender_endpoint, tms);
        } break;
        case PORT_REGISTER:
          // in the register packet the sequence is used to transmit
          // peer2peer flag:
          cid_isalive(rcallerid, sender_endpoint);
          cid_set_peer2peer(rcallerid, seq);
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
  signal(SIGABRT, &sighandler);
  signal(SIGTERM, &sighandler);
  signal(SIGINT, &sighandler);
  try {
    int portno(4464);
    int prio(55);
    secret_t secret(1234);
    const char* options = "p:qr:hv";
    struct option long_options[] = {
        {"rtprio", 1, 0, 'r'}, {"secret", 1, 0, 's'},  {"quiet", 0, 0, 'q'},
        {"port", 1, 0, 'p'},   {"verbose", 0, 0, 'v'}, {"help", 0, 0, 'h'},
        {0, 0, 0, 0}};
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
    udpreceiver_t rec(portno, prio, secret);
    rec.srv();
  }
  catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
