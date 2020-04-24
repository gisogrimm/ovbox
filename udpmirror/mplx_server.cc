#include "common.h"
#include "udpsocket.h"
#include <condition_variable>
#include <vector>
#include <thread>
#include <string.h>


#define BUFSIZE 4096
#define TIMEOUT 20
#define MAXEP 32

class ep_t {
public:
  ep_t();
  endpoint_t ep;
  uint32_t timeout;
  bool announced;
  double pingt_min;
  double pingt_max;
  double pingt_sum;
  uint32_t pingt_n;
};

ep_t::ep_t()
{
  memset(&ep,0,sizeof(ep));
  timeout = 0;
  announced = false;
  pingt_min = 10000;
  pingt_max = 0;
  pingt_sum = 0;
  pingt_n = 0;
}

class udpreceiver_t {
public:
  udpreceiver_t(int portno, int prio, uint64_t secret);
  ~udpreceiver_t();
  const int portno;
  void srv();

private:
  void logsrv();
  std::thread logthread;
  const int prio;

  udpsocket_t socket;
  bool runsession;
  struct sockaddr_in serv_addr;
  uint64_t secret;
  std::vector<ep_t> endpoints;
  std::mutex mstat;
};

udpreceiver_t::udpreceiver_t(int portno, int prio, uint64_t secret)
    : portno(portno), prio(prio), runsession(true), secret(secret)
{
  endpoints.resize(255);
  socket.bind(portno);
  logthread = std::thread(&udpreceiver_t::logsrv, this);

}

udpreceiver_t::~udpreceiver_t()
{
  runsession = false;
}

void udpreceiver_t::logsrv()
{
  uint32_t cnt(6000);
  while(runsession) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    for( callerid_t ep=0; ep!=MAXEP;++ep ){
      if( endpoints[ep].timeout ){
	char buffer[BUFSIZE];
	std::chrono::high_resolution_clock::time_point t1(std::chrono::high_resolution_clock::now());
	size_t n = packmsg(buffer, BUFSIZE, secret, ep, 0, (const char*)(&t1), sizeof(t1) );
	socket.send( buffer, n, endpoints[ep].ep );
	if( !endpoints[ep].announced ){
	  log( portno, "new connection for "+std::to_string(ep)+" from "+ep2str(endpoints[ep].ep));
	  endpoints[ep].announced = true;
	}
	--endpoints[ep].timeout;
      }else{
	if( endpoints[ep].announced ){
	  log( portno, "connection for "+std::to_string(ep)+" lost.");
	  endpoints[ep].announced = false;
	  endpoints[ep].pingt_n = 0;
	  endpoints[ep].pingt_min = 1000;
	  endpoints[ep].pingt_max = 0;
	  endpoints[ep].pingt_sum = 0.0;
	}
      }
    }
    if( !cnt ){
      cnt = 6000;
      std::lock_guard<std::mutex> lk(mstat);
      for( callerid_t ep=0; ep!=MAXEP;++ep ){
	if( endpoints[ep].pingt_n ){
	  char ctmp[1024];
	  sprintf(ctmp,"latency %d min=%1.2fms, mean=%1.2fms, max=%1.2fms",
		  ep,endpoints[ep].pingt_min,
		  endpoints[ep].pingt_sum/endpoints[ep].pingt_n,
		  endpoints[ep].pingt_max);
	  endpoints[ep].pingt_n = 0;
	  endpoints[ep].pingt_min = 1000;
	  endpoints[ep].pingt_max = 0;
	  endpoints[ep].pingt_sum = 0.0;
	  log( portno, ctmp);
	}
      }
    }
    --cnt;
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
      endpoints[callerid].ep = sender_endpoint;
      endpoints[callerid].timeout = TIMEOUT;
      if( destport != 0 ){
	for( callerid_t ep=0; ep!=MAXEP;++ep ){
	  if( (ep != callerid) && (endpoints[ep].timeout > 0) ){
	    socket.send( buffer, n, endpoints[ep].ep );
	  }
	}
      }else{
	if( un == sizeof(std::chrono::high_resolution_clock::time_point) ){
	  std::chrono::high_resolution_clock::time_point t1(*(std::chrono::high_resolution_clock::time_point*)msg);
	  std::chrono::high_resolution_clock::time_point t2(std::chrono::high_resolution_clock::now());
	  std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
	  double tms(1000.0*time_span.count());
	  if( tms > 0 ){
	    if( mstat.try_lock() ){
	      ++endpoints[callerid].pingt_n;
	      endpoints[callerid].pingt_sum += tms;
	      endpoints[callerid].pingt_max = std::max(tms,endpoints[callerid].pingt_max);
	      endpoints[callerid].pingt_min = std::min(tms,endpoints[callerid].pingt_min);
	      mstat.unlock();
	    }
	  }
	}
      }
    }
  }
  log(portno, "Multiplex service stopped");
}

int main(int argc, char** argv)
{
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
