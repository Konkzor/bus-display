#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define API_RATP "api-ratp.pierre-grimaud.fr"
#define INIT_WIFI 0
#define INIT_BUS 1
#define RUN 2

struct BusData {
  String station_name;
  String destination_name;
  String firstBus_time;
  String secondBus_time;
};

size_t MAX_CONTENT_SIZE = 270;

const char addr_ligne_h = 0;
const char addr_ligne_l = 1;
const char addr_station_h = 2;
const char addr_station_l = 3;
const char addr_destination_h = 4;
const char addr_destination_l = 5;

const String NomduReseauWifi = "Bbox-SandyEtMat";
const String MotDePasse = "576CC166AEC4AF5CA513334FEF7DD2";

int Ligne = 0;
int Station = 0;
int Destination = 0;

char state = RUN;
char wifiState = 0;

SoftwareSerial ESP8266(10, 11);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // Read information from EEPROM
  readBusFromEEPROM(&Ligne, &Station, &Destination);
  // Wifi setup
  ESP8266.begin(115200);
  envoieAuESP8266("AT+CIOBAUD=9600");
  recoitDuESP8266(4000);
  ESP8266.begin(9600);
  // Connection
  wifiState = connect2Wifi();
}

void loop() {
  switch(state){
    case(INIT_BUS) :
      Serial.print("READY");
      Ligne = readSerial(2000).toInt();
      Serial.print("ACK");
      Station = readSerial(2000).toInt();
      Serial.print("ACK");
      Destination = readSerial(2000).toInt();
      Serial.print("ACK");
      writeBusToEEPROM(Ligne, Station, Destination);
      state = RUN;
      break;

    case(RUN) :
    // Get bus data (first and second bus, destination)
      BusData busdata;
      updateBusSchedule(&busdata);
      Serial.println(busdata.destination_name);
      Serial.println(busdata.firstBus_time);
      Serial.println(busdata.secondBus_time);
      
    // Display on LCD


      delay(10000);
      state = waitForInitRequest(1000);
      break;
  }
}

short waitForInitRequest(const int timeout){
  Serial.print("INIT ?");
  String request = readSerial(timeout);
  if(request.indexOf("INIT BUS") != -1){
    return INIT_BUS;
  }
  if(request.indexOf("INIT WIFI") != -1){
    return INIT_WIFI;
  }
  else return RUN;
}

String readSerial(const int timeout){
  String reponse = "";
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (Serial.available())
    {
      char c = Serial.read();
      reponse += c;
    }
  }
  return reponse;
}

short updateBusSchedule(BusData* busdata){
  char schedules[MAX_CONTENT_SIZE];
  sprintf(schedules, "{\"schedules\": ");
  
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += API_RATP;
  cmd += "\",80";
  sendDebug(cmd);
  delay(2000);
  if(ESP8266.find("Error")){
    Serial.print("RECEIVED: Error");
    return 0;
  }

  cmd = "GET http://api-ratp.pierre-grimaud.fr/v2/bus/"+String(Ligne)+"/stations/"+String(Station)+"?destination="+String(Destination);
  cmd += "\r\n";
  ESP8266.print("AT+CIPSEND=");
  ESP8266.println(cmd.length());
  if(ESP8266.find(">")){
    Serial.print(">");
    Serial.print(cmd);
    ESP8266.print(cmd);
  }else{
    sendDebug("AT+CIPCLOSE");
    return 0;
  }

  if (ESP8266.find("\"schedules\": "))
  {
    int i;
    for (i = 13; i < MAX_CONTENT_SIZE; i++)
    {
      if (ESP8266.available())  //new characters received?
      {
        char c = ESP8266.read();
        schedules[i] = c;
        if(c == ']') break;
      }
      else i--;  //if not, keep going round loop until we've got all the characters
    }
    schedules[i+1] = '}';
    schedules[i+2] = 0;
  }
  
  //Serial.println(schedules);

  // Parse content
  StaticJsonBuffer<270> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(schedules);

  // Test if parsing succeeded
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return 0;
  }
  Serial.println("parseObject() succeeded");
  
  const char* destination_name = root["schedules"][0]["destination"];
  const char* firstBus_time = root["schedules"][0]["message"];
  const char* secondBus_time = root["schedules"][1]["message"];

  busdata->destination_name = destination_name;
  busdata->firstBus_time = firstBus_time;
  busdata->secondBus_time = secondBus_time;
  
  // Close session
  sendDebug("AT+CIPCLOSE");
  return 1;
}


void writeBusToEEPROM(int ligne, int station, int destination){
  EEPROM.write(addr_ligne_h, (ligne >> 8) & 0xFF);
  EEPROM.write(addr_ligne_l, (ligne >> 0) & 0xFF);
  EEPROM.write(addr_station_h, (station >> 8) & 0xFF);
  EEPROM.write(addr_station_l, (station >> 0) & 0xFF);
  EEPROM.write(addr_destination_h, (destination >> 8) & 0xFF);
  EEPROM.write(addr_destination_l, (destination >> 0) & 0xFF);
}

void readBusFromEEPROM(int* ligne, int* station, int* destination){
  *ligne = ((EEPROM.read(addr_ligne_h) << 8) & 0xFF00) + ((EEPROM.read(addr_ligne_l) << 0) & 0xFF);
  *station = ((EEPROM.read(addr_station_h) << 8) & 0xFF00) + ((EEPROM.read(addr_station_l) << 0) & 0xFF);
  *destination = ((EEPROM.read(addr_destination_h) << 8) & 0xFF00) + ((EEPROM.read(addr_destination_l) << 0) & 0xFF);
}

/******************************************************************************
 *                              WIFI FUNCTIONS                                *
 ******************************************************************************/
short connect2Wifi(void){
  envoieAuESP8266("AT");//+RST"); // WORKING ?
  Serial.print(recoitDuESP8266(2000));
  envoieAuESP8266("AT+CWMODE=1"); // WIFI MODE STATION
  Serial.print(recoitDuESP8266(7000));
  envoieAuESP8266("AT+CWJAP=\"" + NomduReseauWifi + "\",\"" + MotDePasse + "\""); // JOIN ACCESS POINT
  String res = recoitDuESP8266(5000);
  Serial.print(res);
  if (res.indexOf("WIFI GOT IP") != -1) return 1;
  else return 0;
}

short checkWiFi(void){
  envoieAuESP8266("AT+CIFSR");
  delay(2000);
  if(ESP8266.find("OK")){
    return 1;
  }
  else{
    return 0;
  }
}

void envoieAuESP8266(String commande){
  ESP8266.println(commande);
}

String recoitDuESP8266(const int timeout){
  String reponse = "res : ";
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (ESP8266.available())
    {
      char c = ESP8266.read();
      reponse += c;
    }
  }
  return reponse;
}

void sendDebug(String cmd){
  Serial.print("SEND: ");
  Serial.println(cmd);
  ESP8266.println(cmd);
}
