#!/bin/bash -f 

# Change the following as appropriate
export EXTRACT_CONTOURS_PROGRAM="../extract_contours"
export IMAGE_DIR="/Users/jtr4v/Desktop/badimages/" # substitute your image directory
export IMAGE_FILE_REGEX="*jpg" # not case sensitive, using find -iname 

# rubber -> road
echo "### Starting extract contours test: " `date`

# find all the files
echo Found `find $IMAGE_DIR -iname "$IMAGE_FILE_REGEX" | grep -c ''` image files

# loop through each file and call program
for this_image_file in `find $IMAGE_DIR -iname "*$IMAGE_FILE_REGEX"`
do
  time $EXTRACT_CONTOURS_PROGRAM $this_image_file
done

echo "### Done with extract contours test: " `date`