#ifndef COMMON_H
#define COMMON_H

#include <getopt.h>
#include <iostream>
#include <mutex>

// maximum buffer size (mtu)
#define BUFSIZE 4096

// maximum number of clients
#define MAXEP 32

#define DEBUG(x)                                                               \
  std::cerr << __FILE__ << ":" << __LINE__ << ": " << #x << "=" << x           \
            << std::endl

// the first few ports are reserved for control packages:
enum {
  PORT_REGISTER,
  PORT_LISTCID,
  PORT_PINGREQ,
  PORT_PINGRESP,
  PORT_PEERLATREP,
  PORT_SEQREP,
  MAXSPECIALPORT
};

extern int verbose;

void log(int portno, const std::string& s, int v = 1);

void set_thread_prio(unsigned int prio);

void app_usage(const std::string& app_name, struct option* opt,
               const std::string& app_arg = "", const std::string& help = "");

class ErrMsg : public std::exception, private std::string {
public:
  ErrMsg(const std::string& msg);
  ErrMsg(const std::string& msg, int err);
  virtual ~ErrMsg() throw();
  const char* what() const throw();
};

typedef uint32_t secret_t;
typedef uint8_t callerid_t;
typedef uint16_t port_t;
typedef uint8_t sequence_t;
#define POS_CALLERID sizeof(secret_t)
#define POS_PORT (POS_CALLERID + sizeof(callerid_t))
#define POS_SEQ (POS_PORT + sizeof(port_t))
#define HEADERLEN (POS_SEQ + sizeof(sequence_t))

inline secret_t& msg_secret(char* m)
{
  return *((secret_t*)m);
};
inline callerid_t& msg_callerid(char* m)
{
  return *((callerid_t*)(&m[POS_CALLERID]));
};
inline port_t& msg_port(char* m)
{
  return *((port_t*)(&m[POS_PORT]));
};
inline sequence_t& msg_seq(char* m)
{
  return *((sequence_t*)(&m[POS_SEQ]));
};

size_t packmsg(char* destbuf, size_t maxlen, secret_t secret,
               callerid_t callerid, port_t destport, sequence_t seq,
               const char* msg, size_t msglen);

size_t addmsg(char* destbuf, size_t maxlen, size_t currentlen, const char* msg, size_t msglen);

double get_pingtime(const char* msg, size_t msglen);

#endif

/*
 * Local Variables:
 * mode: c++
 * End:
 */
