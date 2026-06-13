#include "gbConfig.h"
#include "PS2Kbd.h"
#include <Arduino.h>

#define gb_max_keymap 32
#define gb_max_keymap32 8
volatile unsigned char gb_keymap[gb_max_keymap]; //256 DIV 8 = 32 bytes packet bit
volatile unsigned int *gb_keymap32=(unsigned int *)gb_keymap;

#ifdef PS2_DIAGNOSTIC_ECHO
 short int gb_keyboard_echo_req= 0;
#endif 

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

 // Definicion de pines (puedes usar cualquiera, no requieren interrupcion externa)
 const int CLOCK_PIN = KEYBOARD_CLK;
 const int DATA_PIN = KEYBOARD_DATA;
 const unsigned char CMD_ECHO = 0xEE;


// TRANSMISION: Envia un byte al teclado usando el Clock del periferico
void enviarBytePS2(unsigned char dato) 
{
  unsigned char paridad = 1; // Paridad impar inicializada en 1
  unsigned int timeout = 0;

  // 1. SOLICITUD DE ENVIO (Inhibir lineas)
  pinMode(CLOCK_PIN, OUTPUT);
  digitalWrite(CLOCK_PIN, LOW);       
  delayMicroseconds(110);             // Mantener bajo > 100us
  
  pinMode(DATA_PIN, OUTPUT);
  digitalWrite(DATA_PIN, LOW);        // Bit de Start (Data = 0)
  
  pinMode(CLOCK_PIN, INPUT_PULLUP);   // Libera Clock para que el teclado tome el control

  timeout = millis();
  // 2. TRANSMISION DE BITS (Sincronizados con el Clock del teclado)
  for (unsigned char i = 0; i < 8; i++) 
  {
    unsigned char bitActual = (dato >> i) & 0x01;
    paridad ^= bitActual;             
    escribirBitConRelojTeclado(bitActual);
  }

  // Enviar bit de Paridad
  escribirBitConRelojTeclado(paridad);

  // Enviar bit de Stop (Data = 1)
  escribirBitConRelojTeclado(HIGH);

  // 3. RESPUESTA DE LINEA (Line ACK de hardware)
  while (digitalRead(DATA_PIN) == HIGH)
  {
   if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return; }
  }
  while (digitalRead(CLOCK_PIN) == LOW)
  {
   if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return; }
  }
  while (digitalRead(DATA_PIN) == LOW)
  {
   if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return; }  
  }
}

// Funcion auxiliar para escribir bits individuales en los flancos del teclado
void escribirBitConRelojTeclado(unsigned char bitVal) 
{
  unsigned int timeout = millis();
  
  while (digitalRead(CLOCK_PIN) == HIGH)
  { // Esperar flanco de bajada
   if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return; }  
  }

  if (bitVal == HIGH)
  {
    pinMode(DATA_PIN, INPUT_PULLUP);  
  }
  else 
  {
    pinMode(DATA_PIN, OUTPUT);
    digitalWrite(DATA_PIN, LOW);      
  }

  while (digitalRead(CLOCK_PIN) == LOW)
  { // Esperar a que el Clock vuelva a subir
   if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return; }  
  }
}

// RECEPCION: Lee un byte completo proveniente del teclado usando Polling puro
short int leerBytePS2() 
{
  unsigned char datoRecibido = 0;
  unsigned long timeout = millis();

  // 1. Esperar el bit de START (El teclado baja la línea DATA)
  // Añadimos un timeout de 500ms por si el teclado no responde
  while (digitalRead(DATA_PIN) == HIGH) 
  {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
  }
  
  // Esperar a que el ciclo de reloj del bit de Start termine
  while (digitalRead(CLOCK_PIN) == HIGH)
  {
   if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
  }
  while (digitalRead(CLOCK_PIN) == LOW)
  {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
  }

  // 2. LEER LOS 8 BITS DE DATOS (LSB primero)
  for (unsigned char i = 0; i < 8; i++) 
  {
    while (digitalRead(CLOCK_PIN) == HIGH)
    { // Esperar a que el Clock baje (El dato ya es valido)
     if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
    }
    
    unsigned char bitLeido = digitalRead(DATA_PIN);
    datoRecibido |= (bitLeido << i);        // Almacenar el bit en su posicion
    
    while (digitalRead(CLOCK_PIN) == LOW)
    { // Esperar a que el Clock suba
      if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
    }
  }

  // 3. LEER BIT DE PARIDAD (Saltar/Ignorar en este ejemplo basico)
  while (digitalRead(CLOCK_PIN) == HIGH)
  {
   if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
  }
  while (digitalRead(CLOCK_PIN) == LOW)
  {
   if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
  }

  // 4. LEER BIT DE STOP
  while (digitalRead(CLOCK_PIN) == HIGH)
  {
   if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
  }
  while (digitalRead(CLOCK_PIN) == LOW)
  {
   if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
  }

  return datoRecibido;
}


 short int PS2SendECHO()
 {
  pinMode(CLOCK_PIN, INPUT_PULLUP);
  pinMode(DATA_PIN, INPUT_PULLUP);

  Serial.println("PS2SendECHO BEGIN");
  delay(PS2_DIAGNOSTIC_ECHO_BOOT_TIME_DELAY);  //delay(1000); // Espera a que el teclado encienda 
  
  Serial.println("--- Probando Comunicacion PS/2 (ECHO) ---");

  // Enviar comando ECHO
  enviarBytePS2(CMD_ECHO);
  
  // Leer la respuesta del teclado por polling
  int respuesta = leerBytePS2();
  
  // Verificar si la respuesta es la correcta
  if (respuesta == CMD_ECHO) 
  {
    Serial.println("¡EXITO! El teclado respondio correctamente con 0xEE (ECHO).");
  } 
  else  
  {
   if (respuesta == -1) 
   {
     Serial.println("ERROR: Tiempo de espera agotado (Timeout). El teclado no respondio.");
   } 
   else 
   {
     Serial.print("ERROR: Respuesta inesperada del teclado: 0x");
     Serial.println(respuesta, HEX);
   }  
  }

  gb_keyboard_echo_req= respuesta;

  delay(PS2_DIAGNOSTIC_ECHO_TIME_DELAY);
  Serial.println("PS2SendECHO END");

  return respuesta;
 }


 short int PS2GetECHOState(void)
 {
  return gb_keyboard_echo_req;
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
