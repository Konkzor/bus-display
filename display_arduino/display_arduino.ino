#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h>

#define API_RATP "api-ratp.pierre-grimaud.fr"
#define INIT_WIFI 0
#define INIT_BUS 1
#define RUN 2

struct BusData {
  int Ligne;
  int Station;
  int Destination;
  char station_name[13];
  String destination_name;
  short firstBus_time;
  short secondBus_time;
};
BusData busdata;
struct TimeData {
  String date;
  String heure;
};
TimeData timedata;

size_t MAX_CONTENT_SIZE = 265;

const char addr_ligne_h = 0;
const char addr_ligne_l = 1;
const char addr_station_h = 2;
const char addr_station_l = 3;
const char addr_destination_h = 4;
const char addr_destination_l = 5;

const String NomduReseauWifi = "Bbox-SandyEtMat";
const String MotDePasse = "576CC166AEC4AF5CA513334FEF7DD2";

char state = RUN;
char wifiState = 0;

SoftwareSerial ESP8266(10, 11);
// LCD
LiquidCrystal lcd(8, 7, 6, 5, 4, 3);

void setup() {
  Serial.begin(9600);
  
  // Read information from EEPROM
  readBusFromEEPROM(&busdata);
  
  // Wifi setup
  ESP8266.begin(115200);
  envoieAuESP8266("AT+CIOBAUD=9600");
  recoitDuESP8266(4000);
  ESP8266.begin(9600);
  
  // Connexion
  wifiState = connect2Wifi();
  
  // LCD setup
  lcd.begin(20, 4);
  lcd.setCursor(5,1);
  lcd.print("DEMARRAGE");
}

