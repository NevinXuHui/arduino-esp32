#include <WiFi.h>
#include <stdlib.h>
#include "PN532_HSU.h"
#include "PN532.h"
#include "PN532_debug.h"


#define LED1 2
#define NFCDATASTART  4


String inputString =""; 
char ssid[20]   = "";
char password[20] = "";
bool stringComplete = false;
bool ssidComplete = false;
bool passwordComplete = false;
int  num = 0;

      
PN532_HSU pn532hsu(Serial2);
PN532 nfc(pn532hsu);


void setup()
{    
    Serial.begin(115200);
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

    pinMode(LED1, OUTPUT);
    digitalWrite(LED1, LOW);
    Serial.println("Init Finish");
}

void WiFi_Function(){
		static uint8_t i = 0;
  
    while (Serial.available()){
        char inChar = (char)Serial.read();
        
        if (inChar == '\n') {
          num++;
          stringComplete = true;
        }
        else{
          inputString += inChar;
        }
    }
    if(stringComplete == true){
      stringComplete = false;
      if(num == 1){
        inputString.toCharArray(ssid, inputString.length()+1);
      }
      else if(num == 2){
        inputString.toCharArray(password, inputString.length()+1);
      }
      Serial.println(inputString);
      inputString =""; 
    }

    if(num == 2){
      num = 0;
      WiFi.begin(ssid, password);
    }
    else{
    ;
    }
		if(WiFi.status() != WL_CONNECTED){
			uint8_t temp = i%2;
			uint8_t temp2 = i%1;
			DMSG("i:");
  		DMSG_INT(i);
  		DMSG("\n");
			DMSG("temp:");
  		DMSG_INT(temp);
  		DMSG("\n");
			DMSG("temp2:");
  		DMSG_INT(temp2);
  		DMSG("\n");
				if(temp== 0){
        	digitalWrite(LED1, HIGH);	
					}
				else if(temp2 == 0){
        	digitalWrite(LED1, LOW);	
					}
				i++;
				
		}else{
				Serial.println("WiFi connected");
      	Serial.println("IP address: ");
      	Serial.println(WiFi.localIP());
      	digitalWrite(LED1, HIGH);
		}

}

int DeCodeNFCReceiceData(uint8_t *data){

  uint8_t index1,index2,index3,index4;
  uint8_t index1Length,index2Length,index3Length,index4Length;
  


  if(0 != data[0] || 0xA4!= data[1] || 0x04 != data[2] || 0x00 != data[3]){
        DMSG("DeCodeNFCReceiceData  error1\n");
		return -1;
    }
  uint8_t Totallength = (data[NFCDATASTART]-0x30)*10+data[NFCDATASTART+1]-0x30;
  DMSG("Totallength");
  DMSG_INT(Totallength);
  DMSG("\n");

  if(0x1F != data[NFCDATASTART+2] || 0x01!= data[NFCDATASTART+3]){
  	DMSG("DeCodeNFCReceiceData  error2\n");
	return -1;
  }

  index1Length = (data[NFCDATASTART+4]-0x30)*10+data[NFCDATASTART+5]-0x30;
	DMSG("index1Length");
	DMSG_INT(index1Length);
	DMSG("\n");

  if(0x5F != data[NFCDATASTART+6+index1Length] || 0x01!= data[NFCDATASTART+7+index1Length]){
  	DMSG("DeCodeNFCReceiceData  error3\n");
	return -1;

  }

  //WiFi账号
 	index2Length = (data[NFCDATASTART+8+index1Length]-0x30)*10+data[NFCDATASTART+9+index1Length]-0x30;

	strncpy(ssid, (char*)(data+NFCDATASTART+10+index1Length), index2Length);
	DMSG_STR(ssid);


	
	DMSG("index2Length");
	DMSG_INT(index2Length);
	DMSG("\n");
  
  if(0x5F != data[NFCDATASTART+10+index1Length+index2Length] || 0x02!= data[NFCDATASTART+11+index1Length+index2Length]){
  	DMSG("DeCodeNFCReceiceData  error4\n");
	return -1;

  }

	index3Length = (data[NFCDATASTART+12+index1Length+index2Length]-0x30)*10+data[NFCDATASTART+13+index1Length+index2Length]-0x30;
	DMSG("index3Length");
	DMSG_INT(index3Length);
	DMSG("\n");

  if(0x9F != data[NFCDATASTART+14+index1Length+index2Length+index3Length] || 0x01!= data[NFCDATASTART+15+index1Length+index2Length+index3Length]){
  	DMSG("DeCodeNFCReceiceData  error5\n");
	return -1;
  	
  }
  //WiFi密码
  	index4Length = (data[NFCDATASTART+16+index1Length+index2Length+index3Length]-0x30)*10+data[NFCDATASTART+17+index1Length+index2Length+index3Length]-0x30;
	DMSG("index4Length");
	DMSG_INT(index4Length);
	DMSG("\n");
	strncpy(password, (char*)(data+NFCDATASTART+18+index1Length+index2Length+index3Length), index4Length);
	DMSG_STR(password);

	num = 2;
//	String temp="ASUS_A4_2G";
//	temp.toCharArray(ssid, temp.length()+1);
//	temp="12345678";
//	temp.toCharArray(password, temp.length()+1);
	
	
}


void NFC_Card_Function(){
   bool success;
  
  uint8_t responseLength = 100;
  
  Serial.println("Waiting for an ISO14443A card");
  
  // set shield to inListPassiveTarget
  success = nfc.inListPassiveTarget();

  if(success) {
   
     Serial.println("Found something!");
                  
    uint8_t selectApdu[] = { 0x00, /* CLA */
                              0xA4, /* INS */
                              0x04, /* P1  */
                              0x00, /* P2  */
                              0x05, /* Length of AID  */
                              0xF2, 0x23, 0x34, 0x45, 0x56, /* AID defined on Android App */
                              0x00  /* Le  */ };
                              
    uint8_t response[100];  
     
    success = nfc.inDataExchange(selectApdu, sizeof(selectApdu), response, &responseLength);

    if(success) {
      
      Serial.print("responseLength: "); Serial.println(responseLength);
       
      nfc.PrintHexChar(response, responseLength);
	  DeCodeNFCReceiceData(response);
      
//      do {
//        uint8_t apdu[] = "Hello from Arduino";
//        uint8_t back[32];
//        uint8_t length = 32; 
//
//        success = nfc.inDataExchange(apdu, sizeof(apdu), back, &length);
//        
//        if(success) {
//         
//          Serial.print("responseLength: "); Serial.println(length);
//           
//          nfc.PrintHexChar(back, length);
//        }
//        else {
//          
//          Serial.println("Broken connection?"); 
//        }
//      }
//      while(success);
    }
    else {
     
      Serial.println("Failed sending SELECT AID"); 
    }
  }
  else {
   
    Serial.println("Didn't find anything!");
  }
  }

void loop()
{
  WiFi_Function();
  NFC_Card_Function();
  delay(100);
}

void printResponse(uint8_t *response, uint8_t responseLength) {
  
   String respBuffer;

    for (int i = 0; i < responseLength; i++) {
      
      if (response[i] < 0x10) 
        respBuffer = respBuffer + "0"; //Adds leading zeros if hex value is smaller than 0x10
      
      respBuffer = respBuffer + String(response[i], HEX) + " ";                        
    }

    Serial.print("response: "); Serial.println(respBuffer);
}

void setupNFC() {
 
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
  
  // configure board to read RFID tags
  nfc.SAMConfig(); 
}
