CC= g++-4.2 -march=x86-64 # or gcc-4.0
LDFLAGS=-ggdb -Wall $(shell pkg-config --libs opencv)

SOURCES_TEMPLATE_MATCHING=template_matching.c
SOURCES_TEMPLATE_MATCHING_JSON=template_matching_json.c
SOURCES_TEMPLATE_MATCHING_WTB=template_matching_with_trackbar.c
SOURCES_EXTRACT_CONTOURS=extract_contours.c
SOURCES_EXTRACT_CONTOURS_TO_MOSAIC=extract_contours_to_mosaic.c
SOURCES_COMPARE_CONTOURS=compare_contours.c

OBJECTS_TEMPLATE_MATCHING=$(SOURCES_TEMPLATE_MATCHING:.cpp=.o)
OBJECTS_TEMPLATE_MATCHING_JSON=$(SOURCES_TEMPLATE_MATCHING_JSON:.cpp=.o)
OBJECTS_TEMPLATE_MATCHING_WTB=$(SOURCES_TEMPLATE_MATCHING_WTB:.cpp=.o)
OBJECTS_EXTRACT_CONTOURS=$(SOURCES_EXTRACT_CONTOURS:.cpp=.o)
OBJECTS_EXTRACT_CONTOURS_TO_MOSAIC=$(SOURCES_EXTRACT_CONTOURS_TO_MOSAIC:.cpp=.o)
OBJECTS_COMPARE_CONTOURS=$(SOURCES_COMPARE_CONTOURS:.cpp=.o)

EXECUTABLE_TEMPLATE_MATCHING=template_matching
EXECUTABLE_TEMPLATE_MATCHING_JSON=template_matching_json
EXECUTABLE_TEMPLATE_MATCHING_WTB=template_matching_with_trackbar
EXECUTABLE_EXTRACT_CONTOURS=extract_contours
EXECUTABLE_EXTRACT_CONTOURS_TO_MOSAIC=extract_contours_to_mosaic
EXECUTABLE_COMPARE_CONTOURS=compare_contours

all: $(SOURCES_TEMPLATE_MATCHING_JSON) $(EXECUTABLE_TEMPLATE_MATCHING_JSON) $(SOURCES_TEMPLATE_MATCHING) $(EXECUTABLE_TEMPLATE_MATCHING) $(SOURCES_TEMPLATE_MATCHING_WTB) $(EXECUTABLE_TEMPLATE_MATCHING_WTB) $(SOURCES_EXTRACT_CONTOURS) $(EXECUTABLE_EXTRACT_CONTOURS) $(SOURCES_EXTRACT_CONTOURS_TO_MOSAIC) $(EXECUTABLE_EXTRACT_CONTOURS_TO_MOSAIC) $(SOURCES_COMPARE_CONTOURS) $(EXECUTABLE_COMPARE_CONTOURS) 

$(EXECUTABLE_TEMPLATE_MATCHING):
	$(CC) $(LDFLAGS) $(OBJECTS_TEMPLATE_MATCHING) -o $@

$(EXECUTABLE_TEMPLATE_MATCHING_JSON):
	$(CC) $(LDFLAGS) $(OBJECTS_TEMPLATE_MATCHING_JSON) -o $@

$(EXECUTABLE_TEMPLATE_MATCHING_WTB):
	$(CC) $(LDFLAGS) $(OBJECTS_TEMPLATE_MATCHING_WTB) -o $@

$(EXECUTABLE_EXTRACT_CONTOURS):
	$(CC) $(LDFLAGS) $(OBJECTS_EXTRACT_CONTOURS) -o $@

$(EXECUTABLE_EXTRACT_CONTOURS_TO_MOSAIC):
	$(CC) $(LDFLAGS) $(OBJECTS_EXTRACT_CONTOURS_TO_MOSAIC) -o $@

$(EXECUTABLE_COMPARE_CONTOURS):
	$(CC) $(LDFLAGS) $(OBJECTS_COMPARE_CONTOURS) -o $@

clean:
	rm -rf *.dSYM *o $(EXECUTABLE_TEMPLATE_MATCHING) $(EXECUTABLE_TEMPLATE_MATCHING_WTB) $(EXECUTABLE_TEMPLATE_MATCHING_JSON) $(EXECUTABLE_EXTRACT_CONTOURS) $(EXECUTABLE_EXTRACT_CONTOURS_TO_MOSAIC) $(EXECUTABLE_COMPARE_CONTOURS) 

test:
	./$(EXECUTABLE_TEMPLATE_MATCHING) test_images/palm_7R_1_4.jpg test_images/palm_7R.jpg && ./$(EXECUTABLE_TEMPLATE_MATCHING_WTB) test_images/palm_7R_1_4.jpg test_images/palm_7R.jpg && ./$(EXECUTABLE_TEMPLATE_MATCHING_JSON) test_images/palm_7R_1_4.jpg test_images/palm_7R.jpg 0.6 && $(EXECUTABLE_EXTRACT_CONTOURS) test_images/letter_b.png && $(EXECUTABLE_EXTRACT_CONTOURS_TO_MOSAIC) test_images/palm_7R.jpg 1

