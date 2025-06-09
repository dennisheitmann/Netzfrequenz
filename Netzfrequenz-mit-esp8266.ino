// Netzfrequenzmessung mit Übertragung via Serial SLIP an einen ESP8266 mit esp-link
// Arduino mit Netztransformator und +2,5 V Offset 
// https://github.com/jeelabs/el-client
// https://github.com/jeelabs/esp-link
// Dennis Heitmann
// ---
#include <ELClient.h>
#include <ELClientCmd.h>
#include <ELClientMqtt.h>

ELClient esp(&Serial, &Serial);
// Initialize CMD client (for GetTime)
ELClientCmd cmd(&esp);
// Initialize the MQTT client
ELClientMqtt mqtt(&esp);

// Callback made from esp-link to notify of wifi status changes
// Here we just print something out for grins
void wifiCb(void* response) {
  ELClientResponse *res = (ELClientResponse*)response;
  if (res->argc() == 1) {
    uint8_t status;
    res->popArg(&status, 1);

    if (status == STATION_GOT_IP) {
      Serial.println("WIFI CONNECTED");
    } else {
      Serial.print("WIFI NOT READY: ");
      Serial.println(status);
    }
  }
}

bool connected;

// Callback when MQTT is connected
void mqttConnected(void* response) {
  Serial.println("MQTT connected!");
  connected = true;
}

// Callback when MQTT is disconnected
void mqttDisconnected(void* response) {
  Serial.println("MQTT disconnected");
  connected = false;
}

// Callback when an MQTT message arrives for one of our subscriptions
void mqttData(void* response) {
  ELClientResponse *res = (ELClientResponse *)response;
}

void mqttPublished(void* response) {
  Serial.println("MQTT published");
}

#define PIN 2
#define MESSZEITRAUM 250 // alle 5 sec.

volatile int impulse = -1;
volatile unsigned long zeit = 0;
volatile unsigned long zeitVorher = 0;

void setup() {

  Serial.begin(115200);

  // Sync-up with esp-link, this is required at the start of any sketch and initializes the
  // callbacks to the wifi status change callback. The callback gets called with the initial
  // status right after Sync() below completes.
  esp.wifiCb.attach(wifiCb); // wifi status change callback, optional (delete if not desired)
  bool ok;
  do {
    ok = esp.Sync();      // sync up with esp-link, blocks for up to 2 seconds
    if (!ok) {
      Serial.println("EL-Client sync failed! Retrying...");
      delay(1000); // Give some time before retrying
    }
  } while (!ok);
  Serial.println("EL-Client synced!");

  // Set-up callbacks for events and initialize with es-link.
  mqtt.connectedCb.attach(mqttConnected);
  mqtt.disconnectedCb.attach(mqttDisconnected);
  mqtt.publishedCb.attach(mqttPublished);
  mqtt.dataCb.attach(mqttData);
  mqtt.setup();

  attachInterrupt(digitalPinToInterrupt(PIN), messung, FALLING);
}

void loop()
{
  esp.Process();
  if (impulse == MESSZEITRAUM)
  {
    detachInterrupt(digitalPinToInterrupt(PIN));
    double frequenz = 1e6 / ((zeit - zeitVorher) / (double)MESSZEITRAUM);
    char float_str[10];
    dtostrf(frequenz, 6, 3, float_str);
    Serial.println(float_str);
    if (connected) {
      mqtt.publish("/esp-link-dh/frequency", float_str);
    }
    impulse = -1;
    zeit = 0;
    zeitVorher = 0;
    attachInterrupt(digitalPinToInterrupt(PIN), messung, FALLING);
  }
  if (impulse > MESSZEITRAUM)
  {
    detachInterrupt(digitalPinToInterrupt(PIN));
    impulse = -1;
    zeit = 0;
    zeitVorher = 0;
    attachInterrupt(digitalPinToInterrupt(PIN), messung, FALLING);
  }
}

void messung()
{
  impulse++;
  if (impulse == 0)
  {
    zeitVorher = micros();
  }
  else if (impulse == MESSZEITRAUM)
  {
    zeit = micros();
  }
}
