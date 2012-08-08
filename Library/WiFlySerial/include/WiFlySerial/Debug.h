/*
   Debug.h -- Utility macros for debug logging

   NOTE: Requires Serial.begin() to have been called previously or the messages
         will be lost.

   If DEBUG_LEVEL is 0 or undefined no logging occurs.

   Higher DEBUG_LEVEL values should cause more detail to be displayed.

 */
#ifndef __DEBUG_H__
#define __DEBUG_H__
#define __LIBMAPLE_H__

#ifndef DEBUG_LVL
#define DEBUG_LVL 2	
#endif

#if DEBUG_LVL == 0

#define DEBUG_LOG(level, message){}

#else

// TODO: Store strings in PROGMEM?
// TODO: Modify so lower level messages/conditionals aren't compiled at all?

#define DEBUG_LOG(level, message) {if (DEBUG_LVL >= level){SerialUSB.print("DEBUG: "); SerialUSB.println(message); }; }

#endif

#endif
