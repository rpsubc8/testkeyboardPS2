//Simple minimal test keyboard ESP32 PS/2 TTGO VGA32
//GPIO: CLK(33)  Data(32)
//Warning: Use REG_READ, not digitalRead (32 GPIOS low or high modification)
//Author: ackerman
//Show 240 scancodes
//Many thanks to mvalder for all the thorough testing with his PERIXX keyboard.
//Press Key F12 to reboot ESP32

#include "gbConfig.h"
#include "PS2Kbd.h"
#include <Arduino.h>

#define maxLinea 80

//milliseconds polling
#define gb_max_poll_ms 100
#define gb_max_reset_keyboard 4000


unsigned char gb_setup_end=0;
unsigned int gb_teclado_prev=0;
unsigned int gb_teclado_cur=0;
unsigned int gb_tiempo_borrar_cur=0,gb_tiempo_borrar_prev=0;
unsigned char isDelayOverflow=0;
unsigned char key_now[240];
unsigned char key_prev[240];
unsigned char cont_run=0;

#ifdef PS2_DIAGNOSTIC_ECHO
 short int gb_keyboard_echo_req= 0;
#endif 

char ps2_to_ascii[256];

void InitPs2ToASCII(void);
void DumpPs2ToASCII(void);
void DumpMapKey(void);
void DumpTeclado(void);
unsigned char KeyChg(void);
void DoEvent(void);
#ifdef PS2_DIAGNOSTIC_ECHO  
 void DumpEchoReq(void);
#endif 

#ifdef PS2_DIAGNOSTIC_ECHO  
 void DumpEchoReq()
 {
  short int respuesta = gb_keyboard_echo_req;
  
  if (respuesta == 0xEE) 
  {
    Serial.println("OK.ECHO 0xEE");
  }
  else
  {
   if (respuesta == -1)
   {
     Serial.println("KO.Timeout");
   }
   else
   {
     Serial.printf("KO.Error:0x%04X\r\n",respuesta);
   }
  }
 }
#endif

void DoEvent()
{
 if (checkKey(KEY_F12))
 {
  Serial.println("Press Key F12 to Reboot RP2040");
  delay(500);
  ESP.restart();
 }
}

unsigned char KeyChg()
{
 unsigned char aReturn=0;

 for (unsigned char i=0; i<240;i++)
 {  
  key_now[i]= checkKey(i);
  if (key_now[i] != key_prev[i])
  {
   key_prev[i]= key_now[i]; 
   aReturn= 1;
  }  
 }

 return aReturn;
}

void InitPs2ToASCII()
{ 
 memset(ps2_to_ascii,' ',sizeof(ps2_to_ascii));

 ps2_to_ascii[PS2_KC_A]='A';
 ps2_to_ascii[PS2_KC_B]='B';
 ps2_to_ascii[PS2_KC_C]='C';
 ps2_to_ascii[PS2_KC_D]='D';
 ps2_to_ascii[PS2_KC_E]='E';
 ps2_to_ascii[PS2_KC_F]='F';
 ps2_to_ascii[PS2_KC_G]='G';
 ps2_to_ascii[PS2_KC_H]='H';
 ps2_to_ascii[PS2_KC_I]='I';
 ps2_to_ascii[PS2_KC_J]='J';
 ps2_to_ascii[PS2_KC_K]='K';
 ps2_to_ascii[PS2_KC_L]='L'; 
 ps2_to_ascii[PS2_KC_M]='M';
 ps2_to_ascii[PS2_KC_N]='N';
 ps2_to_ascii[PS2_KC_O]='O';
 ps2_to_ascii[PS2_KC_P]='P';
 ps2_to_ascii[PS2_KC_Q]='Q';
 ps2_to_ascii[PS2_KC_R]='R';
 ps2_to_ascii[PS2_KC_S]='S';
 ps2_to_ascii[PS2_KC_T]='T';
 ps2_to_ascii[PS2_KC_U]='U';
 ps2_to_ascii[PS2_KC_V]='V';
 ps2_to_ascii[PS2_KC_W]='W';
 ps2_to_ascii[PS2_KC_X]='X';
 ps2_to_ascii[PS2_KC_Y]='Y';
 ps2_to_ascii[PS2_KC_Z]='Z';

 ps2_to_ascii[PS2_KC_SPACE]=' ';

 ps2_to_ascii[PS2_KC_0]='0';
 ps2_to_ascii[PS2_KC_1]='1';
 ps2_to_ascii[PS2_KC_2]='2';
 ps2_to_ascii[PS2_KC_3]='3';
 ps2_to_ascii[PS2_KC_4]='4';
 ps2_to_ascii[PS2_KC_5]='5';
 ps2_to_ascii[PS2_KC_6]='6';
 ps2_to_ascii[PS2_KC_7]='7';
 ps2_to_ascii[PS2_KC_8]='8';
 ps2_to_ascii[PS2_KC_9]='9';

 ps2_to_ascii[PS2_KC_ENTER]= 'r';

 ps2_to_ascii[PS2_KC_SEMI]=';';
 ps2_to_ascii[PS2_KC_COMMA]=',';
 ps2_to_ascii[PS2_KC_EQUAL]='=';
 ps2_to_ascii[PS2_KC_DOT]='.';
 ps2_to_ascii[PS2_KEY_KP_DIV]='/';

 ps2_to_ascii[PS2_KC_L_SHIFT]='s';
 ps2_to_ascii[PS2_KC_R_SHIFT]='s';
 ps2_to_ascii[PS2_KC_CTRL]='c';
 ps2_to_ascii[KEY_ALT_GR]='a';
 ps2_to_ascii[PS2_KC_DOT]='.';
 ps2_to_ascii[PS2_KC_TAB]='t';
 ps2_to_ascii[KEY_DELETE]='d';
 ps2_to_ascii[KEY_BACKSPACE]='b';
 ps2_to_ascii[KEY_ESC]='e';
 ps2_to_ascii[KEY_CURSOR_LEFT]='l';
 ps2_to_ascii[KEY_CURSOR_DOWN]='d';
 ps2_to_ascii[KEY_CURSOR_RIGHT]='r';
 ps2_to_ascii[KEY_CURSOR_UP]='u';
 ps2_to_ascii[PS2_KC_SPACE]=' ';
 
 
 ps2_to_ascii[PS2_KC_F1]='1';
 ps2_to_ascii[KEY_F2]='2';
 ps2_to_ascii[KEY_F3]='3';
 ps2_to_ascii[KEY_F4]='4';
 ps2_to_ascii[KEY_F5]='5';
 ps2_to_ascii[KEY_F6]='6';
 ps2_to_ascii[KEY_F7]='7';
 ps2_to_ascii[KEY_F8]='8';
 ps2_to_ascii[KEY_F9]='9';
 ps2_to_ascii[KEY_F10]='0';
 ps2_to_ascii[KEY_F11]='1';
 ps2_to_ascii[KEY_F12]='2';

 ps2_to_ascii[PS2_KC_KP_PLUS]='+';
 ps2_to_ascii[PS2_KC_KP_MINUS]='-';
 ps2_to_ascii[PS2_KC_KP_TIMES]='*';
}

