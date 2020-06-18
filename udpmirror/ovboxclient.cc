#include "callerlist.h"
#include "common.h"
#include "udpsocket.h"
#include <condition_variable>
#include <map>
#include <thread>

class udpreceiver_t : public endpoint_list_t {
public:
  udpreceiver_t(const std::string& desthost, port_t destport, port_t recport,
                int32_t portoffset, int prio, secret_t secret,
                callerid_t callerid, bool peer2peer, bool donotsend,
                bool downmixonly, port_t loport);
  ~udpreceiver_t();
  void run();
  void announce_new_connection(callerid_t cid, const ep_desc_t& ep);
  void announce_connection_lost(callerid_t cid);
  void announce_latency(callerid_t cid, double lmin, double lmean, double lmax,
                        uint32_t received, uint32_t lost);

private:
  void sendsrv();
  void recsrv();
  void pingservice();
  void handle_endpoint_list_update(callerid_t cid, const endpoint_t& ep);
  const int prio;
  secret_t secret;
  ovbox_udpsocket_t remote_server;
  udpsocket_t local_server;
  port_t toport;
  port_t recport;
  int32_t portoffset;
  callerid_t callerid;
  bool runsession;
  std::thread sendthread;
  std::thread recthread;
  std::thread pingthread;
  epmode_t mode;
};

