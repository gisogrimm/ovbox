#include "common.h"
#include <ctime>
#include <iomanip>
#include <string.h>

std::mutex logmutex;
int verbose(1);

void log(int portno, const std::string& s, int v)
{
  if(v > verbose)
    return;
  std::lock_guard<std::mutex> guard(logmutex);
  std::time_t t(std::time(nullptr));
  std::cerr << std::put_time(std::gmtime(&t), "%c") << " [" << portno << "] "
            << s << std::endl;
}

void set_thread_prio(unsigned int prio)
{
  if(prio > 0) {
    struct sched_param sp;
    memset(&sp, 0, sizeof(sp));
    sp.sched_priority = prio;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp);
  }
}

void app_usage(const std::string& app_name, struct option* opt,
               const std::string& app_arg, const std::string& help)
{
  std::cout << "Usage:\n\n" << app_name << " [options] " << app_arg << "\n\n";
  if(!help.empty())
    std::cout << help << "\n\n";
  std::cout << "Options:\n\n";
  while(opt->name) {
    std::cout << "  -" << (char)(opt->val) << " " << (opt->has_arg ? "#" : "")
              << "\n  --" << opt->name << (opt->has_arg ? "=#" : "") << "\n\n";
    opt++;
  }
}

ErrMsg::ErrMsg(const std::string& msg) : std::string(msg) {}

ErrMsg::ErrMsg(const std::string& msg, int err)
    : std::string(msg + std::string(strerror(err)))
{
}

ErrMsg::~ErrMsg() throw() {}

const char* ErrMsg::what() const throw()
{
  return c_str();
}

size_t packmsg(char* destbuf, size_t maxlen, secret_t secret, callerid_t callerid,
               uint32_t destport, const char* msg, size_t msglen)
{
  if(maxlen < sizeof(secret) + sizeof(callerid) + sizeof(destport) + msglen)
    return 0;
  memcpy(destbuf, &secret, sizeof(secret));
  memcpy(&(destbuf[sizeof(secret)]), &callerid, sizeof(callerid));
  memcpy(&(destbuf[sizeof(secret) + sizeof(callerid)]), &destport,
         sizeof(destport));
  memcpy(&(destbuf[sizeof(secret) + sizeof(callerid) + sizeof(destport)]), msg,
         msglen);
  return sizeof(secret) + sizeof(callerid) + sizeof(destport) + msglen;
}

size_t unpackmsg(const char* srcbuf, size_t msglen, secret_t& secret,
                 callerid_t& callerid, uint32_t& destport, char* msg,
                 size_t maxlen)
{
  secret = 0;
  callerid = 0;
  destport = 0;
  if(msglen < sizeof(secret) + sizeof(callerid) + sizeof(destport))
    return 0;
  size_t olen(msglen - (sizeof(secret) + sizeof(callerid) + sizeof(destport)));
  if(maxlen < olen)
    return 0;
  memcpy(&secret, srcbuf, sizeof(secret));
  memcpy(&callerid, &(srcbuf[sizeof(secret)]), sizeof(callerid));
  memcpy(&destport, &(srcbuf[sizeof(secret) + sizeof(callerid)]),
         sizeof(destport));
  memcpy(msg, &(srcbuf[sizeof(secret) + sizeof(callerid) + sizeof(destport)]),
         olen);
  return olen;
}
