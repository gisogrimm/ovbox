#include <jack/jack.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv)
{
  if(argc < 2) {
    fprintf(stderr, "Usage: jackwaitforport portname\n");
  }
  jack_status_t jstat;
  jack_client_t* jc;
  jack_options_t opt = (jack_options_t)(JackNoStartServer | JackUseExactName);
  jc = jack_client_open("jackwaitforport", opt, &jstat);
  if(!jc) {
    return 1;
  }
  for(uint32_t c = 1; c < argc; ++c) {
    uint32_t k = 3000;
    jack_port_t* jp = NULL;
    while(k && (!jp)) {
      if( k )
	--k;
      usleep(10000);
      jp = jack_port_by_name(jc, argv[c]);
    }
  }
  jack_client_close(jc);
  return 0;
}
