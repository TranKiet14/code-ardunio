#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ESP32Servo.h>
#define DHTTYPE DHT11
#define DHTPIN 4
Servo se;

char* ssid = "Miranda";
char* pass = "hanhlong1234";
const char* mqtt_server = "broker.hivemq.com";
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
float temp = 0;
float hum = 0;
int current_ledState = LOW;
int last_ledState = LOW;
int current_relayState = HIGH;
int last_relayState = HIGH;
int current_servoState = 0;
int last_servoState = 0;
int current_ledAuto = 0;
int current_RelayAuto = 0;
int current_servoAuto = 0;
int ledPin = 2;
int soilPin = 32;
int rainPin = 25;
int relayPin = 5;
int lightPin = 14;
int servoPin = 19;
DHT dht(DHTPIN, DHTTYPE);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void connect_to_broker() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32ClientTK-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("/TranKiet");
      client.publish("/TranKiet", "hello");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("-------new message from broker-----");
  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("message: ");
  Serial.write(payload, length);
  Serial.println();
  if (*payload == '1') current_ledState = HIGH;
  if (*payload == '0') current_ledState = LOW;
  if (*payload == '2') current_relayState = HIGH;
  if (*payload == '3') current_relayState = LOW;
  if (*payload == '4') current_servoState = 180;
  if (*payload == '5') current_servoState = 0;
  if (*payload == '6') current_ledAuto = 0;
  if (*payload == '7') current_ledAuto = 1;
  if (*payload == '8') current_RelayAuto = 0;
  if (*payload == '9') current_RelayAuto = 1;
  if (*payload == 'a') current_servoAuto = 0;
  if (*payload == 'b') current_servoAuto = 1;
}
void setup() {
  Serial.begin(115200);
  setup_wifi();
  dht.begin();
  digitalWrite(relayPin, current_relayState);
  se.attach(servoPin);
  se.write(current_servoState);
  pinMode(ledPin, OUTPUT);
  pinMode(soilPin, INPUT);
  pinMode(rainPin, INPUT);
  pinMode(lightPin, INPUT);
  pinMode(relayPin, OUTPUT);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  connect_to_broker();
  client.publish("/tranKiet/led", String(current_ledState).c_str());
  delay(500);
  client.publish("/tranKiet/relay", String(current_relayState).c_str());
  delay(500);
  client.publish("/tranKiet/servo", String(current_servoState).c_str());
  delay(500);
}
void send_data() {
  // client.publish("TranKiet", "12345678");
  // delay(1000);
  hum = dht.readHumidity();
  temp = dht.readTemperature();
  if (isnan(hum) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  } else {
    String t = String(temp);
    String h = String(hum);
    client.publish("/tranKiet/temp", t.c_str());
    client.publish("/tranKiet/hum", h.c_str());
  }
  delay(1000);
  client.publish("/tranKiet/rain", String(digitalRead(rainPin)).c_str());
}
void loop() {
  client.loop();
  if (!client.connected()) {
    connect_to_broker();
  }
  if (last_ledState != current_ledState) {
    last_ledState = current_ledState;
    digitalWrite(ledPin, current_ledState);
    Serial.println(current_ledState);
  }
  delay(500);
  if (last_relayState != current_relayState) {
    last_relayState = current_relayState;
    digitalWrite(relayPin, current_relayState);
    Serial.println(current_relayState);
  }
  delay(500);
  if (last_servoState != current_servoState) {
    last_servoState = current_servoState;
    se.write(current_servoState);
    Serial.println(current_servoState);
  }
  delay(500);
  send_data();
  delay(500);
  int soilValue = analogRead(soilPin);
  if (current_RelayAuto == 1) {
    if (soilValue > 2000) {
      digitalWrite(relayPin, LOW);
    } else {
      digitalWrite(relayPin, HIGH);
    }
    delay(1000);
  }
  int rain = digitalRead(rainPin);
  if (current_servoAuto == 1) {
    if (rain == 0) {
      se.write(0);
    } else {
      se.write(180);
    }
    delay(1000);
  }
  int light = digitalRead(lightPin);
  if (current_ledAuto == 1) {
    if (light == 0) {
      digitalWrite(ledPin, LOW);
    } else {
      digitalWrite(ledPin, HIGH);
    }
    delay(1000);
  }
}
