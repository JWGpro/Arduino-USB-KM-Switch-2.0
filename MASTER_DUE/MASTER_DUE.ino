//********************************************************************************
//*                                                                              *
//* Arduino KM (Keyboard+Mouse) Switch version 2.0 is now using                  *
//* USB Host Shield MAX3421E, with UHS30 Library using Arduino DUE               *
//* support 4 devices in 1 hub                                                   *
//*                                                                              *
//********************************************************************************

// Load the USB Host System core
#define LOAD_USB_HOST_SYSTEM
// Load USB Host Shield
#define LOAD_USB_HOST_SHIELD
// Use USB hub, you might want this for multiple devices.
#define LOAD_UHS_HUB

// Patch printf so we can use it.
#define LOAD_UHS_PRINTF_HELPER
#define DEBUG_PRINTF_EXTRA_HUGE 0
#define DEBUG_PRINTF_EXTRA_HUGE_UHS_HOST 0
#define DEBUG_PRINTF_EXTRA_HUGE_USB_HID 0
#define DEBUG_PRINTF_EXTRA_HUGE_USB_HUB  0

#define LOAD_UHS_HID
/*#define LOAD_UHS_HID
#define LOAD_UHS_HIDRAWBOOT_KEYBOARD
#define LOAD_UHS_HIDRAWBOOT_MOUSE*/

#define UHS_DEVICE_WINDOWS_USB_SPEC_VIOLATION_DESCRIPTOR_DEVICE 0

#include <Arduino.h>
#ifdef true
#undef true
#endif
#ifdef false
#undef false
#endif

#include <UHS_host.h>

//--------------------------------------------------------------------------------
// Use the Arduino core to set-up the unused USART2 on Serial4
RingBuffer rx_buffer5;
RingBuffer tx_buffer5;
USARTClass Serial4(USART2, USART2_IRQn, ID_USART2, &rx_buffer5, &tx_buffer5);
//void serialEvent4() __attribute__((weak));
//void serialEvent4() { }

void USART2_Handler(void)   // Interrupt handler for UART2
{
  Serial4.IrqHandler();     // In turn calls on the Serial4 interrupt handler
}
//--------------------------------------------------------------------------------

//********************************************************************************

//WARNING: enabled DEBUG can cause mouse sensitivity problem!
bool debug=false; 
bool debugextra=false; 

bool BOOTNUMLOCK=true; 

uint32_t BOOTNUMLOCKtime=0;
bool CAPSLOCKCHANGE=false;
bool NUMLOCKCHANGE=false;
bool CAPSLOCK=false;
bool NUMLOCK=false;

uint8_t SCROLLLOCK=1;

uint8_t VALUE1 = ' '; //MOUSE CMD
uint8_t VALUE2 = 0; //CLICK
uint8_t VALUE3 = 0; //X pos
uint8_t VALUE4 = 0; //Y pos
uint8_t VALUE5 = 0; //Z scroll
uint8_t VALUE6 = ' '; //KEY CMD
uint8_t VALUE7 = 0; //KEY VALUE

bool SHIFT_LEFT_ACTIVE=false;
bool SHIFT_RIGHT_ACTIVE=false;
bool CTRL_LEFT_ACTIVE=false;
bool CTRL_RIGHT_ACTIVE=false;
bool ALT_LEFT_ACTIVE=false;
bool ALT_RIGHT_ACTIVE=false;
bool GUI_LEFT_ACTIVE=false;
bool GUI_RIGHT_ACTIVE=false;

uint8_t KEYBUF[6]={0};
//********************************************************************************
UHS_HID_base *ext_d;



uint8_t leds=0; //Keyboard LEDS inverted 0bit = num 1bit= caps

class myHID_processor : public UHS_HID_PROCESSOR {
   public:
      myHID_processor(void) {}

