# modify this to your actual path
OPENCV_INCLUDE_PATH=/usr/local/opencv-4.2.0/include
OPENCV_LIB_PATH=/usr/local/opencv-4.2.0/lib

# Should not modify the following
CC=g++
IDIR=$(OPENCV_INCLUDE_PATH)
LDIR=$(OPENCV_LIB_PATH)
CFLAGS=-I$(IDIR)
LDFLAGS=-L$(LDIR)
ODIR=obj

LIBS=-lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc

_DEPS = 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: src/%.cpp $(DEPS)
	mkdir obj
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -rf $(ODIR) main
