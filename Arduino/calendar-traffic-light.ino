#include <time.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

#define REDPIN 14
#define YELLOWPIN 12
#define GREENPIN 13

const char* ssid = "wifi-network-name";
const char* psk = "wiki-password";
const char* appsScriptUrl = "https://script.google.com/macros/s/AKfycbzGiL5Qbv6E2A66oGfNUJQ13izsN8XbUlRUKRiMaaaaaaawaaaaaaaaaaaaaaaaaaaa/exec";

time_t now;
struct tm *timeinfo;

void setup() {

  pinMode(REDPIN, OUTPUT);
  pinMode(YELLOWPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  
  Serial.begin(115200);
  WiFi.begin(ssid, psk);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(REDPIN, digitalRead(REDPIN) == LOW ? HIGH : LOW);
    delay(500);
  }
  Serial.println("Connected:");
  Serial.println(WiFi.localIP());
  digitalWrite(REDPIN, HIGH);

  Serial.println("Fetching NTP Time");
  configTzTime("EST5EDT,M3.2.0,M11.1.0", "time.cloudflare.com", "pool.ntp.org", "time.google.com");
  while (now < 946684800) { // Wait until a valid time is received (after 1-Jan-2000)
    Serial.print(".");
    time(&now);
    digitalWrite(YELLOWPIN, digitalRead(YELLOWPIN) == LOW ? HIGH : LOW);
    delay(500);
  }
  time(&now);
  timeinfo = localtime(&now);
  Serial.print("Fetched! Current time: ");
  char time_str[9];
  sprintf(time_str, "%02d:%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  Serial.println(time_str);
  digitalWrite(YELLOWPIN, HIGH);
  
  digitalWrite(GREENPIN, HIGH);
  
}

void loop() {

  time(&now);
  timeinfo = localtime(&now);

  int currentWeekday = timeinfo->tm_wday;
  int currentHour = timeinfo->tm_hour;
  if (currentHour < 8 || currentHour >= 17 || currentWeekday == 0 || currentWeekday == 6) {
    Serial.println("Outside working hours. Disconnecting WiFi...");
    WiFi.disconnect();
    digitalWrite(REDPIN, LOW);
    digitalWrite(YELLOWPIN, LOW);
    digitalWrite(GREENPIN, LOW);
    delay(30000);
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("Problem connecting to WiFi. Restarting...");
    ESP.restart();
  }
    
  WiFiClientSecure client;
  client.setInsecure();
  
  HTTPClient https;
  https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);

  bool initializationSuccess = https.begin(client, appsScriptUrl);
  if (!initializationSuccess) {
    Serial.println("Problem initializing HTTP library. Restarting...");
    ESP.restart();
  }
    
  int responseCode = https.GET();
  if (responseCode == 0) {
    Serial.println("HTTP " + String(responseCode));
    ESP.restart();
  }
  
  String status = https.getString();
  Serial.println("HTTP " + String(responseCode) + ": " + status);
  
  if (status == "r") {
    digitalWrite(REDPIN, HIGH);
    digitalWrite(YELLOWPIN, LOW);
    digitalWrite(GREENPIN, LOW);
  } else if (status == "y") {
    digitalWrite(REDPIN, LOW);
    digitalWrite(YELLOWPIN, HIGH);
    digitalWrite(GREENPIN, LOW);
  } else if (status == "g") {
    digitalWrite(REDPIN, LOW);
    digitalWrite(YELLOWPIN, LOW);
    digitalWrite(GREENPIN, HIGH);
  } else {
    digitalWrite(REDPIN, LOW);
    digitalWrite(YELLOWPIN, LOW);
    digitalWrite(GREENPIN, LOW);
  }
  
  https.end();
  
  delay(30000);
    
}
