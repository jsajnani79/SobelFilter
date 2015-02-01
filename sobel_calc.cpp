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
  double color;

  // Convert to grayscale
  for (int i=0; i<img.rows; i = i++) {
    for (int j=0; j<img.cols; j = j++) {
      // float32x4_t calc1 = {img.data[STEP0*i + STEP1*j], img.data[STEP0*(i+1) + STEP1*(j+1)], img.data[STEP0*(i+2) + STEP1*(j+2)], img.data[STEP0*(i+3) + STEP1*(j+3)]};
      // float32x4_t num1 = {0.114,0.114,0.114,0.114};
      // float32x4_t result = vmulq_f32(calc1, num1);
      // color = vmul_f32(num1, calc1) + 
      //   vmul_f32(0.587, img.data[STEP0*i + STEP1*j + 1]) + 
      //   vmul_f32(0.299, img.data[STEP0*i + STEP1*j + 2]);

      // float32x4_t scalars = {0.144, 0.587, 0.299, 0};
      // float32x4_t dataPoints = {img.data[STEP0*i + STEP1*j], img.data[STEP0*i + STEP1*j + 1], img.data[STEP0*i + STEP1*j + 2], 0};

      color = .114*img.data[STEP0*i + STEP1*j] + 
	.587*img.data[STEP0*i + STEP1*j + 1]+
	.299*img.data[STEP0*i + STEP1*j + 2];
      img_gray_out.data[IMG_WIDTH*i + j] = color;
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

  // // Calculate the x convolution
  // for (int i=1; i<img_gray.rows; i++) {
  //   for (int j=1; j<img_gray.cols; j++) {
  //     sobel = abs(img_gray.data[IMG_WIDTH*(i-1) + (j-1)]- 
		//   img_gray.data[IMG_WIDTH*(i+1) + (j-1)] + 
		//   2*img_gray.data[IMG_WIDTH*(i-1) + (j)] - 
		//   2*img_gray.data[IMG_WIDTH*(i+1) + (j)] + 
		//   img_gray.data[IMG_WIDTH*(i-1) + (j+1)] - 
		//   img_gray.data[IMG_WIDTH*(i+1) + (j+1)]);
 
  //     sobel = (sobel > 255) ? 255 : sobel;
  //     img_outx.data[IMG_WIDTH*(i) + (j)] = sobel;
  //   }
  // }

  // // Calc the y convolution
  // for (int i=1; i<img_gray.rows; i++) {
  //   for (int j=1; j<img_gray.cols; j++) {
  //    sobel = abs(img_gray.data[IMG_WIDTH*(i-1) + (j-1)] - 
		//    img_gray.data[IMG_WIDTH*(i-1) + (j+1)] + 
		//    2*img_gray.data[IMG_WIDTH*(i) + (j-1)] - 
		//    2*img_gray.data[IMG_WIDTH*(i) + (j+1)] + 
		//    img_gray.data[IMG_WIDTH*(i+1) + (j-1)] - 
		//    img_gray.data[IMG_WIDTH*(i+1) + (j+1)]);

  //    sobel = (sobel > 255) ? 255 : sobel;

  //    img_outy.data[IMG_WIDTH*(i) + j] = sobel;
  //   }
  // }


  // Calc both x and y convolution
  for (int i=1; i<img_gray.rows; i++) {
    for (int j=1; j<img_gray.cols; j++) {

      sobel_X = abs(img_gray.data[IMG_WIDTH*(i-1) + (j-1)]- 
      img_gray.data[IMG_WIDTH*(i+1) + (j-1)] + 
      2*img_gray.data[IMG_WIDTH*(i-1) + (j)] - 
      2*img_gray.data[IMG_WIDTH*(i+1) + (j)] + 
      img_gray.data[IMG_WIDTH*(i-1) + (j+1)] - 
      img_gray.data[IMG_WIDTH*(i+1) + (j+1)]);
 
      sobel_X = (sobel_X > 255) ? 255 : sobel_X;
      img_outx.data[IMG_WIDTH*(i) + (j)] = sobel_X;


      sobel_Y = abs(img_gray.data[IMG_WIDTH*(i-1) + (j-1)] - 
       img_gray.data[IMG_WIDTH*(i-1) + (j+1)] + 
       2*img_gray.data[IMG_WIDTH*(i) + (j-1)] - 
       2*img_gray.data[IMG_WIDTH*(i) + (j+1)] + 
       img_gray.data[IMG_WIDTH*(i+1) + (j-1)] - 
       img_gray.data[IMG_WIDTH*(i+1) + (j+1)]);

     sobel_Y = (sobel_Y > 255) ? 255 : sobel_Y;

     img_outy.data[IMG_WIDTH*(i) + j] = sobel_Y;

     sobel_total = img_outx.data[IMG_WIDTH*(i) + j] + 
  img_outy.data[IMG_WIDTH*(i) + j];
      sobel_total = (sobel_total > 255) ? 255 : sobel_total;
      img_sobel_out.data[IMG_WIDTH*(i) + j] = sobel_total;

    }
  }


 //  // Combine the two convolutions into the output image
 //  for (int i=1; i<img_gray.rows; i++) {
 //    for (int j=1; j<img_gray.cols; j++) {
 //      sobel = img_outx.data[IMG_WIDTH*(i) + j] + 
	// img_outy.data[IMG_WIDTH*(i) + j];
 //      sobel = (sobel > 255) ? 255 : sobel;
 //      img_sobel_out.data[IMG_WIDTH*(i) + j] = sobel;
 //    }
 //  }
}
