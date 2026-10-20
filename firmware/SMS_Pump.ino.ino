#include <SoftwareSerial.h>
#include <string.h>
#include <avr/wdt.h>


char *validNums[] = {
  "+381695410508",
  "+38163509052"
};


char bufferString[sizeof("Greska! Stanje releja: OFF; Stanje pumpe: OFF")];

int validNumType = 1;

char newMessageFlag = 0;
char receivedMessage[300];
unsigned char charNum = 0;
unsigned char numberChar = 0;
unsigned char messageChar = 0;

unsigned int extractionStarted = 0;
unsigned int extractionEnded = 0;

unsigned int messageExtractStart = 0;
unsigned int messageExtractEnd = 0;

unsigned long timerNow = 0;
unsigned long timerVal = 0;

char extractedNumber[50];
char extractedMessage[50];

byte relayStatus;

int validation1;

unsigned long globalTimer = 0;
unsigned long systemTime = 0;

int notificationOn = 1;

int notificationCounter = 1;

SoftwareSerial SIM800L(2, 3);

void setup()
{
  pinMode(13, OUTPUT);
  Serial.begin(115200);
  SIM800L.begin(9600);
  while (SIM800L.available() > 0) {
    SIM800L.read();
  }
  for (int i = 0; i < 15; i++)
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("OK");
  while (SIM800L.available() > 0) {
    SIM800L.read();
  }

  SIM800L.println("AT+CMGF=1");
  delay(1000);
  while (SIM800L.available() > 0) {
    SIM800L.read();
  }
  SIM800L.println("AT+CNMI=2,2,0,0,0"); // Probati da se stavi 1,2,0,0,0
  delay(1000);
  while (SIM800L.available() > 0) {
    SIM800L.read();
  }
  SIM800L.println("AT+GSMBUSY=1");
  delay(1000);
  while (SIM800L.available() > 0) {
    SIM800L.read();
  }
  systemTime = millis();

  delay(3000);

  SendSMS(validNums[0], "Sistem se ponovo pokrenuo!");

}