void loop() {
  switch(state){
    case(INIT_BUS) :
      Serial.print("READY");
      busdata.Ligne = readSerial(2000).toInt();
      Serial.print("ACK");
      busdata.Station = readSerial(2000).toInt();
      Serial.print("ACK");
      busdata.Destination = readSerial(2000).toInt();
      Serial.print("ACK");
      writeBusToEEPROM(&busdata);
      state = RUN;
      break;

    case(RUN) :
      // Get bus data (first and second bus, destination)
      delay(1000);
      updateBusSchedule(&busdata);
      delay(1000);
      updateTime(&timedata);
      // Display on LCD
      updateDisplay(&busdata, &timedata);

      delay(5000);
      state = waitForInitRequest(2000);
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

/******************************************************************************
 *                              LCD FUNCTIONS                                 *
 ******************************************************************************/
void updateDisplay(BusData* busdata, TimeData* timedata){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(timedata->date);
  lcd.setCursor(15,0);
  lcd.print(timedata->heure);
  lcd.setCursor(4,1);
  lcd.print(encodeLine(busdata->destination_name, busdata->firstBus_time));
  lcd.setCursor(0,2);
  lcd.print(busdata->Ligne);
  lcd.setCursor(4,3);
  lcd.print(encodeLine(busdata->destination_name, busdata->secondBus_time));
}

String encodeLine(String destination, short wmin){
  String line = destination.substring(0,13);
  line.toUpperCase();
  line += ' ';

  if (wmin <= 9){
    line += ' ';
  }
  line += String(wmin);
  
  return line;
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

char updateTime(TimeData* timedata){
  char buffer_array[MAX_CONTENT_SIZE];
  sprintf(buffer_array, "{\"_meta\": ");

  String cmd = "GET http://api-ratp.pierre-grimaud.fr/v2/";
  cmd += "\r\n";
  
  if(sendRequest(cmd) == 0) return 0;
  else{
    if (ESP8266.find("\"_meta\": "))
    {
      int i;
      for (i = 9; i < MAX_CONTENT_SIZE; i++)
      {
        if (ESP8266.available())  //new characters received?
        {
          char c = ESP8266.read();
          buffer_array[i] = c;
          if(c == '}') break;
        }
        else i--;  //if not, keep going round loop until we've got all the characters
      }
      buffer_array[i+1] = '}';
      buffer_array[i+2] = 0;
    }
    //Serial.println(buffer_array);
    // Parse content
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(buffer_array);
  
    // Test if parsing succeeded
    if (!root.success()) {
      Serial.println("parseObject() failed");
      return 0;
    }
    Serial.println("parseObject() succeeded");
    
    const char* date = root["_meta"]["date"];
    timedata->date = recodeDate(String(date).substring(0,String(date).indexOf('T')));
    timedata->heure = String(date).substring(String(date).indexOf('T')+1, String(date).indexOf(':', String(date).indexOf(':')+1));
  }
  
  // Close session
  sendDebug("AT+CIPCLOSE");
  return 1;
}

char updateBusSchedule(BusData* busdata){
  char buffer_array[MAX_CONTENT_SIZE];
  sprintf(buffer_array, "{\"schedules\": ");

  String cmd = "GET http://api-ratp.pierre-grimaud.fr/v2/bus/"+String(busdata->Ligne)+"/stations/"+String(busdata->Station)+"?destination="+String(busdata->Destination);
  cmd += "\r\n";
  
  if(sendRequest(cmd) == 0) return 0;
  else{
    if (ESP8266.find("\"schedules\": "))
    {
      int i;
      for (i = 13; i < MAX_CONTENT_SIZE; i++)
      {
        if (ESP8266.available())  //new characters received?
        {
          char c = ESP8266.read();
          buffer_array[i] = c;
          if(c == ']') break;
        }
        else i--;  //if not, keep going round loop until we've got all the characters
      }
      buffer_array[i+1] = '}';
      buffer_array[i+2] = 0;
    }
    //Serial.println(buffer_array);
    // Parse content
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(buffer_array);
  
    // Test if parsing succeeded
    if (!root.success()) {
      Serial.println("parseObject() failed");
      return 0;
    }
    Serial.println("parseObject() succeeded");
    
    const char* destination_name = root["schedules"][0]["destination"];
    busdata->destination_name = destination_name;
    busdata->firstBus_time = decodeAttente(root["schedules"][0]["message"]);
    busdata->secondBus_time = decodeAttente(root["schedules"][1]["message"]);
  }
  
  // Close session
  sendDebug("AT+CIPCLOSE");
  return 1;
}

short sendRequest(String request){
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += API_RATP;
  cmd += "\",80";
  sendDebug(cmd);
  delay(2000);
  if(ESP8266.find("Error")){
    Serial.print("RECEIVED: Error");
    return 0;
  }
  
  ESP8266.print("AT+CIPSEND=");
  ESP8266.println(request.length());
  if(ESP8266.find(">")){
    Serial.print(">");
    Serial.print(request);
    ESP8266.print(request);
  }else{
    sendDebug("AT+CIPCLOSE");
    return 0;
  }
  return 1;
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

short decodeAttente(String wtime){
  short wmin = 0;
  if(wtime.indexOf("A l'arret") != -1) return 0;
  if(wtime.indexOf("mn") != -1){
    return wtime.substring(0, wtime.indexOf(" mn")).toInt();
  }
  else return 0;  
}

String recodeDate(String date){ //yyyy-mm-dd ==> dd/mm/yy
  short tiret_am = date.indexOf('-');
  short tiret_mj = date.indexOf('-', tiret_am+1);
  
  String jour = date.substring(tiret_mj+1);
  String mois = date.substring(tiret_am+1, tiret_mj);
  String an = date.substring(0, tiret_am);

  return (jour+"/"+mois+"/"+an.substring(2,4));
}

/******************************************************************************
 *                              EEPROM FUNCTIONS                              *
 ******************************************************************************/
void writeBusToEEPROM(BusData* busdata){
  EEPROM.write(addr_ligne_h, (busdata->Ligne >> 8) & 0xFF);
  EEPROM.write(addr_ligne_l, (busdata->Ligne >> 0) & 0xFF);
  EEPROM.write(addr_station_h, (busdata->Station >> 8) & 0xFF);
  EEPROM.write(addr_station_l, (busdata->Station >> 0) & 0xFF);
  EEPROM.write(addr_destination_h, (busdata->Destination >> 8) & 0xFF);
  EEPROM.write(addr_destination_l, (busdata->Destination >> 0) & 0xFF);
}

void readBusFromEEPROM(BusData* busdata){
  busdata->Ligne = ((EEPROM.read(addr_ligne_h) << 8) & 0xFF00) + ((EEPROM.read(addr_ligne_l) << 0) & 0xFF);
  busdata->Station = ((EEPROM.read(addr_station_h) << 8) & 0xFF00) + ((EEPROM.read(addr_station_l) << 0) & 0xFF);
  busdata->Destination = ((EEPROM.read(addr_destination_h) << 8) & 0xFF00) + ((EEPROM.read(addr_destination_l) << 0) & 0xFF);
}

