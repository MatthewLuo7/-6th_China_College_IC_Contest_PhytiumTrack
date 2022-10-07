CPP = aarch64-linux-gnu-g++ -std=c++17
OPT = -O3

FLOW = xFLOW

BUILD_FLAGS = -Wall
BUILD_FLAGS += -Wl,-rpath-link,/lib \
           -Wl,-rpath-link,/usr/lib \
           -Wl,-rpath-link,/usr/lib/aarch64-linux-gnu \
           -I/usr/include/ \
           -I/usr/include/opencv4 \
           -I/usr/include/aarch64-linux-gnu \
           -I/usr/include/gstreamer-1.0 \
           -I/usr/include/glib-2.0 \
           -I/usr/lib/aarch64-linux-gnu/glib-2.0/include \
           -I/root/mnn/include
BUILD_FLAGS += -lopencv_core -lopencv_highgui -lopencv_videoio -lopencv_imgproc -lopencv_imgcodecs -lopencv_video -lopencv_objdetect -lopencv_dnn -lMNN
BUILD_FLAGS += -lgstreamer-1.0 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lglib-2.0

all: $(FLOW)

$(FLOW): drv_flow.o Process.o Encode.o Decode.o Tree.o TreeNode.o
	@echo "----- Building $(FLOW) -----"
	$(CPP) $(OPT) $^ -o $@ $(BUILD_FLAGS) 
	@echo

%.o: ./src/%.cpp
	$(CPP) $(OPT) -c $< -o $@ $(BUILD_FLAGS) 

.PHONY:	clean

clean:
	rm -rf *.o x* ./picture/*.jpg
