#include <DS1307RTC.h>
#include <Time.h>
#include <TM1637Display.h>
#include <Wire.h>
#include <ClickEncoder.h> // https://github.com/0xPIT/encoder
#include <TimerOne.h> // needed for ClickEncoder
#include <Adafruit_NeoPixel.h>
/****************************************************************
 *                                                              *
 *  IKEA DUKTIG clock/timer - Roald Hendriks, January 2018      *
 *                                                              *
 *  Hardware: Arduino Uno R3 / Arduino Nano                     *
 *            LEDstrips (in oven and over counter)              *
 *            DS3231 SDA analog pin4, SCL analog pin5           *
 *            TM1637 4 digit LED Display                        *
 *            4 buttons (+, -, start, stop)                     *
 *            Buzzer                                            *
 *                                                              *
 ****************************************************************

 Revision History
  
    Date    By  What
  20180124      RH      v1.0 Final version, data cleanup for instructable

 */
 
// Definition of all digital pins
#define ENCODER_BUTTON 10
#define ENCODER_A 11
#define ENCODER_B 12

#define BuzzerPin 4
#define OvenLEDPin 5
#define TM1637CLKPin 3
#define TM1637DataPin 2


#define NEO_PIN 6
#define NEO_NUM_LEDS 16
#define NEO_BRIGHTNESS 50


// Segments for showing BEEP
const uint8_t SEG_BEEP[] = {
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G, // B
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,                 // E
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,                 // E
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,                 // P
  };

// PWM LED brightness
const int intLEDBrightness = 250;

// Create TM1637 object
TM1637Display display(TM1637CLKPin,TM1637DataPin);

// Create RTC time object
tmElements_t RTC_time;

// Create RTC timer objects to hold the timer
// prevTimes is used to determine whether the timer has changed
tmElements_t Timer, prevTimer;

// Create a encoder object
ClickEncoder *encoder;

//Neopixel leds
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEO_NUM_LEDS, NEO_PIN, NEO_GRB + NEO_KHZ800);


int intState;                         // current  state

// All states
#define ShowTime  1
#define SetTimer  2
#define CountDown 3
#define Beeping   4

#define On  HIGH
#define Off LOW

time_t systime; // holds current time for diff calculation

// While setting the timer, a list with gradually increasing steps will be used
// The numbers represent seconds
int TimerSet[] = {5, 10, 15, 20, 25, 30, 40, 50, 60, 75, 90, 105, 120, 135, 150, 165, 180, 210, 240, 270, 300, 330, 360, 420, 480, 540, 600};

// For colon
uint8_t segto;


void timerIsr() {
  encoder->service();

}

uint16_t RainbowStep = 0;

void setup()
// Mandatory setup function
{   
  // Enable serial monitoring
  Serial.begin(19200);
  
  // Sync to DS3231
  setSyncProvider(RTC.get);
  setSyncInterval(10);
  
  // Send time to serial
  RTC.read(RTC_time);
  OutputTimeToSerial();
  
  // Set Timer to 00:00
  Timer.Minute = 0;
  Timer.Second = 0;
  // Set previous timer to timer
  prevTimer = Timer;
  /*
  // Initialize the button pins as inputs:
  pinMode(PlusButtonPin,  INPUT_PULLUP); 
  pinMode(MinusButtonPin, INPUT_PULLUP); 
  pinMode(StartButtonPin, INPUT_PULLUP); 
  pinMode(StopButtonPin,  INPUT_PULLUP);
  */
  // Initialize the output pins:
  pinMode(BuzzerPin,  OUTPUT);
  pinMode(OvenLEDPin, OUTPUT);
  
  // Set initial mode
  intState = ShowTime;
  
  // Set default brightness to 10 on a scale of 15
  display.setBrightness(0x0a);

  // enable the encoder
  pinMode(ENCODER_A,  INPUT_PULLUP); 
  pinMode(ENCODER_B, INPUT_PULLUP); 
  pinMode(ENCODER_BUTTON, INPUT_PULLUP); 
  encoder = new ClickEncoder(ENCODER_A, ENCODER_B, ENCODER_BUTTON, 2, LOW);
  //encoder->setAccelerationEnabled(true);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);

  //
  strip.begin();
  strip.setBrightness(50);
  strip.show(); // Initialize all pixels to 'off'
  
}


