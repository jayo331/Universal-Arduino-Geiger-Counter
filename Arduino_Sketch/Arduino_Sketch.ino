#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define instruction false       // this option allow user to turn on or off the instruction, if you want to see it every time set
                                // it true, else set it false. With instruction turned on the boot process and the set tube process
                                // wil be slown down.


///////////// PIN - VARIABLES - SCREEN DEFINITION ///////////////


#define SCREEN_WIDTH 128    // OLED display width (in pixels)
#define SCREEN_HEIGHT 32    // OLED display height (in pixels)

#define OLED_RESET     -1   // reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // address for the 128x32 oled display

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define interrupt 2   // interrupt pin
#define buzzer 4      // buzzer pin
#define button1 5     // screen button pin
#define button2 6     // buzzer button pin
#define vbat A0       // battery monitor pin

uint16_t cps = 0;     // number of events in one second
uint16_t cpm = 0;     // number of events in one minute
uint8_t sec = 0;      // seconds variable used to track the time

float uSv;            // instant received dose based on cps
float factor;         // conversion factor for the selected tube
float totmSv = 0;      // total received dose since last startup (based on detected cpm)

bool event = false;     // variable used to know when a tic (event) occurred
bool b_active = false;  // variable used to turn on and off the buzzer (at startup buzzer is off)
bool s_active = true;   // variable used to turn on and off the screen (at startup screen is on)



/////////////////// SYMBOLS /////////////////// 

static const unsigned char batt [] PROGMEM = {       // battery symbol (10x6 pixel)

  0x7f, 0xc0, 0x40, 0x40, 0xd5, 0x40, 0xd5, 0x40, 0x40, 0x40, 0x7f, 0xc0
};           

static const unsigned char sum [] PROGMEM = {        // sum symbol (5x7 pixel) 

  0xf8, 0x88, 0x40, 0x20, 0x40, 0x80, 0xf8 
};

static const unsigned char micro [] PROGMEM = {      // micro symbol (4x7 pixel)
  
  0x90, 0x90, 0x90, 0xf0, 0x80, 0x80, 0x80
};

static const unsigned char speaker [] PROGMEM = {    // speaker symbol (7x7 pixel)
  
  0x32, 0x74, 0xf0, 0xf6, 0xf0, 0x74, 0x32
};

static const unsigned char rad [] PROGMEM = {        // radiation symbol (16x16 pixel)
  
  0x0f, 0xf0, 0x38, 0x1c, 0x60, 0x06, 0x4e, 0x72, 0xde, 0x7b, 0x9c, 0x39, 0x9c, 0x39, 0x99, 0x99,
  0x81, 0x81, 0x80, 0x01, 0x81, 0x81, 0xc7, 0xe3, 0x47, 0xe2, 0x63, 0xc6, 0x38, 0x1c, 0x0f, 0xf0
};



////////// TUBE CONVERSION FACTOR (taken from https://www.uradmonitor.com/topic/hardware-conversion-factor) ////////////

float tubeFactor(uint8_t param) {  // function used to store the tube value and switch it

  switch (param) {

    case 0:   return 0.006315;   // GEIGER_TUBE_SBM20
    break;
    case 1:   return 0.010000;   // GEIGER_TUBE_SI29BG
    break;
    case 2:   return 0.001500;   // GEIGER_TUBE_SBM19
    break;
    case 3:   return 0.006666;   // GEIGER_TUBE_STS5
    break;
    case 4:   return 0.001714;   // GEIGER_TUBE_SI22G
    break;
    case 5:   return 0.631578;   // GEIGER_TUBE_SI3BG
    break;
    case 6:   return 0.048000;   // GEIGER_TUBE_SBM21
    break;
    case 7:   return 0.005940;   // GEIGER_TUBE_LND712
    break;
    case 8:   return 0.010900;   // GEIGER_TUBE_SBT9
    break;
    case 9:   return 0.006000;   // GEIGER_TUBE_SI1G
    break;
  }
}



///////////// INTERRUPT HANDLER /////////////

void tic(){ 

  cps++;       // increment cps
  event=true;  // this variable is used to control the buzzer
}



//////////// TIMER OVERFLOW HANDLER //////////

ISR(TIMER1_OVF_vect){                  // the handler will be called every second

  sec++;                               // count seconds so we can track the time
  
  cpm = cpm + cps;                     // update the cpm count
  uSv = ((cps * 60.000000) * factor);  // every second refresh the estimate uSv value based on the cps
  cps=0;                               // reset the cps variable so we can restart the cps count
 
  if(sec==60){                          // if one minute passed
    
    sec = 0 ;                           // reset the sec variable so we can start to count another minute

    totmSv = totmSv + ((cpm * factor)/1000);   // update the total dose variable by adding the real received dose based on measured cpm
    cpm = 0;                                   // every minute reset the cpm variable to start a new cpm count
  }
}



///////// MENU USED TO SET THE CONVERSION FACTOR ///////////

