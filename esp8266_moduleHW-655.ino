#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "nome_wifi";
const char* password = "password";
const char* mqtt_server = "192.168.1.1"; // sostituire con l'indirizzo del broker MQTT
const int LED_PIN = 3; // pin del LED
const unsigned long MESSAGE_INTERVAL = 30000; // 60000 è un min in mills

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMessageTime = 0;
bool relayOn = true;

void setup() {
  Serial.begin(9600);
  delay(10);
  
  pinMode(LED_PIN, OUTPUT); // imposta il pin del LED come uscita
  
  // si connette alla rete WiFi
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // si connette al broker MQTT e si iscrive al topic 'test'
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.print("Connecting to MQTT broker...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe("test");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  // mantiene la connessione al broker MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

/*===================================================================
                          ACTIONS
  ===================================================================*/


void turnOff() {
  Serial.write("\xa0\x01"); // byte sequence for opening relay
  Serial.write(0x00);   // apparently because of the 0x00
  Serial.write(0xa1);   // you need to send on multiple lines
  digitalWrite(LED_PIN, LOW); // accende il LED
  Serial.print("ho attivato il relay");
  relayOn = true;
}

void turnOn() {
  Serial.write("\xa0\x01\x01\xa2"); // byte sequence for closing relay
  digitalWrite(LED_PIN, HIGH); // spegne il LED
  Serial.print("l'ho disattivato");
  relayOn = false;
}


void callback(char* topic, byte* payload, unsigned int length) {
  // gestisce i messaggi ricevuti sul topic 'test'
  Serial.print("Message received: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  turnOn();
  //delay(30000); // aspetta 500 millisecondi
  turnOff();
   delay(MESSAGE_INTERVAL);
}

void reconnect() {
  // tenta di riconnettersi al broker MQTT
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe("test");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}