void Bep(int t)
// Make a beeping sound
{ // Show BEEP in display
  digitalWrite(BuzzerPin,HIGH);
  delay(t); // white
  // Beep off for 500 milliseconds
  digitalWrite(BuzzerPin,LOW);


  
}
void Beep(void)
// Make a beeping sound
{ // Show BEEP in display
  display.setSegments(SEG_BEEP);
  // Beep on for 500 milliseconds
  digitalWrite(BuzzerPin,HIGH);
  colorWipe(strip.Color(255, 255, 255), 30 ); // white
  // Beep off for 500 milliseconds
  digitalWrite(BuzzerPin,LOW);
  colorWipe(strip.Color(0, 0, 0), 30); // white
  // Beep on for 500 milliseconds
  digitalWrite(BuzzerPin,HIGH);
  colorWipe(strip.Color(127, 127, 127), 30 ); // white
  // Beep off for 500 milliseconds
  digitalWrite(BuzzerPin,LOW);
  colorWipe(strip.Color(0, 0, 0), 30); // white
  // Beep on for 1 second
  digitalWrite(BuzzerPin,HIGH);
  colorWipe(strip.Color(127, 127, 127), 60 ); // white
  // Beep off
  digitalWrite(BuzzerPin,LOW);
  colorWipe(strip.Color(0, 0, 0), 60 ); // white
  // After the beep, show time again
  intState = ShowTime;
}

void CountdownTimer()
// Decrease timer by 1 second
{
  int intSeconds;
  // Convert current timer time to seconds
  intSeconds = Timer.Minute * 60 + Timer.Second;
  
  // Decrease by 1
  intSeconds--;
  
  // Update timer
  Timer.Minute = intSeconds / 60;
  Timer.Second = intSeconds % 60;
}

bool DecreaseTimer()
// Decrease timer using standard list
{ int intSeconds, intIndex;
  // Convert current timer time to seconds
  intSeconds = Timer.Minute * 60 + Timer.Second;

  // Loop through standard list and find the first value larger than the current number of seconds
  int i;
  for (i=26; i >= 0; i--){
      // Check whether list value is larger than current timer setting
      if(TimerSet[i] < intSeconds)
        { // Set Timer, minutes is simply dividing by 60, seconds are the rest after division
          Timer.Minute = TimerSet[i] / 60;
          Timer.Second = TimerSet[i] % 60;
          // Break out of for loop
          break;
        }
   }
   if (i>=0){
    return true;
   }else{
    return false;
   }
}

void IncreaseTimer()
// Increase timer using standard list
{ int intSeconds, intIndex;
  // Convert current timer time to seconds
  intSeconds = Timer.Minute * 60 + Timer.Second;

  // Loop through standard list and find the first value smaller than the current number of seconds
  for (int i=0; i <= 26; i++){
      // Check whether list value is smaller than current timer setting
      if(TimerSet[i] > intSeconds)
        { // Set Timer, minutes is simply dividing by 60, seconds are the rest after division
          Timer.Minute = TimerSet[i] / 60;
          Timer.Second = TimerSet[i] % 60;
          // Break out of for loop
          break;
        }
   } 
}

void OutputTimeToSerial()
// Send time to Serial-output
{
  Serial.print("Clock: ");
  Serial.print(RTC_time.Hour);
  Serial.print(":");
  Serial.print(RTC_time.Minute);
  Serial.print(":");
  Serial.println(RTC_time.Second);
}

void OutputTimerToSerial()
// Send timer to Serial-output
{
  Serial.print("Timer: ");
  Serial.print(Timer.Hour);
  Serial.print(":");
  Serial.print(Timer.Minute);
  Serial.print(":");
  Serial.println(Timer.Second);
}

void SetLight( boolean State)
// Set LED strip to On or Off (state HIGH or LOW)
{
  if (State==Off) {
    colorWipe(strip.Color(0, 0, 0), 30 );
  }
  else {
    // For switching on use a 'smooth' approach
    colorWipe(strip.Color(255, 255, 255), 15 );
  }
}

 
void ShowTimeOnDisplay(void)
// Show current time on the display
{ // Convert time to an integer
  RTC.read(RTC_time);
  int intNumber;
  intNumber = RTC_time.Hour * 100 + RTC_time.Minute;
  display.showNumberDec(intNumber,true);
  // For blinking the colon:
  // show colon for even seconds
  // hide colon for odd seconds
  if ((RTC_time.Second%2) == 0) {
    segto = 0x80 | display.encodeDigit((intNumber/100)%10);
    display.setSegments(&segto, 1, 1);
  }
}

