#!/bin/bash -f

for FILE in `ls images/`
do
    for METHOD in 1 2 3
    do
	../extract_contours_to_mosaic images/$FILE $METHOD
    done
done
