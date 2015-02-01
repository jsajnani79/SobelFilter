#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include <err.h>
#include "sobel_alg.h"

int parseOpts(int argc, char **argv)
{
  int c;
  int multiThreaded = 0;
  while ((c = getopt (argc, argv, "m")) != -1) {
    switch (c) {
      case 'm':
        multiThreaded = 1;
        break;
      default:
        multiThreaded = -1;
    }
  }
  return multiThreaded;
}

int mainSingleThread()
{
  runSobelST();
  return 0;
}

// This mutex will be used to allow threads to contest for thread 0 status
pthread_barrier_t endSobel;
pthread_mutex_t thread0 = PTHREAD_MUTEX_INITIALIZER;
pthread_t thread0_id = 0;
int mainMultiThread()
{
  // Thread variables
  pthread_t sobel1, sobel2;

  // Set up a barrier to synchronize both threads at the end of runSobel
  pthread_barrier_init(&endSobel, NULL, 2);

  // Call threads
  int ret;
  if ( (ret = pthread_create( &sobel1, NULL, runSobelMT, NULL)) ){
    printf("Thread creation failed: %d\n", ret);
    exit(1);
  }
  if ( (ret = pthread_create( &sobel2, NULL, runSobelMT, NULL)) ){
    printf("Thread creation failed: %d\n", ret);
    exit(1);
  }

  // Wait for them to finish
  pthread_join(sobel1, NULL);
  pthread_join(sobel2, NULL);

  // Destroy the barriers
  pthread_barrier_destroy(&endSobel);

  // Return ok if sobel returns correctly
  return 0;
}

int main(int argc, char **argv)
{
  int multiThreaded = parseOpts(argc, argv);

  if (multiThreaded == 0) {
    mainSingleThread();
  }
  else if (multiThreaded == 1) {
    mainMultiThread();
  }
  else {  // Invalid argument
   fprintf(stderr,"Usage: %s [-m]\n",argv[0]);
  }
  return 0;
}