void setfactor(){

  uint8_t menu = 0;      // local variable used to switch the element of the menu  
  sec=0;                 // reset sec variable so we can use it as reference for the menu time

  do{

    if(!(digitalRead(button1))){         // if button 1 is pressed

      delay(20);                         // and after 20ms the button are still pressed (for debouncing)
      if(!(digitalRead(button1))){

        sec=0;              // every time we press a button the timer will be resetted so we have max 6 seconds between each press
        if(menu<9){         // if we have not yet reached the end of the menu

          menu++;           // switch to the next tube
        }
        delay(200);
      }
    }

    if(!(digitalRead(button2))){         // if button 2 is pressed

      delay(20);                         // and after 20ms the button are still pressed (for debouncing)
      if(!(digitalRead(button2))){

        sec=0;              // every time we press a button the timer will be resetted so we have max 6 seconds between each press
        if(menu>0){         // if we are at the beginning of the menu

          menu--;           // switch to the previous tube
        }
        delay(200);
      }
    }

    switch(menu){           // menu used to visualize the selected tube

      case 0:   display.clearDisplay();                // clear buffer
                display.setCursor(5,10);               // set diplay cursor to the center
                display.println("GEIGER_TUBE_SBM20");  // print
                display.display();

      break;

      case 1:   display.clearDisplay();                // clear buffer
                display.setCursor(5,10);               // set diplay cursor to the center
                display.println("GEIGER_TUBE_SI29BG"); // print
                display.display();

      break;  

      case 2:   display.clearDisplay();                // clear buffer
                display.setCursor(5,10);               // set diplay cursor to the center
                display.println("GEIGER_TUBE_SBM19");  // print
                display.display();

      break;    

      case 3:   display.clearDisplay();                // clear buffer
                display.setCursor(5,10);               // set diplay cursor to the center
                display.println("GEIGER_TUBE_STS5");   // print
                display.display();

      break; 

      case 4:   display.clearDisplay();                // clear buffer
                display.setCursor(5,10);               // set diplay cursor to the center
                display.println("GEIGER_TUBE_SI22G");  // print
                display.display();

      break;  

      case 5:   display.clearDisplay();                // clear buffer
                display.setCursor(5,10);               // set diplay cursor to the center
                display.println("GEIGER_TUBE_SI3BG");  // print
                display.display();

      break; 

      case 6:   display.clearDisplay();                // clear buffer
                display.setCursor(5,10);               // set diplay cursor to the center
                display.println("GEIGER_TUBE_SBM21");  // print
                display.display();

      break;   

      case 7:   display.clearDisplay();                // clear buffer
                display.setCursor(5,10);               // set diplay cursor to the center
                display.println("GEIGER_TUBE_LND712"); // print
                display.display();

      break;  

      case 8:   display.clearDisplay();                // clear buffer
                display.setCursor(5,10);               // set diplay cursor to the center
                display.println("GEIGER_TUBE_SBT9");   // print
                display.display();                    
 
      break; 

      case 9:   display.clearDisplay();                // clear buffer
                display.setCursor(5,10);               // set diplay cursor to the center
                display.println("GEIGER_TUBE_SI1G");   // print
                display.display();

      break;  
    }
  }
  while (sec<6);                         // if no button are pressed exit after 6 seconds
  
  EEPROM.put(0,tubeFactor(menu));        // put on eeprom the value of the selected tube

  display.clearDisplay();                // clear buffer
  display.setCursor(5,10);               // set diplay cursor to the center
  display.println("TUBE SET CORRECTLY");
  display.println(tubeFactor(menu),6);
  display.display();

  delay(2000);
}



////////// BUTTON CONTROL ///////////

void button_ctrl(){

   if(!digitalRead(button1)){      // if the sceen button is pressed

    delay(20);

    if(!digitalRead(button1)){     // if the screen button is still pressed (the check is done 2 times for debouncing purpose)

      s_active = !s_active;        // toggle the screen status

      if(s_active){                // if display is active

        display.ssd1306_command(SSD1306_DISPLAYOFF);  // switch display off
      }
  
      else{

        display.ssd1306_command(SSD1306_DISPLAYON);  // switch display back on
      }
      delay(300);                  // delay so the user have time to release the button
    }
  }


  if(!digitalRead(button2)){       // if the buzzer button is pressed

    delay(30);

    if(!digitalRead(button2)){     // the check is done 2 times for debouncing purpose

      b_active = !b_active;        // toggle the buzzer status
      delay(300);                  // delay so the user have time to release time                    
    } 
  }
}



////////// BUZZER CONTROL //////////

void buzzer_ctrl(){

  if(b_active){                                        // if the buzzer is active print the logo on screen
                         
    display.drawBitmap(100 ,10, speaker, 7, 7, WHITE); 
  } 

  if(event){                                           // if a tic is detected
 
    event=false;                                       // reset the event flag
    display.drawBitmap(111 ,15, rad, 16, 16, WHITE);   // display rad symbol
 
    if(b_active){                                      // if buzzer is active make a tic

      digitalWrite(buzzer,HIGH);                       
      delay(2);
      digitalWrite(buzzer,LOW);
    }
  }
}



