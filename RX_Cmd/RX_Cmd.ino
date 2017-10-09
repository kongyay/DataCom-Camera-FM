#include<Wire.h>
#include <Adafruit_MCP4725.h>
#include <Adafruit_ADS1015.h>
#ifndef cbi
#define cbi(sfr,bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
# ifndef sbi
#define sbi(sfr,bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
#define max1 950
#define min1 50
#include <AltSoftSerial.h>

AltSoftSerial mySerial;

int prev = 0;
int check = false;
int prev1 = 0;
int upper = 0;
int count = 0;
int delay0 = 0;
int max = 0;
int start = 0;

String Data = "";
bool startRX = false;
int nowFrame = 0;
String allData = "";

#define r_slope 65
#define defaultFreq 1700

/*-------------------------------
 * Sum binary STRING to INT
 *----------------------------- */
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
 * SOLVING CRC REDUNDANT
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
 * Check data recieved
 ----------------------------- */
void checkData(String Data) { // 111111 FF DDDDDDDD RRRR 000000 //
  int inFrame = sumBinary(Data.substring(6,8));
  String ssData = Data.substring(14,16)+Data.substring(12,14)+Data.substring(10,12)+Data.substring(8,10);
  String ssCRC = Data.substring(18,20)+Data.substring(16,18);
  char inData = sumBinary(ssData);
  int inCRC = sumBinary(ssCRC);

  Serial.print("Frame: ");
  Serial.print(inFrame);
  Serial.print(" [now:");
  Serial.println(nowFrame);

  if(inData=='?') {
    // JUST ACK BRO
    Serial.print("GOT ACK");
    Serial.println(inFrame);
    mySerial.print('!');
    mySerial.print(inFrame);
    delay(100);
    return;
  }

  Serial.print("CRC: ");
  Serial.println(inCRC,BIN);
  bool correctData = (MakeCRC(ssData+ssCRC)==0 ? true:false); 
  if(!correctData) {
    Serial.println("CRC Not Match");
    //mySerial.print('?');
    //mySerial.print(nowFrame);
    return;
  }else {
    Serial.println("CRC Match!");
  }
  
  
  
  if(inFrame==nowFrame) {
    Serial.print("================== Data got: ");
    Serial.println(inData);
    allData += inData;
    //Serial.println(allData);
    nowFrame = (nowFrame+1)%4;
    
    return;
  } else {
    Serial.println("Frame not match!");
    
  }
  
  
  
  mySerial.print('?');
  mySerial.print(nowFrame);
}

/*-------------------------------
 * Get data from FM
 ----------------------------- */
void getData() {
  int tmp = analogRead(A0);
  if (tmp - prev1 > r_slope)
  {
    for (int k = 0; k < 13; k++)
    {
        for (int i = 0; i < 65; i++)
        {
          int tmp = analogRead(A0);
  
          if (tmp - prev > r_slope && check == false)
          {
            ////Serial.println(tmp);
            max = 0;
            check = true;
          }
          if (tmp > max)
          {
            max = tmp;
          }
          if (max - tmp > r_slope)
          {
            if (check == true)
            {
              count++;
            }
            check = false;
          }
          prev = tmp;
          delayMicroseconds(delay0);
      }

      if (count == 2)
      {
        Data += "00";
      }
      else if (count == 3)
      {
        Data += "01";
      }
      else if (count == 4)
      {
        Data += "10";
      }
      else if (count == 5)
      {
        Data += "11";
      }
        count = 0;

      
    }

      
      if(Data.length()>=26)  {
        Data = Data.substring(0,26);
        Serial.println(Data);
        if(Data.startsWith("111111")&&Data.endsWith("000000")) {
          
          checkData(Data);
        }
           
        Data = "";
      }
      else if(Data.length()>=6) {
          if(Data.startsWith("111111")) {
            
          } else { 
              Data = "";
          }
      }
            
  }
    
}


/*-------------------------------
 * Set UP
 ----------------------------- */
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  mySerial.begin(9600);
  pinMode(A0, INPUT);
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);

  delay0 = (1000000 / 1300 - 1000000 / defaultFreq) / 4;
  Serial.flush();
  mySerial.flush();
}

/*-------------------------------
 * MAIN LOOP
 *----------------------------- */
void loop() {
  // put your main code here, to run repeatedly:

  // always getting data
  getData();

    // if gotten data starts with $, send to TX (User)
    if(allData[0]=='$' && allData.length() >= 4) {
      Serial.println("Data Recieved...");
      Serial.println((uint8_t)allData[1],DEC);
      Serial.println((uint8_t)allData[2],DEC);
      Serial.println((uint8_t)allData[3],DEC);
      
      mySerial.print('A');  // A = got Color at 135
      mySerial.print(allData[1]);
      mySerial.print('B');  // B = got Color at 90
      mySerial.print(allData[2]);
      mySerial.print('C');  // C = got Color at 45
      mySerial.print(allData[3]);
      
      allData = allData.substring(4); 
    } else if(allData[0]==',' && allData.length() >= 4) {
      Serial.println("Sample Recieved...");
      Serial.println((uint8_t)allData[1],DEC);
      Serial.println((uint8_t)allData[2],DEC);
      Serial.println((uint8_t)allData[3],DEC);
      mySerial.print('J');
      mySerial.print(allData[1]);
      mySerial.print('K');
      mySerial.print(allData[2]);
      mySerial.print('L');
      mySerial.print(allData[3]);
      
      allData = allData.substring(4); 
    } 

    

}

