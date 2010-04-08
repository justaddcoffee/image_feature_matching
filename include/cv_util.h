struct height_width_values { // just a struct to hold and x and y (this is currently mostly for get_max_x_y_for_contour_set)
  int height;
  int width;
  int count;
};


IplImage *CopySubImage(IplImage *full_image,int x_top_left, int y_top_left, int width, int height);
int write_image_file( IplImage *image_to_write, char *file_name );
height_width_values get_max_x_y_for_contour_set( CvSeq* contours );

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

