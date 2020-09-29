//*****************************************************************************
//*  MOUSE & KEYBOARD EMULATOR SWITCH (SLAVE Arduino Leonardo pro micro)
//*  beta version
//*  by Nathalis [10-09-2020]
//******************************************************************************

bool debug = false;

#define KEY_PRNT_SCRN 206 //if not defined in library
#define KEY_PAUSE (76+136)

#include <Keyboard.h>
#include <Mouse.h>

uint8_t VALUE1 = ' ';
uint8_t VALUE2 = ' ';
uint8_t VALUE3 = ' ';
uint8_t VALUE4 = ' ';
uint8_t VALUE5 = ' ';
uint8_t VALUE6 = ' ';
uint8_t VALUE7 = ' ';

byte tmp=0;

//--------------------------------------------------------------------------------
void loadKEYs() {
   VALUE1 = ' ';
   VALUE2 = 0;
   VALUE3 = 0;
   VALUE4 = 0;
   VALUE5 = 0;
   VALUE6 = ' ';
   VALUE7 = 0;
  
   uint8_t data[4]={0}; //Serial data buffer

   tmp=0;
   while (!Serial1.available()) {delayMicroseconds(100);} //wait for Serial ready
   while (Serial1.available() && tmp<4 ) {
      data[tmp]=Serial1.read();
      delayMicroseconds(100); //this is necessary
    
      if (debug) Serial.print(data[tmp],DEC);
      if (debug) Serial.print(".");
      tmp++;   
   }
   //linux FIX:
   while (Serial1.available() ) {
      Serial1.read();
      Serial1.flush();
      delayMicroseconds(100);
   }
   Serial1.flush();

//--------------------------------------------------------------------------------
//DECODE DATA FROM SERIAL
 
   //Z scroll
   VALUE5=0;
   if ((data[0]&0b11000000)==0b11000000) {VALUE1='M';} //nothing but mouse command will call
   else if ((data[0]&0b10000000)!=0) VALUE5=255; 
   else if ((data[0]&0b01000000)!=0) VALUE5=1; 

   //Mouse clicks
   VALUE2=((data[0]>>3)&0x07);

   //Keyboard Command
   VALUE6=' ';
   if ((data[0]&0b00000111)==1) VALUE6='R';
   else if ((data[0]&0b00000111)==2) VALUE6='D';
   else if ((data[0]&0b00000111)==3) VALUE6='U';
   else if ((data[0]&0b00000111)==4) VALUE6='C';
   else if ((data[0]&0b00000111)==5) VALUE6='S';
   else if ((data[0]&0b00000111)==6) VALUE6='A';
   else if ((data[0]&0b00000111)==7) VALUE6='G';

   //Mouse X
   VALUE3=data[1];
   //Mouse Y
   VALUE4=data[2];
   
   //Keyboard KEY
   VALUE7=data[3];

   //if mouse data loaded then set Mouse Command
   if (VALUE2 || VALUE3 || VALUE4 || VALUE5) VALUE1='M';
    
}
//*****************************************************************************
//*  MAIN SETUP
//*****************************************************************************

void setup() {
   // open the serial port:
   if (debug) Serial.begin(115200); //for console
   Serial1.begin(115200); //for input

   //emulate Keyboard and Mouse
   Keyboard.begin();
   Mouse.begin();
}
//*****************************************************************************
//*  MAIN LOOP
//*****************************************************************************

bool CAPSLOCK = false;
bool NUMLOCK = true;
bool NUMMULTIPLY=false;

