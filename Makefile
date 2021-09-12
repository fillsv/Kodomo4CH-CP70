LDFLAGS += -L${KAYA_VISION_POINT_LIB_PATH}
LDFLAGS += -Wl,-rpath,${KAYA_VISION_POINT_LIB_PATH}
LDFLAGS += -Wl,-rpath,'$$ORIGIN'
KYFLAGS+=-lz #only needed when KYFGLib is built with $KAYA_ATTR_OPENCV_LINK_MODE == "local"
#LIBS+=-lm

all: to_ssd to_mem

to_ssd: KYFGLib_to_ssd.c
	gcc -o KYFGLib_to_ssd KYFGLib_to_ssd.c -I$(KAYA_VISION_POINT_INCLUDE_PATH) $(LDFLAGS) -lKYFGLib $(KYFLAGS)

to_mem: KYFGLib_to_mem_to_ssd.c
	gcc -o KYFGLib_to_mem_to_ssd KYFGLib_to_mem_to_ssd.c -I$(KAYA_VISION_POINT_INCLUDE_PATH) $(LDFLAGS) -lKYFGLib $(KYFLAGS) -lm




