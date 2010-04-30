#include <cv.h>
#include <highgui.h>
#include <stdio.h>

#define THRESHOLD_TYPE CV_THRESH_BINARY

IplImage*	image_original = NULL;
//contour_storage = cvCreateMemStorage(0);
CvSeq* contours = NULL;
int		threshold = 10;

int main(int argc, char** argv){

  CvMemStorage* 	contour_storage = NULL;
  contour_storage = cvCreateMemStorage(0);

  if (argc != 2){
    printf("usage: program [image file]\n");
    exit(-42);
  }
  
  const char* file_name = argv[1];
  if( ( image_original = cvLoadImage( file_name, 0 ) ) == 0 ) {
    fprintf(stderr, "Error reading template image file '%s'\n", file_name); 
    return(-1);
  }

  cvNamedWindow( "Original", 1);
  cvShowImage( "Original", image_original );
  cvWaitKey(1000);  

  cvThreshold( image_original, image_original, threshold, 255, THRESHOLD_TYPE );
  cvShowImage( "Original", image_original );
  cvWaitKey(2000);  

  int numContoursFound;
  numContoursFound = cvFindContours( 
				    image_original, 
				    contour_storage, 
				    &contours,
				    sizeof(CvContour),
				    CV_RETR_LIST,
				    CV_CHAIN_APPROX_SIMPLE //CV_CHAIN_APPROX_SIMPLE //CV_CHAIN_CODE
				     );

  fprintf(stderr, "Found %i contours in image file '%s'\n", numContoursFound, file_name); 

  int i = 0;

  for( CvSeq* c=contours; c != NULL; c = c->h_next ) {

    double length = 0;
    length = cvArcLength( 
			 c,
			 CV_WHOLE_SEQ, 
			 -1
			  );

    fprintf(stderr, "  contour %i length: %f\n", i + 1, length);
    i++;

  }

}
