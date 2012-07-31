# foolib rules.mk file.

include $(MAKEDIR)/header.mk

TINYWS_INCLUDES := -I$(d)
CFLAGS_$(d) := $(WIRISH_INCLUDES) $(LIBMAPLE_INCLUDES) $(TINYWS_INCLUDES)
cppSRCS_$(d) := TinyWebServer.cpp

include $(MAKEDIR)/footer.mk
