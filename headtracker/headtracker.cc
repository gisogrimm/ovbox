#include <tascar/serialport.h>
#include <tascar/errorhandling.h>
#include <iostream>
#include <lo/lo.h>
#include <string>
#include <vector>
#include <getopt.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <ctime>
#include <iomanip>

class ser_t : public TASCAR::serialport_t {
public:
  ser_t( ) {};
  int open( const char* dev );

};

int ser_t::open( const char* dev )
{
  int rval(TASCAR::serialport_t::open( dev, 115200, 0, 1 ));
  return rval;
}

std::mutex logmutex;
int verbose(1);

void log(int portno, const std::string& s)
{
  std::lock_guard<std::mutex> guard(logmutex);
  std::time_t t(std::time(nullptr));
  std::cerr << std::put_time(std::gmtime(&t), "%c") << " [" << portno << "] "
            << s << std::endl;
}


void app_usage(const std::string& app_name,struct option * opt,const std::string& app_arg, const std::string& help)
{
  std::cout << "Usage:\n\n" << app_name << " [options] " << app_arg << "\n\n";
  if( !help.empty() )
    std::cout << help << "\n\n";
  std::cout << "Options:\n\n";
  while( opt->name ){
    std::cout << "  -" << (char)(opt->val) << " " << (opt->has_arg?"#":"") <<
      "\n  --" << opt->name << (opt->has_arg?"=#":"") << "\n\n";
    opt++;
  }
}

int main( int argc, char** argv )
{
  uint32_t devidx;
  unsigned int ax(1);
  double scale(1.0);
  std::string port("9877");
  std::vector<std::string> devices;
  devices.push_back("/dev/ttyUSB0");
  devices.push_back("/dev/ttyUSB1");
  devices.push_back("/dev/ttyUSB2");
  std::string path("/scene/master/zyxeuler");
  std::string calib0path("/scene/ego/0/ap1/sndfile/loop");
  std::string calib1path("/scene/ego/0/ap0/sndfile/loop");
  //
  const char *options = "p:r:1:0:a:s:h";
  struct option long_options[] = { 
    { "port",   1, 0, 'p' },
    { "rpath",  1, 0, 'r' },
    { "c1path", 1, 0, '1' },
    { "c0path", 1, 0, '0' },
    { "axis",   1, 0, 'a' },
    { "scale",  1, 0, 's' },
    { "help",   0, 0, 'h' },
    { 0, 0, 0, 0 }
  };
  int opt(0);
  int option_index(0);
  while( (opt = getopt_long(argc, argv, options,
			    long_options, &option_index)) != -1){
    switch(opt){
    case 'h':
      app_usage( "headtracker", long_options, "", "" );
      return 0;
    case 'p':
      port = optarg;
      break;
    case 'd':
      devices.push_back(optarg);
      break;
    case 'r':
      path = optarg;
      break;
    case 's':
      scale = atof(optarg);
      break;
    case '0':
      calib0path = optarg;
      break;
    case '1':
      calib1path = optarg;
      break;
    case 'a':
      ax = atoi(optarg);
      break;
    }
  }
  
  //
  log( 0, "creating destination address");
  lo_address loa(lo_address_new("localhost",port.c_str()));
  if( !loa )
    throw TASCAR::ErrMsg("Invalid lo address");
  log( 0, "destination address ready");
  while( true ){
    try{
      ser_t dev;
      log( 0, "opening device " + devices[devidx]);
      dev.open( devices[devidx].c_str() );
      log( 0, "device open");
      std::vector<double> data(3,0.0);
      while( true ){
	std::string l(dev.readline( 1024, 10 ));
	if( l.size() ){
	  if( l[0] == 'G' ){
	    l = l.substr( 1 );
	    std::string::size_type sz;
	    data[0] = std::stod( l, &sz );
	    l = l.substr( sz+1 );
	    data[1] = std::stod( l, &sz );
	    l = l.substr( sz+1 );
	    data[2] = std::stod( l );
	    lo_send( loa, path.c_str(), "fff", (float)(scale*data[ax]), 0.0f, 0.0f );
	  }else{
	    if( l[0] == 'C' ){
	      if( l.size() > 1 ){
		if( (l[1] == '1') && calib1path.size() )
		  lo_send( loa, calib1path.c_str(), "i", 1 );
		if( (l[1] == '0') && calib0path.size() )
		  lo_send( loa, calib0path.c_str(), "i", 1 );
	      }
	    }
	  }
	}
      }
    }
    catch( const std::exception& e ){
      std::cerr << e.what() << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      ++devidx;
      if( devidx >= devices.size() )
	devidx = 0;
    }
  }
  return 0;
}
