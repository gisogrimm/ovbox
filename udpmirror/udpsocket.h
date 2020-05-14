#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include "common.h"
#include <netinet/ip.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct sockaddr_in endpoint_t;

class udpsocket_t {
public:
  udpsocket_t();
  ~udpsocket_t();
  void bind(port_t port);
  void destination(const char* host);
  size_t send(const char* buf, size_t len, int portno);
  size_t send(const char* buf, size_t len, const endpoint_t& ep);
  size_t recvfrom(char* buf, size_t len, endpoint_t& addr);
  void close();

private:
  int sockfd;
  endpoint_t serv_addr;
  bool isopen;
};

class ovbox_udpsocket_t : public udpsocket_t {
public:
  ovbox_udpsocket_t(secret_t secret);
  void send_ping(callerid_t cid, const endpoint_t& ep);
  void send_registration(callerid_t cid, bool peer2peer, port_t port);
  char* recv_sec_msg(char* inputbuf, size_t& ilen, size_t& len, callerid_t& cid,
                     port_t& destport, sequence_t& seq, endpoint_t& addr);

  sequence_t pingseq[MAXEP];

private:
  secret_t secret;
};

std::string addr2str(const struct in_addr& addr);
std::string ep2str(const endpoint_t& ep);

#endif

/*
 * Local Variables:
 * mode: c++
 * compile-command: "make"
 * End:
 */
