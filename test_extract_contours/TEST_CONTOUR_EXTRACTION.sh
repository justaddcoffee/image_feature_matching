#!/bin/bash -f 

# Change the following as appropriate
export EXTRACT_CONTOURS_PROGRAM="../extract_contours" # substitute name of JM's C blobbing program
export IMAGE_DIR="/Users/jtr4v/Desktop/badimages/" # substitute your image directory
export IMAGE_FILE_REGEX="*jpg" # not case sensitive, using find -iname 

# rubber -> road
echo "### Starting blob test: " `date`

# find all the files
echo Found `find $IMAGE_DIR -iname "$IMAGE_FILE_REGEX" | grep -c ''` image files
set export FILES=`find $IMAGE_DIR -iname "$IMAGE_FILE_REGEX"`

# loop through each file and call JM's blobbing program
for this_image_file in $FILES
do
  time $EXTRACT_CONTOURS_PROGRAM $this_image_file
done

echo "### Done with blob test: " `date`