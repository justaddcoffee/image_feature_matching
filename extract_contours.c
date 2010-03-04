/*
Given an image file (and possibly a threshold?), extract all contours and write to an image file

./extract_contours image_file 

writes out 
   image_file_contours_1
   image_file_contours_2
   image_file_contours_3

*/

#define DEBUG 1

#define THRESHOLD_TYPE CV_THRESH_BINARY
// CV_THRESH_BINARY or CV_THRESH_BINARY_INV

//////////////////////////////////////
//these are for adaptive thresholding
//////////////////////////////////////
#define ADAPTIVE_METHOD CV_ADAPTIVE_THRESH_GAUSSIAN_C // CV_ADAPTIVE_THRESH_MEAN_C or CV_ADAPTIVE_THRESH_GAUSSIAN_C
#define BLOCK_SIZE 3 // how big an area around a pixel to consider while thresholding
#define SUBTRACT_PARAM 15 // 

// standard opencv stuff
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <stdio.h>

IplImage*	image_original = NULL;
IplImage*	image_manip = NULL;
int		threshold = 100; // only used for regular (not adaptive) thresholding
CvMemStorage* 	contour_storage = NULL;

int main(int argc, char** argv){

  if (argc != 2){
    printf("usage: program [image file]\n");
    exit(-42);
  }

  const char* file_name = argv[1];
  if( ( image_original = cvLoadImage( file_name ) ) == 0 ) {
    printf("Error reading template image file '%s'\n", file_name); 
    return(-1);
  }

  #if DEBUG
    cvNamedWindow( "Original", 1);
    cvShowImage( "Original", image_original );
    cvNamedWindow( "Contours", 1 );
  #endif

  ////////////////////////////
  // start manipulating image
  ////////////////////////////
  image_manip = cvCreateImage( cvGetSize( image_original ), 8, 1 );
  contour_storage = cvCreateMemStorage(0);
  CvSeq* contours = 0;

  // convert to grayscale
  cvCvtColor( image_original, image_manip, CV_BGR2GRAY );

  // make contour-ready image using thresholding or canny edge detection 
  cvThreshold( image_manip, image_manip, threshold, 255, THRESHOLD_TYPE );
  //cvAdaptiveThreshold(image_manip, image_manip, 255, ADAPTIVE_METHOD, THRESHOLD_TYPE, BLOCK_SIZE, SUBTRACT_PARAM);
  //cvCanny( image_manip, image_manip, 50, 900, 3 ); // these seem to exclude a lot of crap

  #if DEBUG
     cvNamedWindow( "Thresholded", 1);
     cvShowImage( "Thresholded", image_manip );
  #endif

  cvFindContours( image_manip, contour_storage, &contours );

  // zero out image, then draw contours
  cvZero( image_manip );
  if( contours ){
    cvDrawContours(
		   image_manip,
		   contours,
		   cvScalarAll(255),
		   cvScalarAll(255),
		   100 );
  }

  #if DEBUG
    cvShowImage( "Contours", image_manip );
    cvWaitKey();  
  #endif

  return 0;

}

