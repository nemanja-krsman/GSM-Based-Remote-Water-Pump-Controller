#include "SmsService.h"

SoftwareSerial SIM800L(2, 3);

void SendSMS(String phoneNumber, String message)
{
  // Set the phone number
  SIM800L.print("AT+CMGS=\"");
  SIM800L.print(phoneNumber);
  SIM800L.println("\"");
  delay(100);

  // Send the message
  SIM800L.print(message);
  delay(100);

  // Send (ASCII 26) for message end
  SIM800L.write(26);
  delay(8000);

  while (SIM800L.available()) {
    Serial.write(SIM800L.read());
  }

}
