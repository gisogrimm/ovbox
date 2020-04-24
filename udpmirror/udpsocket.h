#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string>

typedef struct sockaddr_in endpoint_t;

class udpsocket_t {
public:
  udpsocket_t();
  ~udpsocket_t();
  void bind( uint32_t port );
  void destination( const char* host );
  size_t send( const char* buf, size_t len, int portno );
  size_t send( const char* buf, size_t len, const endpoint_t& ep );
  size_t recvfrom( char* buf, size_t len, endpoint_t& addr );
private:
  int sockfd;
  endpoint_t serv_addr;

};

std::string addr2str( const struct in_addr& addr );
std::string ep2str( const endpoint_t& ep );

#endif

/*
 * Local Variables:
 * mode: c++
 * compile-command: "make"
 * End:
 */
