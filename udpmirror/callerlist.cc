#include "callerlist.h"
#include <string.h>

ep_desc_t::ep_desc_t()
{
  memset(&ep, 0, sizeof(ep));
  timeout = 0;
  announced = false;
  pingt_min = 10000;
  pingt_max = 0;
  pingt_sum = 0;
  pingt_n = 0;
  seq = 0;
  num_lost = 0;
  num_received = 0;
  peer2peer = true;
}

endpoint_list_t::endpoint_list_t() : runthread(true)
{
  endpoints.resize(MAXEP);
  statusthread = std::thread(&endpoint_list_t::checkstatus, this);
}

endpoint_list_t::~endpoint_list_t()
{
  runthread = false;
  statusthread.join();
}

void endpoint_list_t::cid_register(callerid_t cid, const endpoint_t& ep,
                                   bool peer2peer)
{
  if(cid < MAXEP) {
    endpoints[cid].ep = ep;
    if(peer2peer != endpoints[cid].peer2peer)
      endpoints[cid].announced = false;
    endpoints[cid].peer2peer = peer2peer;
    endpoints[cid].timeout = TIMEOUT;
  }
}

void endpoint_list_t::cid_setpingtime(callerid_t cid, double pingtime)
{
  if(cid < MAXEP) {
    endpoints[cid].timeout = TIMEOUT;
    if(pingtime > 0) {
      if(mstat.try_lock()) {
        ++endpoints[cid].pingt_n;
        endpoints[cid].pingt_sum += pingtime;
        endpoints[cid].pingt_max = std::max(pingtime, endpoints[cid].pingt_max);
        endpoints[cid].pingt_min = std::min(pingtime, endpoints[cid].pingt_min);
        mstat.unlock();
      }
    }
  }
}

void endpoint_list_t::checkstatus()
{
  uint32_t statlogcnt(STATLOGPERIOD);
  while(runthread) {
    std::this_thread::sleep_for(std::chrono::milliseconds(PINGPERIODMS));
    for(callerid_t ep = 0; ep != MAXEP; ++ep) {
      if(endpoints[ep].timeout) {
        // bookkeeping of connected endpoints:
        if(!endpoints[ep].announced) {
          announce_new_connection(ep, endpoints[ep].ep,
                                  endpoints[ep].peer2peer);
          endpoints[ep].announced = true;
        }
        --endpoints[ep].timeout;
      } else {
        // bookkeeping of disconnected endpoints:
        if(endpoints[ep].announced) {
          announce_connection_lost(ep);
          endpoints[ep] = ep_desc_t();
        }
      }
    }
    if(!statlogcnt) {
      // logging of ping statistics:
      statlogcnt = STATLOGPERIOD;
      std::lock_guard<std::mutex> lk(mstat);
      for(callerid_t ep = 0; ep != MAXEP; ++ep) {
        if(endpoints[ep].pingt_n) {
          announce_latency(ep, endpoints[ep].pingt_min,
                           endpoints[ep].pingt_sum / endpoints[ep].pingt_n,
                           endpoints[ep].pingt_max, endpoints[ep].num_received,
                           endpoints[ep].num_lost);
          endpoints[ep].pingt_n = 0;
          endpoints[ep].pingt_min = 1000;
          endpoints[ep].pingt_max = 0;
          endpoints[ep].pingt_sum = 0.0;
          endpoints[ep].num_received = 0;
          endpoints[ep].num_lost = 0;
        }
      }
    }
    --statlogcnt;
  }
}
