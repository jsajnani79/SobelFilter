#include "opencv2/imgproc/imgproc.hpp"
#include "sobel_alg.h"
#include <arm_neon.h>
using namespace cv;

/*******************************************
 * Model: grayScale
 * Input: Mat img
 * Output: None directly. Modifies a ref parameter img_gray_out
 * Desc: This module converts the image to grayscale
 ********************************************/
void grayScale(Mat& img, Mat& img_gray_out)
{
  // double color;

  // Convert to grayscale
  for (int i=0; i<img.rows; i++) {
    int first = STEP0*i;
    int imgOffset = IMG_WIDTH*i;
    for (int j=0; j<img.cols; j++) {
      int second = first+STEP1*j;

 //      color = .114*img.data[STEP0*i + STEP1*j] + 
	// .587*img.data[STEP0*i + STEP1*j + 1]+
	// .299*img.data[STEP0*i + STEP1*j + 2];

      img_gray_out.data[imgOffset + j] = (116*img.data[second] + 
  601*img.data[second + 1]+
  306*img.data[second + 2]) >> 10;

    }
  }
} 

void grayScaleMt(Mat& img, Mat& img_gray_out, int start, int end)
{
  // Convert to grayscale
  for (int i=start; i<end; i++) {
    int first = STEP0*i;
    int imgOffset = IMG_WIDTH*i;
    for (int j=0; j<img.cols; j++) {
      int second = first+STEP1*j;

      img_gray_out.data[imgOffset + j] = (116*img.data[second] + 
        601*img.data[second + 1]+
        306*img.data[second + 2]) >> 10;
    }
  }



} 

/*******************************************
 * Model: sobelCalc
 * Input: Mat img_in
 * Output: None directly. Modifies a ref parameter img_sobel_out
 * Desc: This module performs a sobel calculation on an image. It first
 *  converts the image to grayscale, calculates the gradient in the x 
 *  direction, calculates the gradient in the y direction and sum it with Gx
 *  to finish the Sobel calculation
 ********************************************/
void sobelCalc(Mat& img_gray, Mat& img_sobel_out)
{
  Mat img_outx = img_gray.clone();
  Mat img_outy = img_gray.clone();
  
  // Apply Sobel filter to black & white image
  unsigned short sobel_total;
  unsigned short sobel_X;
  unsigned short sobel_Y;

  int i, j;
  for (i=1; i<img_gray.rows; i++) {

      int center = IMG_WIDTH*(i);
      int leftWidth = center - IMG_WIDTH;
      int rightWidth = center + IMG_WIDTH;

      for (j=1; j<img_gray.cols; j++) {

        int bottomRight = img_gray.data[rightWidth + j-1];
        int topLeft = img_gray.data[leftWidth + j+1];
        int bLeftMinusRight = img_gray.data[leftWidth + j-1] - img_gray.data[rightWidth + j+1];
        
        sobel_X = abs(bLeftMinusRight - bottomRight + (img_gray.data[leftWidth + j]<<1) - (img_gray.data[rightWidth + j]<<1) + topLeft);

        // sobel_X = (sobel_X > 255) ? 255 : sobel_X;
        // img_outx.data[center + j] = sobel_X;

        sobel_Y = abs(bLeftMinusRight - topLeft + (img_gray.data[center + j-1]<<1) - (img_gray.data[center + j+1]<<1) + bottomRight);

       // sobel_Y = (sobel_Y > 255) ? 255 : sobel_Y;

       // img_outy.data[center + j] = sobel_Y;

       sobel_total = sobel_X + sobel_Y;
       sobel_total = (sobel_total > 255) ? 255 : sobel_total;
       img_sobel_out.data[center + j] = sobel_total;

      }
    }
}

void sobelCalcMt(Mat& img_gray, Mat& img_sobel_out, int start, int end)
{
  Mat img_outx = img_gray.clone();
  Mat img_outy = img_gray.clone();
  
  // Apply Sobel filter to black & white image
  unsigned short sobel_total;
  unsigned short sobel_X;
  unsigned short sobel_Y;

  int i, j;
  for (i=start+1; i<end; i++) {

      int center = IMG_WIDTH*(i);
      int leftWidth = center - IMG_WIDTH;
      int rightWidth = center + IMG_WIDTH;

      for (j=1; j<img_gray.cols; j++) {

        int bottomRight = img_gray.data[rightWidth + j-1];
        int topLeft = img_gray.data[leftWidth + j+1];
        int bLeftMinusRight = img_gray.data[leftWidth + j-1] - img_gray.data[rightWidth + j+1];
        
        sobel_X = abs(bLeftMinusRight - bottomRight + (img_gray.data[leftWidth + j]<<1) - (img_gray.data[rightWidth + j]<<1) + topLeft);

        // sobel_X = (sobel_X > 255) ? 255 : sobel_X;
        // img_outx.data[center + j] = sobel_X;

        sobel_Y = abs(bLeftMinusRight - topLeft + (img_gray.data[center + j-1]<<1) - (img_gray.data[center + j+1]<<1) + bottomRight);

       // sobel_Y = (sobel_Y > 255) ? 255 : sobel_Y;

       // img_outy.data[center + j] = sobel_Y;

       sobel_total = sobel_X + sobel_Y;
       sobel_total = (sobel_total > 255) ? 255 : sobel_total;
       img_sobel_out.data[center + j] = sobel_total;

      }
    }
}
