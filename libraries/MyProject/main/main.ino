
#include <WiFi.h>
#include <stdlib.h>
#include "Arduino.h"

#include "emulatetag.h"
#include "NdefMessage.h"

#include "PN532_HSU.h"
#include "PN532.h"
#include "NfcAdapter.h"


#define LED1 2

#define ReadMode    2

#define NDEF_DEBUG

//

String inputString =""; 
char ssid[20]   = "";
char password[20] = "";
bool stringComplete = false;
bool ssidComplete = false;
bool passwordComplete = false;
int  num = 0;


HardwareSerial NFCSerial(2);
PN532_HSU pn532hsu(NFCSerial);

#if ReadMode==1
NfcAdapter nfc = NfcAdapter(pn532hsu);
#elif ReadMode==2
PN532 nfc(pn532hsu);
#else
EmulateTag nfc(pn532hsu);
uint8_t ndefBuf[120];
NdefMessage message;
int messageSize;
uint8_t uid[3] = { 0x12,0x34,0x56 };
#endif




void setup()
{
    Serial.begin(115200);
    delay(10);

#if ReadMode==1
    Serial.println("NDEF Reader");
    nfc.begin();
#elif ReadMode==2
  Serial.println("-------Peer to Peer HCE--------");
    nfc.begin();
    uint32_t versiondata = nfc.getFirmwareVersion();
    if (! versiondata) {
      Serial.print("Didn't find PN53x board");
      while (1); // halt
    }
    
    // Got ok data, print it out!
    Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
    Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
    Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
    
    // Set the max number of retry attempts to read from a card
    // This prevents us from waiting forever for a card, which is
    // the default behaviour of the PN532.
    //nfc.setPassiveActivationRetries(0xFF);
    
    // configure board to read RFID tags
    nfc.SAMConfig();
#else
    Serial.println("------- Emulate Tag --------");

    message = NdefMessage();
    message.addTextRecord("11111");
    messageSize = message.getEncodedSize();
    Serial.println("messageSize:");
    Serial.println(messageSize);
    if (messageSize > sizeof(ndefBuf)) {
      Serial.println("ndefBuf is too small");
      while (1) { }
    }
  
    Serial.print("Ndef encoded message size: ");
    Serial.println(messageSize);

  //  message.encode(ndefBuf);

//    nfc.setTagWriteable(true);
  
  // comment out this command for no ndef message
    nfc.setNdefFile(ndefBuf, messageSize);
  
  // uid must be 3 bytes!
    nfc.setUid(uid);
  
    nfc.init();
#endif
    
    

    // We start by connecting to a WiFi network

//    Serial.print("Connecting to ");
//    Serial.println(ssid);

//    WiFi.begin(ssid, password);

    pinMode(LED1, OUTPUT);
    digitalWrite(LED1, LOW);
    Serial.println("Init Finish");

//    while (WiFi.status() != WL_CONNECTED) {
//        delay(1000);
//        digitalWrite(LED1, HIGH);
//        delay(1000);
//        digitalWrite(LED1, LOW);
//      //dd  Serial.print(".");
//    }
//
//    Serial.println("WiFi connected");
//    Serial.println("IP address: ");
//    Serial.println(WiFi.localIP());
//    digitalWrite(LED1, HIGH);
}

int value = 0;

