#include <chrono>
#include <iostream>
#include <string.h>
#include <thread>

double get_pingtime(std::chrono::high_resolution_clock::time_point& t1)
{
  std::chrono::high_resolution_clock::time_point t2(
      std::chrono::high_resolution_clock::now());
  std::chrono::duration<double> time_span =
      std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
  t1 = t2;
  return (1000.0 * time_span.count());
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

int main()
{
  set_thread_prio(30);
  std::chrono::high_resolution_clock::time_point t1;
  get_pingtime(t1);
  double tmean(0);
  double tmax(0);
  for(uint32_t k = 0; k < 100000; ++k) {
    std::this_thread::sleep_for(std::chrono::microseconds(2000));
    double t(get_pingtime(t1));
    tmean += t;
    tmax = std::max(t, tmax);
  }
  std::cout << "mean = " << 0.00001 * tmean << "ms, peak = " << tmax - 2.0
            << "ms\n";
  return 0;
}
