#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPIFFS.h>

#include "credentials.h"

//const char* ssid = "IHR_WIFI_SSID";
//const char* password = "IHR_WIFI_PASSWORT";

const long  gmtOffset_sec = 3600;         // Offset to GMT
const int   daylightOffset_sec = 3600;    //  Summertime
String time_str;


const char* sourceURL = "https://abfallkalender.regioit.de/kalender-aachen/downloadfile.jsp?format=ics&jahr=2024&ort=Aachen&strasse=10725577&hnr=10725580&zeit=-%3A00%3A00&fraktion=0&fraktion=1&fraktion=4&fraktion=7";

//
//const int anzTrash = 150;     // Anzahl der Datenobjekte im Array
//TRASH trashData[anzTrash];    // Array von TRASH-Objekten
//
//int anzEvents;                // Anzahl der gelesenen Events
String today;
String eventDate;
String location;
String trash;
//trash.reserve(100);

//  ######

void wifiStart(){
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Connecting Wifi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Wifi RSSI=");
  Serial.println(WiFi.RSSI());
}


void listAllFiles(){
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while(file){
    Serial.print("FILE: ");
    Serial.print(file.name());
    String fileName = file.name();
    File readFile = SPIFFS.open(("/" + fileName), "r");
    if (!readFile) {
      Serial.println("Fehler beim Öffnen der Datei zum Lesen");
    } else {
      Serial.print("\tDateigröße : ");
      Serial.println (readFile.available());
    }
    file = root.openNextFile();
  }
}


void downloadFile(String url) {
  HTTPClient http;

  // HTTP-GET-Anfrage senden
  Serial.print("HTTP GET an: ");
  Serial.println(url);
  http.begin(url);

  // HTTP-GET-Anfrage ausführen und Antwort erhalten
  int httpCode = http.GET();
  if (httpCode > 0) {
    // Wenn die Anfrage erfolgreich war, Dateiinhalt speichern
    String payload = http.getString();
    Serial.println("HTTP GET erfolgreich. Speichern der Datei auf dem ESP32...");
    Serial.println(payload);

    // Datei auf dem ESP32 speichern
    //SPIFFS.remove("/abfall.txt");
    File file = SPIFFS.open("/abfall.txt", "w");
    if (!file) {
      Serial.println("Fehler beim Öffnen der Datei zum Schreiben");
      return;
    }
    file.print(payload);
    file.close();
    } else {
      Serial.print("Fehler beim Herunterladen der Datei. HTTP-Code: ");
      Serial.println(httpCode);
  }
  http.end(); // Schließen der Verbindung
}

void fetchEventData(String termin){
  // Dateiinhalt auslesen
  File readFile = SPIFFS.open("/abfall.txt", "r");
  if (!readFile) {
    Serial.println("Fehler beim Öffnen der Datei zum Lesen");
  }
  Serial.println ("#1#   O P E N   ###");
  while (readFile.available()) {
    String line = readFile.readStringUntil('\n');
    if (line.startsWith("DTSTART:")) {
      String _eventStart = line.substring(8, 16);
      if (_eventStart > termin){
        eventDate = _eventStart;
        Serial.print ("Datum gefunden ");
        Serial.println(eventDate);
        break;
      }
    }
  }
  readFile.close();
  Serial.println("#1#  C L O S E  ###");
}

String fetchTrash(String termin){
  // Dateiinhalt auslesen
  String result;
  result.reserve(100);
  String resultX;
  resultX.reserve(100);
  //
  File readFile = SPIFFS.open("/abfall.txt", "r");
  if (!readFile) {
    Serial.println("Fehler beim Öffnen der Datei zum Lesen");
  }
  int i = 0;
  Serial.print ("Einlesen der Abfallarten für ");
  Serial.println(termin);
  //
  Serial.println ("#2#   O P E N   ###");
  while (readFile.available()) {
    String line = readFile.readStringUntil('\n');
    if (line.startsWith("DTSTART:")) {
      String _eventStart = line.substring(8, 16);
      if (_eventStart == termin){
        while (readFile.available()) {
          String line = readFile.readStringUntil('\n');
          if (line.startsWith("DESCRIPTION:")) {
            String _eventArt = line.substring(12) +"\0";
            Serial.print ("Trash gefunden <");
            Serial.print(_eventArt);
            Serial.println(">");
            resultX += _eventArt;
            //Serial.println(resultX);
            i++;
            //String currentString = "Schleife " + String(i) + "|";
            //result += currentString;
            //Serial.println(result);
            break;
          }
        }
      }
    }
  }
  //
  readFile.close();
  Serial.println("#2#  C L O S E  ###");
  return resultX;
}


void setup() {
  Serial.begin(115200);
  Serial.println();

  // Starten des Dateisystems
  if (!SPIFFS.begin(true)) {
    Serial.println("Fehler beim Starten des Dateisystems");
    return;
  }

  // Verbindung zum Wi-Fi-Netzwerk herstellen
  wifiStart();
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Verbindung zum Wi-Fi-Netzwerk herstellen...");
  }
  Serial.println("Verbunden mit dem Wi-Fi-Netzwerk");

  //downloadFile(sourceURL);

  fetchEventData("20240218");
  String Abfuhren = fetchTrash(eventDate);
  Serial.println("===================");
  Serial.println(Abfuhren);
  Serial.println("===================");
  //
}

void loop(){
  delay(500);
}
