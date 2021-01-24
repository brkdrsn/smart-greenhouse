#include <ESP8266WiFi.h>
#define BLYNK_PRINT Serial
#include <Servo.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#define topraknem_sensoru A0
DHT dht(2, DHT11);
Adafruit_BMP085 bmp;
BlynkTimer timer;
char auth[] = "cEG0nwhf5eEh-OGMeyLxwfva7jrL6zAB";
const char* ssid = "brkdrsn";
const char* password = "12345678";
int ledPin = 13; // GPIO13 ucu D7 ye karsılık geliyor.
int supompasi = 15;
int fan = 0;
WiFiServer server(80);
Servo motor;
int kapi = 0;
int fanDurum;
int suDurum;
int topraknemPin = A0;
void sendSensor()
{
  float nem = dht.readHumidity();
  float sicaklik = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  int topraknemDeger = analogRead(topraknemPin);
  Blynk.virtualWrite(V5, nem);
  Blynk.virtualWrite(V6, sicaklik);
  Blynk.virtualWrite(V7, topraknemDeger);
}
void setup() {
  dht.begin();
  bmp.begin();
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, sendSensor);
  Serial.begin(115200);
  delay(10);
  motor.attach(14);
  pinMode(ledPin, OUTPUT);
  pinMode(topraknemPin, INPUT);
  pinMode(supompasi, OUTPUT);
  pinMode(fan, OUTPUT);
  digitalWrite(ledPin, LOW);
  digitalWrite(fan, LOW);
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

  server.begin();
  Serial.println("Server started");

  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void loop() {
  Blynk.run();
  timer.run();
  float sicaklik = dht.readTemperature();
  float nem = dht.readHumidity();
  //float bmpsicaklik = bmp.readTemperature();
  int basinc = bmp.readPressure();
  int topraknemDeger = analogRead(topraknemPin);
  Serial.print("Sicaklik : ");
  Serial.println(sicaklik);
  Serial.print("Nem : ");
  Serial.println(nem);
  Serial.print("Basinc : ");
  Serial.println(basinc);
  Serial.print("Toprak Nem Degeri : ");
  Serial.println(topraknemDeger);
  Serial.println("");
  delay(3000);
  if (topraknemDeger > 400) {
    digitalWrite(15, LOW);
    delay(5000);
    suDurum = 1;
  }
  else if (topraknemDeger <= 400) {
    digitalWrite(15, HIGH);
    delay(2000);
    suDurum = 0;
  }
  if (sicaklik >= 23 && nem >= 70) {
    digitalWrite(fan, HIGH);
    delay(10000);
    fanDurum = 1;
  }
  else {
    digitalWrite(fan, LOW);
    delay(5000);
    fanDurum = 0;
  }
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // client ın bir data gondermesini bekliyoruz
  Serial.println("new client");
  while (!client.available()) {
    delay(1);
  }

  // gelen istekleri okuyoruz
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  int value = LOW;
  if (request.indexOf("/LED-ACIK") != -1) {
    digitalWrite(ledPin, HIGH);
    value = HIGH;
  }
  if (request.indexOf("/LED-KAPALI") != -1) {
    digitalWrite(ledPin, LOW);
    value = LOW;
  }
  int pos;
  if (request.indexOf("/KAPI-ACIK") != -1)  {
    motor.write(90);
    kapi = 1;
  }
  if (request.indexOf("/KAPI-KAPALI") != -1)  {

    motor.write(0);
    delay(15);
    kapi = 0;
  }

  if (request.indexOf("/FAN-ACIK") != -1)  {
    digitalWrite(fan, HIGH);
    delay(2000);
    fanDurum = 1;
  }
  if (request.indexOf("/FAN-KAPALI") != -1)  {
    digitalWrite(fan, LOW);
    delay(2000);
    fanDurum = 0;
  }

  if (request.indexOf("/SU-ACIK") != -1)  {
    digitalWrite(supompasi, LOW);
    suDurum = 1;
  }
  if (request.indexOf("/SU-KAPALI") != -1)  {
    digitalWrite(supompasi, HIGH);
    suDurum = 0;
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");

  client.print("LED SU AN: ");

  if (value == HIGH) {
    client.print("ACIK   ");
  } else {
    client.print("KAPALI   ");
  }
  client.println("<br></br>");
  client.println("<a href=\"/LED-ACIK\"\"><button>LEDI YAK </button></a>");
  client.println("<a href=\"/LED-KAPALI\"\"><button>LEDI SONDUR </button></a>");
  client.println("<br></br>");

  client.print("KAPI SU AN: ");

  if (kapi == 1) {
    client.print("ACIK   ");
  } else {
    client.print("KAPALI   ");
  }
  client.println("<br></br>");
  client.println("<a href=\"/KAPI-ACIK\"\"><button>KAPIYI AC </button></a>");
  client.println("<a href=\"/KAPI-KAPALI\"\"><button>KAPIYI KAPAT </button></a>");
  client.println("<br></br>");

  client.print("FAN SU AN: ");

  if (fanDurum == 1) {
    client.print("ACIK   ");
  } else {
    client.print("KAPALI   ");
  }
  client.println("<br></br>");
  client.println("<a href=\"/FAN-ACIK\"\"><button>FANI AC </button></a>");
  client.println("<a href=\"/FAN-KAPALI\"\"><button>FANI KAPAT </button></a>");
  client.println("<br></br>");

  client.print("SU SU AN: ");

  if (suDurum == 1) {
    client.print("ACIK   ");
  } else {
    client.print("KAPALI   ");
  }
  client.println("<br></br>");
  client.println("<a href=\"/SU-ACIK\"\"><button>SUYU AC </button></a>");
  client.println("<a href=\"/SU-KAPALI\"\"><button>SUYU KAPAT </button></a>");
  client.println("<br></br>");

  client.println("Sicaklik: ");
  client.println(sicaklik);
  client.println(" (*C)");
  client.println("<br></br>");
  client.println("Nem: ");
  client.println(nem);
  client.println(" (%)");
  client.println("<br></br>");
  client.println("Basinc: ");
  client.println(basinc);
  client.println(" (Pascal)");
  client.println("<br></br>");
  client.println("Toprak Nem: ");
  client.println(topraknemDeger);
  client.println("<br></br>");

  client.println("</html>");
  delay(1000);
  Serial.println("Client disconnected");
  Serial.println("");

}
