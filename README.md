<H1>Minimal Test Keyboard PS/2</H1>
<ul>
 <li>Test in Arduino IDE 1.8.16.</li> 
 <li>Output serial (240 scancodes) 115200 baudios.</li>
 <li>Serial Output ECHO 0xEE CMD</li>
 <li>The serial port output consists of a matrix of 3 rows of 80 columns, i.e. 240 scancodes, ASCII value key pressed.</li>
 <li>The serial port output consists of a matrix of 3 rows of 80 columns, i.e. 240 scancodes, where a 1 is a key pressed and 0 not pressed.</li>
 <li>Auto detection Protocol Switching USB PS/2, send ECHO CMD (0xEE). Keyboards Perixx, Holtek, Cypres.</li>
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
I have created a <b>PS2_DIAGNOSTIC_ECHO</b> pragma in <b>gbConfig.h</b> to enable this fix.

<br><br>
<h1>Acknowledgements</h1>
Thanks to <b>mvalder</b> for his tests using the PERIXX physical keyboard.<br>
I don’t have a PERIXX keyboard.
