#include "common.h"
#include "udpsocket.h"
#include <condition_variable>
#include <map>
#include <thread>

#define BUFSIZE 4096

struct ep_t {
  uint32_t timeout;
};

class udpreceiver_t {
public:
  udpreceiver_t(const std::string& desthost, uint32_t destport,
                uint32_t recport, int32_t portoffset, int prio, secret_t secret,
                callerid_t callerid);
  ~udpreceiver_t();
  void run();

private:
  void sendsrv();
  void recsrv();
  const int prio;
  secret_t secret;
  udpsocket_t toserver;
  udpsocket_t sender;
  uint32_t toport;
  uint32_t recport;
  int32_t portoffset;
  callerid_t callerid;
  bool runsession;
  std::map<uint32_t, ep_t> endpoints;
  std::thread sendthread;
  std::thread recthread;
};

udpreceiver_t::udpreceiver_t(const std::string& desthost, uint32_t destport,
                             uint32_t recport, int32_t portoffset, int prio,
                             secret_t secret, callerid_t callerid)
    : prio(prio), secret(secret), toport(destport), recport(recport),
      portoffset(portoffset), callerid(callerid), runsession(true)

{
  toserver.destination(desthost.c_str());
  sender.destination("localhost");
  sendthread = std::thread(&udpreceiver_t::sendsrv, this);
  recthread = std::thread(&udpreceiver_t::recsrv, this);
}

udpreceiver_t::~udpreceiver_t()
{
  runsession = false;
}

void udpreceiver_t::sendsrv()
{
  try{
    set_thread_prio(prio);
    char buffer[BUFSIZE];
    char msg[BUFSIZE];
    endpoint_t sender_endpoint;
    callerid_t rcallerid;
    secret_t ssecret;
    uint32_t destport;
    while(runsession) {
      ssize_t n = toserver.recvfrom(buffer, BUFSIZE, sender_endpoint);
      size_t un =
        unpackmsg(buffer, n, ssecret, rcallerid, destport, msg, BUFSIZE);
      if(ssecret == secret) {
	if(destport != 0) {
	  sender.send(msg, un, destport + portoffset);
	}else{
	  toserver.send( buffer, n, toport );
	}
      }
    }
  }
  catch( const std::exception& e ){
    std::cerr << "Error: " << e.what() << std::endl;
    runsession = false;
  }
}

void udpreceiver_t::recsrv()
{
  try{
    sender.bind(recport);
    set_thread_prio(prio);
    char buffer[BUFSIZE];
    char msg[BUFSIZE];
    endpoint_t sender_endpoint;
    log(recport,"listening");
    while(runsession) {
      ssize_t n = sender.recvfrom(buffer, BUFSIZE, sender_endpoint);
      size_t un = packmsg(msg, BUFSIZE, secret, callerid, recport,
			  buffer, n);
      toserver.send(msg, un, toport);
    }
  }
  catch( const std::exception& e ){
    std::cerr << "Error: " << e.what() << std::endl;
    runsession = false;
  }
}

void udpreceiver_t::run()
{
  char buffer[BUFSIZE];
  size_t msglen = packmsg(buffer, BUFSIZE, secret, callerid, 0, "", 0);
  while(runsession) {
    toserver.send(buffer, msglen, toport);
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
    std::string desthost("localhost");
    const char* options = "c:d:p:o:qr:hvl:";
    struct option long_options[] = {{"callerid", 1, 0, 'c'},
                                    {"dest", 1, 0, 'd'},
                                    {"rtprio", 1, 0, 'r'},
                                    {"secret", 1, 0, 's'},
                                    {"quiet", 0, 0, 'q'},
                                    {"port", 1, 0, 'p'},
                                    {"portoffset", 1, 0, 'o'},
                                    {"listenport", 1, 0, 'l'},
                                    {"verbose", 0, 0, 'v'},
                                    {"help", 0, 0, 'h'},
                                    {0, 0, 0, 0}};
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
                      callerid);
    rec.run();
  }
  catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
