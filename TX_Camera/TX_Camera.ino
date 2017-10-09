#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <Adafruit_ADS1015.h>
#include<String.h>
#include<math.h>
#include <AltSoftSerial.h>

AltSoftSerial mySerial;
#define defaultFreq 1700
#define freq0 500 
#define freq1 750 
#define freq2 1000 
#define freq3 1250 
int delay0, delay1, delay2, delay3;
const uint16_t S_DAC[4] = {2048,4095, 2048, 0};
Adafruit_MCP4725 dac;

#ifndef cbi
#define cbi(sfr,bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
# ifndef sbi
#define sbi(sfr,bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define max1 950
#define min1 50

int prev = 0;
int check = false;
int prev1 = 0;
int upper = 0;
int count = 0;
int max = 0;
int start = 0;

String toSend = "";
int nowFrame = -1;
long timeOut = 200000;

#define r_slope 65
#define defaultFreq 1700

/*-------------------------------
 * FINDING CRC REDUNDANT
 ----------------------------- */
byte MakeCRC(String BitString)
{
   static char Res[5];                                 // CRC Result
   char CRC[4];
   int  i;
   char DoInvert;
   
   for (i=0; i<4; ++i)  CRC[i] = 0;                    // Init before calculation
   
   for (i=0; i<BitString.length(); ++i)
      {
      DoInvert = ('1'==BitString[i]) ^ CRC[3];         // XOR required?

      CRC[3] = CRC[2];
      CRC[2] = CRC[1];
      CRC[1] = CRC[0] ^ DoInvert;
      CRC[0] = DoInvert;
      }
      
   for (i=0; i<4; ++i)  Res[3-i] = CRC[i] ? '1' : '0'; // Convert binary to ASCII
   Res[4] = 0;                                         // Set string terminator

   byte redun = 0;
   for(i=0;i<4;i++) {
      redun |= (CRC[i]<<i);
   }

   return(redun);
}

/*-------------------------------
 * Making & Sending Frame
 *----------------------------- */
void makeFrame(int inData,int sequenceNumber,byte redundance)
{
  //----------Frame Start 11 11 11 -------------//

    Serial.print(3,BIN);
    for (int sl = 0; sl < 3*5; sl++)
    {
       for (int s = 0; s < 4; s++)
       {
          dac.setVoltage(S_DAC[s], false);
          delayMicroseconds(delay3);
       }
    }

  dac.setVoltage(0, false);
  //----------Frame Sequence 0x ----------//
  Serial.print("|");
  Serial.print(sequenceNumber,BIN);
  if(sequenceNumber==0) // 00
  {
     for (int sl = 0; sl < 2 ; sl++)
        {
          for (int s = 0; s < 4; s++)
          {
            dac.setVoltage(S_DAC[s], false); 
            delayMicroseconds(delay0);
          }
        }
        delayMicroseconds(1);
  }
  else if(sequenceNumber==1) // 01
  {
    for (int sl = 0; sl < 3; sl++)
    {
         for (int s = 0; s < 4; s++)
         {
           dac.setVoltage(S_DAC[s], false);
           delayMicroseconds(delay1);
         }
    }
  }
   else if(sequenceNumber==2) // 01
  {
    for (int sl = 0; sl < 4; sl++)
    {
         for (int s = 0; s < 4; s++)
         {
           dac.setVoltage(S_DAC[s], false);
           delayMicroseconds(delay2);
         }
    }
  }
   else if(sequenceNumber==3) // 01
  {
    for (int sl = 0; sl < 5; sl++)
    {
         for (int s = 0; s < 4; s++)
         {
           dac.setVoltage(S_DAC[s], false);
           delayMicroseconds(delay3);
         }
    }
  }
  else
  {
    Serial.println("Error sequence number");
  }
   dac.setVoltage(0, false);

  //----------Frame Data xx xx xx xx--------------//
  Serial.print("|");
  for (int k = 3; k >= 0; k--) {
      int temp = inData & 3;
      inData >>= 2;
      Serial.print(temp,BIN);
      Serial.print(" ");
      if (temp == 0) {
        for (int sl = 0; sl < 2 ; sl++)
        {
          for (int s = 0; s < 4; s++)
          {
            dac.setVoltage(S_DAC[s], false); 
            delayMicroseconds(delay0);
          }
        }
      }
      else if (temp == 1) {
        for (int sl = 0; sl < 3; sl++)
        {
          for (int s = 0; s < 4; s++)
          {
            dac.setVoltage(S_DAC[s], false);
            delayMicroseconds(delay1);
          }
        }
      }
      else if (temp == 2) {
        for (int sl = 0; sl < 4; sl++)
        {
          for (int s = 0; s < 4; s++)
          {
            dac.setVoltage(S_DAC[s], false);
            delayMicroseconds(delay2);
          }
        }
      }
      else if (temp == 3) {
        for (int sl = 0; sl < 5; sl++)
        {
          for (int s = 0; s < 4; s++)
          {
            dac.setVoltage(S_DAC[s], false);
            delayMicroseconds(delay3);
          }
        }
      }
    }
  
    dac.setVoltage(0, false);

    //------------ Frame Redundant xx xx-------//
    Serial.print("|");
    for(int i=0;i<2;i++)
    {
      int redun=redundance&3;
      redundance>>=2;
      Serial.print(redun,BIN);
      Serial.print(" ");  
      if (redun == 0) {
        for (int sl = 0; sl < 2 ; sl++)
        {
          for (int s = 0; s < 4; s++)
          {
            dac.setVoltage(S_DAC[s], false); 
            delayMicroseconds(delay0);
          }
        }
      }
      else if (redun == 1) {
        for (int sl = 0; sl < 3; sl++)
        {
          for (int s = 0; s < 4; s++)
          {
            dac.setVoltage(S_DAC[s], false);
            delayMicroseconds(delay1);
          }
        }
      }
      else if (redun == 2) {
        for (int sl = 0; sl < 4; sl++)
        {
          for (int s = 0; s < 4; s++)
          {
            dac.setVoltage(S_DAC[s], false);
            delayMicroseconds(delay2);
          }
        }
      }
      else if (redun == 3) {
        for (int sl = 0; sl < 5; sl++)
        {
          for (int s = 0; s < 4; s++)
          {
            dac.setVoltage(S_DAC[s], false);
            delayMicroseconds(delay3);
          }
        }
      }
    }
    
    //------------Frame Stop 00 00 00 --------------//
    Serial.print("|");
       Serial.print(0);
        for (int sl = 0; sl < 2*3 ; sl++)
        {
          for (int s = 0; s < 4; s++)
          {
            dac.setVoltage(S_DAC[s], false); 
            delayMicroseconds(delay0);
          }
          delayMicroseconds(1);
        }
    //------------End Frame--------------//
    Serial.println();
    dac.setVoltage(0, false);
    
}
/*-------------------------------
 * Sending a Character
 *----------------------------- */
