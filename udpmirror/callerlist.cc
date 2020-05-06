#include "callerlist.h"
#include <string.h>

ep_desc_t::ep_desc_t()
{
  memset(&ep,0,sizeof(ep));
  timeout = 0;
  announced = false;
  pingt_min = 10000;
  pingt_max = 0;
  pingt_sum = 0;
  pingt_n = 0;
  seq = 0;
  peer2peer = true;
}

endpoint_list_t::endpoint_list_t()
  : runthread(true)
{
  endpoints.resize(MAXEP);
  statusthread = std::thread(&endpoint_list_t::checkstatus, this);
}


endpoint_list_t::~endpoint_list_t()
{
  runthread = false;
  statusthread.join();
}

void endpoint_list_t::cid_set_peer2peer( callerid_t cid, bool peer2peer )
{
  if( cid < MAXEP ){
    endpoints[cid].peer2peer = peer2peer;
  }
}

void endpoint_list_t::cid_isalive( callerid_t cid, const endpoint_t& ep, double pingtime )
{
  if( cid < MAXEP ){
    endpoints[cid].ep = ep;
    endpoints[cid].timeout = TIMEOUT;
    if( pingtime > 0 ){
      if( mstat.try_lock() ){
	++endpoints[cid].pingt_n;
	endpoints[cid].pingt_sum += pingtime;
	endpoints[cid].pingt_max = std::max(pingtime,endpoints[cid].pingt_max);
	endpoints[cid].pingt_min = std::min(pingtime,endpoints[cid].pingt_min);
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
    for( callerid_t ep=0; ep!=MAXEP;++ep ){
      if( endpoints[ep].timeout ){
	// bookkeeping of connected endpoints:
	if( !endpoints[ep].announced ){
	  announce_new_connection( ep, endpoints[ep].ep );
	  endpoints[ep].announced = true;
	}
	--endpoints[ep].timeout;
      }else{
	// bookkeeping of disconnected endpoints:
	if( endpoints[ep].announced ){
	  announce_connection_lost( ep );
	  endpoints[ep].announced = false;
	  endpoints[ep].pingt_n = 0;
	  endpoints[ep].pingt_min = 1000;
	  endpoints[ep].pingt_max = 0;
	  endpoints[ep].pingt_sum = 0.0;
	}
      }
    }
    if( !statlogcnt ){
      // logging of ping statistics:
      statlogcnt = STATLOGPERIOD;
      std::lock_guard<std::mutex> lk(mstat);
      for( callerid_t ep=0; ep!=MAXEP;++ep ){
	if( endpoints[ep].pingt_n ){
	  announce_latency( ep, endpoints[ep].pingt_min,
			    endpoints[ep].pingt_sum/endpoints[ep].pingt_n,
			    endpoints[ep].pingt_max);
	  endpoints[ep].pingt_n = 0;
	  endpoints[ep].pingt_min = 1000;
	  endpoints[ep].pingt_max = 0;
	  endpoints[ep].pingt_sum = 0.0;
	}
      }
    }
    --statlogcnt;
  }
}