void loop()
{
//    while (Serial.available()){
//        char inChar = (char)Serial.read();
//        
//        if (inChar == '\n') {
//          num++;
//          stringComplete = true;
//        }
//        else{
//          inputString += inChar;
//        }
//    }
//    if(stringComplete == true){
//      stringComplete = false;
//      if(num == 1){
//        inputString.toCharArray(ssid, inputString.length()+1);
//      }
//      else if(num == 2){
//        inputString.toCharArray(password, inputString.length()+1);
//      }
//      Serial.println(inputString);
//      inputString =""; 
//    }
//
//    if(num == 2){
//      num = 0;
//      WiFi.begin(ssid, password);
//      while (WiFi.status() != WL_CONNECTED) {
//        delay(1000);
//        digitalWrite(LED1, HIGH);
//        delay(1000);
//        digitalWrite(LED1, LOW);
//      }
//      digitalWrite(LED1, HIGH);
//      Serial.println("WiFi connected");
//      Serial.println("IP address: ");
//      Serial.println(WiFi.localIP());
//      digitalWrite(LED1, HIGH);
//    }
//    else{
//    ;
//    }




#if ReadMode==1
     Serial.println("\nScan a NFC tag\n");
    if (nfc.tagPresent())
    {
        NfcTag tag = nfc.read();
        tag.print();
    }
#elif ReadMode==2
  bool success;
  
  uint8_t responseLength = 32;
  
  Serial.println("Waiting for an ISO14443A card");
  
  // set shield to inListPassiveTarget
  success = nfc.inListPassiveTarget();

  if(success) {
   
     Serial.println("Found something!");
                  
    uint8_t selectApdu[] = { 0x00, /* CLA */
                              0xA4, /* INS */
                              0x04, /* P1  */
                              0x00, /* P2  */
                              0x07, /* Length of AID  */
                              0xF0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, /* AID defined on Android App */
                              0x00  /* Le  */ };
                              
    uint8_t response[32];  
     
    success = nfc.inDataExchange(selectApdu, sizeof(selectApdu), response, &responseLength);
    
    if(success) {
      
      Serial.print("responseLength: "); Serial.println(responseLength);
       
      nfc.PrintHexChar(response, responseLength);
      
      do {
        uint8_t apdu[] = "Hello from Arduino";
        uint8_t back[32];
        uint8_t length = 32; 

        success = nfc.inDataExchange(apdu, sizeof(apdu), back, &length);
        
        if(success) {
         
          Serial.print("responseLength: "); Serial.println(length);
           
          nfc.PrintHexChar(back, length);
        }
        else {
          
          Serial.println("Broken connection?"); 
        }
      }
      while(success);
    }
    else {
     
      Serial.println("Failed sending SELECT AID"); 
    }
  }
  else {
   
    Serial.println("Didn't find anything!");
  }
#else
     // start emulation (blocks)
    nfc.emulate();
        
    // or start emulation with timeout
    /*if(!nfc.emulate(1000)){ // timeout 1 second
      Serial.println("timed out");
    }*/
    
    // deny writing to the tag
    // nfc.setTagWriteable(false);
    
    if(nfc.writeOccured()){
       Serial.println("\nWrite occured !");
       uint8_t* tag_buf;
       uint16_t length;
       
       nfc.getContent(&tag_buf, &length);
       NdefMessage msg = NdefMessage(tag_buf, length);
       msg.print();
    }
#endif



    delay(1000);
//
//    Serial.print("connecting to ");
//    Serial.println(host);
//
//    // Use WiFiClient class to create TCP connections
//    WiFiClient client;
//    const int httpPort = 80;
//    if (!client.connect(host, httpPort)) {
//        Serial.println("connection failed");
//        return;
//    }
//
//    // We now create a URI for the request
//    String url = "/input/";
//    url += streamId;
//    url += "?private_key=";
//    url += privateKey;
//    url += "&value=";
//    url += value;
//
//    Serial.print("Requesting URL: ");
//    Serial.println(url);
//
//    // This will send the request to the server
//    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
//                 "Host: " + host + "\r\n" +
//                 "Connection: close\r\n\r\n");
//    unsigned long timeout = millis();
//    while (client.available() == 0) {
//        if (millis() - timeout > 5000) {
//            Serial.println(">>> Client Timeout !");
//            client.stop();
//            return;
//        }
//    }
//
//    // Read all the lines of the reply from server and print them to Serial
//    while(client.available()) {
//        String line = client.readStringUntil('\r');
//        Serial.print(line);
//    }
//
//    Serial.println();
//    Serial.println("closing connection");
}
