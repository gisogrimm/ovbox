#include "callerlist.h"
#include "common.h"
#include "udpsocket.h"
#include <condition_variable>
#include <signal.h>
#include <string.h>
#include <thread>
#include <vector>

#define BUFSIZE 4096

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
  void logsrv();
  std::thread logthread;
  void quitwatch();
  std::thread quitthread;
  const int prio;

  udpsocket_t socket;
  bool runsession;
  struct sockaddr_in serv_addr;
  secret_t secret;
};

udpreceiver_t::udpreceiver_t(int portno, int prio, secret_t secret)
    : portno(portno), prio(prio), runsession(true), secret(secret)
{
  endpoints.resize(255);
  socket.bind(portno);
  logthread = std::thread(&udpreceiver_t::logsrv, this);
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

// this thread sends ping and participant messages
void udpreceiver_t::logsrv()
{
  // participand announcement counter:
  uint32_t participantannouncementcnt(PARTICIPANTANNOUNCEPERIOD);
  while(runsession) {
    std::this_thread::sleep_for(std::chrono::milliseconds(PINGPERIODMS));
    for(callerid_t ep = 0; ep != MAXEP; ++ep) {
      if(endpoints[ep].timeout) {
        // send ping message:
        char buffer[BUFSIZE];
        std::chrono::high_resolution_clock::time_point t1(
            std::chrono::high_resolution_clock::now());
        size_t n = packmsg(buffer, BUFSIZE, secret, ep, PORT_SRVPING,
                           (const char*)(&t1), sizeof(t1));
        socket.send(buffer, n, endpoints[ep].ep);
      }
    }
    if(!participantannouncementcnt) {
      // announcement of connected participants to all clients:
      participantannouncementcnt = PARTICIPANTANNOUNCEPERIOD;
      for(callerid_t ep = 0; ep != MAXEP; ++ep) {
        if(endpoints[ep].timeout) {
          for(callerid_t epl = 0; epl != MAXEP; ++epl) {
            if(endpoints[epl].timeout) {
              // endpoint is alive, send info of epl to ep:
              char buffer[BUFSIZE];
              size_t n = packmsg(buffer, BUFSIZE, secret, epl, PORT_LISTCID,
                                 (const char*)(&(endpoints[epl].ep)),
                                 sizeof(endpoints[epl].ep));
              socket.send(buffer, n, endpoints[ep].ep);
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
  char msg[BUFSIZE];
  log(portno, "Multiplex service started");
  endpoint_t sender_endpoint;
  callerid_t callerid;
  secret_t ssecret;
  uint32_t destport;
  while(runsession) {
    ssize_t n = socket.recvfrom(buffer, BUFSIZE, sender_endpoint);
    if(verbose > 2)
      log(portno, "received " + std::to_string(n) + " bytes");
    size_t un = unpackmsg(buffer, n, ssecret, callerid, destport, msg, BUFSIZE);
    if((ssecret == secret) && (callerid < MAXEP)) {
      cid_isalive(callerid, sender_endpoint);
      // retransmit data:
      if(destport > MAXSPECIALPORT) {
        for(callerid_t ep = 0; ep != MAXEP; ++ep) {
          if((ep != callerid) && (endpoints[ep].timeout > 0)) {
            socket.send(buffer, n, endpoints[ep].ep);
          }
        }
      } else {
        // this is a control message:
        switch(destport) {
        case PORT_SRVPING:
          if(un == sizeof(std::chrono::high_resolution_clock::time_point)) {
            std::chrono::high_resolution_clock::time_point t1(
                *(std::chrono::high_resolution_clock::time_point*)msg);
            std::chrono::high_resolution_clock::time_point t2(
                std::chrono::high_resolution_clock::now());
            std::chrono::duration<double> time_span =
                std::chrono::duration_cast<std::chrono::duration<double>>(t2 -
                                                                          t1);
            double tms(1000.0 * time_span.count());
            cid_isalive(callerid, sender_endpoint, tms);
          }
          break;
	case PORT_PEERLATREP:
	  if( un == 4*sizeof(double) ){
	    double* data((double*)msg);
	    char ctmp[1024];
	    sprintf(ctmp, "peerlat %d-%g min=%1.2fms, mean=%1.2fms, max=%1.2fms", callerid, data[0], data[1],
		    data[2], data[3]);
	    log(portno, ctmp);
	  }
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