//////////// BATTERY CONTROL - PRINT ///////////

void battery_ctrl(){

  uint8_t perc; 

  perc = map(analogRead(vbat), 613, 879, 0, 100);      // map from 3v - 4.2 to 0 - 100 (battery voltage)
  
  display.drawBitmap(78 ,0, batt, 10, 6, WHITE);      // display.drawBitmap(x position, y position, bitmap data, bitmap width, bitmap height, color)
  display.setCursor(89,0);           
  display.print(":");
  display.print(perc);
  display.print(" %");  
}



/////////// PRINT DATA ///////////

void print_data(){
  
  display.setCursor(0,0);                         // print CPS
  display.print("CPS: ");
  display.println(cps);
  
  display.setCursor(0,8);                         // print CPM
  display.print("CPM: ");
  display.println(cpm);
              
  display.drawBitmap(0 ,16, micro, 4, 7, WHITE);  // print estimated uSv
  display.setCursor(6,16);
  display.print("Sv: ");
  display.println(uSv,2);

  display.setCursor(0,24);                        // print total dose received
  display.drawBitmap(0 ,24, sum, 5, 7, WHITE);
  display.setCursor(9,24);
  display.print("mSv: ");
  display.println(totmSv,3);
}


///////////// SETUP AND LOOP FUNCTION ///////////

void setup() {

  pinMode(vbat,INPUT);                 // set battery voltage pin as input

  pinMode(interrupt,INPUT_PULLUP);     // set interrupt pin as input

  pinMode(button1,INPUT_PULLUP);       // set screen button as input 
  pinMode(button2,INPUT_PULLUP);       // set buzzer button as input

  pinMode(buzzer,OUTPUT);       // set buzzer button as input

  attachInterrupt(digitalPinToInterrupt(interrupt), tic, FALLING);    // initialize interrupt on pin 2


  ////////////// INITIALIZE SCREEN ///////////////

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();                // clear buffer
  display.display();
  display.setTextSize(1);                // set 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);   // Draw white text


  ////// TIMER1 SETUP (1 sec) //////

  TCCR1A = 0;              // set register
  TCCR1B = 0;              // set register
  TCCR1B |= (1<<CS12);     // prescaler 256
  TIMSK1 |= (1<<TOIE1);    // enable timer overflow
  TCNT1 = 3035;            // initialize tcnt1 register to 3035 so it need precisely 1 sec to overflow (instead of 1.048 sec) 


  //////// STARTUP PAGE ////////
 
  if(instruction){
    
    EEPROM.get(0,factor);                  //read the tube conversion factor from eeprom

    display.clearDisplay();                // clear buffer
    display.setCursor(0,0);                // set diplay cursor
    display.print("Set factor: ");         // show current factor
    display.println(factor,6);
    display.println("Keep buzzer & screen");
    display.println("button pressed to");
    display.println("enter the set menu");
    display.display();                     // send buffer

    delay(12000);                          // wait before clear so the user can read the set factor  
  }
  

  if(!digitalRead(button1) && !digitalRead(button2)){    //if both button 1 and 2 are pressed

    delay(20);                                           // and after 20ms the button are still pressed (for debouncing)

    if(!digitalRead(button1) && !digitalRead(button2)){
      
      display.clearDisplay();                // clear buffer
      display.setCursor(10,10);              // set diplay cursor to the center
      display.setTextSize(2);                // set 1:2 pixel scale only for this text
      display.println("SET MENU");           // set text
      display.display();                     // send buffer
      display.setTextSize(1);                // set 1:1 pixel scale (return to normal)


      delay(2000);

      if(instruction){

        do{                                          // show user the menu instruction 
          display.clearDisplay();                    // clear buffer and show first page
          display.setCursor(0,0);                    
          display.println("1) Release the screen");  
          display.println("and buzzer buttons");
          display.println("only after you fully");
          display.println("read the instructions.");
          display.display();
          delay(6000);

          display.clearDisplay();                    // clear buffer and show second page
          display.setCursor(0,0);
          display.println("2) Use the screen");
          display.println("and buzzer buttons");
          display.println("to scroll the menu.");
          display.display();
          delay(6000);
        
          display.clearDisplay();                     // clear buffer and show the other page
          display.setCursor(0,0);
          display.println("3) After 6 seconds");
          display.println("the selected tube");
          display.println("will be automatically");
          display.println("set.");
          display.display();
          delay(10000);
        }
        while(!digitalRead(button1) && !digitalRead(button2));
      }
      
      setfactor();                            // enter to the set factor menu
    }
  }

  EEPROM.get(0,factor);                  //load the tube conversion factor from eeprom

  display.clearDisplay();                // clear the buffer screen for the main interface
  display.display();                     // send buffer

}



void loop() {

  display.clearDisplay();           // clear screen so we can print the new info

  button_ctrl();                    // check buttons status
  buzzer_ctrl();                    // if there is an event tic the buzzer and display the rad symbol
  battery_ctrl();                   // check battery voltage 
  print_data();                     // print the value
  
  display.display();                // send info to the display
}
