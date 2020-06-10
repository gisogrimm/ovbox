#include <chrono>
#include <ctime>
#include <fcntl.h>
#include <getopt.h>
#include <iomanip>
#include <iostream>
#include <lo/lo.h>
#include <mutex>
#include <stdint.h>
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <vector>

#define DEBUG(x)                                                               \
  std::cerr << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__      \
            << " " << #x << "=" << x << std::endl

class ErrMsg : public std::exception, private std::string {
public:
  ErrMsg(const std::string &msg);
  virtual ~ErrMsg() throw();
  const char *what() const throw();
};

ErrMsg::ErrMsg(const std::string &msg) : std::string(msg) {}

ErrMsg::~ErrMsg() throw() {}

const char *ErrMsg::what() const throw() { return c_str(); }

class serialport_t {
public:
  serialport_t();
  ~serialport_t();
  int open(const char *dev, int speed, int parity = 0, int stopbits = 0);
  void set_interface_attribs(int speed, int parity, int stopbits);
  void set_blocking(int should_block);
  bool isopen();
  std::string readline(uint32_t maxlen, char delim);
  void close();

protected:
  int fd;
};

serialport_t::serialport_t() : fd(-1) {}

serialport_t::~serialport_t() {
  if (fd > 0)
    close();
}

bool serialport_t::isopen() { return (fd > 0); }

int serialport_t::open(const char *dev, int speed, int parity, int stopbits) {
  fd = ::open(dev, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0)
    throw ErrMsg(std::string("Unable to open device ") + dev);
  set_interface_attribs(speed, parity, stopbits);
  set_blocking(1);
  return fd;
}

void serialport_t::set_interface_attribs(int speed, int parity, int stopbits) {
  struct termios tty;
  memset(&tty, 0, sizeof tty);
  if (tcgetattr(fd, &tty) != 0)
    throw ErrMsg("Error from tcgetattr");
  cfsetospeed(&tty, speed);
  cfsetispeed(&tty, speed);
  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
  // disable IGNBRK for mismatched speed tests; otherwise receive break
  // as \000 chars
  tty.c_iflag &= ~IGNBRK; // disable break processing
  tty.c_iflag |= BRKINT;  // enable break processing
  tty.c_lflag = 0;        // no signaling chars, no echo,
  // no canonical processing
  tty.c_oflag = 0;                        // no remapping, no delays
  tty.c_cc[VMIN] = 0;                     // read doesn't block
  tty.c_cc[VTIME] = 5;                    // 0.5 seconds read timeout
  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff flow ctrl
  tty.c_cflag |= (CLOCAL | CREAD);        // ignore modem controls,
  // enable reading
  tty.c_cflag &= ~(PARENB | PARODD); // shut off parity
  tty.c_cflag |= parity;
  tty.c_cflag &= ~CSTOPB;
  if (stopbits == 2)
    tty.c_cflag |= CSTOPB;
  tty.c_cflag &= ~CRTSCTS;
  if (tcsetattr(fd, TCSANOW, &tty) != 0)
    throw ErrMsg("error from tcsetattr");
  int flags;
  ioctl(fd, TIOCMGET, &flags);
  flags &= ~TIOCM_RTS;
  flags &= ~(TIOCM_RTS | TIOCM_DTR);
  ioctl(fd, TIOCMSET, &flags);
}

void serialport_t::set_blocking(int should_block) {
  struct termios tty;
  memset(&tty, 0, sizeof tty);
  if (tcgetattr(fd, &tty) != 0)
    throw ErrMsg("error from tggetattr");
  tty.c_cc[VMIN] = should_block ? 1 : 0;
  tty.c_cc[VTIME] = 5; // 0.5 seconds read timeout
  if (tcsetattr(fd, TCSANOW, &tty) != 0)
    throw ErrMsg("error setting term attributes");
}

void serialport_t::close() { ::close(fd); }

std::string serialport_t::readline(uint32_t maxlen, char delim) {
  std::string r;
  while (isopen() && maxlen) {
    maxlen--;
    char c(0);
    if (::read(fd, &c, 1) == 1) {
      if (c != delim)
        r += c;
      else
        return r;
    }
  }
  return r;
}

class ser_t : public serialport_t {
public:
  ser_t(){};
  int open(const char *dev);
};

int ser_t::open(const char *dev) {
  int rval(serialport_t::open(dev, B115200, 0, 1));
  return rval;
}

std::mutex logmutex;
int verbose(1);

void log(int portno, const std::string &s) {
  std::lock_guard<std::mutex> guard(logmutex);
  std::time_t t(std::time(nullptr));
  std::cerr << std::put_time(std::gmtime(&t), "%c") << " [" << portno << "] "
            << s << std::endl;
}

void app_usage(const std::string &app_name, struct option *opt,
               const std::string &app_arg, const std::string &help) {
  std::cout << "Usage:\n\n" << app_name << " [options] " << app_arg << "\n\n";
  if (!help.empty())
    std::cout << help << "\n\n";
  std::cout << "Options:\n\n";
  while (opt->name) {
    std::cout << "  -" << (char)(opt->val) << " " << (opt->has_arg ? "#" : "")
              << "\n  --" << opt->name << (opt->has_arg ? "=#" : "") << "\n\n";
    opt++;
  }
}

int main(int argc, char **argv) {
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
  struct option long_options[] = {{"port", 1, 0, 'p'},   {"rpath", 1, 0, 'r'},
                                  {"c1path", 1, 0, '1'}, {"c0path", 1, 0, '0'},
                                  {"axis", 1, 0, 'a'},   {"scale", 1, 0, 's'},
                                  {"help", 0, 0, 'h'},   {0, 0, 0, 0}};
  int opt(0);
  int option_index(0);
  while ((opt = getopt_long(argc, argv, options, long_options,
                            &option_index)) != -1) {
    switch (opt) {
    case 'h':
      app_usage("headtracker", long_options, "", "");
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
  log(0, "creating destination address");
  lo_address loa(lo_address_new("localhost", port.c_str()));
  if (!loa)
    throw ErrMsg("Invalid lo address");
  log(0, "destination address ready");
  while (true) {
    try {
      ser_t dev;
      log(0, "opening device " + devices[devidx]);
      dev.open(devices[devidx].c_str());
      log(0, "device open");
      std::vector<double> data(3, 0.0);
      while (true) {
        std::string l(dev.readline(1024, 10));
        if (l.size()) {
          if (l[0] == 'G') {
            l = l.substr(1);
            std::string::size_type sz;
            data[0] = std::stod(l, &sz);
            l = l.substr(sz + 1);
            data[1] = std::stod(l, &sz);
            l = l.substr(sz + 1);
            data[2] = std::stod(l);
            lo_send(loa, path.c_str(), "fff", (float)(scale * data[ax]), 0.0f,
                    0.0f);
          } else {
            if (l[0] == 'C') {
              if (l.size() > 1) {
                if ((l[1] == '1') && calib1path.size())
                  lo_send(loa, calib1path.c_str(), "i", 1);
                if ((l[1] == '0') && calib0path.size())
                  lo_send(loa, calib0path.c_str(), "i", 1);
              }
            }
          }
        }
      }
    } catch (const std::exception &e) {
      std::cerr << e.what() << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      ++devidx;
      if (devidx >= devices.size())
        devidx = 0;
    }
  }
  return 0;
}
