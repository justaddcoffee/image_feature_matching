/*
Given an image file (and possibly a threshold?), extract all contours and write out an image file
containing a comparison 
    ./compare_contours image_file

Problems: 
- Dies when pictures has more than 3-4 contours, gotta sort this out
- OpenCV doesn't seem to support bounding boxes for Freeman Chain-type contours, which I think we'll need to use. Might need to compute bounding box manually? (Why doesn't OpenCV support this?)
*/

#define DEBUG 1
#define DEBUG_BBS 0
#define THRESHOLD_TYPE CV_THRESH_BINARY
// CV_THRESH_BINARY or CV_THRESH_BINARY_INV

// maximum number of rows contour pairs and scores to show, so we don't end up with an image with 8 gagillion rows
#define MAX_CONTOUR_COMPARISONS 500 

// these are minimum sizes (for both length and width)
#define MIN_CONTOUR_PIXEL_SIZE 3
#define MAX_CONTOUR_PIXEL_SIZE 60

// these are just to make the results more managable, not probably for use in final algorithm
#define MIN_QUALITY_CUTOFF 0.0000001 // just > 0 so we don't get contours compared with themselves
#define MAX_QUALITY_CUTOFF 0.09

#define MAX_TRIES_FOR_ONE_CONTOUR 300

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
#include <cmath>
#include <cv_util.h>

IplImage*	image_original = NULL;
IplImage*	image_bw_copy = NULL;
IplImage*	image_bw_copy2 = NULL;
IplImage*	image_manip = NULL;
CvMemStorage* 	contour_storage1 = NULL;
CvMemStorage* 	contour_storage2 = NULL;

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

  int edge_method = 1;
  fprintf(stderr, "I'm going to use THRESHOLD edge_method '%i'\n", edge_method); 

#if DEBUG
  //cvNamedWindow( "Original", 1);
  //cvShowImage( "Original", image_original );
  //cvNamedWindow( "Contours", 1 );
  //cvWaitKey(); 
#endif

  int threshold = 60; // only used for regular (not adaptive) thresholding
  
  ////////////////////////////
  // start manipulating image
  ////////////////////////////
  image_manip = cvCreateImage( cvGetSize( image_original ), 8, 1 );
  // bw copy is what we'll extract contours from
  image_bw_copy = cvCreateImage( cvGetSize( image_original ), 8, 1 ); 

  contour_storage1 = cvCreateMemStorage(0);
  contour_storage2 = cvCreateMemStorage(0);
  CvSeq* contours1 = NULL;
  CvSeq* contours2 = NULL;
  
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
  else if( edge_method == 3 ){ // ADAPTIVE THRESHOLD
    cvAdaptiveThreshold(image_manip, image_manip, 255, ADAPTIVE_METHOD, THRESHOLD_TYPE, BLOCK_SIZE, SUBTRACT_PARAM);
  }
  else {
    fprintf(stderr, "edge_method = %i WTF?", edge_method);
    exit(-1);
  }
    
#if DEBUG
  //cvNamedWindow( "Thresholded", 1);
  //cvShowImage( "Thresholded", image_manip );
  //cvWaitKey(); 
