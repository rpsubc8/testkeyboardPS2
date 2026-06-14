#ifndef _GB_CONFIG_H
 #define _GB_CONFIG_H

 //GPIO PS/2 Keyboard Data
 #define KEYBOARD_DATA 32
 //GPIO PS/2 Keyboard CLK
 #define KEYBOARD_CLK 33
 
 //Auto detection Protocol Switching USB PS/2 (fix Perixx, Holtek, Cypres)
 #define PS2_DIAGNOSTIC_ECHO
 
 //Time delay before send ECHO
 #define PS2_DIAGNOSTIC_ECHO_BOOT_TIME_DELAY 500
 //Time delay after send ECHO
 #define PS2_DIAGNOSTIC_ECHO_TIME_DELAY 500
 //Timeout flag keyboard
 #define PS2_DIAGNOSTIC_ECHO_TIMEOUT 500

 
#endif
 
