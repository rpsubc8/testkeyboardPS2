#include "gbConfig.h"
#include "PS2Kbd.h"
#include <Arduino.h>

#define gb_max_keymap 32
#define gb_max_keymap32 8
volatile unsigned char gb_keymap[gb_max_keymap]; //256 DIV 8 = 32 bytes packet bit
volatile unsigned int *gb_keymap32=(unsigned int *)gb_keymap;

unsigned char keyup = 0;

//#define DEBUG_LOG_KEYSTROKES 1


static unsigned char gb_kb_bitcount = 0;
static unsigned char gb_kb_incoming = 0;
static unsigned int gb_kb_prev_ms = 0; 

void __not_in_flash_func()kb_interruptHandler()
{
 unsigned int now_ms;
 unsigned char n, val;

 #ifdef PS2_FAST_BIT
  if digitalReadFast2040(KEYBOARD_CLK) { return; }
  val= digitalReadFast2040(KEYBOARD_DATA);
 #else
  if (digitalRead(KEYBOARD_CLK) == 1) { return; }
  val = digitalRead(KEYBOARD_DATA);
 #endif
   
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
   //if (gb_keymap[gb_kb_incoming] == 0)
   if (((gb_keymap[(gb_kb_incoming>>3)]>>(gb_kb_incoming & 0x07)) & 0x01) == 0)
   {
    gb_keymap[(gb_kb_incoming>>3)] |= (1<<(gb_kb_incoming & 0x07));
   }
   else
   {
    //Serial.println("WARNING: Keyboard cleaned");    
    for (unsigned char gg = 0; gg < gb_max_keymap32; gg++)
    { //cambio      
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


#ifdef PS2_DIAGNOSTIC_ECHO
 short int kb_begin()
 {
  short int aReturn=-1;

  memset((void *)gb_keymap, 0xFF, sizeof(gb_keymap));

  delay(PS2_BOOT_TIME_DELAY); //Wait, stabilize voltage

  aReturn= PS2SendECHO();
  
  pinMode(KEYBOARD_DATA, INPUT_PULLUP);
  pinMode(KEYBOARD_CLK, INPUT_PULLUP);
  digitalWrite(KEYBOARD_DATA, true);
  digitalWrite(KEYBOARD_CLK, true);
      
  attachInterrupt(digitalPinToInterrupt(KEYBOARD_CLK), kb_interruptHandler, FALLING);
  
  return aReturn;
 }
#else
 void kb_begin()
 {
  memset((void *)gb_keymap, 0xFF, sizeof(gb_keymap));

  delay(PS2_BOOT_TIME_DELAY); //Wait, stabilize voltage
  
  pinMode(KEYBOARD_DATA, INPUT_PULLUP);
  pinMode(KEYBOARD_CLK, INPUT_PULLUP);
  digitalWrite(KEYBOARD_DATA, true);
  digitalWrite(KEYBOARD_CLK, true);
      
  attachInterrupt(digitalPinToInterrupt(KEYBOARD_CLK), kb_interruptHandler, FALLING);
 }
#endif


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
  
 return valor;
}


#ifdef PS2_DIAGNOSTIC_ECHO
 // TRANSMISION: Envia un byte al teclado usando el Clock del periferico
 void enviarBytePS2(unsigned char dato) 
 {
  unsigned char paridad = 1; // Paridad impar inicializada en 1
  unsigned int timeout = 0;
  unsigned char bitActual;

  #ifdef PS2_FAST_BIT
   // 1. SOLICITUD DE ENVIO (Inhibir lineas)
   pinMode(KEYBOARD_CLK, OUTPUT);
   digitalWriteFast2040(KEYBOARD_CLK,0);    //digitalWrite(KEYBOARD_CLK, LOW);
   delayMicroseconds(110);                  // Mantener bajo > 100us
     
   pinMode(KEYBOARD_DATA, OUTPUT);
   digitalWriteFast2040(KEYBOARD_DATA, 0);  //digitalWrite(KEYBOARD_DATA, LOW);        // Bit de Start (Data = 0)
     
   pinMode(KEYBOARD_CLK, INPUT_PULLUP);   // Libera Clock para que el teclado tome el control   

   timeout = millis();
   // 2. TRANSMISION DE BITS (Sincronizados con el Clock del teclado)
   for (unsigned char i = 0; i < 8; i++)
   {
    bitActual = (dato >> i) & 0x01;
    paridad ^= bitActual;
    escribirBitConRelojTeclado(bitActual);
   }

   // Enviar bit de Paridad
   escribirBitConRelojTeclado(paridad);

   // Enviar bit de Stop (Data = 1)
   escribirBitConRelojTeclado(HIGH);

   // 3. RESPUESTA DE LINEA (Line ACK de hardware)   
   while (digitalReadFast2040(KEYBOARD_DATA) == HIGH)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return; }
   }
   while (digitalReadFast2040(KEYBOARD_CLK) == LOW)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return; }
   }
   while (digitalReadFast2040(KEYBOARD_DATA) == LOW)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return; }
   }
  #else
   // 1. SOLICITUD DE ENVIO (Inhibir lineas)
   pinMode(KEYBOARD_CLK, OUTPUT);   
  
   digitalWrite(KEYBOARD_CLK, LOW);   
   delayMicroseconds(110);             // Mantener bajo > 100us
  
   pinMode(KEYBOARD_DATA, OUTPUT);   
  
   digitalWrite(KEYBOARD_DATA, LOW);        // Bit de Start (Data = 0)   
  
   pinMode(KEYBOARD_CLK, INPUT_PULLUP);   // Libera Clock para que el teclado tome el control

   timeout = millis();
   // 2. TRANSMISION DE BITS (Sincronizados con el Clock del teclado)
   for (unsigned char i = 0; i < 8; i++) 
   {
    bitActual = (dato >> i) & 0x01;
    paridad ^= bitActual;             
    escribirBitConRelojTeclado(bitActual);
   }

   // Enviar bit de Paridad
   escribirBitConRelojTeclado(paridad);

   // Enviar bit de Stop (Data = 1)
   escribirBitConRelojTeclado(HIGH);

   // 3. RESPUESTA DE LINEA (Line ACK de hardware)   
   while (digitalRead(KEYBOARD_DATA) == HIGH)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return; }
   }
   while (digitalRead(KEYBOARD_CLK) == LOW)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return; }
   }
   while (digitalRead(KEYBOARD_DATA) == LOW)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return; }
   }
  #endif
 }

 // Funcion auxiliar para escribir bits individuales en los flancos del teclado
 void escribirBitConRelojTeclado(unsigned char bitVal) 
 {
  unsigned int timeout = millis();

  #ifdef PS2_FAST_BIT
   while (digitalReadFast2040(KEYBOARD_CLK) == HIGH)
   {// Esperar flanco de bajada
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return; }  
   }

   if (bitVal == HIGH)
   {
    pinMode(KEYBOARD_DATA, INPUT_PULLUP);              //entrada
   }
   else 
   {
    pinMode(KEYBOARD_DATA, OUTPUT);

    digitalWriteFast2040(KEYBOARD_DATA, LOW);
   }
   
   while (digitalReadFast2040(KEYBOARD_CLK) == LOW)
   {// Esperar a que el Clock vuelva a subir
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return; }  
   }
  #else   
   while (digitalRead(KEYBOARD_CLK) == HIGH)
   {// Esperar flanco de bajada
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return; }  
   }

   if (bitVal == HIGH)
   {
    pinMode(KEYBOARD_DATA, INPUT_PULLUP);              //entrada
   }
   else 
   {
    pinMode(KEYBOARD_DATA, OUTPUT);

    digitalWrite(KEYBOARD_DATA, LOW);
   }
   
   while (digitalRead(KEYBOARD_CLK) == LOW)
   {// Esperar a que el Clock vuelva a subir
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return; }  
   }  
  #endif 
 }

 // RECEPCION: Lee un byte completo proveniente del teclado usando Polling puro
 short int leerBytePS2() 
 {
  unsigned char datoRecibido = 0;
  unsigned char bitLeido;
  unsigned int timeout = millis();

  #ifdef PS2_FAST_BIT
   // 1. Esperar el bit de START (El teclado baja la línea DATA)
   // Timeout de 500ms por si el teclado no responde   
   while (digitalReadFast2040(KEYBOARD_DATA) == HIGH)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
   }
  
   // Esperar a que el ciclo de reloj del bit de Start termine   
   while (digitalReadFast2040(KEYBOARD_CLK) == HIGH)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
   }   
   while (digitalReadFast2040(KEYBOARD_CLK) == LOW)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
   }

   // 2. LEER LOS 8 BITS DE DATOS (LSB primero)
   for (unsigned char i = 0; i < 8; i++) 
   {    
    while (digitalReadFast2040(KEYBOARD_CLK) == HIGH)
    { //Esperar a que el Clock baje (El dato ya es valido)
     if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
    }
        
    bitLeido = digitalReadFast2040(KEYBOARD_DATA);
    datoRecibido |= (bitLeido << i);        // Almacenar el bit en su posicion
        
    while (digitalReadFast2040(KEYBOARD_CLK) == LOW)
    { //Esperar a que el Clock suba
      if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
    }
   }

   // 3. LEER BIT DE PARIDAD (Saltar/Ignorar en este ejemplo basico)   
   while (digitalReadFast2040(KEYBOARD_CLK) == HIGH)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
   }   
   while (digitalReadFast2040(KEYBOARD_CLK) == LOW)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
   }

   // 4. LEER BIT DE STOP   
   while (digitalReadFast2040(KEYBOARD_CLK) == HIGH)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
   }   
   while (digitalReadFast2040(KEYBOARD_CLK) == LOW)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
   }

   return datoRecibido;
  #else
   // 1. Esperar el bit de START (El teclado baja la línea DATA)
   // Timeout de 500ms por si el teclado no responde   
   while (digitalRead(KEYBOARD_DATA) == HIGH)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
   }
  
   // Esperar a que el ciclo de reloj del bit de Start termine   
   while (digitalRead(KEYBOARD_CLK) == HIGH)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
   }   
   while (digitalRead(KEYBOARD_CLK) == LOW)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
   }

   // 2. LEER LOS 8 BITS DE DATOS (LSB primero)
   for (unsigned char i = 0; i < 8; i++) 
   {    
    while (digitalRead(KEYBOARD_CLK) == HIGH)
    { //Esperar a que el Clock baje (El dato ya es valido)
     if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
    }
        
    bitLeido = digitalRead(KEYBOARD_DATA);
    datoRecibido |= (bitLeido << i);        // Almacenar el bit en su posicion
        
    while (digitalRead(KEYBOARD_CLK) == LOW)
    { //Esperar a que el Clock suba
      if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
    }
   }

   // 3. LEER BIT DE PARIDAD (Saltar/Ignorar en este ejemplo basico)   
   while (digitalRead(KEYBOARD_CLK) == HIGH)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
   }   
   while (digitalRead(KEYBOARD_CLK) == LOW)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
   }

   // 4. LEER BIT DE STOP   
   while (digitalRead(KEYBOARD_CLK) == HIGH)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
   }   
   while (digitalRead(KEYBOARD_CLK) == LOW)
   {
    if (millis() - timeout > PS2_DIAGNOSTIC_ECHO_TIMEOUT) { return -1; }
   }

   return datoRecibido;
  #endif  
 }


 short int PS2SendECHO()
 {
  short int respuesta=-1;
  
  pinMode(KEYBOARD_CLK, INPUT_PULLUP);
  pinMode(KEYBOARD_DATA, INPUT_PULLUP);

  Serial.println("PS2SendECHO BEGIN");
  delay(PS2_DIAGNOSTIC_ECHO_BOOT_TIME_DELAY);  //delay(1000); // Espera a que el teclado encienda. Wait, stabilize voltage
  
  Serial.println("--- Testing PS/2 Communication (ECHO) ---");
  enviarBytePS2(CMD_ECHO);   //Enviar comando ECHO  
  respuesta = leerBytePS2(); //Leer la respuesta del teclado por polling
  
  // Verificar si la respuesta es la correcta
  if (respuesta == CMD_ECHO) 
  {
    Serial.println("OK. Keyboard request 0xEE (ECHO).");
  } 
  else  
  {
   if (respuesta == -1) 
   {
     Serial.println("ERROR. Timeout keyboard.");
   } 
   else 
   {
     Serial.print("ERROR. Unexpected keyboard response:0x");
     Serial.println(respuesta, HEX);
   }  
  }
  
  delay(PS2_DIAGNOSTIC_ECHO_TIME_DELAY);
  Serial.println("PS2SendECHO END");

  return respuesta;
 }
 
#endif


void ClearOverFlow()
{//Generalmente mas de 4 teclas a una misma linea generan overflow. Se puede llegar a 6.
 gb_keymap[0]= gb_keymap[0] | 0x01;  //00000001 0x01  Es logica negada El bit 0 del Byte 0. Posicion 0 buffer, lo ponemos a 1, reset buffer overflow (maximas teclas)
}
