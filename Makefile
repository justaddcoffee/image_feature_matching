CC=g++ # or gcc
CFLAGS=-ggdb -Wall 
LDFLAGS=-I /usr/local/include/opencv -lcxcore -lcv -lhighgui -lcvaux -lml
SOURCES=template_matching.c
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=template_matching

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

clean:
	rm -rf *o template_matching template_matching.dSYM

test:
	./template_matching test_images/palm_7R_1_4.jpg test_images/palm_7R.jpg

