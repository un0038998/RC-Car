#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define JOYSTICK_MIN_READ 0
#define JOYSTICK_MAX_READ 1023
#define JOYSTICK_CENTER_READ 511
#define JOYSTICK_CENTER_DEAD_BAND 30

#define MAPPED_VALUE_MIN 0
#define MAPPED_VALUE_MAX 254
#define MAPPED_VALUE_CENTER 127

#define SWITCH_PIN 2
//#define DEBUG_PRINT    //This is to print the debug output on serial monitor

const uint64_t pipeOut = 0xF9E8F0F0E1LL;   //IMPORTANT: The same as in the receiver 0xF9E8F0F0E1LL

RF24 radio(8, 9); // select CE,CSN pin

struct PacketData 
{
  byte xAxisValue;
  byte yAxisValue;
  byte switchStatus;
} data;

void setup()
{
  #ifdef DEBUG_PRINT
  Serial.begin(9600);
  #endif
  
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(pipeOut);
  radio.stopListening(); //start the radio Transmitter 

  pinMode(SWITCH_PIN,INPUT_PULLUP);
  data.xAxisValue = MAPPED_VALUE_CENTER; // Center
  data.yAxisValue = MAPPED_VALUE_CENTER; // Center
  data.switchStatus = HIGH;
}

// Map joystick values to be sent per byte. So max value for 1 byte is 255. We select max as 254 so that center value is 127.
int mapAndAdjustValues(int val, int lower, int middle, int upper, bool reverse)
{
  val = constrain(val, lower, upper);

  //middle == 0: This is the case when joystick is not auto centered type. We map full range here.
  if (middle == 0)
  {
    val = map(val, lower , upper, MAPPED_VALUE_MIN , MAPPED_VALUE_MAX);
    return (reverse ? MAPPED_VALUE_MAX - val : val);
  }
  
  if ( val <= middle - JOYSTICK_CENTER_DEAD_BAND )
  {
    val = map(val, lower , middle - JOYSTICK_CENTER_DEAD_BAND, MAPPED_VALUE_MIN , MAPPED_VALUE_CENTER);
  }
  else if (val >= middle + JOYSTICK_CENTER_DEAD_BAND)
  {
      val = map(val, middle + JOYSTICK_CENTER_DEAD_BAND , upper, MAPPED_VALUE_CENTER, MAPPED_VALUE_MAX);
  }
  else
  {
    val = MAPPED_VALUE_CENTER;
  }

  return (reverse ? MAPPED_VALUE_MAX - val : val);
}

void readSwitchValue()
{
  if (!digitalRead(SWITCH_PIN))
  {
    data.switchStatus = !(data.switchStatus);
    delay(1000);
  }
}

void loop()
{
  int xAxisValue = analogRead(A0);
  int yAxisValue = analogRead(A1); 
 
  data.xAxisValue = mapAndAdjustValues( xAxisValue, JOYSTICK_MIN_READ, JOYSTICK_CENTER_READ, JOYSTICK_MAX_READ, false );     // "true" for reverse direction 
  data.yAxisValue = mapAndAdjustValues( yAxisValue, JOYSTICK_MIN_READ, JOYSTICK_CENTER_READ, JOYSTICK_MAX_READ, true );       // "true" for reverse direction 
  readSwitchValue(); 
  #ifdef DEBUG_PRINT
  Serial.println("The raw value is ");
  Serial.println(xAxisValue);
  Serial.println(yAxisValue);   
  Serial.println("The mapped value is ");
  Serial.println(data.xAxisValue);
  Serial.println(data.yAxisValue);
  Serial.println(radio.isChipConnected());
  delay(500);
  #endif
  
  radio.write(&data, sizeof(PacketData));
}
