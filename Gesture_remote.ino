#include <IRremote.h>
#include <IRremoteInt.h>

/* Swipe Remote Control
 
 This sketch uses an ultrasonic rangefinder to determine the user's gesture and outputs an IR signal to a sony TV based on the command given.
 - High swipe (> 10in) = Channel Up
 - Low swipe = Channel Down
 - High hold (> 10in) = Volume Up
 - Low hold = Volume Down
 - Cover sensor (< 3in) = Turn On / Off
 
*/

// Defines for control functions
#define CONTROL_CH 1 // Channel change
#define CONTROL_VOL 2 // Volume
#define CONTROL_POW 3 // Power

#define CONTROL_UP 1 //to dectect the control
#define CONTROL_DOWN -1 

#define DIST_MAX 20 // Maximum distance in inches, anything above is ignored.
#define DIST_DOWN 10 // Threshold for up/down commands. If higher, command is "up". If lower, "down".
#define DIST_POW 3 // Threshold for power command, lower than = power on/off

// IR PIN
const int irPin = 3; // this is defined in the library, this var is just a reminder. CHANGING THIS WILL NOT CHANGE PIN IN LIBRARY
// 2 Pin Ping Sensor
const int pingPin = 8; //for the hc-sr04 or hc-sr05 sensor
const int echoPin = 7;
// Confirmation LED Pins
const int led = 13; //internal LED for up/down debugging
const int ledR = 11; //Red Led
const int ledG = 10; //Green Led
const int ledB = 9; //Blue Led
// LED on timer
unsigned long timer; 
// IR transmitter object
IRsend irsend;
// Power confirmation flag (needs two swipes to send signal)
boolean powerConfirmed = false;

void setup() {
  // initialize serial communication and set pins
  Serial.begin(9600);
  pinMode(led, OUTPUT);
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);
  pinMode(pingPin, OUTPUT); //converts the seconds to the distance
  pinMode(echoPin, INPUT); //provides the input for the distance to be measured
  timer = millis();
}

void loop()
{

  //  Serial.println(millis());
  long duration, inches;
  int value;

  // Check for a reading
  duration = doPing();

  // Timer to confirm actions (currently only power)
  if (timer && timer < (millis() - 5000) && (millis() > 5000))
  {
    Serial.println("timer reset");
    timer = false;
  }

  digitalWrite(led, LOW);
  setColor(0, 0, 0); // off

  // convert the time into a distance
  inches = microsecondsToInches(duration);

  // If less than max inches away, then
  if (inches < DIST_MAX)
  {
    // Debug output
    Serial.print(inches);
    Serial.println("in");

    // If very close, it is a "power" signal
    if (inches < DIST_POW)
    {
      Serial.println(timer);
      // on or off
      if (timer)
      {
        doIR(CONTROL_POW, 0);
        timer = false;
        delay(2000); // don't want to be sending this more than once. 2 second delay
      }
      else
      {
        Serial.println("power flag set");
        timer = millis();
        setColor(255,50,50);
        delay(500);
      }
    }
    else // is volume or channel
    {
      // Distance determines control direction
      value = handleDist(inches);
      // wait half a second
      delay(300);
      // check again for hand
      if (microsecondsToInches(doPing()) > DIST_MAX)
      {
        doIR(CONTROL_CH, value); // swipe
      }
      else
      {
        // volume
        int d = 500; // first delay is longer for single volume change
        // repeat until hand is removed
        while (inches < DIST_MAX)
        {
          value = handleDist(inches); // is up or down?
          doIR(CONTROL_VOL, value); // send IR signal
          delay(d); // wait
          inches = microsecondsToInches(doPing()); // check for hand again
          d = 100; // delays are shorter for quick multiple volume adjustment
        }
        delay(500); // to prevent accidental channel change after volume adjustment
      }
    }
  }
  delay(50); // Short enough to detect all swipes.
}
/*
* If distance is within threshold, mark as 'up' and turn on corresponding LED.
 */
int handleDist(int inches)
{
  if (inches > DIST_DOWN)
  {
    digitalWrite(led, HIGH);
    return CONTROL_UP;
  }
  else
  {
    digitalWrite(led, LOW);
    return CONTROL_DOWN;
  }
}

/*
* Send correct IR code
 */
void doIR(int control, int val)
{
  switch(control)
  {
  case CONTROL_POW:
    // power
    Serial.println("power on / off 0xE0E040BF");
    for (int i = 0; i < 3; i++)
    {
      setColor(255, 0, 0);
      irsend.sendSAMSUNG(0xE0E040BF, 32); // Sony TV power code
      delay(40);
    }
    break;
  case CONTROL_CH:
    setColor(0, 255, 0);
    // output 'channel up / down' depending on val
    if (val == CONTROL_UP)
    {
      digitalWrite(led, HIGH);
      for (int i = 0; i < 3; i++)
      {
        irsend.sendSAMSUNG(0xE0E048B7, 32);
        delay(40);
      }
      Serial.println("channel up 0xE0E048B7");
    }
    else // down
    {
      for (int i = 0; i < 3; i++)
      {
        irsend.sendSAMSUNG(0xE0E008F7, 32);
        delay(40);
      }
      Serial.println("channel down 0xE0E008F7");
    }
    break;
  case CONTROL_VOL:
    setColor(0, 0, 255);
    // output 'volume up / down' depending on val
    if (val == CONTROL_UP)
    {
      digitalWrite(led, HIGH);
      for (int i = 0; i < 3; i++)
      {
        irsend.sendSAMSUNG(0xE0E0E01F, 32);
        delay(40);
      }
      Serial.println("volume up 0xE0E0E01F");
    }
    else //down
    {
      for (int i = 0; i < 3; i++)
      {
        irsend.sendSAMSUNG(0xE0E0D02F, 32);
        delay(40);
      }
      Serial.println("volume down 0xE0E0D02F");
    }
    break;
  }
}
void setColor(int red, int green, int blue)
{
  analogWrite(ledR, red);
  analogWrite(ledG, green);
  analogWrite(ledB, blue);
}

long doPing()
{
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);
  return pulseIn(echoPin, HIGH);
}
//sensor sends for ultrasound and listens for the echo when it bounces off an object
long microsecondsToInches(long microseconds)
{
  //https://www.arduino.cc/en/Tutorial/Ping for more info
  return microseconds / 74 / 2;
}
//currently this function is not required since setting up a scale in centimeters will
//be hard to decipher by the user. 
long microsecondsToCentimeters(long microseconds)
{
  return microseconds / 29 / 2;
}