udpreceiver_t::udpreceiver_t(const std::string& desthost, port_t destport,
                             port_t recport, int32_t portoffset, int prio,
                             secret_t secret, callerid_t callerid,
                             bool peer2peer_, bool donotsend_,
                             bool downmixonly_, port_t loport)
    : prio(prio), secret(secret), remote_server(secret), toport(destport),
      recport(recport), portoffset(portoffset), callerid(callerid),
      runsession(true), mode(0)
{
  if(peer2peer_)
    mode |= B_PEER2PEER;
  if(downmixonly_)
    mode |= B_DOWNMIXONLY;
  if(donotsend_)
    mode |= B_DONOTSEND;
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

void udpreceiver_t::announce_new_connection(callerid_t cid, const ep_desc_t& ep)
{
  log(recport,
      "new connection for " + std::to_string(cid) + " from " + ep2str(ep.ep) +
          " in " + ((ep.mode & B_PEER2PEER) ? "peer-to-peer" : "server") +
          "-mode" + ((ep.mode & B_DOWNMIXONLY) ? " downmixonly" : "") +
          ((ep.mode & B_DONOTSEND) ? " donotsend" : "") + " v" + ep.version);
}

void udpreceiver_t::announce_connection_lost(callerid_t cid)
{
  log(recport, "connection for " + std::to_string(cid) + " lost.");
}

void udpreceiver_t::announce_latency(callerid_t cid, double lmin, double lmean,
                                     double lmax, uint32_t received,
                                     uint32_t lost)
{
  char ctmp[1024];
  if(lmean > 0) {
    sprintf(ctmp, "latency %d min=%1.2fms, mean=%1.2fms, max=%1.2fms", cid,
            lmin, lmean, lmax);
    log(recport, ctmp);
  }
  sprintf(ctmp, "packages from %d received=%d lost=%d (%1.2f%%)", cid, received,
          lost, 100.0 * (double)lost / (double)(std::max(1u, received + lost)));
  log(recport, ctmp);
  double data[6];
  data[0] = cid;
  data[1] = lmin;
  data[2] = lmean;
  data[3] = lmax;
  data[4] = received;
  data[5] = lost;
  char buffer[BUFSIZE];
  size_t n = packmsg(buffer, BUFSIZE, secret, callerid, PORT_PEERLATREP, 0,
                     (const char*)data, 6 * sizeof(double));
  remote_server.send(buffer, n, toport);
}

void udpreceiver_t::handle_endpoint_list_update(callerid_t cid,
                                                const endpoint_t& ep)
{
}

// ping service
void udpreceiver_t::pingservice()
{
  while(runsession) {
    std::this_thread::sleep_for(std::chrono::milliseconds(PINGPERIODMS));
    // send registration to server:
    remote_server.send_registration(callerid, mode, toport);
    // send ping to other peers:
    for(auto ep : endpoints) {
      if(ep.timeout) {
        remote_server.send_ping(callerid, ep.ep);
      }
    }
  }
}

// this thread receives messages from the server:
void udpreceiver_t::sendsrv()
{
  try {
    set_thread_prio(prio);
    char buffer[BUFSIZE];
    endpoint_t sender_endpoint;
    callerid_t rcallerid;
    port_t destport;
    while(runsession) {
      size_t n(BUFSIZE);
      size_t un(BUFSIZE);
      sequence_t seq(0);
      char* msg(remote_server.recv_sec_msg(buffer, n, un, rcallerid, destport,
                                           seq, sender_endpoint));
      if(msg) {
        // the first port numbers are reserved for the control infos:
        if(destport > MAXSPECIALPORT) {
          if(rcallerid != callerid) {
            sequence_t dseq(seq - endpoints[rcallerid].seq);
            if(dseq != 0) {
              local_server.send(msg, un, destport + portoffset);
              ++endpoints[rcallerid].num_received;
              if(dseq < 0) {
                size_t un =
                    packmsg(buffer, BUFSIZE, secret, callerid, PORT_SEQREP, 0,
                            (char*)(&rcallerid), sizeof(rcallerid));
                un = addmsg(buffer, BUFSIZE, un, (char*)(&dseq), sizeof(dseq));
                remote_server.send(buffer, un, toport);
              } else {
                endpoints[rcallerid].num_lost += (dseq - 1);
              }
              endpoints[rcallerid].seq = seq;
            }
          }
        } else {
          switch(destport) {
          case PORT_PING:
            msg_port(buffer) = PORT_PONG;
            msg_callerid(buffer) = callerid;
            remote_server.send(buffer, n, sender_endpoint);
            break;
          case PORT_PONG:
            if(rcallerid != callerid) {
              double tms(get_pingtime(msg, un));
              if(tms > 0)
                cid_setpingtime(rcallerid, tms);
            }
            break;
          case PORT_LISTCID:
            if((un == sizeof(endpoint_t)) && (rcallerid != callerid)) {
              // seq is peer2peer flag:
              cid_register(rcallerid, *((endpoint_t*)msg), seq, "");
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
    sequence_t seq(0);
    while(runsession) {
      ssize_t n = local_server.recvfrom(buffer, BUFSIZE, sender_endpoint);
      ++seq;
      size_t un =
          packmsg(msg, BUFSIZE, secret, callerid, recport, seq, buffer, n);
      bool sendtoserver(!(mode & B_PEER2PEER));
      if(mode & B_PEER2PEER) {
        for(auto ep : endpoints)
          if(ep.timeout) {
            if((ep.mode & B_PEER2PEER) && (!(ep.mode & B_DONOTSEND))) {
              remote_server.send(msg, un, ep.ep);
            } else {
              sendtoserver = true;
            }
          }
      }
      if(sendtoserver) {
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
  while(runsession) {
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
    int loport(9876);
    secret_t secret(1234);
    callerid_t callerid(0);
    bool peer2peer(false);
    bool donotsend(false);
    bool downmixonly(false);
    std::string desthost("localhost");
    const char* options = "c:d:p:o:qr:hvl:2s:mn";
    struct option long_options[] = {{"callerid", 1, 0, 'c'},
                                    {"dest", 1, 0, 'd'},
                                    {"rtprio", 1, 0, 'r'},
                                    {"secret", 1, 0, 's'},
                                    {"quiet", 0, 0, 'q'},
                                    {"port", 1, 0, 'p'},
                                    {"peer2peer", 0, 0, '2'},
                                    {"portoffset", 1, 0, 'o'},
                                    {"listenport", 1, 0, 'l'},
                                    {"verbose", 0, 0, 'v'},
                                    {"help", 0, 0, 'h'},
                                    {"ctlport", 1, 0, 'x'},
                                    {"downmixonly", 0, 0, 'm'},
                                    {"donotsend", 0, 0, 'n'},
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
      case 'n':
        donotsend = true;
        break;
      case 'm':
        downmixonly = true;
        break;
      case 'p':
        destport = atoi(optarg);
        break;
      case 'l':
        recport = atoi(optarg);
        break;
      case 'x':
        loport = atoi(optarg);
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
        secret = atoll(optarg);
        break;
      }
    }
    udpreceiver_t rec(desthost, destport, recport, portoffset, prio, secret,
                      callerid, peer2peer, donotsend, downmixonly, loport);
    rec.run();
  }
  catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