void ShowTimerOnDisplay(void)
// Show timer on the display
{ // Convert timer to an integer
  int intNumber;
  intNumber = Timer.Minute * 100 + Timer.Second;
  // Show on display
  display.showNumberDec(intNumber,true);
  // Show colon
  segto = 0x80 | display.encodeDigit((intNumber/100)%10);
  display.setSegments(&segto, 1, 1);
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}
void rainbowCycleStep() {
 uint16_t i; 
 for(i=0; i< strip.numPixels(); i++) {
   strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + RainbowStep) & 255));
 }
 strip.show();
 
 if (RainbowStep>=256*5) RainbowStep=0; else RainbowStep++;
  

}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


void loop(void)
// The mandatory loop
{
  
    ClickEncoder::Button b = encoder->getButton();
    if (b != ClickEncoder::Open) {
      Serial.println("Pushed");
    }
    
    if(b == ClickEncoder::Clicked) {
      Serial.println("Released");
        switch(intState) {
          case ShowTime:
           // Ignore
           break;
          case SetTimer:
           Bep(10);
           // Start countdown
           intState = CountDown;
           break;
          case CountDown:
           Bep(10);
           SetLight(false);
           // Show time
           intState = ShowTime;
           // Reset timer
           Timer.Minute = 0;
           Timer.Second = 0;
        };
    }


    int16_t encoder_pulse = encoder->getValue();
    if (encoder_pulse > 0){
        // add values
        switch(intState) {
          case ShowTime:
           // Change mode to SetTimer
           intState = SetTimer;
           Bep(1);
           SetLight(true);
           // Increase timer (from 00:00 to 00:05)
           IncreaseTimer();
           // Show timer
           ShowTimerOnDisplay();           
           break;
          case SetTimer:
           Bep(1);
           // Increase the timer to smallest standard setting larger than current timer
           IncreaseTimer();
           break;
          case CountDown:
           Bep(1);
           // Increase the timer to smallest standard setting larger than current timer
           IncreaseTimer();
           break;
        };
      
    }else if (encoder_pulse < 0 ){
       // decrease
       switch(intState) {
          case ShowTime:
           break;
          case SetTimer:
           Bep(1);
           // Increase the timer to largest standard setting smaller than current timer
           if( DecreaseTimer() == false){
             SetLight(false);
             // Show time
             intState = ShowTime;
             // Reset timer
             Timer.Minute = 0;
             Timer.Second = 0;
           }
           break;
          case CountDown:
           Bep(1);
           // Increase the timer to largest standard setting smaller than current timer
           DecreaseTimer();
           break;
        };
      
    }

  // Based on current state, do ...
  switch(intState) {
    case ShowTime:
     // ShowTime on Display, but only when the time has changed
     if (systime != now()) { // wait for new second to do anything
        // Update systime
        systime = now();
        // ShowTime on Display
        ShowTimeOnDisplay();
     }
     break;
    case SetTimer:
     // Only update display when the timer has changed
     if (Timer.Minute != prevTimer.Minute || Timer.Second != prevTimer.Second) {
       prevTimer = Timer;
       // ShowTimer on Display
       ShowTimerOnDisplay();
     }
     break;
    case CountDown:
     // Correct Timer, but only when the time has changed
     if (systime != now()) { // wait for new second to do anything
        systime = now();
        // ShowTimer on Display
        ShowTimerOnDisplay();
        // And of course, count down the timer
        CountdownTimer();
     }
     rainbowCycleStep();
     if (Timer.Minute == 0 && Timer.Second == 0) {
       // Delay so 00:01 is shown
       delay(900);
       intState = Beeping;
     }
     
     break;
    case Beeping:
     // Well, BEEP!!!
     Beep();
     break;
  }

/*
  lastPlusButtonState  = readPlusPin;
  lastMinusButtonState = readMinusPin;
  lastStartButtonState = readStartPin;
  lastStopButtonState  = readStopPin;
  */

}
