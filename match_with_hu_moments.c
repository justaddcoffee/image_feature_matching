#include <cv.h>
#include <highgui.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////
/* given two images, compare using Hu moments and print out */
//////////////////////////////////////////////////////////////

IplImage *image1, *image2;

int main(int argc, char** argv){

  const char* image1_fn;
  const char* image2_fn;
  char* method_arg;

  if (argc < 3){
    printf("error: wrong number of arguments (%d instead of 3 or 4)\n", argc );
    exit(-1);
  }

  image1_fn = argv[1];
  image2_fn = argv[2];
  if (argc > 3){
    method_arg = argv[3];
    if (method_arg[0] != '1' && method_arg[0] != '2' && method_arg[0] != '3'){
      printf("error: third argument (%s) should be 1, 2 or 3\n", method_arg );
      exit(-1);
    }
  }

  //printf("image1: %s\nimage2: %s\n", image1_fn, image2_fn);

  //Read in the image2ate to be used for matching:
  if((image1=cvLoadImage(image1_fn, CV_LOAD_IMAGE_GRAYSCALE ))== 0) {
    printf("\"error\":\"Error on reading image1 %s\"}", image1_fn ); 
    exit(-1);
  }

  //Read in the source image to be searched:
  if((image2=cvLoadImage(image2_fn, CV_LOAD_IMAGE_GRAYSCALE ))== 0) {
    printf("\"error\":\"Error on reading image2 %s\"}", image2_fn ); 
    exit(-1);
  }

  double foo;

  if (argc < 4 || method_arg[0] == '1'){
    foo = cvMatchShapes( 
			image1,
			image2,
			CV_CONTOURS_MATCH_I1,
			0
			 );
  }
  else if ( method_arg[0] == '2'){
    foo = cvMatchShapes( 
			image1,
			image2,
			CV_CONTOURS_MATCH_I2,
			0
			 );
  }
  else if ( method_arg[0] == '3'){
    foo = cvMatchShapes( 
			image1,
			image2,
			CV_CONTOURS_MATCH_I3,
			0
			 );
  }

  printf("Match value: %f\n", foo);

  cvReleaseImage(&image1);
  cvReleaseImage(&image2);
  return 1;

}

