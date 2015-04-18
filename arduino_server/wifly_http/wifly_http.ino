#include <IRremote.h>

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFly.h>
#include "HTTPClient.h"

#define SSID      "Xiaomi_JXITC"
#define KEY       "xjiang318"
// WIFLY_AUTH_OPEN / WIFLY_AUTH_WPA1 / WIFLY_AUTH_WPA1_2 / WIFLY_AUTH_WPA2_PSK
#define AUTH      WIFLY_AUTH_WPA2_PSK

#define HTTP_GET_URL "http://192.168.31.184:5000/"
#define HTTP_POST_URL "http://httpbin.org/post"
#define HTTP_POST_DATA "Hello world!"

#define SEND_NEC

// Pins' connection
// Arduino       WiFly
//  2    <---->    TX
//  3    <---->    RX
SoftwareSerial uart(2, 3);
WiFly wifly(uart);
HTTPClient http;
char get;

// IR Code
unsigned long IR_ON_OFF = 0x32A650AF;
unsigned long IR_VOL_UP = 0x32A6A857;
unsigned long IR_VOL_DOWN = 0x32A638C7;
unsigned long IR_CHN_UP = 0x32A6F807;
unsigned long IR_CHN_DOWN = 0x32A67887;
IRsend irsend;
// board setup is easy! connect LED distroted pin to #3, and straight pin to GND


// Xiao's command findvars
char *line = new char[1000];
int linePos = 0;

const char *CMD_PREFIX = "CMD\0";
const int CMD_PREFIX_LEN = 3;
int cmdPrefixIdx = 0;

/*
 * Find command in Stream
 */
int getCommand(char c) {
  if (cmdPrefixIdx == CMD_PREFIX_LEN) {
    cmdPrefixIdx = 0;  // reset
    return c - '0';
  }

  if (CMD_PREFIX[cmdPrefixIdx] == c) {
    if (cmdPrefixIdx < CMD_PREFIX_LEN) {
      // 0,1
      cmdPrefixIdx++;
    }
  } else {
    cmdPrefixIdx = 0;
    return -1;
  }

  return -1;
}

void sendIRSignal(unsigned long signal, int repeat) {
  for (int i = 0; i < repeat; i++) {

  }
}


void setup() {
  Serial.begin(9600);
  Serial.println("------- WIFLY HTTP --------");

  const char *cmdPrefix = "Hello";
  int iCur = 0;

  uart.begin(9600);         // WiFly UART Baud Rate: 9600
  // Wait WiFly to init
  //  delay(3000);

  // IR setup
  /*
  Serial.println("Start IR setup");
  pinMode(3, OUTPUT);
  digitalWrite(3, HIGH);
  delay(1000);
  digitalWrite(3, LOW);
  Serial.println("End IR setup");
  */

  // check if WiFly is associated with AP(SSID)
  if (!wifly.isAssociated(SSID)) {
    while (!wifly.join(SSID, KEY, AUTH)) {
      Serial.println("Failed to join " SSID);
      Serial.println("Wait 0.1 second and try again...");
      delay(100);
    }

    wifly.save();    // save configuration,
  }

  wifly.sendCommand("set com remote 0\r");
  Serial.println("Connected to network, start loop querying...");

  while (1) {
    Serial.println("------------------------------");
    Serial.println("\r\nQuerying server for commands - " HTTP_GET_URL);
    Serial.println("------------------------------");
    while (http.get(HTTP_GET_URL, 10000) < 0) {
    }

    while (wifly.receive((uint8_t *)&get, 1, 1000) == 1) {
      //Serial.print(get);
      int cmdId = getCommand(get);
      if (cmdId != -1) {
        Serial.print("Detected command on char: ");
        Serial.print(get);
        Serial.print("  cmd_id: ");
        Serial.println(cmdId);

        Serial.println("Send IR signal");
        irsend.sendNEC(IR_VOL_UP, 32);
        delay(100);
        irsend.sendNEC(IR_VOL_UP, 32);
        delay(100);
        irsend.sendNEC(IR_VOL_DOWN, 32);
        delay(100);
        irsend.sendNEC(IR_VOL_DOWN, 32);
        delay(100);
      }
    }
    delay(1);
  }

  Serial.println("\r\n\r\nTry to post data to url - " HTTP_POST_URL);
  Serial.println("-------------------------------");
  while (http.post(HTTP_POST_URL, HTTP_POST_DATA, 10000) < 0) {
  }
  while (wifly.receive((uint8_t *)&get, 1, 1000) == 1) {
    Serial.print(get);
  }

  if (wifly.commandMode()) {
    Serial.println("\r\n\r\nEnter command mode. Send \"exit\"(with \\r) to exit command mode");
  }
}


void loop() {
  int c;
  while (wifly.available()) {
    c = wifly.read();
    if (c > 0) {
      Serial.write((char)c);
    }
  }

  while (Serial.available()) {
    c = Serial.read();
    if (c >= 0) {
      wifly.write((char)c);
    }
  }
}




