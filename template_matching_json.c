#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <stdio.h>

#define MAX_MATCHES 2000 // maximum number of hits to look for in target image
#define BLOT_PIXELS 5 // how many pixels around local max to blot out after noting local max

IplImage *src, *templ, *match_matrix, *src_marked_up;

double get_matrix_max( IplImage *match_matrix );
int mark_best_hits_by_threshold( IplImage *image, IplImage *match_matrix, double threshold );
int check_min_max( IplImage *match_matrix );

int main(int argc, char** argv){

  const char* templ_fn;
  const char* src_fn;
  float threshold;

  printf("{");

  if (argc != 4){
    printf("\"error\":\"wrong number of arguments (%d instead of 4) given on command line\"}", argc );
    exit(-1);
  }

  templ_fn = argv[1];
  src_fn = argv[2];
  threshold = atof(argv[3]);

  printf("\"image\":\"%s\",\"query_image\":\"%s\",", templ_fn, src_fn);

  //Read in the template to be used for matching:
  if((templ=cvLoadImage(templ_fn, CV_LOAD_IMAGE_GRAYSCALE ))== 0) {
    printf("\"error\":\"Error on reading image %s\"}", templ_fn ); 
    exit(-1);
  }

  //Read in the source image to be searched:
  if((src=cvLoadImage(src_fn, CV_LOAD_IMAGE_GRAYSCALE ))== 0) {
    printf("\"error\":\"Error on reading query image %s\"}", src_fn ); 
    exit(-1);
  }

  // make match matrix
  int matrix_width = src->width - templ->width + 1;
  int matrix_height = src->height - templ->height + 1;
  match_matrix = cvCreateImage( cvSize( matrix_width, matrix_height ), IPL_DEPTH_32F, 1 );

  cvMatchTemplate( src, templ, match_matrix, CV_TM_CCOEFF_NORMED);
  printf("\"matches\":[");
  mark_best_hits_by_threshold( src, match_matrix, threshold);
  printf("]}"); // close json

  cvReleaseImage(&src);
  cvReleaseImage(&templ);
  cvReleaseImage(&match_matrix);
  cvReleaseImage(&src_marked_up);

  return 1;

}

int mark_best_hits_by_threshold( IplImage *image, IplImage *match_matrix, double threshold ){

#if DEBUG
  fprintf(stderr, "in mark_best_hits_by_threshold");
  check_min_max( match_matrix );
#endif

  int add_to_x = image->width - match_matrix->width + 1;
  int add_to_y = image->height - match_matrix->height + 1;

  // prevent an infinite loop by breaking out after a crazy number of iterations (hacktastic), 
  // if for example there is a problem with zeroing out the max hits
  int count = 0; 

  do { // do loop once, then check if max value is gt threshold on each subsequent loop

#if DEBUG
    fprintf(stderr, "max of match_matrix is now %lf\n", get_matrix_max( match_matrix ) );
#endif

    if ( count > MAX_MATCHES - 1 ){
      printf("], \"error\": \"exceeded maximum hits (%d), I'm going to stop looking for more matches\"}", MAX_MATCHES ); 
      exit(-1);
      break;
    }
    count++;

    // find mix/max
    double minVal, maxVal;
    CvPoint minLoc, maxLoc;
    cvMinMaxLoc( match_matrix, &minVal, &maxVal, &minLoc, &maxLoc, NULL);

    // calculate upper left and lower right coord of match
    CvPoint pt_upper_left, pt_lower_right;
    pt_upper_left = maxLoc;
    pt_lower_right = maxLoc;
    pt_lower_right.x += add_to_x;
    pt_lower_right.y += add_to_y;

    printf("[%d,%d,%d,%d,%5.2f],",
	   pt_upper_left.x, pt_upper_left.y, pt_lower_right.x, pt_lower_right.y, get_matrix_max( match_matrix ));
    
    // now zero out current max
    // blot out square around peak too
    CvScalar s; // so strange
#if DEBUG
    s=cvGet2D(match_matrix, maxLoc.y, maxLoc.x);
    fprintf(stderr, "resetting element %i, %i, which is %f\n", maxLoc.x, maxLoc.y, s.val[0] );
#endif
    s.val[0]=0.0;
    s.val[1]=0.0;
    s.val[2]=0.0;

    int i, j;
    for ( i = - (BLOT_PIXELS); i < BLOT_PIXELS; i++){
      for ( j = - (BLOT_PIXELS); j < BLOT_PIXELS; j++){

#if DEBUG
	fprintf(stderr, "counters %i, %i\n", i, j );
	fprintf(stderr, "col %i row %i\n", (maxLoc.y + i),  (maxLoc.x + j)  );
	fprintf(stderr, "(height %i, width %i\n", match_matrix->height, match_matrix->width);
#endif
	if ( (maxLoc.y + i) < 0 || (maxLoc.x + j) < 0 || 
	     (maxLoc.y + i) > match_matrix->height - 1 || (maxLoc.x + j) > match_matrix->width - 1
	     ){ // out of range
	}
	else {
	  cvSet2D(match_matrix, maxLoc.y + i, maxLoc.x + j, s );
	}
      }
    }

  } while ( get_matrix_max( match_matrix ) > threshold );

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

int check_min_max( IplImage *match_matrix ){ // method just to see what kinds of values we get

  double minVal, maxVal;
  CvPoint minLoc, maxLoc;
  
  cvMinMaxLoc( match_matrix,
	       &minVal,
	       &maxVal,
	       &minLoc,
	       &maxLoc,
	       NULL);
  
  fprintf(stderr, "found min (%5.2f ) at %i, %i and max (%5.2f ) at %i, %i\n", 
	 minVal, (int) minLoc.x, (int) minLoc.y,
	 maxVal, (int) maxLoc.x, (int) maxLoc.y
	 );

  fflush(stdout);
  return 1;
  
}

