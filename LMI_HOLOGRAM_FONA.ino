#include <LMI.h>
#include <HologramSIMCOM.h>

//#define SIMULATE
#define HOLO_KEY "" //replace w/your SIM key
#define FONA_RX  9
#define FONA_TX  8
#define FONA_RST 4
#define MESSAGE_TIMER  (unsigned long)300000 // wait 5 minutes
#define THRESHOLD_DETECTION 50 // System will send a message if measure pressure is up to +-50Pa
#define SENSOR_REFRESH  (1* 1000) // wait 5s
LMI Sensor;
unsigned long nextMessage = 0x00;
unsigned long nextRefresh = 0x00;

void SendMessage(String sToSend);

HologramSIMCOM Hologram(FONA_TX, FONA_RX, FONA_RST, HOLO_KEY);
void setup() {
  Serial.begin(115200);
  while(!Serial);
  Sensor.begin(0x5F);
  Serial.println();
  // Start modem and connect to Hologram's global network
  Hologram.debug();
  bool cellConnected = Hologram.begin(19200, 8888); // set baud to 19200 and start server on port 8888
  if(cellConnected) {
      Serial.println(F("Cellular is connected"));
  } else {
      Serial.println(F("Cellular connection failed"));
  }
  Serial.println(F("Setup complete"));
}

void loop() {
   Hologram.debug();
   String LMIInfo = "F#" + Sensor.GetFWVersion() + "P#" + Sensor.GetPartNumber();
   float SensorValue = 0.0;

   SensorValue = Sensor.GetPressure();
   nextRefresh =  millis() + SENSOR_REFRESH;

   if(Hologram.cellService())
   {
     SendMessage(LMIInfo);
     LMIInfo = "V#";
     LMIInfo += String(SensorValue);
     SendMessage(LMIInfo);
     nextMessage = (unsigned long)millis() + MESSAGE_TIMER;
   }

   while(1){

      if(Hologram.availableMessage() > 0) {
        // readMessage() returns both incoming TCP data or incoming SMS messages
        // once you read a message the buffer is wiped, if called again nothing will be there
        Serial.print("Incoming Message: ");
        Serial.println(Hologram.readMessage());
      }
      // timeout send new value
      if ( millis() > nextMessage){
           Serial.println("Send New Value");
           LMIInfo = "V#";
           LMIInfo += String(SensorValue);
           SendMessage(LMIInfo);
           nextMessage = (unsigned long)millis() + MESSAGE_TIMER;
           String txt = String(nextMessage);
           txt += "-";
           txt+=String(millis());
           Serial.println(txt);
      }
      if( millis() > nextRefresh ){

        SensorValue = Sensor.GetPressure();
        // Send Alarm
        if(SensorValue < -THRESHOLD_DETECTION || SensorValue > THRESHOLD_DETECTION){
           LMIInfo = "A#";
           LMIInfo += String(SensorValue);
           SendMessage(LMIInfo);
        }
        nextRefresh =  millis() + SENSOR_REFRESH;
      }
   }

}

void SendMessage(String sToSend){
  sToSend = "{Dev:" + Sensor.GetSerialNumber() + ",Message:" + sToSend+"}";
  #ifndef SIMULATE
   Hologram.send(sToSend);
  #else
    Serial.println(sToSend);
  #endif
}