void loop()
{
  //Serial.println(analogRead(A0));
  charNum = 0;
  while (SIM800L.available() > 0)
  {
    timerNow = millis();
    receivedMessage[charNum] = SIM800L.read();
    delay(1);
    newMessageFlag = 1;
    charNum++;
  }
  timerVal = millis() - timerNow;
  if (newMessageFlag && timerVal > 10000)
  {
    for (int i = 3; i < 299; i++)
    {
      if (receivedMessage[i] == '"' && extractionEnded == 0)
      {
        if (extractionStarted == 0)
        {
          numberChar = 0;
          extractionStarted = 1;
        }
        else
        {
          extractionStarted = 0;
          extractionEnded = 1;
        }
      }

      if (receivedMessage[i] == 0x0D && messageExtractEnd == 0)
      {
        if (messageExtractStart == 0)
        {
          messageChar = 0;
          messageExtractStart = 1;
        }
        else
        {
          messageExtractStart = 0;
          messageExtractEnd = 1;
        }
      }

      if (extractionStarted)
      {
        extractedNumber[numberChar] = receivedMessage[i + 1]; // left '"'
        numberChar++;
      }
      if (messageExtractStart)
      {
        //Serial.write(receivedMessage[i+1]);
        extractedMessage[messageChar] = receivedMessage[i + 1];
        messageChar++;
      }
      Serial.write(receivedMessage[i]);
      receivedMessage[i] = NULL;
    }
    newMessageFlag = 0;
  }

  if (extractionEnded)
  {
    validation1 = 1;
    extractionEnded = 0;
    Serial.println("");
    for (int i = 0; i < 12; i++)
    {
      Serial.write(extractedNumber[i]);
      if ( (extractedNumber[i] != validNums[0][i]) && (extractedNumber[i] != validNums[1][i]) )
      {
        validation1 = 0;
      }
    }
    Serial.println("");
    Serial.println(validation1);
  }

  if (messageExtractEnd)
  {
    int validation = 1;
    messageExtractEnd = 0;
    Serial.println("");

    if (extractedMessage[1] == 'O' && extractedMessage[2] == 'F' && extractedMessage[3] == 'F' && validation1)
    {
      validation = 2;
      globalTimer = 0;
      digitalWrite(13, LOW);
      int valueOfPump = analogRead(A0);
      for (int i = 0; i < 10; i++) {
        delay(1000);
        valueOfPump = analogRead(A0);
      }
      valueOfPump = analogRead(A0);
      Serial.print("Value of pump = ");
      Serial.println(valueOfPump);
      if (valueOfPump > 512)
      {
        validNumType = 1;
        for (int i = 0; i < 12; i++)
        {
          if ((extractedNumber[i] != validNums[0][i]))
          {
            validNumType = 2;
          }
        }
        for (int i = 0; i < 12; i++)
        {
          if ((extractedNumber[i] != validNums[1][i]) && validNumType == 2)
          {
            validNumType = 0;
          }
        }
        sprintf(bufferString, "Greska! Stanje releja: OFF; Stanje pumpe: ON");
        Serial.println(bufferString);
        if (validNumType == 1)
          SendSMS(validNums[0], bufferString);
        else if (validNumType == 2)
          SendSMS(validNums[1], bufferString);

      }
      else
      {
        validNumType = 1;
        for (int i = 0; i < 12; i++)
        {
          if ((extractedNumber[i] != validNums[0][i]))
          {
            validNumType = 2;
          }
        }
        for (int i = 0; i < 12; i++)
        {
          if ((extractedNumber[i] != validNums[1][i]) && validNumType == 2)
          {
            validNumType = 0;
          }
        }
        sprintf(bufferString, "Uspesno ste iskljucili pumpu!");
        Serial.println(bufferString);
        if (validNumType == 1 && notificationOn)
          SendSMS(validNums[0], bufferString);
        else if (validNumType == 2 && notificationOn)
          SendSMS(validNums[1], bufferString);
      }
    }
    else if (extractedMessage[1] == 'O' && extractedMessage[2] == 'N' && validation1)
    {
      if (extractedMessage[3] >= '0' && extractedMessage[3] <= '9' && extractedMessage[4] >= '0' && extractedMessage[4] <= '9' && extractedMessage[4] >= '0' && extractedMessage[4] <= '9')
      {
        globalTimer = (extractedMessage[3] - 48) * 100 +  (extractedMessage[4] - 48) * 10 + (extractedMessage[5] - 48);
        sprintf(bufferString, "Timer set to %d\n", globalTimer);
        Serial.println(bufferString);
        validation = 1;
        digitalWrite(13, HIGH);
        delay(2000);
        validNumType = 1;
        for (int i = 0; i < 12; i++)
        {
          if ((extractedNumber[i] != validNums[0][i]))
          {
            validNumType = 2;
          }
        }
        for (int i = 0; i < 12; i++)
        {
          if ((extractedNumber[i] != validNums[1][i]) && validNumType == 2)
          {
            validNumType = 0;
          }
        }
        if (analogRead(A0) < 512)
        {
          sprintf(bufferString, "Greska! Stanje releja: ON; Stanje pumpe: OFF");
          Serial.println(bufferString);
          if (validNumType == 1)
            SendSMS(validNums[0], bufferString);
          else if (validNumType == 2)
            SendSMS(validNums[1], bufferString);
          delay(3000);
        }
        else
        {
          sprintf(bufferString, "Uspesno ste ukljucili pumpu!");
          Serial.println(bufferString);
          if (validNumType == 1 && notificationOn)
            SendSMS(validNums[0], bufferString);
          else if (validNumType == 2 && notificationOn)
            SendSMS(validNums[1], bufferString);
        }
      }
	  else if(extractedMessage[3] == 'X')
	  {
		globalTimer = 9999;
		sprintf(bufferString, "Timer set to %d\n", globalTimer);
        Serial.println(bufferString);
        validation = 1;
        digitalWrite(13, HIGH);
        delay(2000);
        validNumType = 1;
        for (int i = 0; i < 12; i++)
        {
          if ((extractedNumber[i] != validNums[0][i]))
          {
            validNumType = 2;
          }
        }
        for (int i = 0; i < 12; i++)
        {
          if ((extractedNumber[i] != validNums[1][i]) && validNumType == 2)
          {
            validNumType = 0;
          }
        }
        if (analogRead(A0) < 512)
        {
          sprintf(bufferString, "Greska! Stanje releja: ON; Stanje pumpe: OFF");
          Serial.println(bufferString);
          if (validNumType == 1)
            SendSMS(validNums[0], bufferString);
          else if (validNumType == 2)
            SendSMS(validNums[1], bufferString);
          delay(3000);
        }
        else
        {
          sprintf(bufferString, "Uspesno ste ukljucili pumpu!");
          Serial.println(bufferString);
          if (validNumType == 1 && notificationOn)
            SendSMS(validNums[0], bufferString);
          else if (validNumType == 2 && notificationOn)
            SendSMS(validNums[1], bufferString);
        }  
	  }
      else
      {
        validNumType = 1;
        for (int i = 0; i < 12; i++)
        {
          if ((extractedNumber[i] != validNums[0][i]))
          {
            validNumType = 2;
          }
        }
        for (int i = 0; i < 12; i++)
        {
          if ((extractedNumber[i] != validNums[1][i]) && validNumType == 2)
          {
            validNumType = 0;
          }
        }
        if (digitalRead(13) == 1)
        {
          if (validNumType == 1)
          {
            SendSMS(validNums[0], "Primljena je poruka koja je neprihvacena! RELAY: ON");
          }
          else if (validNumType == 2)
          {
            SendSMS(validNums[1], "Primljena je poruka koja je neprihvacena! RELAY: ON");
          }
        }

        else
        {
          if (validNumType == 1)
          {
            SendSMS(validNums[0], "Primljena je poruka koja je neprihvacena! RELAY: OFF");
          }
          else if (validNumType == 2)
          {
            SendSMS(validNums[1], "Primljena je poruka koja je neprihvacena! RELAY: OFF");
          }
        }

        validation = 0;
      }
    }
    else if (extractedMessage[1] == 'C' && extractedMessage[2] == 'H' && validation1)
    {
      validNumType = 1;
      for (int i = 0; i < 12; i++)
      {
        if ((extractedNumber[i] != validNums[0][i]))
        {
          validNumType = 2;
        }
      }
      for (int i = 0; i < 12; i++)
      {
        if ((extractedNumber[i] != validNums[1][i]) && validNumType == 2)
        {
          validNumType = 0;
        }
      }
      validation = 3;
      if (validNumType == 1)
      {
        if (digitalRead(13) == 1)
        {
          if (analogRead(A0) > 512)
          {
            sprintf(bufferString, "Relej:ON\n Preostalo vreme: %d min\n Pumpa:ON", globalTimer);
          }
          else
          {
            sprintf(bufferString, "Relej:ON\n Preostalo vreme: %d min\n Pumpa:OFF", globalTimer);
          }
          SendSMS(validNums[0], bufferString);
        }
        else
        {
          if (analogRead(A0) > 512)
          {
            sprintf(bufferString, "Relej:OFF\n Pumpa:ON", globalTimer);
          }
          else
          {
            sprintf(bufferString, "Relej:OFF\n Pumpa:OFF", globalTimer);
          }
          SendSMS(validNums[0], bufferString);
        }
      }
      else if (validNumType == 2)
      {
        if (digitalRead(13) == 1)
        {
          if (analogRead(A0) > 512)
          {
            sprintf(bufferString, "Relej:ON\n Preostalo vreme: %d min\n Pumpa:ON", globalTimer);
          }
          else
          {
            sprintf(bufferString, "Relej:ON\n Preostalo vreme: %d min\n Pumpa:OFF", globalTimer);
          }
          SendSMS(validNums[1], bufferString);
        }
        else
        {
          if (analogRead(A0) > 512)
          {
            sprintf(bufferString, "Relej:OFF\n Pumpa:ON", globalTimer);
          }
          else
          {
            sprintf(bufferString, "Relej:OFF\n Pumpa:OFF", globalTimer);
          }
          SendSMS(validNums[1], bufferString);
        }
      }
    }

    else if (extractedMessage[1] == 'N' && extractedMessage[2] == '1' && validation1)
    {
      notificationOn = 1;
      Serial.println("Notifications are now on!");
    }

    else if (extractedMessage[1] == 'N' && extractedMessage[2] == '0' && validation1)
    {
      notificationOn = 0;
      Serial.println("Notifications are now off");
    }


    else
    {
/*       if (digitalRead(13) == 1)
        SendSMS(validNums[0], "Primljena je poruka koja je neprihvacena! RELAY: ON");
      else
        SendSMS(validNums[0], "Primljena je poruka koja je neprihvacena! RELAY: OFF");

      validation = 0; */
    }

    Serial.println("");
    Serial.println(validation);
  }
  checkSsystemValidation();

  if (globalTimer > 9999)
  {
    globalTimer = 0;
    digitalWrite(13, LOW); // turn off after timer expired
  }

  else if (globalTimer > 0)
  {
    if (millis() - systemTime > 60000)
    {
      systemTime = millis();
      globalTimer--;
    }
    digitalWrite(13, HIGH); // in case of bugs
    //sprintf(bufferString, "GlobalTimer = %d\n", globalTimer);
    //Serial.println(bufferString);
    notificationCounter = 0;
  }

  else
  {
    if (notificationCounter == 0 && digitalRead(13) == 1)
    {
      notificationCounter = 1;
      sprintf(bufferString, "Automatsko iskljucenje pumpe!");
      Serial.println(bufferString);
      if (validNumType == 1 && notificationOn)
        SendSMS(validNums[0], bufferString);
      else if (validNumType == 2 && notificationOn)
        SendSMS(validNums[1], bufferString);
    }
    digitalWrite(13, LOW);
  }
}


