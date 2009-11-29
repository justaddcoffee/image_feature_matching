#define DEBUG 0
#define DISPLAY_IMAGES 1
#define WRITE_OUT_IMAGE 1
#define DO_CANNY_EDGE_DETECTION 1
#define THRESHOLD_VAL 0.3

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <stdio.h>

int display_image( IplImage *image, char *winname ){

  cvNamedWindow( winname, 1);
  cvShowImage( winname, image );

  return 1;

}

int write_image_file( IplImage *image_to_write, char *file_name ){

  if(cvSaveImage( file_name, image_to_write, NULL )){
    return 1;
  } else{
    printf("Could not save: %s\n", file_name);
    return 0;
  }

}

int check_min_max( IplImage *match_matrix ){ // method just to see what kinds of values we get

  double minVal, maxVal;
  CvPoint minLoc, maxLoc;
  
  cvMinMaxLoc( match_matrix,
	       &minVal,
	       &maxVal,
	       &minLoc,
	       &maxLoc,
	       NULL);
  
  printf("found min (%5.2f ) at %i, %i and max (%5.2f ) at %i, %i\n", 
	 minVal, (int) minLoc.x, (int) minLoc.y,
	 maxVal, (int) maxLoc.x, (int) maxLoc.y
	 );

  fflush(stdout);
  return 1;
  
}

double get_matrix_max( IplImage *match_matrix ){ 

  double minVal, maxVal;
  CvPoint minLoc, maxLoc;
  
  cvMinMaxLoc( match_matrix,
	       &minVal,
	       &maxVal,
	       &minLoc,
	       &maxLoc,
	       NULL);
  
  return maxVal;
  
}

int mark_best_hit( IplImage *image, IplImage *match_matrix ){

#if DEBUG
  printf("in mark_best_hit: ");
  check_min_max( match_matrix );
#endif

  int add_to_x = image->width - match_matrix->width + 1;
  int add_to_y = image->height - match_matrix->height + 1;

  double minVal, maxVal;
  CvPoint minLoc, maxLoc;

  cvMinMaxLoc( match_matrix, &minVal, &maxVal, &minLoc, &maxLoc, NULL);

  CvPoint pt_upper_left, pt_lower_right;

#if DEBUG
  printf("max is x = %i, y = %i\n", (int) maxLoc.x, (int) maxLoc.y);
  fflush(stdout);
#endif 

  pt_upper_left = maxLoc;
  pt_lower_right = maxLoc;
  pt_lower_right.x += add_to_x;
  pt_lower_right.y += add_to_y;
  
  //  void cvRectangle(CvArr* img, CvPoint pt1, CvPoint pt2, CvScalar color, int thickness=1, int line_type=8, int shift=0
  cvRectangle( image,
	       pt_upper_left, 
	       pt_lower_right, 
	       CV_RGB(255,50,50), 
	       1,
	       8,
	       0
	       );

  return 1;

}

int mark_best_hits_by_threshold( IplImage *image, IplImage *match_matrix, double threshold ){

#if DEBUG
  printf("in mark_best_hits_by_threshold");
  check_min_max( match_matrix );
#endif

  int add_to_x = image->width - match_matrix->width + 1;
  int add_to_y = image->height - match_matrix->height + 1;

  // prevent an infinite loop by breaking out after a crazy number of iterations (hacktastic), 
  // if for example there is a problem with zeroing out the max hits
  const int max_count = 3000; 
  int count = 0; 

  do { // do loop once, then check if max value is gt threshold on each subsequent loop

#if DEBUG
    printf("max of match_matrix is now %lf\n", get_matrix_max( match_matrix ) );
#endif

    if ( count > max_count ){
      printf("crazy number of iterations (%i), I'm going to stop looking for more max values\n", count);
      break;
    }
    count++;

    // write rectangle around new max
    double minVal, maxVal;
    CvPoint minLoc, maxLoc;
    cvMinMaxLoc( match_matrix, &minVal, &maxVal, &minLoc, &maxLoc, NULL);

    CvPoint pt_upper_left, pt_lower_right;
    pt_upper_left = maxLoc;
    pt_lower_right = maxLoc;
    pt_lower_right.x += add_to_x;
    pt_lower_right.y += add_to_y;
    
    //  void cvRectangle(CvArr* img, CvPoint pt1, CvPoint pt2, CvScalar color, int thickness=1, int line_type=8, int shift=0
    cvRectangle( image,
		 pt_upper_left, 
		 pt_lower_right, 
		 CV_RGB(255,50,50), 
		 2,
		 8,
		 0
		 );

    // now zero out current max
    // blot out square around peak too
    CvScalar s; // so strange
#if DEBUG
    s=cvGet2D(match_matrix, maxLoc.y, maxLoc.x);
    printf("resetting element %i, %i, which is %f\n", maxLoc.x, maxLoc.y, s.val[0] );
#endif
    s.val[0]=0.0;
    s.val[1]=0.0;
    s.val[2]=0.0;

    int blot_square_size = 5; 
    int i, j;
    for ( i = - (blot_square_size); i < blot_square_size; i++){
      for ( j = - (blot_square_size); j < blot_square_size; j++){

#if DEBUG
	printf("counters %i, %i\n", i, j );
	printf("col %i row %i\n", (maxLoc.y + i),  (maxLoc.x + j)  );
	printf("(height %i, width %i\n", match_matrix->height, match_matrix->width);
#endif
	if ( (maxLoc.y + i) < 0 || (maxLoc.x + j) < 0 || 
	     (maxLoc.y + i) > match_matrix->height - 1 || (maxLoc.x + j) > match_matrix->width - 1
	     ){ // out of range
	}
	else {
	  cvSet2D(match_matrix, maxLoc.y + i, maxLoc.x + j, s );
	  //((uchar *)(match_matrix->imageData + (maxLoc.x + j) * match_matrix->widthStep))[(maxLoc.y + i)] = 0;
	}
      }
    }

  } while ( get_matrix_max( match_matrix ) > threshold );

  return 1;
    
}

