CC= g++-4.0 # or gcc-4.0
CFLAGS=
LDFLAGS=-ggdb -Wall -I /usr/local/include/ -lcxcore.2.0 -lcv.2.0 -lhighgui.2.0 -lcvaux.2.0 -lml

SOURCES_TEMPLATE_MATCHING=template_matching.c
SOURCES_TEMPLATE_MATCHING_JSON=template_matching_json.c
SOURCES_TEMPLATE_MATCHING_WTB=template_matching_with_trackbar.c

OBJECTS_TEMPLATE_MATCHING=$(SOURCES_TEMPLATE_MATCHING:.cpp=.o)
OBJECTS_TEMPLATE_MATCHING_JSON=$(SOURCES_TEMPLATE_MATCHING_JSON:.cpp=.o)
OBJECTS_TEMPLATE_MATCHING_WTB=$(SOURCES_TEMPLATE_MATCHING_WTB:.cpp=.o)

EXECUTABLE_TEMPLATE_MATCHING=template_matching
EXECUTABLE_TEMPLATE_MATCHING_JSON=template_matching_json
EXECUTABLE_TEMPLATE_MATCHING_WTB=template_matching_with_trackbar

all: $(SOURCES_TEMPLATE_MATCHING_JSON) $(EXECUTABLE_TEMPLATE_MATCHING_JSON) $(SOURCES_TEMPLATE_MATCHING) $(EXECUTABLE_TEMPLATE_MATCHING) $(SOURCES_TEMPLATE_MATCHING_WTB) $(EXECUTABLE_TEMPLATE_MATCHING_WTB) 

$(EXECUTABLE_TEMPLATE_MATCHING):
	$(CC) $(LDFLAGS) $(OBJECTS_TEMPLATE_MATCHING) -o $@

$(EXECUTABLE_TEMPLATE_MATCHING_JSON):
	$(CC) $(LDFLAGS) $(OBJECTS_TEMPLATE_MATCHING_JSON) -o $@

$(EXECUTABLE_TEMPLATE_MATCHING_WTB):
	$(CC) $(LDFLAGS) $(OBJECTS_TEMPLATE_MATCHING_WTB) -o $@

clean:
	rm -rf *.dSYM *o $(EXECUTABLE_TEMPLATE_MATCHING) $(EXECUTABLE_TEMPLATE_MATCHING_WTB) $(EXECUTABLE_TEMPLATE_MATCHING_JSON)

test:
	./$(EXECUTABLE_TEMPLATE_MATCHING) test_images/palm_7R_1_4.jpg test_images/palm_7R.jpg && ./$(EXECUTABLE_TEMPLATE_MATCHING_WTB) test_images/palm_7R_1_4.jpg test_images/palm_7R.jpg && ./$(EXECUTABLE_TEMPLATE_MATCHING_JSON) test_images/palm_7R_1_4.jpg test_images/palm_7R.jpg 0.6

