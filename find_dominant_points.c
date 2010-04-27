#include <cv.h>
#include <highgui.h>
#include <stdio.h>

/* 
   Given an image, 
      start up a window with trackbar for dmin dmax dn and theta, 
      find contours, then find dominant points given the values
      write contours and dominant points onto image
*/

IplImage*	g_image = NULL;
IplImage*	g_gray = NULL;
int		dmin = 7;
int		dmax = 9;
int		dn = 9;
int		theta = 150;
CvMemStorage* 	g_storage = NULL;
CvMemStorage*   g_find_dom_points_storage = NULL;

void on_trackbar(int){
  if( g_storage == NULL ){
    g_gray = cvCreateImage( cvGetSize( g_image ), 8, 1 );
    g_storage = cvCreateMemStorage(0);
    g_find_dom_points_storage = cvCreateMemStorage(0);
  } else {
    cvClearMemStorage( g_storage );
    cvClearMemStorage( g_find_dom_points_storage );
  }

  if ( dmin > dmax ){
    printf("dmin %i > dmax %i!!! setting dmin = dmax", dmin, dmax);
    dmin = dmax;
  }

  if ( dn > dmax ){
    printf("dn %i > dmax %i!!! setting dn = dmax", dn, dmax);
    dn = dmax;
  }
  
  CvSeq* contours = 0;
  cvCvtColor( g_image, g_gray, CV_BGR2GRAY );
  cvThreshold( g_gray, g_gray, 150, 255, CV_THRESH_BINARY );
  cvFindContours( g_gray, g_storage, &contours );
  cvZero( g_gray );

  if( contours ){

    /*
      cvDrawContours(
		   g_gray,
		   contours,
		   cvScalarAll(255),
		   cvScalarAll(100),
		   100 );
    */

    printf("dmin: %i\ndmax: %i\ndn: %i\ntheta: %i\n", dmin, dmax, dn, theta);

    cvDrawContours(
		   g_gray,
		   contours,
		   cvScalarAll(10),
		   cvScalarAll(10),
		   1 );

    cvFindDominantPoints( 
			 contours,
			 g_find_dom_points_storage,
			 CV_DOMINANT_IPAN,
			 dmin,
			 dmax,
			 dn,
			 theta
			  );

    cvDrawContours(
		   g_gray,
		   contours,
		   cvScalarAll(255),
		   cvScalarAll(255),
		   1 );
    
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

  /*
  cvNamedWindow( "Original", 1);
  cvShowImage( "Original", g_image );
  */

  cvNamedWindow( "Contours", 1 );
  cvCreateTrackbar( "dmin", "Contours", &dmin, 18, on_trackbar );
  cvCreateTrackbar( "dmax", "Contours", &dmax, 18, on_trackbar );
  cvCreateTrackbar( "dn", "Contours", &dn, 18, on_trackbar );
  cvCreateTrackbar( "theta", "Contours", &theta, 180, on_trackbar );
  on_trackbar(0);
  cvWaitKey();
  
  return 0;
  
}

