//Simple minimal test keyboard ESP32 PS/2 TTGO VGA32
//GPIO: CLK(33)  Data(32)
//Warning: Use REG_READ, not digitalRead (32 GPIOS low or high modification)
//Author: ackerman
//Show 240 scancodes

#include "PS2Kbd.h"
#include <Arduino.h>

//milliseconds polling
#define gb_max_poll_ms 100
#define gb_max_reset_keyboard 4000


unsigned char gb_setup_end=0;
unsigned int gb_teclado_prev=0;
unsigned int gb_teclado_cur=0;
unsigned int gb_tiempo_borrar_cur=0,gb_tiempo_borrar_prev=0;

void DumpTeclado(void);

void DumpTeclado()
{ 
 unsigned char dato;
 #define maxLinea 80
 unsigned char contLinea=0;

 //Clear putty BEGIN
 Serial.write(27);       // ESC command
 Serial.print("[2J");    // clear screen command
 Serial.write(27);
 Serial.print("[H");     // cursor to home command
 //Clear putty END

 //Clear terminal Arduino IDE BEGIN
 for (unsigned short i=0;i<30;i++)
 {
  Serial.printf("\r\n");
 }
 //Clear terminal Arduino IDE END

 for (unsigned short int i=0; i<240;i++)
 {
  dato= checkKey(i);
  Serial.printf("%d",dato);

  contLinea++;
  if (contLinea>=maxLinea)
  {
   contLinea=0;
   Serial.printf("\r\n");
  }  
 }
}

void setup() 
{
 gb_setup_end=0;
 Serial.begin(115200);          

 kb_begin();
 delay(100);
  
 gb_setup_end= 1;  
}




void loop() 
{
 if (gb_setup_end)
 {
  gb_teclado_cur= millis();
  unsigned int aux= (gb_teclado_cur-gb_teclado_prev);
  if(aux>=(gb_max_poll_ms-1))
  {
   gb_teclado_prev= gb_teclado_cur;
   
   DumpTeclado();   
  }
 }
}