      void onRelease(UHS_HID_base *d) {
         if (debugextra) printf_P(PSTR("HID driver type %d no longer available.\r\n"), d->driver);
      }
      void onStart(UHS_HID_base *d) {
         if (debugextra) printf_P(PSTR("HID driver type %d started, Subclass %02x, Protocol %02x\r\n"), d->driver, d->parent->bSubClass, d->parent->bProtocol);
//--------------------------------------------------------------------------------
         if (d->parent->bProtocol==1) ext_d=d; //for bootup numlock
//--------------------------------------------------------------------------------
      }
      void onPoll(UHS_HID_base *d, uint8_t *data, uint16_t length) {
         switch(d->driver) {
            case UHS_HID_raw:
               if (debugextra) printf_P(PSTR("RAW input %d bytes interface %d, Subclass %02x, Protocol %02x Data:"), length, d->parent->bIface, d->parent->bSubClass, d->parent->bProtocol);
               for (int i=0; i < length; i++) {
                  if (debugextra) printf_P(PSTR(" %02x"), data[i]);
               }
               if (debugextra) printf_P(PSTR("\r\n"));
//********************************************************************************
               if (CAPSLOCKCHANGE) {
                  if (CAPSLOCK) {
                     leds|=2;
                     d->parent->SetReport(d->parent->bIface, 2, 0, 1, &leds); 
                     if (debug) Serial.println("CAPSOCK ON");
                  }  
                  if (!CAPSLOCK) {
                     leds&=253;
                     d->parent->SetReport(d->parent->bIface, 2, 0, 1, &leds); 
                     if (debug) Serial.println("CAPSOCK OFF");
                  }   
                  CAPSLOCKCHANGE=false;
               }
//................................................................................
               if (NUMLOCKCHANGE) {
                  if (NUMLOCK) {
                     leds|=1;
                     d->parent->SetReport(d->parent->bIface, 2, 0, 1, &leds); 
                     if (debug) Serial.println("NUMLOCK ON");     
                  }  
                  if (!NUMLOCK) {
                     leds&=254;
                     d->parent->SetReport(d->parent->bIface, 2, 0, 1, &leds); 
                     if (debug) Serial.println("NUMLOCK OFF");     
                  }   
                  NUMLOCKCHANGE=false;
               }
//********************************************************************************
               if (d->parent->bProtocol==1) { //KEYBOARD
                  if ((data[0]&1)!=0) {
                     if (!CTRL_LEFT_ACTIVE) { VALUE6='C';  VALUE7=(char)1; }
                     CTRL_LEFT_ACTIVE=true; 
                  } else {
                     if (CTRL_LEFT_ACTIVE) {VALUE6='C';  VALUE7=(char)0;}
                     CTRL_LEFT_ACTIVE=false;
                  }
                  if ((data[0]&2)!=0) {
                     if (!SHIFT_LEFT_ACTIVE) {VALUE6='S';  VALUE7=(char)1;}
                     SHIFT_LEFT_ACTIVE=true; 
                  } else {
                     if (SHIFT_LEFT_ACTIVE) {VALUE6='S';  VALUE7=(char)0;}
                     SHIFT_LEFT_ACTIVE=false;
                  }
                  if ((data[0]&4)!=0) {
                     if (!ALT_LEFT_ACTIVE) {VALUE6='A';  VALUE7=(char)1;}
                     ALT_LEFT_ACTIVE=true; 
                  } else {
                     if (ALT_LEFT_ACTIVE) {VALUE6='A';  VALUE7=(char)0;}
                     ALT_LEFT_ACTIVE=false;
                  }
                  if ((data[0]&8)!=0) {
                     if (!GUI_LEFT_ACTIVE) {VALUE6='G';  VALUE7=(char)1;}
                     GUI_LEFT_ACTIVE=true; 
                  } else {
                     if (GUI_LEFT_ACTIVE) {VALUE6='G';  VALUE7=(char)0;}
                     GUI_LEFT_ACTIVE=false;
                  }
                  if ((data[0]&16)!=0) {
                     if (!CTRL_RIGHT_ACTIVE) {VALUE6='C';  VALUE7=(char)3;}
                     CTRL_RIGHT_ACTIVE=true; 
                  } else {
                     if (CTRL_RIGHT_ACTIVE) {VALUE6='C';  VALUE7=(char)2;}
                     CTRL_RIGHT_ACTIVE=false;
                  }
                  if ((data[0]&32)!=0) {
                     if (!SHIFT_RIGHT_ACTIVE) {VALUE6='S';  VALUE7=(char)3;}
                     SHIFT_RIGHT_ACTIVE=true; 
                  } else {
                     if (SHIFT_RIGHT_ACTIVE) {VALUE6='S';  VALUE7=(char)2;}
                     SHIFT_RIGHT_ACTIVE=false;
                  }
                  if ((data[0]&64)!=0) {
                     if (!ALT_RIGHT_ACTIVE) {VALUE6='A';  VALUE7=(char)3;}
                     ALT_RIGHT_ACTIVE=true; 
                  } else {
                     if (ALT_RIGHT_ACTIVE) {VALUE6='A';  VALUE7=(char)2;}
                     ALT_RIGHT_ACTIVE=false;
                  }
                  if ((data[0]&128)!=0) {
                     if (!GUI_RIGHT_ACTIVE) {VALUE6='G';  VALUE7=(char)3;}
                     GUI_RIGHT_ACTIVE=true; 
                  } else {
                     if (GUI_RIGHT_ACTIVE) {VALUE6='G';  VALUE7=(char)2;}
                     GUI_RIGHT_ACTIVE=false;
                  } 
//--------------------------------------------------------------------------------
                  uint8_t offset=0;
                  for (uint8_t tmpd=0;tmpd<6;tmpd++) {
                     if (data[2+tmpd]!=KEYBUF[tmpd] && KEYBUF[tmpd]==0) {
                        if (debug) Serial.print("KEY SET:");
                        if (debug) Serial.println(data[2+tmpd]);
                        VALUE6='D';
                        VALUE7=data[2+tmpd];
                     }
                     if (data[2+tmpd-offset]!=KEYBUF[tmpd] && KEYBUF[tmpd]!=0) {
                        if (debug) Serial.print("KEY RELEASE:");
                        if (debug) Serial.println(KEYBUF[tmpd]);
                        offset++;
                        VALUE6='U';
                        VALUE7=KEYBUF[tmpd];          
                     }
                  }
                  for (uint8_t tmpd=0;tmpd<6;tmpd++) KEYBUF[tmpd]=data[2+tmpd]; 
                  if (data[0]==0 && data[1]==0 && data[2]==0 && data[3]==0 && data[4]==0 && data[5]==0 && data[6]==0 && data[7]==0) VALUE6='R';
               }
//********************************************************************************
               if (d->parent->bProtocol==2) { //MOUSE
  
                  if (length==4) { //Classic Mouse
                     VALUE1='M';
                     VALUE2=(char)(data[0] ); //click
                     VALUE3=(char)(data[1] ); //X
                     VALUE4=(char)(data[2] ); //Y
                     VALUE5=(char)(data[3] ); //Z (scroll)
                  }
                  if (length==7 && data[0]==1) { //Genius Navigator 335 working
                     VALUE1='M';
                     VALUE2=(char)(data[1] ); //click
                     VALUE3=(char)(data[2] ); //X
                     VALUE4=(char)(data[3] ); //Y
                     VALUE5=(char)(data[4] ); //Z (scroll)
                  }
                  if (length==9) { //Asus ROG SICA working
                     VALUE1='M';

/*                     uint8_t Xsensitivity=(uint8_t)data[1];
                     uint8_t Ysensitivity=(uint8_t)data[2];

/*                     if (Xsensitivity>0) Xsensitivity=Xsensitivity<<2;
                     else Xsensitivity=Xsensitivity>>2;
                     if (Ysensitivity>0) Ysensitivity=Ysensitivity<<2;
                     else Ysensitivity=Ysensitivity>>2;*/
/*         
                     VALUE2=(char)(data[0] ); //click
                     VALUE3=(unsigned char)(Xsensitivity ); //X
                     VALUE4=(unsigned char)(Ysensitivity ); //Y
                     VALUE5=(char)(data[3] ); //Z (scroll)       
*/                     
                     
                     VALUE2=(char)(data[0] ); //click
                     VALUE3=(char)(data[1] ); //X
                     VALUE4=(char)(data[2] ); //Y
                     VALUE5=(char)(data[3] ); //Z (scroll)
                     
                  }
               }

//********************************************************************************
               break;
            default:
               break;
         }
      }
};