void loop() {
   loadKEYs(); //read keyboard/mouse codes from Serial1.
    
   if (debug) {
      Serial.print((char)VALUE1);  //Print codes to console...
      Serial.print(" ");
      Serial.print(VALUE2, HEX);
      Serial.print(" ");
      Serial.print(VALUE3, HEX);
      Serial.print(" ");
      Serial.print(VALUE4, HEX);
      Serial.print(" ");
      Serial.print(VALUE5, HEX);
      Serial.print(" ");
      Serial.print((char)VALUE6);
      Serial.print(" ");
      Serial.print(VALUE7, HEX);
      Serial.println(" ");
   }

//*****************************************************************************
//*  MOUSE
//*****************************************************************************

   if (VALUE1 == 'M') {
      Mouse.move(VALUE3, VALUE4, VALUE5);
  
      if (VALUE1 == 'M' && VALUE2 == 0) {
         Mouse.release(MOUSE_LEFT);
         Mouse.release(MOUSE_RIGHT);
         Mouse.release(MOUSE_MIDDLE);
      }
      if (VALUE1 == 'M' && (VALUE2&1) != 0) {
         Mouse.press(MOUSE_LEFT);
      } else if (VALUE1 == 'M' && (VALUE2&2) == 0) {
         Mouse.release(MOUSE_LEFT);
      }
      if (VALUE1 == 'M' && (VALUE2&2) != 0) {
         Mouse.press(MOUSE_RIGHT);
      } else if (VALUE1 == 'M' && (VALUE2&2) == 0) {
         Mouse.release(MOUSE_RIGHT);
      }
      if (VALUE1 == 'M' && (VALUE2&4) != 0) {
         Mouse.press(MOUSE_MIDDLE);
      } else if (VALUE1 == 'M' && (VALUE2&4) == 0) {
         Mouse.release(MOUSE_MIDDLE);
      }  
   }

//*****************************************************************************
//*  KEYBOARD
//*****************************************************************************

   if (VALUE6=='R') Keyboard.releaseAll();

   if (VALUE6!=' ') {
      if (VALUE6 == 'D' && VALUE7 == 0x53) {
         if (NUMLOCK) NUMLOCK = false;
         else NUMLOCK = true;
         Keyboard.press(83+136 );
      } else if (VALUE6 == 'U' && VALUE7 == 0x53) {
         Keyboard.release(83+136 );
      }
      if (VALUE6 == 'D' && VALUE7 == 0x39) {
         if (CAPSLOCK) CAPSLOCK = false;
         else CAPSLOCK = true;
         Keyboard.press(57+136 );
      } else if (VALUE6 == 'U' && VALUE7 == 0x39) {
         Keyboard.release(57+136 );
      }

      //a..z /// not need capslock detection 
      if (VALUE6 == 'D' && VALUE7 >= 4  && VALUE7 <= 0x1d) {
         Keyboard.press('a' - 4 + VALUE7);
      }
      if (VALUE6 == 'U' && VALUE7 >= 4  && VALUE7 <= 0x1d ) {
         Keyboard.release('a' - 4 + VALUE7 );
      }

      //~-=[];'\,./
      if (VALUE6 == 'D' && VALUE7 == 0x35) {
         Keyboard.press('`');
      }
      if (VALUE6 == 'U' && VALUE7 == 0x35) {
         Keyboard.release('`');
      }
      if (VALUE6 == 'D' && VALUE7 == 0x2D) {
         Keyboard.press('-');
      }
      if (VALUE6 == 'U' && VALUE7 == 0x2D) {
         Keyboard.release('-');
      }
      if (VALUE6 == 'D' && VALUE7 == 0x2E) {
         Keyboard.press('=');
      }
      if (VALUE6 == 'U' && VALUE7 == 0x2E) {
         Keyboard.release('=');
      }
      if (VALUE6 == 'D' && VALUE7 == 0x2F) {
         Keyboard.press('[');
      }
      if (VALUE6 == 'U' && VALUE7 == 0x2F) {
         Keyboard.release('[');
      }
      if (VALUE6 == 'D' && VALUE7 == 0x30) {
         Keyboard.press(']');
      }
      if (VALUE6 == 'U' && VALUE7 == 0x30) {
         Keyboard.release(']');
      }
      if (VALUE6 == 'D' && VALUE7 == 0x33) {
         Keyboard.press(';');
      }
      if (VALUE6 == 'U' && VALUE7 == 0x33) {
         Keyboard.release(';');
      }
      if (VALUE6 == 'D' && VALUE7 == 0x34) {
         Keyboard.press('\'');
      }
      if (VALUE6 == 'U' && VALUE7 == 0x34) {
         Keyboard.release('\'');
      }
      if (VALUE6 == 'D' && VALUE7 == 0x32) {
         Keyboard.press('\\');
      }
      if (VALUE6 == 'U' && VALUE7 == 0x32) {
         Keyboard.release('\\');
      }
      if (VALUE6 == 'D' && VALUE7 == 0x36) {
         Keyboard.press(',');
      }
      if (VALUE6 == 'U' && VALUE7 == 0x36) {
         Keyboard.release(',');
      }
      if (VALUE6 == 'D' && VALUE7 == 0x37) {
         Keyboard.press('.');
      }
      if (VALUE6 == 'U' && VALUE7 == 0x37) {
         Keyboard.release('.');
      }
      if (VALUE6 == 'D' && VALUE7 == 0x38) {
         Keyboard.press('/');
      }
      if (VALUE6 == 'U' && VALUE7 == 0x38) {
         Keyboard.release('/');
      }

      //NUMBERS 0 (KEYPAD)
      if (VALUE6 == 'D' && VALUE7 == 0x27) {
         Keyboard.press(48);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x27) {
         Keyboard.release(48);
      }
      //NUMBERS 1..9 (KEYPAD)
      if (VALUE6 == 'D' && VALUE7 >= 0x1e  && VALUE7 <= 0x26) {
         Keyboard.press(VALUE7 + 0x13);
      }
      if (VALUE6 == 'U' && VALUE7 >= 0x1e  && VALUE7 <= 0x26 ) {
         Keyboard.release(VALUE7 + 0x13);
      }

      //NUMBERS 0..9 (NUMPAD)
      if (NUMLOCK == true && VALUE6 == 'D' && VALUE7 >= 0x59 && VALUE7 <= 0x61) {
         ///Keyboard.press(VALUE7 - 0x28);
         Keyboard.press(VALUE7 + 136);
      }
      if (NUMLOCK == true && VALUE6 == 'U' && VALUE7 >= 0x59 && VALUE7 <= 0x61) {
         ///Keyboard.release(VALUE7 - 0x28);
         Keyboard.release(VALUE7 + 136);
      }
      if (NUMLOCK == true && VALUE6 == 'D' && VALUE7 == 0x62) {
         ///Keyboard.press('0');
         Keyboard.press(VALUE7 + 136);
      }
      if (NUMLOCK == true && VALUE6 == 'U' && VALUE7 == 0x62) {
         ///Keyboard.release('0');
         Keyboard.press(VALUE7 + 136);
      }

      //numpad no numlock
      if (NUMLOCK == false && VALUE6 == 'D' && VALUE7 == 0x62) {
         Keyboard.press(KEY_INSERT);
      }
      if (NUMLOCK == false && VALUE6 == 'U' && VALUE7 == 0x62) {
         Keyboard.release(KEY_INSERT);
      }
      if (NUMLOCK == false && VALUE6 == 'D' && VALUE7 == 0x59) {
         Keyboard.press(KEY_END);
      }
      if (NUMLOCK == false && VALUE6 == 'U' && VALUE7 == 0x59) {
         Keyboard.release(KEY_END);
      }
      if (NUMLOCK == false && VALUE6 == 'D' && VALUE7 == 0x5A) {
         Keyboard.press(KEY_DOWN_ARROW);
      }
      if (NUMLOCK == false && VALUE6 == 'U' && VALUE7 == 0x5A) {
         Keyboard.release(KEY_DOWN_ARROW);
      }
      if (NUMLOCK == false && VALUE6 == 'D' && VALUE7 == 0x5B) {
         Keyboard.press(KEY_PAGE_DOWN);
      }
      if (NUMLOCK == false && VALUE6 == 'U' && VALUE7 == 0x5B) {
         Keyboard.release(KEY_PAGE_DOWN);
      }
      if (NUMLOCK == false && VALUE6 == 'D' && VALUE7 == 0x5C) {
         Keyboard.press(KEY_LEFT_ARROW);
      }
      if (NUMLOCK == false && VALUE6 == 'U' && VALUE7 == 0x5C) {
         Keyboard.release(KEY_LEFT_ARROW);
      }

      if (NUMLOCK == false && VALUE6 == 'D' && VALUE7 == 0x5E) {
         Keyboard.press(KEY_RIGHT_ARROW);
      }
      if (NUMLOCK == false && VALUE6 == 'U' && VALUE7 == 0x5E) {
         Keyboard.release(KEY_RIGHT_ARROW);
      }
      if (NUMLOCK == false && VALUE6 == 'D' && VALUE7 == 0x5F) {
         Keyboard.press(KEY_HOME);
      }
      if (NUMLOCK == false && VALUE6 == 'U' && VALUE7 == 0x5F) {
         Keyboard.release(KEY_HOME);
      }
      if (NUMLOCK == false && VALUE6 == 'D' && VALUE7 == 0x60) {
         Keyboard.press(KEY_UP_ARROW);
      }
      if (NUMLOCK == false && VALUE6 == 'U' && VALUE7 == 0x60) {
         Keyboard.release(KEY_UP_ARROW);
      }
      if (NUMLOCK == false && VALUE6 == 'D' && VALUE7 == 0x61) {
         Keyboard.press(KEY_PAGE_DOWN);
      }
      if (NUMLOCK == false && VALUE6 == 'U' && VALUE7 == 0x61) {
         Keyboard.release(KEY_PAGE_DOWN);
      }

      if (VALUE6 == 'D' && VALUE7 == 0x63) {
         if (NUMMULTIPLY) {
            NUMMULTIPLY=false;
            Keyboard.release('*'); // Unexpected chars fix...
         }
         Keyboard.press('.');
      }
      if (VALUE6 == 'U' && VALUE7 == 0x63) {
         Keyboard.release('.');
      }
      if (VALUE6 == 'D' && VALUE7 == 0x57) {
         if (NUMMULTIPLY) {
            NUMMULTIPLY=false;
            Keyboard.release('*'); // Unexpected chars fix...
         }
         Keyboard.press('+');
      }
      if (VALUE6 == 'U' && VALUE7 == 0x57) {
         Keyboard.release('+');
      }
      if (VALUE6 == 'D' && VALUE7 == 0x56) {
         if (NUMMULTIPLY) {
            NUMMULTIPLY=false;
            Keyboard.release('*'); // Unexpected chars fix...
         }
         Keyboard.press('-');
      }
      if (VALUE6 == 'U' && VALUE7 == 0x56) {
         Keyboard.release('-');
      }
      if (VALUE6 == 'D' && VALUE7 == 0x55) {
         Keyboard.press('*');
         NUMMULTIPLY=true;  
      }
      if (VALUE6 == 'U' && VALUE7 == 0x55) {
         if (NUMMULTIPLY) Keyboard.release('*');
         NUMMULTIPLY=false;
      }
      if (VALUE6 == 'D' && VALUE7 == 0x54) {
         if (NUMMULTIPLY) {
            NUMMULTIPLY=false;
            Keyboard.release('*'); // Unexpected chars fix...
         }
         Keyboard.press('/');
      }
      if (VALUE6 == 'U' && VALUE7 == 0x54) {
         Keyboard.release('/');
      }

      //ctrl
      if (VALUE6 == 'C' && VALUE7 == 1) {
         Keyboard.press(KEY_LEFT_CTRL);
      }
      if (VALUE6 == 'C' && VALUE7 == 0) {
         Keyboard.release(KEY_LEFT_CTRL);
      }
      if (VALUE6 == 'C' && VALUE7 == 3) {
         Keyboard.press(KEY_RIGHT_CTRL);
      }
      if (VALUE6 == 'C' && VALUE7 == 2) {
         Keyboard.release(KEY_RIGHT_CTRL);
      }

      //shift
      if (VALUE6 == 'S' && VALUE7 == 1) {
         Keyboard.press(KEY_LEFT_SHIFT);
      }
      if (VALUE6 == 'S' && VALUE7 == 0) {
         Keyboard.release(KEY_LEFT_SHIFT);
      }
      if (VALUE6 == 'S' && VALUE7 == 3) {
         Keyboard.press(KEY_RIGHT_SHIFT);
      }
      if (VALUE6 == 'S' && VALUE7 == 2) {
         Keyboard.release(KEY_RIGHT_SHIFT);
      }

      //alt
      if (VALUE6 == 'A' && VALUE7 == 1) {
         Keyboard.press(KEY_LEFT_ALT);
      }
      if (VALUE6 == 'A' && VALUE7 == 0) {
         Keyboard.release(KEY_LEFT_ALT);
      }
      if (VALUE6 == 'A' && VALUE7 == 3) {
         Keyboard.press(KEY_RIGHT_ALT);
      }
      if (VALUE6 == 'A' && VALUE7 == 2) {
         Keyboard.release(KEY_RIGHT_ALT);
      }

      //GUI
      if (VALUE6 == 'G' && VALUE7 == 1) {
         Keyboard.press(KEY_LEFT_GUI);
      }
      if (VALUE6 == 'G' && VALUE7 == 0) {
         Keyboard.release(KEY_LEFT_GUI);
      }
      if (VALUE6 == 'G' && VALUE7 == 3) {
         Keyboard.press(KEY_RIGHT_GUI);
      }
      if (VALUE6 == 'G' && VALUE7 == 2) {
         Keyboard.release(KEY_RIGHT_GUI);
      }

      //ENTER
      if (VALUE6 == 'D' && VALUE7 == 0x28) {
         Keyboard.press(KEY_RETURN);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x28) {
         Keyboard.release(KEY_RETURN);
      }
      if (VALUE6 == 'D' && VALUE7 == 0x58) {
         Keyboard.press(KEY_RETURN);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x58) {
         Keyboard.release(KEY_RETURN);
      }

      //spacebar
      if (VALUE6 == 'D' && VALUE7 == 0x2C) {
         Keyboard.press(' ');
      }
      if (VALUE6 == 'U' && VALUE7 == 0x2C) {
         Keyboard.release(' ');
      }
      //tab
      if (VALUE6 == 'D' && VALUE7 == 0x2B) {
         Keyboard.press(KEY_TAB);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x2B) {
         Keyboard.release(KEY_TAB);
      }
      //backspace
      if (VALUE6 == 'D' && VALUE7 == 0x2A) {
         Keyboard.press(KEY_BACKSPACE);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x2A) {
         Keyboard.release(KEY_BACKSPACE);
      }
      //esc
      if (VALUE6 == 'D' && VALUE7 == 0x29) {
         Keyboard.press(KEY_ESC);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x29) {
         Keyboard.release(KEY_ESC);
      }

      //------------------------
      //keypad LEFT
      if (VALUE6 == 'D' && VALUE7 == 0x50) {
         Keyboard.press(KEY_LEFT_ARROW);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x50) {
         Keyboard.release(KEY_LEFT_ARROW);
      }
      //keypad RIGHT
      if (VALUE6 == 'D' && VALUE7 == 0x4F) {
         Keyboard.press(KEY_RIGHT_ARROW);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x4F) {
         Keyboard.release(KEY_RIGHT_ARROW);
      }
      //keypad UP
      if (VALUE6 == 'D' && VALUE7 == 0x52) {
         Keyboard.press(KEY_UP_ARROW);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x52) {
         Keyboard.release(KEY_UP_ARROW);
      }
      //keypad DOWN
      if (VALUE6 == 'D' && VALUE7 == 0x51) {
         Keyboard.press(KEY_DOWN_ARROW);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x51) {
         Keyboard.release(KEY_DOWN_ARROW);
      }

      //keypad HOME
      if (VALUE6 == 'D' && VALUE7 == 0x4A) {
         Keyboard.press(KEY_HOME);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x4A) {
         Keyboard.release(KEY_HOME);
      }
      //keypad END
      if (VALUE6 == 'D' && VALUE7 == 0x4D) {
         Keyboard.press(KEY_END);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x4D) {
         Keyboard.release(KEY_END);
      }
      //keypad PAGE UP
      if (VALUE6 == 'D' && VALUE7 == 0x4B) {
         Keyboard.press(KEY_PAGE_UP);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x4B) {
         Keyboard.release(KEY_PAGE_UP);
      }
      //keypad PAGE DOWN
      if (VALUE6 == 'D' && VALUE7 == 0x4E) {
         Keyboard.press(KEY_PAGE_DOWN);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x4E) {
         Keyboard.release(KEY_PAGE_DOWN);
      }
      //keypad INSERT
      if (VALUE6 == 'D' && VALUE7 == 0x49) {
         Keyboard.press(KEY_INSERT);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x49) {
         Keyboard.release(KEY_INSERT);
      }
      //keypad DELETE
      if (VALUE6 == 'D' && VALUE7 == 0x4C) {
         Keyboard.press(KEY_DELETE);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x4C) {
         Keyboard.release(KEY_DELETE);
      }


      if (VALUE6 == 'D' && VALUE7 == 0x3A) {
         Keyboard.press(KEY_F1);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x3A) {
         Keyboard.release(KEY_F1);
      }
      if (VALUE6 == 'D' && VALUE7 == 0x3B) {
         Keyboard.press(KEY_F2);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x3B) {
         Keyboard.release(KEY_F2);
      }
      if (VALUE6 == 'D' && VALUE7 == 0x3C) {
         Keyboard.press(KEY_F3);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x3C) {
         Keyboard.release(KEY_F3);
      }
      if (VALUE6 == 'D' && VALUE7 == 0x3D) {
         Keyboard.press(KEY_F4);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x3D) {
         Keyboard.release(KEY_F4);
      }
      if (VALUE6 == 'D' && VALUE7 == 0x3E) {
         Keyboard.press(KEY_F5);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x3E) {
         Keyboard.release(KEY_F5);
      }
      if (VALUE6 == 'D' && VALUE7 == 0x3F) {
         Keyboard.press(KEY_F6);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x3F) {
         Keyboard.release(KEY_F6);
      }
      if (VALUE6 == 'D' && VALUE7 == 0x40) {
         Keyboard.press(KEY_F7);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x40) {
         Keyboard.release(KEY_F7);
      }
      if (VALUE6 == 'D' && VALUE7 == 0x41) {
         Keyboard.press(KEY_F8);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x41) {
         Keyboard.release(KEY_F8);
      }
      if (VALUE6 == 'D' && VALUE7 == 0x42) {
         Keyboard.press(KEY_F9);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x42) {
         Keyboard.release(KEY_F9);
      }
      if (VALUE6 == 'D' && VALUE7 == 0x43) {
         Keyboard.press(KEY_F10);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x43) {
         Keyboard.release(KEY_F10);
      }
      if (VALUE6 == 'D' && VALUE7 == 0x44) {
         Keyboard.press(KEY_F11);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x44) {
         Keyboard.release(KEY_F11);
      }
      if (VALUE6 == 'D' && VALUE7 == 0x45) {
         Keyboard.press(KEY_F12);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x45) {
         Keyboard.release(KEY_F12);
      }

      //print screen
      if (VALUE6 == 'D' && VALUE7 == 0x46) {
         Keyboard.press(KEY_PRNT_SCRN);
      }
      if (VALUE6 == 'U' && VALUE7 == 0x46) {
         Keyboard.release(KEY_PRNT_SCRN);
      }
      //pause break
      if (VALUE6 == 'D' && VALUE7 == 0x48) {
         Keyboard.press(72+136); //corrected value
      }
      if (VALUE6 == 'U' && VALUE7 == 0x48) {
         Keyboard.release(72+136);
      }  
      //ContexMenu button
      if (VALUE6 == 'D' && VALUE7 == 101) {
         Keyboard.press(101+136); //corrected value
      }
      if (VALUE6 == 'U' && VALUE7 == 101) {
         Keyboard.release(101+136);
      }  
   }
}
