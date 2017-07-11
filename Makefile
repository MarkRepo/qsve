EXECDIR=/home/thinputer/Downloads/libQsve
EXEC=$(EXECDIR)/libqsvenc.so  $(EXECDIR)/testexe
INCS= \
  include/common_utils.h \
  include/common_vaapi.h \
  include/qsv_log.h \
  include/qsvenc.h \
  include/qsv_h264.h 
  
SRCS= \
  src/qsv_h264.cpp \
  src/qsvenc.cpp \
  src/common_utils.cpp \
  src/common_utils_linux.cpp \
  src/common_vaapi.cpp \
  src/qsv_log.cpp 

ifeq ($(dbg),1)
      CFLAGS   += -g
endif

CFLAGS += -Wall -I/usr/local/include -I./include -I/opt/intel/mediasdk/include -I /opt/intel/compilers_and_libraries_2017.4.196/linux/ipp/include
LFLAGS=-shared -fPIC -L$(MFX_HOME)/opensource/mfx_dispatch/build/__lib/ -L/opt/intel/compilers_and_libraries_2017.4.196/linux/ipp/lib/intel64 -lippcc -lippcore -ldispatch_shared -lva -lva-drm -lpthread -lrt -ldl

all: $(EXEC)

$(EXECDIR)/libqsvenc.so: $(SRCS) Makefile
	rm -f /lib64/libh264_route.so
	g++ -o $@ $(SRCS) $(CFLAGS) $(LFLAGS)
	cp -f $@ /lib64

$(EXECDIR)/testexe: src/h264_route_test.o
	g++ -g -Wall -o $@ $+ -lqsvenc -ldl -lpthread

%.o: %.cpp
	g++ -g $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm $(EXEC) src/h264_route_test.o
