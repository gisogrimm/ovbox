#ifndef COMMON_H
#define COMMON_H

#include <mutex>
#include <iostream>
#include <getopt.h>

#define DEBUG(x) std::cerr << __FILE__ << ":" << __LINE__ << ": " << #x << "=" << x << std::endl

// the first few ports are reserved for control packages:
#define MAXSPECIALPORT 10

extern int verbose;

void log( int portno, const std::string& s, int v = 1 );

void set_thread_prio( unsigned int prio );

void app_usage(const std::string& app_name,struct option * opt,const std::string& app_arg="", const std::string& help="" );

class ErrMsg : public std::exception, private std::string {
public:
  ErrMsg(const std::string& msg);
  ErrMsg(const std::string& msg, int err);
  virtual ~ErrMsg() throw();
  const char* what() const throw();
};

typedef uint8_t callerid_t;
typedef uint32_t secret_t;

size_t packmsg( char* destbuf, size_t maxlen, secret_t secret, callerid_t callerid, uint32_t destport, const char* msg, size_t msglen );
size_t unpackmsg( const char* srcbuf, size_t len, secret_t& secret, callerid_t& callerid, uint32_t& destport, char* msg, size_t maxlen );

#endif

/*
 * Local Variables:
 * mode: c++
 * End:
 */
