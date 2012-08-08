# foolib rules.mk file.

include $(MAKEDIR)/header.mk

TINYWS_INCLUDES := -I$(d)
WIFLY_INCLUDES := -I$(d)/../WiFlySerial/include
CFLAGS_$(d) := $(WIRISH_INCLUDES) $(LIBMAPLE_INCLUDES) $(TINYWS_INCLUDES) $(WIFLY_INCLUDES)
cppSRCS_$(d) := TinyWebServer.cpp

include $(MAKEDIR)/footer.mk