#endif
  
  int numContoursFound = 0;
  numContoursFound = cvFindContours( 
				    image_manip, 
				    contour_storage1, 
				    &contours1,
				    sizeof(CvContour),
				    CV_RETR_CCOMP,
				    CV_CHAIN_APPROX_SIMPLE //CV_CHAIN_APPROX_SIMPLE //CV_CHAIN_CODE
				     );
  
  fprintf(stderr, "Found %i contours in image file '%s'\n", numContoursFound, file_name); 
  
  //
  /* make new big image to write out all contours to */
  //
  // - let the space in which we print each contour be X by X pixels, where X is the max of (width of the widest contour, length of the longest contour)
  // - then make the big image 4 * X wide and X^2 long

  height_width_values max_x_y = get_max_x_y_for_contour_set( contours1, MIN_CONTOUR_PIXEL_SIZE, MAX_CONTOUR_PIXEL_SIZE  );
  int subimage_dimension = fmax( max_x_y.width, max_x_y.height); // "x" above

  int num_rows = ( max_x_y.count * max_x_y.count );
  if ( num_rows > MAX_CONTOUR_COMPARISONS ){
    num_rows = MAX_CONTOUR_COMPARISONS;
  }

  printf("num usable contours (max_x_y.count): %i\n", max_x_y.count);
  printf("num_rows: %i\n", num_rows);
  
  IplImage* mosaic_of_contours = cvCreateImage( 
					       cvSize(
						      // width 4 places - one place for first contour, one for second contour and two for a score, to leave enough room
						      (subimage_dimension * 4), 
						      // height
						      (subimage_dimension * num_rows ) 
						      ), 
					       8, 1);
  cvZero( mosaic_of_contours );
  
  int this_row=0; 
  for( CvSeq* c=contours1; c != NULL; c = c->h_next ) {
    
#if DEBUG
    fflush(stdout);
    printf("row: %i\n", this_row);
#endif
    
    // get contour image
    CvRect bbs;
    CvPoint pt_upper_left, pt_lower_right;
    bbs = cvBoundingRect(c);
    pt_lower_right.x = bbs.x;
    pt_lower_right.y = bbs.y;
    pt_upper_left.x = pt_lower_right.x + bbs.width;
    pt_upper_left.y = pt_lower_right.y + bbs.height;
    
    if  (bbs.width < MIN_CONTOUR_PIXEL_SIZE || bbs.height < MIN_CONTOUR_PIXEL_SIZE || bbs.width > MAX_CONTOUR_PIXEL_SIZE || bbs.height > MAX_CONTOUR_PIXEL_SIZE // too big or too small
	 ){
      //printf("rejected contour of size %i by %i\n", bbs.width, bbs.height);
      continue;
    }
    
    IplImage* extracted_contour;
    extracted_contour = CopySubImage( image_bw_copy, bbs.x, bbs.y, bbs.width, bbs.height);
    
    // now write to contour1 to mosaic
    CvRect ROIrect = cvRect(
			    0,
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

    // now get contours again, and compare each to contour1
    cvFindContours( 
		   image_manip, 
		   contour_storage2, 
		   &contours2,
		   sizeof(CvContour),
		   CV_RETR_CCOMP,
		   CV_CHAIN_APPROX_SIMPLE //CV_CHAIN_APPROX_SIMPLE //CV_CHAIN_CODE
		    );

    for( CvSeq* d=contours2; d != NULL; d = d->h_next ) {

      // get contour image
      CvRect bbs2;
      CvPoint pt_upper_left2, pt_lower_right2;
      bbs2 = cvBoundingRect(d);
      pt_lower_right2.x = bbs2.x;
      pt_lower_right2.y = bbs2.y;
      pt_upper_left2.x = pt_lower_right2.x + bbs2.width;
      pt_upper_left2.y = pt_lower_right2.y + bbs2.height;

      // skip contours that are too big or too small
      if  (bbs2.width < MIN_CONTOUR_PIXEL_SIZE || bbs2.height < MIN_CONTOUR_PIXEL_SIZE || bbs2.width > MAX_CONTOUR_PIXEL_SIZE || bbs2.height > MAX_CONTOUR_PIXEL_SIZE // too big or too small
	   ){
	//printf("rejected contour of size %i by %i\n", bbs2.width, bbs2.height);
	continue;
      }

      IplImage* extracted_contour2;
      extracted_contour2 = CopySubImage( image_bw_copy, bbs2.x, bbs2.y, bbs2.width, bbs2.height);

      double match_val = pghMatchShapes(c, d);
      // skip comparison if it's too bad 
      if ( match_val >  MAX_QUALITY_CUTOFF || match_val < MIN_QUALITY_CUTOFF ){
	continue;
      }

      // now write to contour2 to mosaic
      CvRect ROIrect = cvRect(
			      subimage_dimension,
			      this_row * subimage_dimension,
			      extracted_contour2->width,
			      extracted_contour2->height
			      );
      cvSetImageROI( mosaic_of_contours,  ROIrect);
      cvCopy( extracted_contour2, mosaic_of_contours );
      cvResetImageROI( mosaic_of_contours );
      cvReleaseImage( &extracted_contour2 );

      /* write out score too */
      char match_val_string[10];
      sprintf (match_val_string, "%f", match_val);

      CvFont font;
      cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.3, 0.3, 0, 1, CV_AA);
      cvPutText(mosaic_of_contours, match_val_string, cvPoint(2 * subimage_dimension + 2, this_row * subimage_dimension + 10), &font, cvScalar(255, 255, 255, 0));

#if DEBUG
      fflush(stdout);
      printf("\t(inner loop) row: %i\n", this_row);
#endif

      this_row++;
      if ( this_row > MAX_CONTOUR_COMPARISONS - 1){
	break;
      }

     }

    if ( this_row > MAX_CONTOUR_COMPARISONS - 1){
      break;
    }

  }

  // write out image
  char contour_file_name[(int) strlen(file_name)];
  sprintf (contour_file_name, "%s_method%i_contour_%i.jpg", file_name, edge_method, threshold );
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

