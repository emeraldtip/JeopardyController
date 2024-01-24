/*
 * Arduino Jeopardy
 * by Emerald
 * 
 * Supports up to 5 players
 * 5 Buttons (pins 8-12) and 5 APA102 lighstrips (clock - pin 2, signals - pins 3-7) + a status APA102 strip on pin A5 (with a clock on A3) 
 * or a single LED on pin 13
 * 
 * NOTE: The APA102 light strips that were in mind when making this project had alternating RGB and W pixels.
 *       Also for some reason 3 of the strips had addresses in the reverse order to the rest so I had to implement extra code for that
 *       To change the assignments of the reversed strips check the setStrip() function at line 70
 *       These lightstrips were not daisy-chainable. 
 *       Initially I tried to connect +5V,clock and GND lines of multiple strips together, with separate wires for data, but settled for
 *       a solution, where each strip is connected to a central lighting break-out box
 *       
 * External power supply recommended
 */
#define DISABLE_STARTUP_SEQUENCE false //change to true to disable startup test sequence

 
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>

#define NUM_LEDS 82
#define STATUS_LEDS 10 //had to reduce the amount of leds to 10 due to memory issues

CRGB strips[5][NUM_LEDS];
CRGB statusStrip[STATUS_LEDS]; 

bool testMode = true;
bool expectingAnswers = false;

PROGMEM const int playerColors[5][3] = {{255,0,0},{127,255,0},{0,255,0},{0,255,255},{255,0,255}};

void setup() {
  delay(3000); // 3 second delay for recovery

  //player strips
  FastLED.addLeds<APA102, 3, 2, BGR>(strips[0], NUM_LEDS);
  FastLED.addLeds<APA102, 4, 2, BGR>(strips[1], NUM_LEDS);
  FastLED.addLeds<APA102, 5, 2, BGR>(strips[2], NUM_LEDS);
  FastLED.addLeds<APA102, 6, 2, BGR>(strips[3], NUM_LEDS);
  FastLED.addLeds<APA102, 7, 2, BGR>(strips[4], NUM_LEDS);

  //statusstrip
  FastLED.addLeds<APA102, A5, A3, BGR>(statusStrip, STATUS_LEDS); //separate pin due to signal reflection or something idk

  //master brightness
  FastLED.setBrightness(20);

  //other pin shenanigans
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  Serial.begin(9600);

  //startup sequence to make sure that all lights work properly
  #if DISABLE_STARTUP_SEQUENCE
    digitalWrite(13,HIGH);
    testMode=false;
  #else
    startupSequence();
  #endif
}


void setStrip(int stripNum, int index, int r, int g, int b) {
  if (stripNum == 3 || stripNum == 4) //reversed + longer strips
  {
    index = NUM_LEDS - 1 - index; //there's actually a slight problem with this, but I can't be bothered to fix it
    //if you haven't noticed it so far, then don't bother looking for it, it's such a minor issue
  }
  strips[stripNum][index] = CRGB(r,g,b);
}

#if !(DISABLE_STARTUP_SEQUENCE)
  void startupSequence() {
    for (int i = 1; i < 10; i+=1)
    {
      statusStrip[i] = CRGB(0,0,0);    
    }    
    FastLED.show();
    for (int e = 0; e < 5; e++)
    {
      for (int i = 1; i < NUM_LEDS; i+=2)
      {    
        setStrip(e,i,pgm_read_word(&playerColors[e][0]), pgm_read_word(&playerColors[e][1]), pgm_read_word(&playerColors[e][2]));    
        FastLED.show();
      }
      answerCountDown(e,10);
    }
    
    for (int i = 1; i < STATUS_LEDS; i+=2)
    {
      statusStrip[i] = CRGB(255,255,255);    
    }
    FastLED.show();
    
    delay(3000);
    for (int i = 1; i < STATUS_LEDS; i+=2)
    {
      statusStrip[i] = CRGB(0,0,0);    
      FastLED.show();
    }
    
    digitalWrite(13, HIGH);
  }
#endif



