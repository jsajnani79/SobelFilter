#ifndef PERF_COUNTER_H
 #define PERF_COUNTER_H

  #ifdef __arm__
    #include <perfmon/pfmlib.h>
    #include <perfmon/pfmlib_perf_event.h>
  #else
struct pfm_perf_encode_arg_t{};
struct perf_event_attr{};
typedef unsigned long uint64_t;
  #endif

struct perf_counter_t{
  struct perf_event_attr attr;
  pfm_perf_encode_arg_t arg;
  int fd;
  uint64_t count, values[3];
};

struct counters_t{
  perf_counter_t cycles;
  perf_counter_t l1_misses;
  perf_counter_t ic;
};


void pc_init(counters_t *counters, int pid);
void pc_start(counters_t *counters);
void pc_stop(counters_t *counters);

#endif
