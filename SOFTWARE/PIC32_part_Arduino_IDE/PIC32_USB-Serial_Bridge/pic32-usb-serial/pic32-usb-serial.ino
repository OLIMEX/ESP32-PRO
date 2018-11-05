int DTR=5;
int RTS=4;

bool dtr;
bool rts;

int baud=115200;
void readback()
{
  if (dtr != Serial.getDTR())
    {
      dtr = Serial.getDTR();
      if (dtr)
        digitalWrite(DTR, LOW);
      else
        digitalWrite(DTR, HIGH);

    }

  if (rts != Serial.getRTS())
    {
       rts = Serial.getRTS();

       if(rts)
          digitalWrite(RTS,LOW);
       else
          digitalWrite(RTS,HIGH);
    }
 
}


void setup() {
 
 pinMode(DTR,OUTPUT);
 pinMode(RTS,OUTPUT);
 digitalWrite(RTS, LOW);
 digitalWrite(DTR, HIGH);
 delay(100);
 digitalWrite(RTS, HIGH);

 ANSELC = 0x00;
 LATC = LATC & (~0x04);
 TRISC |= 0x04;
// CNPUC = CNPUC & (~0x04);
 

 dtr = Serial.getDTR();
 rts = Serial.getRTS();

 if (dtr) digitalWrite(DTR, LOW);
 else digitalWrite(DTR,HIGH);

 if(rts) digitalWrite(RTS, LOW);
 else digitalWrite(RTS,HIGH);
 
 Serial.begin(1000000);
 delay(100);
 Serial0.begin(115200);
 delay(100); 
 Serial1.begin(115200);
 delay(100);
}

void loop()
{

  readback();
  //digitalWrite(DTR, !Serial.getDTR());
  //digitalWrite(RTS, !Serial.getRTS());

  while(Serial.available() || Serial0.available() || Serial1.available())
  {
    if (Serial.available())
    {
      Serial0.write(Serial.read());
    }
    
    if (Serial0.available())
    {
      Serial.write(Serial0.read());
    }

    if(Serial1.available())
    {
      Serial1.write(Serial1.read());
    }
         
  }
 
    if (Serial.getBaudRate() != baud)
    {
        baud = Serial.getBaudRate();  
        Serial0.begin(Serial.getBaudRate());
                
    }
    
}

