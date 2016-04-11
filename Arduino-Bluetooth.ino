// UTouch_ButtonTest 
// Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved
// web: http://www.RinkyDinkElectronics.com/
//
// This program is a quick demo of how create and use buttons.
//
// This program requires the UTFT library.
//
// It is assumed that the display module is connected to an
// appropriate shield or that you know how to change the pin 
// numbers in the setup.
//

#include <UTFT.h>
#include <UTouch.h>
#include <Servo.h>
#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>


// Initialize display
// ------------------
// Set the pins to the correct ones for your development board
// -----------------------------------------------------------
UTFT    myGLCD(ILI9325D_8,38,39,40,41);

// Initialize touchscreen
// ----------------------
// Set the pins to the correct ones for your development board
// -----------------------------------------------------------
UTouch  myTouch( 6, 5, 4, 3, 2);

// Declare which fonts we will be using
extern uint8_t BigFont[];
extern uint8_t SmallFont[];

int x, y;
char stCurrent[20]="";
int stCurrentLen=0;
char stLast[20]="";
bool locked = true;
int iLockPin = 53;
String key;

/*************************
**   Custom functions   **
*************************/

void drawLock()
{
      myGLCD.clrScr();
    if (locked)
    {
      myGLCD.setColor(0, 0, 255);
      myGLCD.fillCircle(70,70,50);
      myGLCD.setColor(0, 0, 0);
      myGLCD.fillCircle(70,70,40);
    }else
    {
      myGLCD.setColor(0, 0, 255);
      myGLCD.fillCircle(160,70,50);
      myGLCD.setColor(0, 0, 0);
      myGLCD.fillCircle(160,70,40);
      myGLCD.fillRoundRect(110, 70, 260, 120);     
    }
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRoundRect(20, 70, 120, 170);
    myGLCD.setColor(255, 255, 255);
    myGLCD.print("Pedal", 32, 100);
    myGLCD.print("Pac", 45, 130);
    
}




static byte buf_len = 0;

void ble_write_string(byte *bytes, uint8_t len)
{
  if (buf_len + len > 20)
  {
    for (int j = 0; j < 15000; j++)
      ble_do_events();
    
    buf_len = 0;
  }
  
  for (int j = 0; j < len; j++)
  {
    ble_write(bytes[j]);
    buf_len++;
  }
    
  if (buf_len == 20)
  {
    for (int j = 0; j < 15000; j++)
      ble_do_events();
    
    buf_len = 0;
  }  
}

/*************************
**  Required functions  **
*************************/

void setup()
{
// Initial setup
  Serial.begin(57600);
  Serial.println("BLE Arduino Slave");

  digitalWrite(iLockPin, HIGH);
  pinMode(iLockPin, OUTPUT);

  // Set your BLE Shield name here, max. length 10
  ble_set_name("PLocker-01");
  
  ble_begin(); 
  myGLCD.InitLCD();
  myGLCD.clrScr();

  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);

  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(0, 0, 255);
  
  drawLock();
    
}

void loop()
{
    drawLock();  
  int buf[318];
  int x, x2;
  int y, y2;
  int r;
  while (true)
  {
    if (myTouch.dataAvailable())
    {
      //myTouch.read();
      //x=myTouch.getX();
      //y=myTouch.getY();
      locked = true;
      drawLock();
    }
      
    ble_do_events();
    if ( ble_available() )
   {
      Serial.print("Recieved:");
      String Scmd = "";
      while(ble_available())
      {
        char cmd;
        cmd = ble_read();
        Scmd.concat(cmd);
      }
        // Parse data here
        Serial.println();
        Serial.println("CMD:" + Scmd);
  
        myGLCD.setColor(0, 255, 0);
        myGLCD.print(Scmd, CENTER, 192);
        delay(500);
        myGLCD.print("            ", CENTER, 192);
        myGLCD.setColor(0, 255, 0);
        
        if (Scmd == "UNLOCK"){
            Serial.println("Unlocking");
            byte buf[] = {'D','O','N','E'};         
            ble_write_string(buf, 4);
            
            locked = false;
            drawLock();
            
            myGLCD.setColor(0, 255, 0);
            myGLCD.print("UNLOCKED", CENTER, 192);
            //unlock door for 5 seconds
            digitalWrite(iLockPin, LOW);
            delay(5000);
            
            digitalWrite(iLockPin, HIGH);
            myGLCD.print("            ", CENTER, 192);
            myGLCD.setColor(0, 255, 0);   

            locked = true;
            drawLock();
            
          }else if(Scmd == "AUTH"){
            key = String(millis());
            key = key + String(random(10, 99));
            Serial.println(key);
            Serial.println(String(key.length()));
           
            byte keybyte[key.length()];
            
            Serial.println(String(sizeof(keybyte)));
            key.getBytes(keybyte,key.length());
            ble_write_string(keybyte,sizeof(keybyte));
            
          }else if(Scmd == key){
            Serial.println("Unlocking");
            byte buf[] = {'D','O','N','E'};         
            ble_write_string(buf, 4);
            
            locked = false;
            drawLock();
            
            myGLCD.setColor(0, 255, 0);
            myGLCD.print("UNLOCKED", CENTER, 192);
            //unlock door for 5 seconds
            digitalWrite(iLockPin, LOW);
            delay(5000);
            
            digitalWrite(iLockPin, HIGH);
            myGLCD.print("            ", CENTER, 192);
            myGLCD.setColor(0, 255, 0);   

            locked = true;
            drawLock();
          }else
        {
          Serial.println("Unknown command");
          byte buf[] = {'E','R','R','O','R'};         
          ble_write_string(buf, 5);
        }
            // send out any outstanding data
        buf_len = 0;
        return; // only do this task in this loop
    }
}
}