int main(int argc, char** argv){

  if (argc != 3){
    printf("usage: program [template image file] [image file]\n");
    exit(-42);
  }

  const char* templ_fn = argv[1];
  const char* src_fn = argv[2];

  IplImage *src, *templ, *match_matrix, *src_marked_up; 
  // src is source image, templ is template image, match_matrix is a matrix corresponding to how well templ matches src at any given pixel, 
  // src_marked_up is the src image, marked up with hits
  int i;
  
  //Read in the template to be used for matching:
  if((templ=cvLoadImage(templ_fn, CV_LOAD_IMAGE_GRAYSCALE ))== 0) {
    printf("Error on reading src image %s\n","templ"); 
    return(-1);
  }

  //Read in the source image to be searched:
  if((src=cvLoadImage(src_fn, CV_LOAD_IMAGE_GRAYSCALE ))== 0) {
    printf("Error reading template image file '%s'\n",src_fn); 
    return(-1);
  }

  // load up another copy of src that we can mark up
  if((src_marked_up=cvLoadImage(src_fn, CV_LOAD_IMAGE_COLOR ))== 0) {
    printf("Error reading template image file '%s' into marked up output file (iteration %i)\n",src_fn, i); 
    return(-1);
  }

  //////////////////////////////////////////////
  // Allocate output matrices and src_marked_up
  //////////////////////////////////////////////
  int matrix_width = src->width - templ->width + 1;
  int matrix_height = src->height - templ->height + 1;
#if DEBUG
  printf("dimensions for output matrix:\nmatrix_width: %i\nmatrix_height: %i\n", matrix_width, matrix_height);
#endif

  // reality check of dimensions
  if ( matrix_width < 0 || matrix_height < 0){
    printf("one or both dimensions for output matrix are negative - did you reverse the template and source image files?\n");
  }

  //match_matrix = cvCreateImage( cvSize( matrix_width, matrix_height ), 32, 1 );
  match_matrix = cvCreateImage( cvSize( matrix_width, matrix_height ), IPL_DEPTH_32F, 1 );

  /////////////////////////////////////  /////////////////////////////////////  
  // Do the matching of the template with the image
  /////////////////////////////////////  /////////////////////////////////////
  
  //////////////////////////////////////////
  // Canny edge detection of src and templ
  //////////////////////////////////////////
  //cvCanny( fling, wibbit, 50, 200, 3 ); // default parameters, produces a lot of noise
  //cvCanny( fling, wibbit, 50, 900, 3 ); // these seem to exclude a lot of crap

#if DO_CANNY_EDGE_DETECTION
  cvCanny( src, src, 50, 200, 3 );
  cvCanny( templ, templ, 50, 200, 3 );
#endif
  
  //////////////////////////////////////////
  // Do the actual matching
  //////////////////////////////////////////

  // maybe try extracting contours here and matching them instead

  //cvMatchTemplate( src, templ, match_matrix, CV_TM_SQDIFF);  // signal is inverted, darker is better hit
  //cvMatchTemplate( src, templ, match_matrix, CV_TM_SQDIFF_NORMED); // signal is inverted, darker is better hit
  //cvMatchTemplate( src, templ, match_matrix, CV_TM_CCORR); // signal is washed out with false positives on both sides
  //cvMatchTemplate( src, templ, match_matrix, CV_TM_CCORR_NORMED);
  //cvMatchTemplate( src, templ, match_matrix, CV_TM_CCOEFF);
  cvMatchTemplate( src, templ, match_matrix, CV_TM_CCOEFF_NORMED);

#if DEBUG
  display_image( match_matrix, "match_matrix (unnormalized)");
#endif 

  cvNormalize( match_matrix, match_matrix, 1, 0, CV_MINMAX, NULL );

#if DEBUG  
  // see what kinds of values we get
  printf("checking match_matrix\n");
  check_min_max( match_matrix );
#endif
  
  //mark_best_hit( src_marked_up, match_matrix );
  mark_best_hits_by_threshold( src_marked_up, match_matrix, THRESHOLD_VAL );
  
  // Display results

#if DISPLAY_IMAGES
  display_image( templ, "templ");
  display_image( src, "src");
  display_image( match_matrix, "test_matrix");
  display_image( src_marked_up, "src_marked_up");
  cvWaitKey(0);
#endif
  
  char output_winname[(int) strlen(src_fn)];
  sprintf (output_winname, "%s_marked_up.jpg", src_fn );

#if WRITE_OUT_IMAGE
  write_image_file( src_marked_up, output_winname );
#endif

  cvReleaseImage(&src);
  cvReleaseImage(&templ);
  cvReleaseImage(&match_matrix);
  cvReleaseImage(&src_marked_up);

  return 0;
}