//--------------------------------------------------------------------------------
/// keyboard and mouse has 2 protocols each other, 
/// but using only one protocol, 
/// but you need define 2*HID processor per device 
/// supported max 4 HID device:
//--------------------------------------------------------------------------------

myHID_processor HID_processor1;  
myHID_processor HID_processor2;
myHID_processor HID_processor3;
myHID_processor HID_processor4;
myHID_processor HID_processor5;  
myHID_processor HID_processor6;
myHID_processor HID_processor7;
myHID_processor HID_processor8;

MAX3421E_HOST UHS_Usb;

UHS_HID hid1(&UHS_Usb, &HID_processor1);
UHS_HID hid2(&UHS_Usb, &HID_processor2);
UHS_HID hid3(&UHS_Usb, &HID_processor3);
UHS_HID hid4(&UHS_Usb, &HID_processor4);
UHS_HID hid5(&UHS_Usb, &HID_processor5);
UHS_HID hid6(&UHS_Usb, &HID_processor6);
UHS_HID hid7(&UHS_Usb, &HID_processor7);
UHS_HID hid8(&UHS_Usb, &HID_processor8);

//--------------------------------------------------------------------------------
//Hub definition must be last!
UHS_USBHub hub1(&UHS_Usb);
//--------------------------------------------------------------------------------


