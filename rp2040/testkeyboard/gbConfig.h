#ifndef _GB_CONFIG_H
 #define _GB_CONFIG_H

 //use_lib_keyboard_ps2usb(USB connector (middle) next to HDMI) 
 //use_lib_keyboard_ps2(GPIOs) 
 //#define use_lib_keyboard_ps2
 #define use_lib_keyboard_ps2usb


 //Fast DigitalWrite, DigitalRead
 //#define PS2_FAST_BIT

 //Time delay before init keyb
 #define PS2_BOOT_TIME_DELAY 1000
 
 //Auto detection Protocol Switching USB PS/2 (fix Perixx, Holtek, Cypres)
 //#define PS2_DIAGNOSTIC_ECHO
 
 //Time delay before send ECHO
 #define PS2_DIAGNOSTIC_ECHO_BOOT_TIME_DELAY 2000
 //Time delay after send ECHO
 #define PS2_DIAGNOSTIC_ECHO_TIME_DELAY 500
 //Timeout flag keyboard
 #define PS2_DIAGNOSTIC_ECHO_TIMEOUT 1000




 //IMPORTANT!!! Do not delete
 //GPIO PS/2 Keyboard CLK
 //GPIO PS/2 Keyboard Data 
 #ifdef use_lib_keyboard_ps2
  #define KEYBOARD_CLK 4
  #define KEYBOARD_DATA 5
 #else
  #ifdef use_lib_keyboard_ps2usb
   #define KEYBOARD_CLK 6
   #define KEYBOARD_DATA 7
  #endif 
 #endif
 
#endif
 