void DumpPs2ToASCII()
{
 unsigned char dato; 
 unsigned char contLinea=0;
 char cadOut[90];
 unsigned char contCad=0;
 unsigned char statusOverflow= checkKey(0); 
 
 isDelayOverflow= 0;
  
 if (statusOverflow==1)
 {
  ClearOverFlow();
  Serial.println("Keyboard buffer overflow");
  isDelayOverflow=1;  
 }
 
 for (unsigned short int i=0; i<240;i++)
 {
  dato= checkKey(i);  
  //Serial.printf("%c", (dato==1)? ps2_to_ascii[i]: '.');
  cadOut[contCad]= (dato==1)? ps2_to_ascii[i]: '.';  
  contCad++;
  cadOut[contCad]='\0';

  contLinea++;
  if (contLinea>=maxLinea)
  {
   contLinea=0;
   //Serial.printf("\r\n");
   cadOut[contCad]= '\0';
   contCad=0;
   Serial.printf("%s\r\n", cadOut);
  }  
 }
}

void DumpMapKey()
{
 unsigned char dato; 
 unsigned char contLinea=0;  
 unsigned char contCad=0;
 char cadOut[90];
   
 for (unsigned short int i=0; i<240;i++)
 {
  dato= checkKey(i);
  cadOut[contCad]= dato ? '1':'0';
  contCad++;
  cadOut[contCad]='\0';
  //Serial.printf("%d",dato);

  contLinea++;
  if (contLinea>=maxLinea)
  {
   contLinea=0;
   //Serial.printf("\r\n");
   cadOut[contCad]= '\0';
   contCad=0;
   Serial.printf("%s\r\n", cadOut);   
  }  
 }
 }

void DumpTeclado()
{
 //Clear putty BEGIN
 Serial.write(27);       // ESC command
 Serial.print("[2J");    // clear screen command
 Serial.write(27);
 Serial.print("[H");     // cursor to home command
 //Clear putty END

 //Clear terminal Arduino IDE BEGIN 
 #ifdef PS2_DIAGNOSTIC_ECHO
  for (unsigned short i=0;i<30;i++)
 #else
  for (unsigned short i=0;i<26;i++)
 #endif
 {
  Serial.printf("\r\n");
 }
 //Clear terminal Arduino IDE END

 #ifdef PS2_DIAGNOSTIC_ECHO
  DumpEchoReq();
 #endif 

 DumpPs2ToASCII();
 DumpMapKey();

 if (isDelayOverflow==1)
 {
  delay(500);
 }
}

void setup() 
{
 gb_setup_end=0;
 Serial.begin(115200);

 memset(key_now,0,240);
 memset(key_prev,0,240);
 InitPs2ToASCII();

 #ifdef PS2_DIAGNOSTIC_ECHO
  gb_keyboard_echo_req= kb_begin();
 #else
  kb_begin();
 #endif 
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

   if ((KeyChg()==1)||(cont_run==0))
   {
    cont_run=1;
    DumpTeclado();
    DoEvent();
   }
  }
 }
}
