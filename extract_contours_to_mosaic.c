/*
Given an image file (and possibly a threshold?), extract all contours and write to an image file

./extract_contours image_file [1 = canny|2 = threshold|3 = adaptive]

writes out a single image file with all the contours written out into it

Problems: 
- OpenCV doesn't seem to support bounding boxes for Freeman Chain-type contours, which I think we'll need to use. Might need to compute bounding box manually? (Why doesn't OpenCV support this?)

*/

#define DEBUG 0
#define DEBUG_BBS 0
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
#include <cmath>

struct height_width_values { // just a struct to hold and x and y (this is currently mostly for get_max_x_y_for_contour_set)
  int height;
  int width;
  int count;
};

IplImage *CopySubImage(IplImage *full_image,int x_top_left, int y_top_left, int width, int height);
int write_image_file( IplImage *image_to_write, char *file_name );
height_width_values get_max_x_y_for_contour_set( CvSeq* contours );

IplImage*	image_original = NULL;
IplImage*	image_bw_copy = NULL;
IplImage*	image_manip = NULL;
int		threshold = 100; // only used for regular (not adaptive) thresholding
CvMemStorage* 	contour_storage = NULL;

int main(int argc, char** argv){
  
  if (argc != 3){
    printf("usage: program [image file] [1|2|3, 1 = canny|2 = threshold|3 = adaptive]\n");
    exit(-42);
  }
  
  const char* file_name = argv[1];
  if( ( image_original = cvLoadImage( file_name ) ) == 0 ) {
    fprintf(stderr, "Error reading template image file '%s'\n", file_name); 
    return(-1);
  }

  // man C sure makes these things painful
  int edge_method;
  char* edge_method_arg = argv[2];
  if( edge_method_arg[0] == '1' ){
    edge_method = 1;
    fprintf(stderr, "I'm going to use CANNY edge_method '%i'\n", edge_method); 
  }
  else if( edge_method_arg[0] == '2' ){
    edge_method = 2;
    fprintf(stderr, "I'm going to use THRESHOLD edge_method '%i'\n", edge_method); 
  }
  else if( edge_method_arg[0] == '3' ){
    edge_method = 3;
    fprintf(stderr, "I'm going to use ADAPTIVE THRESHOLD edge_method '%i'\n", edge_method); 
  }
  else {
    fprintf(stderr, "I don't know that edge_method\n");
    exit(-1);
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
  image_bw_copy = cvCreateImage( cvGetSize( image_original ), 8, 1 );
  contour_storage = cvCreateMemStorage(0);
  CvSeq* contours = NULL;
  
  // convert to grayscale
  cvCvtColor( image_original, image_manip, CV_BGR2GRAY );
  
  // make another b/w copy
  cvCvtColor( image_original, image_bw_copy, CV_BGR2GRAY );
  
  // make contour-ready image using thresholding or canny edge detection 

  if( edge_method == 1 ){ // CANNY 
    cvCanny( image_manip, image_manip, 50, 900, 3 ); // these seem to exclude a lot of crap
  }
  else if( edge_method == 2 ){ // THRESHOLD
    cvThreshold( image_manip, image_manip, threshold, 255, THRESHOLD_TYPE );
  }
  else if( edge_method == 3 ){
    cvAdaptiveThreshold(image_manip, image_manip, 255, ADAPTIVE_METHOD, THRESHOLD_TYPE, BLOCK_SIZE, SUBTRACT_PARAM);
  }
  else {
    fprintf(stderr, "edge_method = %i WTF?", edge_method);
    exit(-1);
  }
  
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

  //
  /* make new big image to write out all contours to */
  //
  // quick and dirty algorithm for sizing the big image: 
  // - make the each contour subimage x by x pixels, where x is the max of (width of the widest contour, length of the longest contour)
  // - then make the big image i columns by i rows of subimages, where i = ceil( sqrt( num of contours ) )

  height_width_values max_x_y = get_max_x_y_for_contour_set( contours );
  int subimage_dimension = fmax( max_x_y.width, max_x_y.height); // "x" above
  int num_rows_columns = ceil( sqrt( max_x_y.count )); // "i" above

  IplImage* mosaic_of_contours = cvCreateImage( 
					       cvSize(
						      (subimage_dimension * num_rows_columns), // width
						      (subimage_dimension * num_rows_columns) // height
						      ), 
					       8, 1);

  int n=0; 
  for( CvSeq* c=contours; c != NULL; c = c->h_next ) {

#if DEBUG
    printf("%i ", n);
    fflush(stdout);
#endif    

    if ( n == 520 ){
      printf("disco: %i ", n);
    }
    // set ROI in mosaic_of_contours
    // first, what col are we in? 
    int this_col = n % num_rows_columns;
    // now what row?
    int this_row = floor( n / num_rows_columns);

#if DEBUG
    printf("row: %i col: %i\n", this_row, this_col);
#endif

    // get contour image
    CvRect bbs;
    CvPoint pt_upper_left, pt_lower_right;
    bbs = cvBoundingRect(c);
    pt_lower_right.x = bbs.x;
    pt_lower_right.y = bbs.y;
    pt_upper_left.x = pt_lower_right.x + bbs.width;
    pt_upper_left.y = pt_lower_right.y + bbs.height;

    IplImage* extracted_contour;
    extracted_contour = CopySubImage( image_bw_copy, bbs.x, bbs.y, bbs.width, bbs.height);

    // now write to contour to mosaic
    CvRect ROIrect = cvRect(
			    this_col * subimage_dimension,
			    this_row * subimage_dimension,
			    extracted_contour->width, 
			    extracted_contour->height
			    );
    cvSetImageROI( mosaic_of_contours,  ROIrect);
    cvCopy( extracted_contour, mosaic_of_contours );
    cvResetImageROI( mosaic_of_contours );

    cvReleaseImage( &extracted_contour );
    /*
      cvRelease( &bbs );
      cvReleaseData( &pt_upper_left );
      cvReleaseData( &pt_upper_right );
      cvReleaseData( &ROIrect );
    */
    n++;
    continue;

  }

  char contour_file_name[(int) strlen(file_name)];
  sprintf (contour_file_name, "%s_method%i_contour.jpg", file_name, edge_method );
  printf("writing to %s...", contour_file_name);
  write_image_file(
		   mosaic_of_contours,
		   contour_file_name
		   );
  printf("done\n");
  
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

height_width_values get_max_x_y_for_contour_set( CvSeq* contours ){

  // get maximum x and y contour size, make then make image of size (# contours * max X length) by (# contours * max Y length)
  height_width_values max_height_width;
  max_height_width.width = 0;
  max_height_width.height = 0;
  max_height_width.count = 0;
  for( CvSeq* c=contours; c != NULL; c = c->h_next ) {
    max_height_width.count++;

    CvRect bounding_box;    
    bounding_box = cvBoundingRect(c);
    
    if ( bounding_box.width > max_height_width.width ){
      max_height_width.width = bounding_box.width;
#if DEBUG_BBS
      printf("new max width is %i\n", max_height_width.width);
#endif
    }

    if ( bounding_box.height > max_height_width.height ){
      max_height_width.height = bounding_box.height;
#if DEBUG_BBS
      printf("new max height is %i\n", max_height_width.height);
#endif
    }

  }

  return max_height_width;

}