//*****************************************************************************
//*  MAIN SETUP
//*****************************************************************************

void setup() {
   while(UHS_Usb.Init(1000) != 0);
   if (debug || debugextra) USB_HOST_SERIAL.begin(115200);

   if (debug) Serial.println( "USB Keyboard and Mouse Switch by Nathalis" );
  
   Serial1.begin(115200); //for output
   Serial2.begin(115200); //for output
   Serial3.begin(115200); //for output
   //SERIAL 4 [ A11 (TX4) and D52 (RX4) ]
   PIO_Configure(PIOB, PIO_PERIPH_A, PIO_PB20A_TXD2 | PIO_PB21A_RXD2, PIO_DEFAULT);
   Serial4.begin(115200);          
}
//********************************************************************************

int8_t lastMouseX_val=0;
int8_t lastMouseY_val=0;
int8_t XmouseDiff=0;
int8_t YmouseDiff=0;  

uint8_t stat0 = 128;
uint8_t stat1 = 128;
uint8_t stat2 = 128;
uint8_t stat3 = 128;
uint8_t stat4 = 128;
uint8_t stat5 = 128;
uint8_t stat6 = 128;
uint8_t stat7 = 128;
uint8_t stat8 = 128;


uint32_t SCROLLLOCK_millis=0;

//*****************************************************************************
//*  MAIN LOOP
//*****************************************************************************