void checkSsystemValidation()
{
  static char validationErrorRelayOn = 0;
  static char validationErrorRelayOff = 0;

  if ( (digitalRead(13) == 1) && (analogRead(A0) < 512) && (validationErrorRelayOn == 0) )
  {
    validationErrorRelayOn = 1;
    Serial.println("Greska! Stanje releja je ON dok je stanje pumpe OFF");
    SendSMS(validNums[0], "Greska! Stanje releja je ON dok je stanje pumpe OFF");
  }
  else if ( (digitalRead(13) == 0) && (analogRead(A0) > 512) && (validationErrorRelayOff == 0))
  {
    delay(5000);
    if ( (digitalRead(13) == 0) && (analogRead(A0) > 512) && (validationErrorRelayOff == 0))
    {
      validationErrorRelayOff = 1;
      Serial.println("Greska! Stanje releja je OFF dok je stanje pumpe ON");
      SendSMS(validNums[0], "Greska! Stanje releja je OFF dok je stanje pumpe ON");
    }
  }
  else if ( ((digitalRead(13) == 1) && (analogRead(A0) > 512)) || ((digitalRead(13) == 0) && (analogRead(A0) < 512)) )
  {
    validationErrorRelayOn = 0;
    validationErrorRelayOff = 0;
  }
}


void SendSMS(String phoneNumber, String message)
{
  // Postavi broj telefona
  SIM800L.print("AT+CMGS=\"");
  SIM800L.print(phoneNumber);
  SIM800L.println("\"");
  delay(100);

  // Pošalji poruku
  SIM800L.print(message);
  delay(100);

  // Pošalji Ctrl+Z (ASCII 26) da označiš kraj poruke
  SIM800L.write(26);
  delay(8000);

  while (SIM800L.available()) {
    Serial.write(SIM800L.read());
  }

}
