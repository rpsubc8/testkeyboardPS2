#include "gbConfig.h"
#include "PS2Kbd.h"
#include <Arduino.h>

#define gb_max_keymap 32
#define gb_max_keymap32 8
volatile unsigned char gb_keymap[gb_max_keymap]; //256 DIV 8 = 32 bytes packet bit
volatile unsigned int *gb_keymap32=(unsigned int *)gb_keymap;

//unsigned int gb_keymap_state_prev32[gb_max_keymap32]; //Estado anterior

volatile unsigned char keyup = 0;

//#define DEBUG_LOG_KEYSTROKES 1

volatile unsigned char gb_kb_bitcount = 0;
volatile unsigned char gb_kb_incoming = 0;
volatile unsigned int gb_kb_prev_ms = 0;

//unsigned int gb_tiempo_borrar_cur=0,gb_tiempo_borrar_prev=0;


void IRAM_ATTR kb_interruptHandler()
{
 unsigned int now_ms;
 unsigned char n, val;
 
 //if (digitalRead(KEYBOARD_CLK) == 1) 
 if ((REG_READ(GPIO_IN1_REG) >> (KEYBOARD_CLK- 32))==1)
 {
  return;
 }

 //val = digitalRead(KEYBOARD_DATA);
 val = (REG_READ(GPIO_IN1_REG) >> (KEYBOARD_DATA- 32))&0x01;
 now_ms = millis(); 
 if ((now_ms - gb_kb_prev_ms) > 250)
 {
  gb_kb_bitcount = 0;
  gb_kb_incoming = 0;
 }
 gb_kb_prev_ms = now_ms;
 n = gb_kb_bitcount - 1;
 if (n <= 7)
 {
  gb_kb_incoming |= (val << n);
 }
 gb_kb_bitcount++;
 if (gb_kb_bitcount == 11)
 {
  if (keyup == 1) 
  {//Se libera tecla pasa a 1   
   if (((gb_keymap[(gb_kb_incoming>>3)]>>(gb_kb_incoming & 0x07)) & 0x01) == 0)
   {//DIV 8 MOD 8
    gb_keymap[(gb_kb_incoming>>3)] |= (1<<(gb_kb_incoming & 0x07));     
   }
   else
   {
    //Serial.println("WARNING: Keyboard cleaned");
    for (unsigned char gg = 0; gg < gb_max_keymap32; gg++)
    {
     gb_keymap32[gg]=(unsigned int)0xFFFFFFFF;
    }     
   }
   keyup = 0;
  }
  else
  {//Se pulsa tecla pasa a 0
   gb_keymap[(gb_kb_incoming>>3)] &= ((~(1<<(gb_kb_incoming & 0x07))) & 0xFF);        
  }

  //#ifdef DEBUG_LOG_KEYSTROKES
  // #ifdef use_lib_log_serial
  //  Serial.printf("PS2Kbd[%s]: %02X\r\n", keyup ? " up " : "down", gb_kb_incoming);
  // #endif    
  //#endif
   
  keyup= (gb_kb_incoming == 240) ? 1 : 0;
  
  gb_kb_bitcount = 0;
  gb_kb_incoming = 0;
 }
}


void kb_begin()
{
 memset((void *)gb_keymap, 0xFF, sizeof(gb_keymap));
// SaveStateKeyboard();

 #ifdef PS2_DIAGNOSTIC_ECHO
  PS2SendECHO();
 #endif
 
 pinMode(KEYBOARD_DATA, INPUT_PULLUP);
 pinMode(KEYBOARD_CLK, INPUT_PULLUP);
 digitalWrite(KEYBOARD_DATA, true);
 digitalWrite(KEYBOARD_CLK, true);

 attachInterrupt(digitalPinToInterrupt(KEYBOARD_CLK), kb_interruptHandler, FALLING);
}


