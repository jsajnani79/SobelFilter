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

      int leftWidth = IMG_WIDTH*(i-1);
      int rightWidth = IMG_WIDTH*(i+1);
      int center = IMG_WIDTH*(i);

      for (j=1; j<img_gray.cols; j++) {

        // int top = j+1;
        // int bottom = j-1;
        int bottomRight = img_gray.data[rightWidth + j-1];
        int topLeft = img_gray.data[leftWidth + j+1];
        int bLeftMinusRight = img_gray.data[leftWidth + j-1] - img_gray.data[rightWidth + j+1];
        
        sobel_X = abs(bLeftMinusRight - bottomRight + 2*img_gray.data[leftWidth + j] - 2*img_gray.data[rightWidth + j] + topLeft);

        sobel_X = (sobel_X > 255) ? 255 : sobel_X;
        img_outx.data[center + j] = sobel_X;

        sobel_Y = abs(bLeftMinusRight - topLeft + 2*img_gray.data[center + j-1] - 2*img_gray.data[center + j+1] + bottomRight);

       sobel_Y = (sobel_Y > 255) ? 255 : sobel_Y;

       img_outy.data[center + j] = sobel_Y;

       sobel_total = sobel_X + sobel_Y;
       sobel_total = (sobel_total > 255) ? 255 : sobel_total;
       img_sobel_out.data[center + j] = sobel_total;

      }
    }


  // Calc both x and y convolution
  // for (int i=1; i<img_gray.rows; i++) {
  //   for (int j=1; j<img_gray.cols; j++) {

  //     int IWiP1 = IMG_WIDTH*(i+1);
  //     int IWiM1 = IMG_WIDTH*(i-1);
  //     int IWi = IMG_WIDTH*(i);
  //     int jM1 = j-1;
  //     int jP1 = j+1;
  //     int M1PM1 = img_gray.data[IWiM1 + jM1];
  //     int P1PM1 = img_gray.data[IWiP1 + jM1];
  //     int M1PJ = img_gray.data[IWiM1 + (j)];
  //     int P1PJ = img_gray.data[IWiP1 + (j)];
  //     int M1PP1 = img_gray.data[IWiM1 + jP1];
  //     int P1PP1 = img_gray.data[IWiP1 + jP1];


  //     sobel_X = abs(M1PM1 - P1PM1 + 2*M1PJ - 2*P1PJ + M1PP1 - P1PP1);

  //     // sobel_X = abs(img_gray.data[IWiM1 + jM1]- 
  //     // img_gray.data[IWiP1 + jM1] + 
  //     // 2*img_gray.data[IWiM1 + (j)] - 
  //     // 2*img_gray.data[IWiP1 + (j)] + 
  //     // img_gray.data[IWiM1 + jP1] - 
  //     // img_gray.data[IWiP1 + jP1]);
 
  //     sobel_X = (sobel_X > 255) ? 255 : sobel_X;
  //     img_outx.data[IWi + (j)] = sobel_X;

  //     sobel_Y = abs(M1PM1 - 
  //       img_gray.data[IWiM1 + jP1] + 
  //       2*img_gray.data[IWi + jM1] - 
  //       2*img_gray.data[IWi + jP1] + 
  //      img_gray.data[IWiP1 + jM1] - 
  //      img_gray.data[IWiP1 + jP1]);
  //     // sobel_Y = abs(img_gray.data[IWiM1 + jM1] - 
  //     //   img_gray.data[IWiM1 + jP1] + 
  //     //   2*img_gray.data[IWi + jM1] - 
  //     //   2*img_gray.data[IWi + jP1] + 
  //     //  img_gray.data[IWiP1 + jM1] - 
  //     //  img_gray.data[IWiP1 + jP1]);

  //    sobel_Y = (sobel_Y > 255) ? 255 : sobel_Y;

  //    img_outy.data[IWi + j] = sobel_Y;

  //    sobel_total = img_outx.data[IWi + j] + 
  // img_outy.data[IWi + j];
  //     sobel_total = (sobel_total > 255) ? 255 : sobel_total;
  //     img_sobel_out.data[IWi + j] = sobel_total;

  //   }
  // }

}
