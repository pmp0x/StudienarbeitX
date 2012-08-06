# foolib rules.mk file.

include $(MAKEDIR)/header.mk

WIFLYSERIAL_INCLUDES := -I$(d)/include
WIFLY_INCLUDES := -I$(d)/../WiFlyShield/include
CFLAGS_$(d) := $(WIRISH_INCLUDES) $(WIFLY_INCLUDES) $(LIBMAPLE_INCLUDES) $(WIFLYSERIAL_INCLUDES) 

cppSRCS_$(d) := WiFlySerial.cpp
cppSRCS_$(d) += WFSEthernet.cpp
cppSRCS_$(d) += WFSEthernetServer.cpp
cppSRCS_$(d) += WFSEthernetClient.cpp
cppSRCS_$(d) += WFSIPAddress.cpp


include $(MAKEDIR)/footer.mk
