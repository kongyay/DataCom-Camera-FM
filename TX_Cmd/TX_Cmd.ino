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
bool sampleGot = false;

String toSend = "";
int nowFrame = -1;
long timeOut = 200000;

char colorDATA[3] = {0,0,0};

#define r_slope 65
#define defaultFreq 1700

int sumBinary(String str)//For Reciever
{
  int sum=0;
  int mul=1;
//  //Serial.print("***");
//  //Serial.print(str);
  for(int i=str.length()-1;i>=0;i--)
  {
    int strBit=str[i]-'0';
    int tmp = strBit*mul;
    mul*=2;
    sum+=(tmp);
  }
  return sum;
}
/*-------------------------------
 * FINDING CRC REDUNDANT
 ----------------------------- */
byte MakeCRC(String BitString)
{ 
   static char Res[5];                                 
   char CRC[4];
   int  i;
   char DoInvert; //10000110
   
   for (i=0; i<4; ++i)  CRC[i] = 0;                    
   
   for (i=0; i<BitString.length(); ++i)
   {
      DoInvert = ('1'==BitString[i]) ^ CRC[3];         

      CRC[3] = CRC[2];
      CRC[2] = CRC[1];
      CRC[1] = CRC[0] ^ DoInvert;
      CRC[0] = DoInvert;
   }

   byte redun = 0;                        // convert int array {x,x,x,x} -> byte xxxx
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

    //Serial.print(3,BIN);
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
  //Serial.print("|");
  //Serial.print(sequenceNumber,BIN);
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
  //Serial.print("|");
  for (int k = 3; k >= 0; k--) {
      int temp = inData & 3;
      inData >>= 2;
      //Serial.print(temp);
      //Serial.print(" ");
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
    //Serial.print("|");
    for(int i=0;i<2;i++)
    {
      int redun=redundance&3;
      redundance>>=2;
      //Serial.print(redun);
      //Serial.print(" ");  
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
    //Serial.print("|");
       //Serial.print(0);
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
    //Serial.println();
    dac.setVoltage(0, false);
    
}

/*-------------------------------
 * Sending a Character
 *----------------------------- */
void SendChar(char inData) {
  bool isSent = false;
  long Time = 0; 
  byte Redun = MakeCRC(String(inData,BIN)); // Finding CRC yo!
  nowFrame = (nowFrame+1)%4; // Increase Frame

  // Sending Process Loop...
  //Serial.print("Sending ");
  //Serial.print((char)inData);
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

    //Serial.print("ACK Recieved ");
    //Serial.print('!');
    //Serial.print(ackFrame,BIN);
    //Serial.print(" [now:");
    //Serial.println(nowFrame);
    
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

  //Serial.print("Sending ACK ");
  //Serial.print((char)inData);
  //Serial.println(frameNo);

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
    

    // User Type Command.......
    if (inData >= 32) {
      toSend += inData;
      if(toSend == "Start") { // START COMMAND
        Serial.println("Starting....");
        SendChar('0'); 
        toSend = "";
      } else if(toSend == "45") {              // Manual Rotate Command
        Serial.println("Turn to 45...");
        SendChar('1'); 
        toSend = "";
      } else if(toSend == "90") {
        Serial.println("Turn to 90...");
        SendChar('2'); 
        toSend = "";
      } else if(toSend == "135") {
        Serial.println("Turn to 135...");
        SendChar('3'); 
        toSend = "";
      } 
      
      
      
      
      else if(toSend == "Rotate ") {              // Rotate after start command
        char toRot = Serial.parseInt();
        if(toRot==colorDATA[0]&&colorDATA[0]!=0) {
          Serial.println("Turn to 135...");
          SendChar('3'); 
        } else if(toRot==colorDATA[1]&&colorDATA[1]!=0) {
          Serial.println("Turn to 90...");
          SendChar('2'); 
        } else if(toRot==colorDATA[2]&&colorDATA[2]!=0) {
          Serial.println("Turn to 45...");
          SendChar('1'); 
        }
        
        toSend = "";
      } 

      else if(toSend == "Get ") {              // Rotate after start command
        char toRot = Serial.parseInt();
        SendChar('A'+toRot-1);
        Serial.println("Getting Sample...");
        
        toSend = "";
      } 

      else if(toSend == "Find ") {                // Finding old color with new position
        char toRot = Serial.parseInt();
        if(toRot==colorDATA[0]&&colorDATA[0]!=0) {
          Serial.print("Finding... ");
          Serial.println(colorDATA[0],DEC);
          SendChar('6'); 
        } else if(toRot==colorDATA[1]&&colorDATA[1]!=0) {
          Serial.print("Finding... ");
          Serial.println(colorDATA[1],DEC);
          SendChar('5'); 
        } else if(toRot==colorDATA[2]&&colorDATA[2]!=0) {
          Serial.print("Finding... ");
          Serial.println(colorDATA[2],DEC);
          SendChar('4'); 
        }
        
        toSend = "";
      } 
      
      
    }

    if(inData == '*') {
      toSend = "";
    }

  }

   // DATA Sent from RX
  if (mySerial.available() >= 2) {
    // Input data
    char inData = mySerial.read();

 
    if(inData == 'A') { // A = got Color at 135
        colorDATA[0] = mySerial.read();
        Serial.print("Average Color at 135 = ");
        Serial.println((uint8_t)colorDATA[0],DEC);
    } else if(inData == 'B') { // B = got Color at 90
        colorDATA[1] = mySerial.read();
        Serial.print("Average Color at 90 = ");
        Serial.println((uint8_t)colorDATA[1],DEC);
    } else if(inData == 'C') { // C = got Color at 45
        colorDATA[2] = mySerial.read();
        Serial.print("Average Color at 45 = ");
        Serial.println((uint8_t)colorDATA[2],DEC);
    }
    
    
    else if(inData == 'J') { 
      char dat = mySerial.read();
        Serial.print("Sample = ");
        Serial.print((uint8_t)dat,DEC);
    } else if(inData == 'K') { 
      char dat = mySerial.read();
        Serial.print("(");
        Serial.print((uint8_t)dat,DEC);
    } else if(inData == 'L') { 
      char dat = mySerial.read();
        Serial.print(",");
        Serial.print((uint8_t)dat,DEC);
        Serial.println(")");
    } 
    
    else if (inData == '?') { // ? = RX want to send ACK
       int frameNo = mySerial.parseInt();
      for(int i=0;i<10;i++) {
        SendACK(inData,frameNo); // Sending data...
        delay(2);
      }
    } else if(inData == '!' ) { // ! = TRASH ACK
        mySerial.read(); // // JUST CLEARING TRASH ACK
    }


  }

}
