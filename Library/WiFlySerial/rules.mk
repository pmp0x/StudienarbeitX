# foolib rules.mk file.

include $(MAKEDIR)/header.mk

WIFLYSERIAL_INCLUDES := -I$(d)/include
CFLAGS_$(d) := $(WIRISH_INCLUDES) $(LIBMAPLE_INCLUDES) $(WIFLYSERIAL_INCLUDES) $(WIFLY_INCLUDES)

cppSRCS_$(d) := WiFlySerial.cpp
#cppSRCS_$(d) +=

include $(MAKEDIR)/footer.mk
