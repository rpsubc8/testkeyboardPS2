<H1>Minimal Test Keyboard PS/2</H1>
<ul>
 <li>Test in Arduino IDE 1.8.16.</li> 
 <li>Output serial (240 scancodes) 115200 baudios.</li>
 <li>Serial Output ECHO 0xEE CMD</li>
 <li>The serial port output consists of a matrix of 3 rows of 80 columns, i.e. 240 scancodes, ASCII value key pressed.</li>
 <li>The serial port output consists of a matrix of 3 rows of 80 columns, i.e. 240 scancodes, where a 1 is a key pressed and 0 not pressed.</li>
 <li>Auto detection Protocol Switching USB PS/2, send ECHO CMD (0xEE). Keyboards Perixx, Holtek, Cypres.</li>
 <li>The F12 key resets the ESP32 or RP2040. This is useful for testing, such as connecting a working keyboard while the device is running, allowing you to reset it and then connect the keyboard that may be causing problems with the 0xEE command.</li>
 <li>Send r or R monitor serial Arduino UART to reboot rp2040 or ESP32.</li>
</ul>

<center><img src='https://raw.githubusercontent.com/rpsubc8/testkeyboardPS2/main/preview/previewTerminal.gif'></center>

<br><br>
<h1>ESP32</h1>
<ul>
 <li>Arduino ESP32 boards 2.0.17</li>
 <li>Espressif System 1.0.6</li>
</ul>

<a href='https://github.com/espressif/arduino-esp32/blob/master/package/package_esp32_index.template.json'>https://github.com/espressif/arduino-esp32/blob/master/package/package_esp32_index.template.json</a>

<br><br>
<h1>RP2040</h1>
Raspberry PiPico/RP2040 by Earle F. Philhower III version 2.6.4<br><br>

<a href='https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json'>https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json</a>


<br><br>
<h1>Switching USB PS/2</h1>
Perixx, Holtek and Cypres keyboards remain in USB mode upon startup, continuously sending the 0xAA command and restarting.<br>
To break the loop and switch to PS/2 mode, the ECHO 0xEE command must be sent.<br>
To exit test boot mode, it doesn’t have to be exactly 0xEE; specifically, the keyboard’s restart command also works, as do the functions for activating Num Lock, Caps Lock and code page selection.<br>
I have created a <b>PS2_DIAGNOSTIC_ECHO</b> pragma in <b>gbConfig.h</b> to enable this fix.<br>


<br><br>
<h1>Overflow</h1>
When more than 4 keys on the same row are pressed, a buffer overflow is triggered. There are special cases involving up to 6 keys, but this is more common with USB keyboards and the USB protocol.<br>
The fact that they are on the same row does not mean that more than 4 keys from different rows cannot be pressed.<br>
In such cases, a warning is displayed on the serial terminal as:

<pre>
 Keyboard buffer overflow 
</pre>
When an overflow occurs, a keyboard returns 0x00, as this is the logical negation.


<br><br>
<h1>Time delay</h1>
PS/2 keyboards require a delay to stabilise the voltage and current before the routine is initialised. Furthermore, it is recommended to use keyboards without backlighting.<br>
The delay is set by the <b>PS2_BOOT_TIME_DELAY</b> in <b>the gbConfig.h</b> file.<br>
PERIXX keyboards require a longer wait time. Once a keyboard has booted up, it has to pass the boot tests.<br>
A minimum timeout period is also required whilst waiting for the response to command 0xEE.<br>

<b>mvalder</b> has carried out tests and has managed to get his PERIXX keyboard to work correctly with the following timing settings:<br>

<pre>
 //Time delay before init keyb
 #define PS2_BOOT_TIME_DELAY 500
 
 //Auto detection Protocol Switching USB PS/2 (fix Perixx, Holtek, Cypres)
 #define PS2_DIAGNOSTIC_ECHO
 
 //Time delay before send ECHO
 #define PS2_DIAGNOSTIC_ECHO_BOOT_TIME_DELAY 500
 //Time delay after send ECHO
 #define PS2_DIAGNOSTIC_ECHO_TIME_DELAY 500
 //Timeout flag keyboard
 #define PS2_DIAGNOSTIC_ECHO_TIMEOUT 400
</pre>

If we want maximum speed when reading and writing bits to the keyboard, we must use the pragma:
<pre>
#define PS2_FAST_BIT
</pre>


<br><br>
<h1>Acknowledgements</h1>
Thanks to <b>mvalder</b> for his tests using the PERIXX physical keyboard.<br>
I don’t have a PERIXX keyboard.