void initAnswer(int playerNumber)
{
  for (int i = 1; i < NUM_LEDS; i+=2)
  {
    setStrip(playerNumber,i,pgm_read_word(&playerColors[playerNumber][0]), pgm_read_word(&playerColors[playerNumber][1]), pgm_read_word(&playerColors[playerNumber][2]));    
  }

  //statusstrip off
  for (int i = 1; i < STATUS_LEDS; i+=1) //1 just in case something glitches out
  {
      statusStrip[i] = CRGB(0,0,0);    
  }
  //player color on status strip
  for (int i = 0; i < STATUS_LEDS; i+=2)
  {
    statusStrip[i] = CRGB(pgm_read_word(&playerColors[playerNumber][0]),pgm_read_word(&playerColors[playerNumber][1]),pgm_read_word(&playerColors[playerNumber][2]));    
  }
  FastLED.show();
  answerCountDown(playerNumber, 150);
}

void answerCountDown(int playerNumber, int interval)
{
  for (int e = 0; e<41; e++)
  {
    Serial.println(F("answering")); //should probably use bytes, but, eh
    setStrip(playerNumber,e+1,0,0,0);
    setStrip(playerNumber,NUM_LEDS-1-e,0,0,0);
    FastLED.show();
    delay(interval); 
    if (Serial.available() && Serial.readString()==F("cancel")) //really should be using bytes
    {
      interval = 0;
    }
  }
  answerLedsOff();
}

void answerLedsOn()
{
  for (int i = 1; i < STATUS_LEDS; i+=2)
  {
      statusStrip[i] = CRGB(255,255,255);    
  }
  FastLED.show();
}

void answerLedsOff()
{
  for (int i = 0; i < STATUS_LEDS; i+=1)
  {
      statusStrip[i] = CRGB(0,0,0);    
  }
  FastLED.show();
}

long int openTime = 0; //timestamp since answers opened
long int penalties[5] = {0,0,0,0,0};

void checkPlayerHolding() //check if player is holding down button before answers are open and penalise them
{
  if (digitalRead(8) == LOW)
  {
    penalties[4] = openTime+1000;
  }
  if (digitalRead(9) == LOW)
  {
    penalties[3] = openTime+1000;
  }
  if (digitalRead(10) == LOW)
  {
    penalties[2] = openTime+1000;
  }
  if (digitalRead(11) == LOW)
  {
    penalties[1] = openTime+1000;
  }
  if (digitalRead(12) == LOW)
  {
    penalties[0] = openTime+1000;
  }
}


void loop() {
  if (testMode || expectingAnswers)
  { 
    if (testMode)
    {
      Serial.println(F("testing")); //who uses bytes anyway
    }
    if (expectingAnswers)
    {
      Serial.println(F("accepting")); //smh my head
    }
    
    if (Serial.available() && Serial.readString()==F("stop")) //🚨🚨🚨 STRING USER DETECTED!!!! 🚨🚨🚨
    {
      testMode = false;
      expectingAnswers = false;
      answerLedsOff();
    }
    
    
    if (digitalRead(8) == LOW && millis()>penalties[4])
    {
      Serial.println(F("answering"));
      digitalWrite(13, LOW);
      expectingAnswers = false;
      initAnswer(4);
    }
    if (digitalRead(9) == LOW && millis()>penalties[3])
    {
      Serial.println(F("answering"));
      digitalWrite(13, LOW);
      expectingAnswers = false;
      initAnswer(3);
    }
    if (digitalRead(10) == LOW && millis()>penalties[2])
    {
      Serial.println(F("answering"));
      digitalWrite(13, LOW);
      expectingAnswers = false;
      initAnswer(2);      
    }
    if (digitalRead(11) == LOW && millis()>penalties[1])
    {
      Serial.println(F("answering"));
      digitalWrite(13, LOW);
      expectingAnswers = false;
      initAnswer(1);
    }
    if (digitalRead(12) == LOW && millis()>penalties[0])
    {
      Serial.println(F("answering"));
      digitalWrite(13, LOW);
      expectingAnswers = false;
      initAnswer(0);
    }
  }
  else
  {
    digitalWrite(13, LOW);
    Serial.println(F("idle")); //lmao
    if (Serial.available())
    {
      String tring = Serial.readString();
      if  (tring==F("accept")) //only real garbage programmers use strings
      {
        openTime = millis(); //set answer opening timestamp
        checkPlayerHolding();
        digitalWrite(13, HIGH);
        expectingAnswers = true;
        answerLedsOn();
      }
      else if (tring==F("test")) //bytes are overrated
      {
        digitalWrite(13, HIGH);
        testMode = true;
        answerLedsOn();
      }
    }
  }
}