void loop() {
   if (hub1.GetAddress() != stat0) {
      stat0 = hub1.GetAddress();
      if (debugextra) printf_P(PSTR("Hub address %u\r\n"), stat0);
   }
   if (hid1.GetAddress() != stat1) {
      stat1 = hid1.GetAddress();
      if (debug) printf_P(PSTR("hid1 address %u\r\n"), stat1);
   }
   if (hid2.GetAddress() != stat2) {
      stat2 = hid2.GetAddress();
      if (debug) printf_P(PSTR("hid2 address %u\r\n"), stat2);
   }
   if (hid3.GetAddress() != stat3) {
      stat3 = hid3.GetAddress();
      if (debug) printf_P(PSTR("hid3 address %u\r\n"), stat3);
   }
   if (hid4.GetAddress() != stat4) {
      stat4 = hid4.GetAddress();
      if (debug) printf_P(PSTR("hid4 address %u\r\n"), stat4);
   }
   if (hid5.GetAddress() != stat5) {
      stat5 = hid5.GetAddress();
      if (debug) printf_P(PSTR("hid5 address %u\r\n"), stat5);
   }
   if (hid6.GetAddress() != stat6) {
      stat6 = hid6.GetAddress();
      if (debug) printf_P(PSTR("hid6 address %u\r\n"), stat6);
   }
   if (hid7.GetAddress() != stat7) {
      stat7 = hid7.GetAddress();
      if (debug) printf_P(PSTR("hid7 address %u\r\n"), stat7);
   }
   if (hid8.GetAddress() != stat8) {
      stat8 = hid8.GetAddress();
      if (debug) printf_P(PSTR("hid8 address %u\r\n"), stat8);
   }        
//********************************************************************************
   if (BOOTNUMLOCK) {
      if (BOOTNUMLOCKtime==0) BOOTNUMLOCKtime=millis();
      if (BOOTNUMLOCKtime>0 && BOOTNUMLOCKtime+5000<millis()) { //5second delay after init
         if (ext_d->parent) {
            BOOTNUMLOCK=false;     
            leds=0b00000001;  
            ext_d->parent->SetReport(ext_d->parent->bIface, 2, 0, 1, &leds); 
            NUMLOCK=true;  
         }
      }
   }
//********************************************************************************
   //Fix mouse lags
   if (VALUE1=='M')  {

      XmouseDiff=(int)VALUE3-(int)lastMouseX_val;
      YmouseDiff=(int)VALUE4-(int)lastMouseY_val;

      if (debug) Serial.print("XmouseDiff: ");
      if (debug) Serial.println(XmouseDiff);
      if (debug) Serial.print("YmouseDiff: ");
      if (debug) Serial.println(YmouseDiff);
  
      lastMouseX_val=VALUE3;
      lastMouseY_val=VALUE4;

      if (XmouseDiff>16 || XmouseDiff<-16) VALUE3=0;
      if (YmouseDiff>16 || YmouseDiff<-16) VALUE4=0;
   }
//********************************************************************************
   if (VALUE1!=' ' || VALUE6!=' ') {
   
      VALUE1 = VALUE1 & 0xff;
      VALUE2 = VALUE2 & 0xff;
      VALUE3 = VALUE3 & 0xff;
      VALUE4 = VALUE4 & 0xff;
      VALUE5 = VALUE5 & 0xff;
      VALUE6 = VALUE6 & 0xff;
      VALUE7 = VALUE7 & 0xff;
        
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
//--------------------------------------------------------------------------------    
      if (VALUE6=='D' && VALUE7==0x47) SCROLLLOCK_millis=millis(); //ScrollLock pressed
            
      if (SCROLLLOCK_millis>0 && VALUE6=='D' && VALUE7==0x3A) { //ScrollLock + F1
         VALUE6=' '; 
         VALUE7=0;
         SCROLLLOCK_millis=0; 
         SCROLLLOCK=1;
      } else if (SCROLLLOCK_millis>0 && VALUE6=='D' && VALUE7==0x3B) { //ScrollLock + F2 
         VALUE6=' '; 
         VALUE7=0;
         SCROLLLOCK_millis=0; 
         SCROLLLOCK=2; 
      } else if (SCROLLLOCK_millis>0 && VALUE6=='D' && VALUE7==0x3C) { //ScrollLock + F3
         VALUE6=' '; 
         VALUE7=0;
         SCROLLLOCK_millis=0; 
         SCROLLLOCK=3;  
      } else if (SCROLLLOCK_millis>0 && VALUE6=='D' && VALUE7==0x3D) { //ScrollLock + F4
         VALUE6=' '; 
         VALUE7=0;
         SCROLLLOCK_millis=0; 
         SCROLLLOCK=4;  
      } else if (SCROLLLOCK_millis>0 && SCROLLLOCK_millis+500<millis()) {  //wait 1 second to switch scrolllock
         SCROLLLOCK_millis=0; //reset SCROLLLOCK call 
 
         //Switch ScrollLock 1..4
         if (SCROLLLOCK==0) SCROLLLOCK=1;
         else if (SCROLLLOCK==1) SCROLLLOCK=2;
         else if (SCROLLLOCK==2) SCROLLLOCK=3;
         else if (SCROLLLOCK==3) SCROLLLOCK=4;
         else if (SCROLLLOCK==4) SCROLLLOCK=1;
      }
      if (debug) Serial.print("SCROLLLOCK: ");
      if (debug) Serial.println(SCROLLLOCK); 
//--------------------------------------------------------------------------------    
      if (SCROLLLOCK>0) {
      //ENCODE DATA FOR SERIAL SEND
   
         uint8_t UARTDATA1=0; //Commands + clicks + mouse Z     
         uint8_t UARTDATA2=0; //mouse X
         uint8_t UARTDATA3=0; //mouse Y
         uint8_t UARTDATA4=0; //KEY

         UARTDATA1=0;
         if (VALUE6=='R') UARTDATA1=1;
         else if (VALUE6=='D') UARTDATA1=2;
         else if (VALUE6=='U') UARTDATA1=3;
         else if (VALUE6=='C') UARTDATA1=4;
         else if (VALUE6=='S') UARTDATA1=5;
         else if (VALUE6=='A') UARTDATA1=6;
         else if (VALUE6=='G') UARTDATA1=7;
         UARTDATA1 = UARTDATA1 | ((VALUE2&0x07)<<3); // Mouse Clsicks
         if (VALUE5==1) {UARTDATA1 = UARTDATA1 | 0b01000000; } //Z SCROLL
         else if  (VALUE5==0xff) { UARTDATA1 = UARTDATA1 | 0b10000000; } //Z SCROLL
         else if (VALUE1=='M') UARTDATA1 = UARTDATA1 | 0b11000000;  //call mouse command


         UARTDATA2=VALUE3;
         UARTDATA3=VALUE4;
         UARTDATA4=VALUE7;

//--------------------------------------------------------------------------------
//WRITE DATA TO SLAVE ARDUINO 1..4

         if (SCROLLLOCK==1) {
            Serial1.flush();
            Serial1.print((char)UARTDATA1);
            Serial1.print((char)UARTDATA2);
            Serial1.print((char)UARTDATA3);
            Serial1.print((char)UARTDATA4);
         }
         if (SCROLLLOCK==2) {
            Serial2.flush();
            Serial2.print((char)UARTDATA1);
            Serial2.print((char)UARTDATA2);
            Serial2.print((char)UARTDATA3);
            Serial2.print((char)UARTDATA4);
         }
         if (SCROLLLOCK==3) {
            Serial3.flush();
            Serial3.print((char)UARTDATA1);
            Serial3.print((char)UARTDATA2);
            Serial3.print((char)UARTDATA3);
            Serial3.print((char)UARTDATA4);
         }
         if (SCROLLLOCK==4) {
            Serial4.flush();
            Serial4.print((char)UARTDATA1);
            Serial4.print((char)UARTDATA2);
            Serial4.print((char)UARTDATA3);
            Serial4.print((char)UARTDATA4);
         }
      }
//--------------------------------------------------------------------------------
      if (VALUE6=='D' && VALUE7==83) { //NUMLOCK
         if (NUMLOCK==true) {
            if (debug==true) Serial.println("___NUMLOCK OFF call");
            NUMLOCK=false; 
         } else {        
            if (debug==true) Serial.println("___NUMLOCK ON call");
            NUMLOCK=true;
         }
         NUMLOCKCHANGE=true;
      }
      if (VALUE6=='D' && VALUE7==57) { //CAPSLOCK
         if (CAPSLOCK==true) {
            if (debug==true) Serial.println("___CAPSLOCK OFF call");
            CAPSLOCK=false; 
         } else {        
            if (debug==true) Serial.println("___CAPSLOCK ON call");
            CAPSLOCK=true;
         }
         CAPSLOCKCHANGE=true;
      }
   
//--------------------------------------------------------------------------------    
      VALUE1=' ';
      VALUE2=0;
      VALUE3=0;
      VALUE4=0;
      VALUE5=0;
      VALUE6=' ';
      VALUE7=0;
   }
}

//********************************************************************************       
