#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define SIGNAL_TIMEOUT 500  // This is signal timeout in milli seconds.

const uint64_t pipeIn = 0xF9E8F0F0E1LL;
RF24 radio(8, 9); 
unsigned long lastRecvTime = 0;

struct PacketData
{
  byte xAxisValue;    
  byte yAxisValue;
  byte switchStatus;
} receiverData;

int enableMotor1=3;
int motor1Pin1=2;
int motor1Pin2=4;

int enableMotor2=5;
int motor2Pin1=6;
int motor2Pin2=7;


void setup()
{
  pinMode(enableMotor1,OUTPUT);
  pinMode(motor1Pin1,OUTPUT);
  pinMode(motor1Pin2,OUTPUT);
  
  pinMode(enableMotor2,OUTPUT);
  pinMode(motor2Pin1,OUTPUT);
  pinMode(motor2Pin2,OUTPUT);

  rotateMotor(0,0);
    
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1,pipeIn);
  radio.startListening(); //start the radio receiver 

}

void loop()
{
    int motorSpead1=0;
    int motorSpead2=0;
    // Check if RF is connected and packet is available 
    if(radio.isChipConnected() && radio.available())
    {
      radio.read(&receiverData, sizeof(PacketData)); 
      //This is default mode to run motors. Switch is open and HIGH if not pushed.
      if (receiverData.switchStatus)
      {
        moveCar();
      }
      else
      {
        logicToTurnMotorsOnBothSide();
      }
      lastRecvTime = millis();  
    }
    else
    {
      //Signal lost. Reset the motor to stop
      unsigned long now = millis();
      if ( now - lastRecvTime > SIGNAL_TIMEOUT ) 
      {
        rotateMotor(0, 0);   
     }
   }
}

void moveCar()
{
    int motorSpead1=0;
    int motorSpead2=0;  
    int throttle = map(receiverData.yAxisValue, 0, 254, -255, 255); 
    int mappedXValue = map(receiverData.xAxisValue, 0, 254, -255, 255); 
    int motorDirection = 1;     
    if (throttle < 0)
    {
      motorDirection = -1;
    }
  
    motorSpead1 = abs(throttle) - mappedXValue;
    motorSpead2 = abs(throttle) + mappedXValue;

    motorSpead1 = constrain(motorSpead1, 0, 255);
    motorSpead2 = constrain(motorSpead2, 0, 255);
    
    rotateMotor(motorSpead1 * motorDirection, motorSpead2 * motorDirection);
}

void rotateMotor(int speed1, int speed2)
{
  if (speed1 < 0)
  {
    digitalWrite(motor1Pin1,LOW);
    digitalWrite(motor1Pin2,HIGH);    
  }
  else if (speed1 >= 0)
  {
    digitalWrite(motor1Pin1,HIGH);
    digitalWrite(motor1Pin2,LOW);      
  }

  if (speed2 < 0)
  {
    digitalWrite(motor2Pin1,LOW);
    digitalWrite(motor2Pin2,HIGH);    
  }
  else if (speed2 >= 0)
  {
    digitalWrite(motor2Pin1,HIGH);
    digitalWrite(motor2Pin2,LOW);      
  }

  analogWrite(enableMotor1,abs(speed1));
  analogWrite(enableMotor2,abs(speed2));    
}

void logicToTurnMotorsOnBothSide()
{
    int motorSpead1=0;
    int motorSpead2=0;  
    int throttle = map(receiverData.yAxisValue, 0, 254, -255, 255); 
    int mappedXValue = map(receiverData.xAxisValue, 0, 254, -255, 255);      
    if (throttle < 0)
    {
      mappedXValue = -mappedXValue;
    }
  
    motorSpead1 = throttle - mappedXValue;
    motorSpead2 = throttle + mappedXValue;

    motorSpead1 = constrain(motorSpead1, -255, 255);
    motorSpead2 = constrain(motorSpead2, -255, 255);
    
    rotateMotor(motorSpead1, motorSpead2);
}
