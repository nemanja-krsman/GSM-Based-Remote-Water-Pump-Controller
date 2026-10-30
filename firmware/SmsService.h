#ifndef SMS_SERVICE_H
#define SMS_SERVICE_H

#include <Arduino.h>
#include <SoftwareSerial.h>

extern SoftwareSerial SIM800L;

void SendSMS(String phoneNumber, String message);

#endif
