#include <stdio.h>
#include <stdlib.h>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include <sys/ioctl.h>
#include <err.h>
#include <pthread.h>

#include "sobel_alg.h"
#include "pc.h"

// Replaces img.step[0] and img.step[1] calls in sobel calc

using namespace cv;

static ofstream results_file;

// Define image mats to pass between function calls
static Mat img_gray, img_sobel;
static float total_fps, total_ipc, total_epf;
static float gray_total, sobel_total, cap_total, disp_total;
static float sobel_ic_total, sobel_l1cm_total;
pthread_barrier_t grayBarr;
pthread_barrier_t sobelBarr;

/*******************************************
 * Model: runSobelMT
 * Input: None
 * Output: None
 * Desc: This method pulls in an image from the webcam, feeds it into the
 *   sobelCalc module, and displays the returned Sobel filtered image. This
 *   function processes NUM_ITER frames.
 ********************************************/
void *runSobelMT(void *ptr)
{
  // Set up variables for computing Sobel
  string top = "Sobel Top";
  Mat src;
  uint64_t cap_time, gray_time, sobel_time, disp_time, sobel_l1cm, sobel_ic;
  pthread_t myID = pthread_self();
  counters_t perf_counters;

  // Allow the threads to contest for thread0 (controller thread) status
  pthread_mutex_lock(&thread0);
  
  pthread_barrier_init(&grayBarr, NULL, 1);
  pthread_barrier_init(&sobelBarr, NULL, 1);

  // Check to see if this thread is first to this part of the code
  if (thread0_id == 0) {
    thread0_id = myID;
  }
  pthread_mutex_unlock(&thread0);

  // For now, we just kill the second thread. It's up to you to get it to compute
  // the other half of the image.
  /*if (myID != thread0_id) {
    pthread_barrier_wait(&endSobel);
    return NULL;
  } */

  pc_init(&perf_counters, 0);
  
  // Start algorithm
  CvCapture* web_cam_cap;

  web_cam_cap = cvCreateCameraCapture(0);
  cvSetCaptureProperty(web_cam_cap, CV_CAP_PROP_FRAME_WIDTH, IMG_WIDTH);
  cvSetCaptureProperty(web_cam_cap, CV_CAP_PROP_FRAME_HEIGHT, IMG_HEIGHT);

  // Keep track of the frames
  int i = 0;

  while (1) {
    // Allocate memory to hold grayscale and sobel images
    img_gray = Mat(IMG_HEIGHT, IMG_WIDTH, CV_8UC1);
    img_sobel = Mat(IMG_HEIGHT, IMG_WIDTH, CV_8UC1);

    pc_start(&perf_counters);
    src = cvQueryFrame(web_cam_cap);
    pc_stop(&perf_counters);

    cap_time = perf_counters.cycles.count;
    sobel_l1cm = perf_counters.l1_misses.count;
    sobel_ic = perf_counters.ic.count;

    // Lab1, part 2: Start parallel section
    pc_start(&perf_counters);
    if (thread0_id == pthread_self()) {
      // do top half grayscale
    } else {
      // do bottom half grayscale
    }
    grayScale(src, img_gray);

    int grc = pthread_barrier_wait(&grayBarr);
    if(grc != 0 && grc != PTHREAD_BARRIER_SERIAL_THREAD)
    {
        printf("Could not wait on gray barrier\n");
        exit(-1);
    }
    pc_stop(&perf_counters);

    gray_time = perf_counters.cycles.count;
    sobel_l1cm += perf_counters.l1_misses.count;
    sobel_ic += perf_counters.ic.count;


    // add barrier to wait for threads to be finished

    pc_start(&perf_counters);
    if (thread0_id == pthread_self()) {
      // do top half sobelcalc
    } else {
      // do bottom half sobelcalc
    }
    sobelCalc(img_gray, img_sobel);
    int src = pthread_barrier_wait(&sobelBarr);
    if(src != 0 && src != PTHREAD_BARRIER_SERIAL_THREAD)
    {
        printf("Could not wait on sobel barrier\n");
        exit(-1);
    }
    pc_stop(&perf_counters);

    sobel_time = perf_counters.cycles.count;
    // Lab1, part2: End parallel section

    pc_start(&perf_counters);
    namedWindow(top, CV_WINDOW_AUTOSIZE);
    imshow(top, img_sobel);
    pc_stop(&perf_counters);

    disp_time = perf_counters.cycles.count;
    sobel_l1cm += perf_counters.l1_misses.count;
    sobel_ic += perf_counters.ic.count;

    cap_total += cap_time;
    gray_total += gray_time;
    sobel_total += sobel_time;
    sobel_l1cm_total += sobel_l1cm;
    sobel_ic_total += sobel_ic;
    disp_total += disp_time;
    total_fps += PROC_FREQ/float(cap_time + disp_time + gray_time + sobel_time);
    total_ipc += float(sobel_ic/float(gray_time + sobel_time));

    // Press q to exit
    char c = cvWaitKey(10);
    if (c == 'q') {
      break;
    }
    i++;
  }

  total_epf = PROC_EPC*NCORES/(total_fps/i);
  float total_time = float(gray_total + sobel_total + cap_total + disp_total);

  results_file.open("sobel_perf_mt.csv", ios::out);
  results_file << "Percent of time per function" << endl;
  results_file << "Capture, " << (cap_total/total_time)*100 << "%" << endl;
  results_file << "Grayscale, " << (gray_total/total_time)*100 << "%" << endl;
  results_file << "Sobel, " << (sobel_total/total_time)*100 << "%" << endl;
  results_file << "Display, " << (disp_total/total_time)*100 << "%" << endl;
  results_file << "\nSummary" << endl;
  results_file << "Frames per second, " << total_fps/i << endl;  
  results_file << "Cycles per frame, " << total_time/i << endl;
  results_file << "Energy per frames (mJ), " << total_epf*1000 << endl;  
  results_file << "Total frames, " << i << endl;
  results_file << "\nHardware Stats (Cap + Gray + Sobel + Display)" << endl;
  results_file << "Instructions per cycle, " << total_ipc/i << endl;
  results_file << "L1 misses per frame, " << sobel_l1cm_total/i << endl;
  results_file << "L1 misses per instruction, " << sobel_l1cm_total/sobel_ic_total << endl;
  results_file << "Instruction count per frame, " << sobel_ic_total/i << endl;

  cvReleaseCapture(&web_cam_cap);
  results_file.close();
  pthread_barrier_destroy(&grayBarr);
  pthread_barrier_destroy(&sobelBarr);
  pthread_barrier_wait(&endSobel);
  return NULL;
}
