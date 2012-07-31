# foolib rules.mk file.

include $(MAKEDIR)/header.mk

WIFLY_INCLUDES := -I$(d)/include
CFLAGS_$(d) := $(WIRISH_INCLUDES) $(LIBMAPLE_INCLUDES) $(WIFLY_INCLUDES)

cppSRCS_$(d) := SpiUart.cpp
#cppSRCS_$(d) += WiFlyDevice.cpp

include $(MAKEDIR)/footer.mk
