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

size_t packmsg(char* destbuf, size_t maxlen, secret_t secret,
               callerid_t callerid, port_t destport, sequence_t seq,
               const char* msg, size_t msglen)
{
  if(maxlen < HEADERLEN + msglen)
    return 0;
  msg_secret(destbuf) = secret;
  msg_callerid(destbuf) = callerid;
  msg_port(destbuf) = destport;
  msg_seq(destbuf) = seq;
  memcpy(&(destbuf[HEADERLEN]), msg, msglen);
  return HEADERLEN + msglen;
}

size_t addmsg(char* destbuf, size_t maxlen, size_t currentlen, const char* msg, size_t msglen)
{
  if( maxlen < currentlen + msglen )
    return 0;
  memcpy(&(destbuf[currentlen]), msg, msglen);
  return currentlen + msglen;
}

double get_pingtime(const char* msg, size_t msglen)
{
  if(msglen == sizeof(std::chrono::high_resolution_clock::time_point)) {
    const std::chrono::high_resolution_clock::time_point t1(
							    *(std::chrono::high_resolution_clock::time_point*)msg);
    std::chrono::high_resolution_clock::time_point t2(
						      std::chrono::high_resolution_clock::now());
    std::chrono::duration<double> time_span =
      std::chrono::duration_cast<std::chrono::duration<double>>(t2 -
								t1);
    return (1000.0 * time_span.count());
  }
  return -1;
}