void SendChar(byte inData) {
  bool isSent = false;
  long Time = 0; 
  byte Redun = MakeCRC(String(inData,BIN)); // Finding CRC yo!
  nowFrame = (nowFrame+1)%4; // Increase Frame

  // Sending Process Loop...
  Serial.print("Sending ");
  Serial.print((char)inData);
  while(!isSent) {
    makeFrame(inData,nowFrame,Redun); // makeFrame + SendFrame
    // Waiting for respond (ACK)
    while(mySerial.available()<2 && Time < timeOut) {
      Time++;  
    }

    if(Time >= timeOut) {
      Time = 0;
        //Serial.print("Resend");
        continue;
    }

    char resp = mySerial.read();
    char ackFrame = mySerial.read();
    ackFrame = (ackFrame>'0')? ackFrame-'0':ackFrame;

    Serial.print("ACK Recieved ");
    Serial.print('!');
    Serial.print(ackFrame,BIN);
    Serial.print(" [now:");
    Serial.println(nowFrame);
    
   if(resp=='!') {
      if(ackFrame>nowFrame || (ackFrame==0 && nowFrame==3) ) {
        // ACK
        Serial.println("Sent!");
        isSent = true;
        break;
      }
    } 
    
  }
    
  
}
/*-------------------------------
 * Sending ACK (for RX command)
 *----------------------------- */
void SendACK(int inData,int frameNo) {
  byte Redun = MakeCRC(String(inData,BIN)); // Finding CRC yo!

  Serial.print("Sending ACK ");
  Serial.print((char)inData);
  Serial.println(frameNo);

  makeFrame(inData,frameNo,Redun); // makeFrame + SendFrame
}

/*-------------------------------
 * Set UP
 *----------------------------- */
void setup() {
    // put your setup code here, to run once:
  Serial.begin(9600);
  mySerial.begin(9600);
  dac.begin(0x62);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
  pinMode(13, OUTPUT);

  delay0 = (1000000 / freq0 - 1000000 / defaultFreq) / 4;
  delay1 = (1000000 / freq1 - 1000000 / defaultFreq) / 4;
  delay2 = (1000000 / freq2 - 1000000 / defaultFreq) / 4;
  delay3 = (1000000 / freq3 - 1000000 / defaultFreq) / 4;
  
  Serial.flush();
  mySerial.flush();
}

/*-------------------------------
 * MAIN LOOP
 *----------------------------- */
void loop() {
  // put your main code here, to run repeatedly:
  
  if (Serial.available() > 0) {

    // Input data
    char inData = Serial.read();

    // Camera want to send color data
    if(inData == '$') {
      // LED just for Debugging
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
      
      SendChar(inData);
      while(Serial.available()==0) {}
      byte sendd = Serial.read();
      SendChar(sendd);
      while(Serial.available()==0) {}
      sendd = Serial.read();
      SendChar(sendd);
      while(Serial.available()==0) {}
      sendd = Serial.read();
      SendChar(sendd);
    } // Camera want to send color data
    else if(inData == ',') {
      // LED just for Debugging
      digitalWrite(13, HIGH);
      delay(50);
      digitalWrite(13, LOW);
      delay(50);
      digitalWrite(13, HIGH);
      delay(50);
      digitalWrite(13, LOW);
      delay(50);
      digitalWrite(13, HIGH);
      delay(50);
      digitalWrite(13, LOW);
      delay(50);

      SendChar(inData);
      while(Serial.available()==0) {}
      byte sendd = Serial.read();
      SendChar(sendd);
      while(Serial.available()==0) {}
      sendd = Serial.read();
      SendChar(sendd);
      while(Serial.available()==0) {}
      sendd = Serial.read();
      SendChar(sendd);
    }
  }

  // DATA Sent from RX

  if (mySerial.available() >= 2) {

    // Input data
    char inData = mySerial.read();
    int frameNo = mySerial.parseInt();
    
    // Check if want to send '?' ACK
    if (inData == '?') {
      for(int i=0;i<10;i++) {
        SendACK(inData,frameNo); // Sending data...
        delay(2);
      }
      
    }
  }
 
}
