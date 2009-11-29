#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <stdio.h>

IplImage*	g_image = NULL;
IplImage*	g_gray = NULL;
int		g_thresh = 100;
CvMemStorage* 	g_storage = NULL;

void on_trackbar(int){
	if( g_storage == NULL ){
		g_gray = cvCreateImage( cvGetSize( g_image ), 8, 1 );
		g_storage = cvCreateMemStorage(0);
	} else {
		cvClearMemStorage( g_storage );
	}

	CvSeq* contours = 0;
	cvCvtColor( g_image, g_gray, CV_BGR2GRAY );
	cvThreshold( g_gray, g_gray, g_thresh, 255, CV_THRESH_BINARY );
	cvFindContours( g_gray, g_storage, &contours );
	cvZero( g_gray );
	if( contours ){
		cvDrawContours(
			g_gray,
			contours,
			cvScalarAll(255),
			cvScalarAll(255),
			100 );
	}
	cvShowImage( "Contours", g_gray );
}


int main(int argc, char** argv)
{

  if (argc != 2){
    printf("usage: program [image file]\n");
    exit(-42);
  }

  const char* file_name = argv[1];

  if( ( g_image = cvLoadImage( file_name ) ) == 0 ) {
    printf("Error reading template image file '%s'\n", file_name); 
    return(-1);
  }

  cvNamedWindow( "Contours", 1 );
  cvCreateTrackbar( "Threshold", "Contours", &g_thresh, 255, on_trackbar );
  on_trackbar(0);
  cvWaitKey();
 
  return 0;

}

