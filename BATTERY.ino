#include <WiFi.h>
#include <HTTPClient.h>
#include "JbdBms.h"

const char* ssid = "IN_Note1";
const char* password = "Rishabh24";

const char* server = "http://api.thingspeak.com/update";
String apiKey = "UIMJ9E3RW490E7SD";

// BMS and Serial Setup
HardwareSerial BmsSerial(2);
JbdBms myBms(&BmsSerial);

void setup() {
  Serial.begin(115200);
  BmsSerial.begin(9600, SERIAL_8N1, 16, 17);

  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}
void loop() {
  if (myBms.readBmsData() && myBms.readPackData()) {
    float voltage = myBms.getVoltage();
    float current = myBms.getCurrent();
    float soc = myBms.getChargePercentage();
    float temp2 = myBms.getTemp2();
    uint16_t cellHigh = myBms.getPackCellInfo().CellHigh;
    uint16_t cellLow = myBms.getPackCellInfo().CellLow;
    uint16_t cellDiff = myBms.getPackCellInfo().CellDiff;
    float cell1 = myBms.getPackCellInfo().CellVoltage[0] / 1000.0; // in Volts

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String url = server;
      url += "?api_key=" + apiKey;
      url += "&field1=" + String(voltage);
      url += "&field2=" + String(current);
      url += "&field3=" + String(soc);
      url += "&field4=" + String(cell1, 3); // Cell 1 instead of Temp1
      url += "&field5=" + String(temp2);
      url += "&field6=" + String(cellHigh);
      url += "&field7=" + String(cellLow);
      url += "&field8=" + String(cellDiff);

      http.begin(url);
      int httpCode = http.GET();
      if (httpCode > 0) {
        Serial.println("ThingSpeak updated, code: " + String(httpCode));
      } else {
        Serial.println("Error posting to ThingSpeak");
      }
      http.end();
    }
  } else {
    Serial.println("BMS communication error");
  }

  delay(20000); // ThingSpeak rate limit: 1 update every 15 sec minimum
}
