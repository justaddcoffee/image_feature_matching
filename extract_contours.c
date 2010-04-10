/*
Given an image file (and possibly a threshold?), extract all contours and write to an image file

./extract_contours image_file 1|2|3

writes out 
   image_file_contours_1
   image_file_contours_2
   image_file_contours_3

Problems: 
- OpenCV doesn't seem to support bounding boxes for Freeman Chain-type contours, which I think we'll need to use. Might need to compute bounding box manually? (Why doesn't OpenCV support this?)

*/

#define DEBUG 0
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
#include <highgui.h>
#include <stdio.h>

IplImage *CopySubImage(IplImage *full_image,int x_top_left, int y_top_left, int width, int height);
int write_image_file( IplImage *image_to_write, char *file_name );

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
    fprintf(stderr, "Error reading template image file '%s'\n", file_name); 
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
  CvSeq* contours = NULL;
  
  // convert to grayscale
  cvCvtColor( image_original, image_manip, CV_BGR2GRAY );
  
  // make contour-ready image using thresholding or canny edge detection 
  //cvThreshold( image_manip, image_manip, threshold, 255, THRESHOLD_TYPE );
  //cvAdaptiveThreshold(image_manip, image_manip, 255, ADAPTIVE_METHOD, THRESHOLD_TYPE, BLOCK_SIZE, SUBTRACT_PARAM);
  cvCanny( image_manip, image_manip, 50, 900, 3 ); // these seem to exclude a lot of crap
  
#if DEBUG
  cvNamedWindow( "Thresholded", 1);
  cvShowImage( "Thresholded", image_manip );
#endif
    
  int numContoursFound = 0;
  //cvFindContours( src, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
  numContoursFound = cvFindContours( 
				 image_manip, 
				 contour_storage, 
				 &contours,
				 sizeof(CvContour),
				 CV_RETR_CCOMP,
				 CV_CHAIN_APPROX_SIMPLE //CV_CHAIN_APPROX_SIMPLE //CV_CHAIN_CODE
				  );

  fprintf(stderr, "Found %i contours in image file '%s'\n", numContoursFound, file_name); 

  int n=0; 
  for( CvSeq* c=contours; c != NULL; c = c->h_next ) {

#if DEBUG
      cvDrawContours(
		   image_original, 
		   c, 
		   CV_RGB(255,50,50), // cvScalarAll(0),
		   CV_RGB(255,50,50), //cvScalarAll(255),
		   0, // how deep into contour tree to print
		   1.5,
		   1
		   ); 
#endif

    // get box around contour
    CvRect bbs;
    CvPoint pt_upper_left, pt_lower_right;
    bbs = cvBoundingRect(c);
    pt_lower_right.x = bbs.x;
    pt_lower_right.y = bbs.y;
    pt_upper_left.x = pt_lower_right.x + bbs.width;
    pt_upper_left.y = pt_lower_right.y + bbs.height;

#if DEBUG
    cvRectangle( 
		image_original,
		pt_upper_left, 
		pt_lower_right, 
		CV_RGB(255,50,50), 
		1,
		8,
		0
		 );
#endif

    IplImage* extracted_contour;
    //CopySubImage(IplImage *full_image,int x_top_left, int y_top_left, int width, int height);
    extracted_contour = CopySubImage( image_original, bbs.x, bbs.y, bbs.width, bbs.height);

    char contour_file_name[(int) strlen(file_name)];
    //char output_winname[(int) strlen(file)];
    sprintf (contour_file_name, "%s_contour_%i.jpg", file_name, n );
    write_image_file(
		     extracted_contour,
		     contour_file_name
		     );

#if DEBUG
    printf("Contour number %d\n", n );
    cvShowImage( "Image", image_original );
    printf(" %d elements:\n", c->total );
#endif

    n++;
  }

#if DEBUG
    cvShowImage( "Contours", image_original );
    cvWaitKey();  
#endif

  return 0;

}

IplImage *CopySubImage(IplImage *full_image,int x_top_left, int y_top_left, int width, int height) {

  CvRect roi;
  IplImage *sub_image;
  
  roi.x = x_top_left;
  roi.y = y_top_left;
  roi.width = width;
  roi.height = height;
  
  cvSetImageROI(full_image,roi);
  sub_image = cvCreateImage( cvSize(roi.width, roi.height),
			    full_image->depth, full_image->nChannels );
  cvCopy(full_image,sub_image);
  cvResetImageROI(full_image);
  return sub_image;

}
 
int write_image_file( IplImage *image_to_write, char *file_name ){

  if(cvSaveImage( file_name, image_to_write, NULL )){
    return 1;
  } else{
    printf("Could not save: %s\n", file_name);
    return 0;
  }

}
