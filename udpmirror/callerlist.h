#ifndef CALLERLIST_H
#define CALLERLIST_H

#include "common.h"
#include "udpsocket.h"
#include <thread>
#include <vector>

// ping period time in milliseconds:
#define PINGPERIODMS 100
// period time of ping statistic loggin, in ping periods:
#define STATLOGPERIOD 600
// timeout of caller actvity, in ping periods:
#define TIMEOUT 30

class ep_desc_t {
public:
  ep_desc_t();
  endpoint_t ep;
  uint32_t timeout;
  bool announced;
  bool peer2peer;
  double pingt_min;
  double pingt_max;
  double pingt_sum;
  uint32_t pingt_n;
  uint32_t num_received;
  uint32_t num_lost;
  sequence_t seq;
};

class endpoint_list_t {
public:
  endpoint_list_t();
  ~endpoint_list_t();
  void add_endpoint(const endpoint_t& ep);

protected:
  virtual void announce_new_connection(callerid_t cid, const endpoint_t& ep,
                                       bool peer2peer){};
  virtual void announce_connection_lost(callerid_t cid){};
  virtual void announce_latency(callerid_t cid, double lmin, double lmean,
                                double lmax, uint32_t received,
                                uint32_t lost){};
  void cid_setpingtime(callerid_t cid, double pingtime);
  void cid_register(callerid_t cid, const endpoint_t& ep, bool peer2peer);
  std::vector<ep_desc_t> endpoints;

private:
  void checkstatus();
  bool runthread;
  std::thread statusthread;
  std::mutex mstat;
};

#endif

/*
 * Local Variables:
 * mode: c++
 * End:
 */
