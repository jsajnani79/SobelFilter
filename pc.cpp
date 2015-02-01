#include "pc.h"
#include <stdio.h>
#include <stdlib.h>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <time.h>
#include <iostream>
#include <fstream>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include <sys/ioctl.h>
#include <err.h>

#ifdef __arm__
#include <perfmon/pfmlib.h>
#include <perfmon/pfmlib_perf_event.h>
#endif

// Setup the counters and populate the counters struct with their data
void pc_init(counters_t *counters, int pid)
{
  int ret;

#ifndef __arm__
  return;

#else
  ret = pfm_initialize();

  if (ret != PFM_SUCCESS) {
    errx(1, "cannot initialize library: %s", pfm_strerror(ret));
  }

  // Set values for getting cycle count
  memset(&counters->cycles.attr, 0, sizeof(counters->cycles.attr));
  memset(&counters->l1_misses.attr, 0, sizeof(counters->l1_misses.attr));
  memset(&counters->ic.attr, 0, sizeof(counters->ic.attr));

  memset(&counters->cycles.arg, 0, sizeof(counters->cycles.arg));
  memset(&counters->l1_misses.arg, 0, sizeof(counters->l1_misses.arg));
  memset(&counters->ic.arg, 0, sizeof(counters->ic.arg));

  counters->cycles.count = 0;
  counters->l1_misses.count = 0;
  counters->ic.count = 0;

  counters->cycles.arg.size = sizeof(counters->cycles.arg);
  counters->l1_misses.arg.size = sizeof(counters->l1_misses.arg);
  counters->ic.arg.size = sizeof(counters->ic.arg);

  counters->cycles.arg.attr = &counters->cycles.attr;
  counters->l1_misses.arg.attr = &counters->l1_misses.attr;
  counters->ic.arg.attr = &counters->ic.attr;

  // Get the encoding for the events
  // cycles
  ret = pfm_get_os_event_encoding("cycles", PFM_PLM0|PFM_PLM3, PFM_OS_PERF_EVENT, &counters->cycles.arg);
  if (ret != PFM_SUCCESS) {
    err(1,"Cycles: cannot get encoding %s", pfm_strerror(ret));
  }
  // l1 cache misses
  ret = pfm_get_os_event_encoding("l1-dcache-load-misses", PFM_PLM0|PFM_PLM3, PFM_OS_PERF_EVENT, &counters->l1_misses.arg);
  if (ret != PFM_SUCCESS) {
    err(1,"L1 Cache Misses:cannot get encoding %s", pfm_strerror(ret));
  }

  // instruction count misses
  ret = pfm_get_os_event_encoding("instructions", PFM_PLM0|PFM_PLM3, PFM_OS_PERF_EVENT, &counters->ic.arg);
  if (ret != PFM_SUCCESS) {
    err(1,"Instruction Count:cannot get encoding %s", pfm_strerror(ret));
  }

  // Set more options
  counters->cycles.attr.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
  counters->l1_misses.attr.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
  counters->ic.attr.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;

  // do not start immediately after perf_event_open()
  counters->cycles.attr.disabled = 1;
  counters->l1_misses.attr.disabled = 1;
  counters->ic.attr.disabled = 1;

  // Open the counters
  counters->cycles.fd = perf_event_open(&counters->cycles.attr, pid, -1, -1, 0);
  if (counters->cycles.fd < 0) {
    err(1, "Cycle: cannot create event");
  }
  
  counters->l1_misses.fd = perf_event_open(&counters->l1_misses.attr, pid, -1, -1, 0);
  if (counters->l1_misses.fd < 0) {
    err(1, "L1 miss: cannot create event");
  }

  counters->ic.fd = perf_event_open(&counters->ic.attr, pid, -1, -1, 0);  
  if (counters->ic.fd < 0) {
    err(1, "Instruction count: cannot create event");
  }
  return;
#endif
}

void pc_start(counters_t *counters)
{
  counters->cycles.count = 0;
  counters->l1_misses.count = 0;
  counters->ic.count = 0;

#ifndef __arm__
  return;

#else
  int ret;
  ret = ioctl(counters->cycles.fd, PERF_EVENT_IOC_RESET, 0);
  if (ret) {
    err(1, "ioctl(reset) failed");
  }

  ret = ioctl(counters->l1_misses.fd, PERF_EVENT_IOC_RESET, 0);
  if (ret) {
    err(1, "ioctl(reset) failed");
  }

  ret = ioctl(counters->ic.fd, PERF_EVENT_IOC_RESET, 0);
  if (ret) {
    err(1, "ioctl(reset) failed");
  }

  ret = ioctl(counters->cycles.fd, PERF_EVENT_IOC_ENABLE, 0);    
  if (ret) {
    err(1, "ioctl(enable) failed");
  }

  ret = ioctl(counters->l1_misses.fd, PERF_EVENT_IOC_ENABLE, 0);    
  if (ret) {
    err(1, "ioctl(enable) failed");
  }

  ret = ioctl(counters->ic.fd, PERF_EVENT_IOC_ENABLE, 0);
  if (ret) {
    err(1, "ioctl(enable) failed");
  }
  return;
#endif
}

void pc_stop(counters_t *counters)
{
#ifndef __arm__
  return;
#else
  ioctl(counters->cycles.fd, PERF_EVENT_IOC_DISABLE, 0);    
  ioctl(counters->l1_misses.fd, PERF_EVENT_IOC_DISABLE, 0);    
  ioctl(counters->ic.fd, PERF_EVENT_IOC_DISABLE, 0);
  
  read(counters->cycles.fd, counters->cycles.values, sizeof(counters->cycles.values));
  read(counters->l1_misses.fd, counters->l1_misses.values, sizeof(counters->l1_misses.values));
  read(counters->ic.fd, counters->ic.values, sizeof(counters->ic.values));
  
  if (counters->cycles.values[2]) {
    counters->cycles.count = (uint64_t)((double)counters->cycles.values[0] * 
		     counters->cycles.values[1]/counters->cycles.values[2]);
  }
  else {
    counters->cycles.count = (uint64_t)counters->cycles.values[0];
  }
  
  if (counters->l1_misses.values[2]) {
    counters->l1_misses.count = (uint64_t)
      ((double)counters->l1_misses.values[0] * 
       counters->l1_misses.values[1]/counters->l1_misses.values[2]);
  }
  else {
    counters->l1_misses.count = (uint64_t)counters->l1_misses.values[0];
  }
  
  if (counters->ic.values[2]) {
    counters->ic.count = (uint64_t)((double)counters->ic.values[0] * 
			    counters->ic.values[1]/counters->ic.values[2]);
  }
  else {
    counters->ic.count = (uint64_t)counters->ic.values[0];
  }  
  return;
#endif
}
