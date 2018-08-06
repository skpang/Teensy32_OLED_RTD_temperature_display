/* Teensy CAN-Bus with OLED 128x64 demo
 *  
 * Remote Temperature Display for Dual PT100 RTD to CAN-Bus Converter
 * http://skpang.co.uk/catalog/dual-pt100-rtd-to-canbus-converter-p-1561.html
 *  
 * www.skpang.co.uk
 * 
 * V1.0 Aug 2018
 *  
 * For use with Teensy CAN-Bus Breakout board:
 * http://skpang.co.uk/catalog/teensy-canbus-breakout-board-include-teensy-32-p-1507.html
 * requires OLED display
 * http://skpang.co.uk/catalog/oled-128x64-display-for-teensy-breakout-board-p-1508.html
 * 
 * Also requres new FlexCAN libarary
 * https://github.com/collin80/FlexCAN_Library
 *  
 */
#include <FlexCAN.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#define OLED_DC     6
#define OLED_CS     10
#define OLED_RESET  5
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);

int led = 13;

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

static CAN_message_t msg,rxmsg;
volatile uint32_t count = 0;
IntervalTimer TX_timer;
String CANStr(""); 
volatile uint32_t can_msg_count = 0;
float converted_data;

#define TEMP_ID1 0x700  // Channel 1 CAN-ID
#define TEMP_ID2 0x701  // Channel 2 CAN-ID

uint8_t no_data1 = 0;   // No data counter for channel 1
uint8_t no_data2 = 0;   // No data counter for channel 2

IntervalTimer data_timer;
unsigned char data[4];

void setup()
{
  delay(1000);
  Can0.begin(500000); //set speed here. 
  pinMode(led, OUTPUT);  

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  // Clear the buffer.
  display.clearDisplay();
   
  Serial.println(F("CAN Bus Tx test"));
  display.setTextSize(0);
  display.setTextColor(WHITE);
  display.setCursor(0,15);
  display.println(" Teensy PT100 Demo");
  display.println(" ");
  display.println(" skpang.co.uk");
  display.println(" ");
  display.println(" 08/18");
  display.display();

  digitalWrite(led, HIGH);
  delay(500);
  digitalWrite(led, LOW);
  delay(500);

  delay(1000);
  
  display.clearDisplay();

  msg.rtr =0 ;
  //msg.flags.extended = 0; 
  //msg.flags.remote = 0;
  //msg.timeout = 500;

  display.setFont(&FreeSansBold9pt7b);
  display.setTextColor(WHITE);

  display.setCursor(50,16);
 
  display.println(" C");
  display.drawCircle(55,2,2,WHITE);
   
  display.setCursor(50,60);
  display.println(" C");
  
  display.drawCircle(55,45,2,WHITE);
  display.display();

  data_timer.begin(data_count, 1000000);   // Start no data interrupt counter
 
}


/* From Timer Interrupt */
void data_count(void)
{
  no_data1++;
  no_data2++;
}

void loop() 
{

  int i;

  while(Can0.read(rxmsg))
  { 
     String CANStr(""); 
     for (int i=0; i < 8; i++) {     

         CANStr += String(rxmsg.buf[i],HEX);
         CANStr += (" ") ;
     }
    
    if(rxmsg.id == TEMP_ID1)
    {
      data[0] = rxmsg.buf[0];
      data[1] = rxmsg.buf[1];
      data[2] = rxmsg.buf[2];
      data[3] = rxmsg.buf[3];
      
      memcpy(&converted_data, data, 4);   //Convert data back to float
      display.setFont(&FreeSansBold9pt7b); 
      display.fillRect(0,0,45, 20,BLACK);
      display.setCursor(0,15);
      display.println(converted_data,2);  //Display temperature with 2 decimal places
      display.display();

      Serial.print(rxmsg.id,HEX); 
      Serial.print(' '); 
      Serial.print(rxmsg.len,HEX); 
      Serial.print(' ');
      Serial.print(CANStr); 

      Serial.print("Ch1 Temperature : ");
      Serial.println(converted_data,4);
      no_data1 = 0;
    }

    if(rxmsg.id == TEMP_ID2)  // Indoor temperature
    {
      data[0] = rxmsg.buf[0];
      data[1] = rxmsg.buf[1];
      data[2] = rxmsg.buf[2];
      data[3] = rxmsg.buf[3];

      memcpy(&converted_data, data, 4);     //Convert data back to float
      display.setFont(&FreeSansBold9pt7b);
      display.fillRect(0,30,45, 60,BLACK);
      display.setCursor(0,60);
      display.println(converted_data,2);  //Display temperature with 2 decimal places
      display.display();
      
      Serial.print(rxmsg.id,HEX); 
      Serial.print(' '); 
      Serial.print(rxmsg.len,HEX); 
      Serial.print(' ');
      Serial.print(CANStr); 

      Serial.print("Ch2 Temperature : ");
      Serial.println(converted_data,4);
      
      no_data2 = 0;
    }
  }
 
   digitalWrite(led, HIGH);
   digitalWrite(led, LOW);
  
   if(no_data1 >2)  //Check data still coming in within 2 second
   {
      display.fillRect(0,0,45, 20,BLACK);
      display.setCursor(0,15);
      display.println("------");
      display.display();
      no_data1 = 3;  // Prevet counter rollover
    }
   
   if(no_data2 >2)  //Check data still coming in within 2 second
   {
      display.fillRect(0,30,45, 60,BLACK);
      display.setCursor(0,60);
      display.println("------");
      display.display();
      no_data2 = 3;  // Prevet counter rollover
    }
}



