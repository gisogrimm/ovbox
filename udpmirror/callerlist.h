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

#define B_PEER2PEER 1
#define B_DOWNMIXONLY 2
#define B_DONOTSEND 4

class ep_desc_t {
public:
  ep_desc_t();
  endpoint_t ep;
  endpoint_t localep;
  uint32_t timeout;
  bool announced;
  epmode_t mode;
  // bool peer2peer;
  // bool downmixonly;
  double pingt_min;
  double pingt_max;
  double pingt_sum;
  uint32_t pingt_n;
  uint32_t num_received;
  uint32_t num_lost;
  sequence_t seq;
  std::string version;
};

class endpoint_list_t {
public:
  endpoint_list_t();
  ~endpoint_list_t();
  void add_endpoint(const endpoint_t& ep);

protected:
  virtual void announce_new_connection(stage_device_id_t cid,
                                       const ep_desc_t& ep){};
  virtual void announce_connection_lost(stage_device_id_t cid){};
  virtual void announce_latency(stage_device_id_t cid, double lmin,
                                double lmean, double lmax, uint32_t received,
                                uint32_t lost){};
  void cid_setpingtime(stage_device_id_t cid, double pingtime);
  void cid_register(stage_device_id_t cid, const endpoint_t& ep, epmode_t mode,
                    const std::string& rver);
  void cid_setlocalip(stage_device_id_t cid, const endpoint_t& ep);
  uint32_t get_num_clients();
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
