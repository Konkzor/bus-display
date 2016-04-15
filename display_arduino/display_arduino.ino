#include <EEPROM.h>

int addr_ligne_h = 0;
int addr_ligne_l = 1;
int addr_station_h = 2;
int addr_station_l = 3;
int addr_destination_h = 4;
int addr_destination_l = 5;

int myligne = 0;
int mystation = 0;
int mydestination = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.print("INIT");
  int count = 10;
  while(count > 0){
    if(Serial.available() > 0){
      if(Serial.find("HELLO FROM ARDUINO")) break;
    }
    count--;
    delay(200);
  }

  if(count > 0){ // Start init
    Serial.print("ACK");
    myligne = readSerial(1000).toInt();
    Serial.print("ACK");
    mystation = readSerial(1000).toInt();
    Serial.print("ACK");
    mydestination = readSerial(1000).toInt();
    writeToEEPROM(myligne, mystation, mydestination);
  }
  else // Get from EEPROM
    readFromEEPROM(&myligne, &mystation, &mydestination);  
}

void loop() {
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

