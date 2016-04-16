#include <EEPROM.h>

#define INIT_WIFI 0
#define INIT_BUS 1
#define RUN 2

int addr_ligne_h = 0;
int addr_ligne_l = 1;
int addr_station_h = 2;
int addr_station_l = 3;
int addr_destination_h = 4;
int addr_destination_l = 5;

int Ligne = 0;
int Station = 0;
int Destination = 0;
String NomduReseauWifi = "Bbox-SandyEtMat";
String MotDePasse = "576CC166AEC4AF5CA513334FEF7DD2";

short state = RUN;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // Read information from EEPROM
  readFromEEPROM(&Ligne, &Station, &Destination);  
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
      writeToEEPROM(Ligne, Station, Destination);
      state = RUN;
      break;

    case(RUN) :
  // Stuffs to do (Requests, Display)
      Serial.println(Ligne);
      Serial.println(Station);
      Serial.println(Destination);
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

void writeToEEPROM(int ligne, int station, int destination){
  EEPROM.write(addr_ligne_h, (ligne >> 8) & 0xFF);
  EEPROM.write(addr_ligne_l, (ligne >> 0) & 0xFF);
  EEPROM.write(addr_station_h, (station >> 8) & 0xFF);
  EEPROM.write(addr_station_l, (station >> 0) & 0xFF);
  EEPROM.write(addr_destination_h, (destination >> 8) & 0xFF);
  EEPROM.write(addr_destination_l, (destination >> 0) & 0xFF);
}

void readFromEEPROM(int* ligne, int* station, int* destination){
  *ligne = ((EEPROM.read(addr_ligne_h) << 8) & 0xFF00) + ((EEPROM.read(addr_ligne_l) << 0) & 0xFF);
  *station = ((EEPROM.read(addr_station_h) << 8) & 0xFF00) + ((EEPROM.read(addr_station_l) << 0) & 0xFF);
  *destination = ((EEPROM.read(addr_destination_h) << 8) & 0xFF00) + ((EEPROM.read(addr_destination_l) << 0) & 0xFF);
}