// Check if key is pressed and clean it
unsigned char checkAndCleanKey(unsigned char scancode)
{
 unsigned char auxId= scancode>>3; //DIV 8
 unsigned char auxOffs= scancode & 0x07; //MOD 8
 unsigned char valor= ((gb_keymap[auxId])>>auxOffs)&0x01;

 if (valor == 0)
 {
  gb_keymap[auxId] |= (1<<auxOffs);

  return 1;     
 }
 return 0;
}

//*****************************************
unsigned char checkKey(unsigned char scancode)
{
 unsigned char auxId= scancode>>3; //DIV 8
 unsigned char auxOffs= scancode & 0x07; //MOD 8

 unsigned char valor= (~((gb_keymap[auxId])>>auxOffs))&0x01;

 //gb_tiempo_borrar_cur= millis();
 //if ((gb_tiempo_borrar_cur-gb_tiempo_borrar_prev)>200)
 //{
 // gb_tiempo_borrar_prev= gb_tiempo_borrar_cur;    
 //}
 
 return valor;
}

#ifdef PS2_DIAGNOSTIC_ECHO
 void PS2SendECHO()
 {
  //unsigned char comando = 0xEE; // Comando ECHO 
  pinMode(KEYBOARD_CLK, OUTPUT);
  pinMode(KEYBOARD_DATA, OUTPUT);
  delay(PS2_DIAGNOSTIC_ECHO_BOOT_TIME_DELAY);  //delay(1000); // Espera a que el teclado encienda 

  Serial.printf("PS2SendECHO BEGIN\r\n");

  // 1. Peticion de envio (CLK bajo por 110 micro segundos)
  digitalWrite(KEYBOARD_CLK, LOW);
  delayMicroseconds(110);
  
  // 2. Bit de inicio (DATA bajo)
  digitalWrite(KEYBOARD_DATA, LOW);
  delayMicroseconds(15);

  // 3. Enviar los 8 bits directamente del byte usando tiempos fijos (12.5 kHz)
  for (unsigned char i = 0; i < 8; i++) {
    digitalWrite(KEYBOARD_CLK, LOW);    
    digitalWrite(KEYBOARD_DATA, (0xEE >> i) & 1); //digitalWrite(KEYBOARD_DATA, (comando >> i) & 1);
    delayMicroseconds(40);
    
    digitalWrite(KEYBOARD_CLK, HIGH);
    delayMicroseconds(40);
  }

  // 4. Bit de Paridad (Para 0xEE siempre es 0)
  digitalWrite(KEYBOARD_CLK, LOW);
  digitalWrite(KEYBOARD_DATA, LOW);
  delayMicroseconds(40);
  digitalWrite(KEYBOARD_CLK, HIGH);
  delayMicroseconds(40);

  // 5. Bit de Parada (Siempre es 1)
  digitalWrite(KEYBOARD_CLK, LOW);
  digitalWrite(KEYBOARD_DATA, HIGH);
  delayMicroseconds(40);
  digitalWrite(KEYBOARD_CLK, HIGH);
  delayMicroseconds(40);

  delay(PS2_DIAGNOSTIC_ECHO_TIME_DELAY);
  Serial.printf("PS2SendECHO END\r\n");
 }
#endif


//void ResetKeyboard()
//{
// for (unsigned char i = 0; i < gb_max_keymap32; i++)
// {
//  gb_keymap32[i]=(unsigned int)0xFFFFFFFF;  
// }     
//}

//void SaveStateKeyboard()
//{ 
// unsigned int dato;
// 
// for (unsigned char i = 0; i < gb_max_keymap32; i++)
// {
//  dato= gb_keymap32[i];
//  gb_keymap_state_prev32[i]= dato;
// }
//}

//unsigned char ChangeStateKeyboard()
//{
// unsigned int dato;
// 
// for (unsigned char i = 0; i < gb_max_keymap32; i++)
// {
//  dato= gb_keymap32[i];
//  if (gb_keymap_state_prev32[i]!= dato)
//  {
//   return 1;
//  }
// }
//
// return 0;
//}
