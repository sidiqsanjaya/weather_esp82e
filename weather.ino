#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>
#include <BH1750.h>
#include <Wire.h>



#define DHTPIN D3                     // Pin yang terhubung ke sensor DHT22
#define DHTTYPE DHT22                 // DHT 22  (AM2302)
const int rainA = A0;              // sensor hujan analog
const int rainD = D0;              // sensor hujan digital
const char* ssid = "iot";             //nama wifi
const char* password = "iottest123";  //password wifi

Adafruit_SSD1306 display(128, 32, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE);             //init dht
Adafruit_BMP085 bmp;                  //init bmp
BH1750 lux;                           //init lux

struct SensorData {
  float suhu;
  float kelembapan;
  float tekanan;
  float suhubmp;
  float cahaya;
  float intensitas;
  float air;
};

void setup() {
  pinMode (rainA, INPUT);
  pinMode (rainD, INPUT);

  Serial.begin(9600);
  WiFi.begin(ssid, password);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Wire.begin(D2, D1);
  dht.begin();

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan ke WiFi...");
  }
  if (!lux.begin()) {
    Serial.println("Sensor LUX tidak ditemukan!");
    while (1);
  }
  if (!bmp.begin()) {
    Serial.println("Sensor BMP tidak ditemukan!");
    while (1);
  }
  Serial.println("Kelembapan(%)   Suhu dht22('c)    Tekanan(hpa)    suhu BMP('c)   cahaya(LUX)   intensitas   hujan?");
}

SensorData SensorDHT() {
  SensorData data;
  data.suhu = dht.readTemperature();
  data.kelembapan = dht.readHumidity();

  Serial.print(String(data.kelembapan) +"           "+ String(data.suhu));

  return data;
}

SensorData SensorBMP() {
  SensorData data;
  data.suhubmp = bmp.readTemperature();
  data.tekanan = bmp.readPressure() / 100.0;
        
  Serial.print("             " + String(data.tekanan) +"          " + String(data.suhubmp));
  return data;
}

SensorData SensorLUX() {
  SensorData data;
  data.cahaya = lux.readLightLevel();
        
  Serial.print("          " + String(data.cahaya));
  return data;
}

SensorData SensorRAIN() {
  SensorData data;
  data.intensitas = 1024 - analogRead(rainA);
  data.air = 1 - digitalRead(rainD);
  Serial.print("         " + String(data.intensitas));
  Serial.print("         " + String(data.air));
  Serial.println("");
  return data;
}

void displaySensorData(SensorData dhtData, SensorData bmpData, SensorData luxData, SensorData rainData, int displaysensors) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.setTextSize(1.2);

  switch (displaysensors) {
    case 0:
      display.println("DHT22 Data:");
      display.print("Suhu: ");
      display.print(dhtData.suhu);
      display.println(" 'C");
      display.print("Kelembapan: ");
      display.print(dhtData.kelembapan);
      display.println(" %");
      break;
    case 1:
      display.println("BMP180 Data:");
      display.print("Suhu: ");
      display.print(bmpData.suhubmp);
      display.println(" 'C");
      display.print("Tekanan: ");
      display.print(bmpData.tekanan);
      display.println(" hPa");
      break;
    case 2:
      display.println("LDR Data:");
      display.print("Cahaya: ");
      display.print(luxData.cahaya);
      display.println(" LUX");
      break;
    case 3:
      display.println("Rain Data:");
      display.print("Hujan: ");
      display.print(rainData.air);
      display.println("");
      display.print("Intensitas: ");
      display.print(rainData.intensitas);
      display.println("");
      break;
    default:
      break;
  }

  display.display();
}

void kirimdata(SensorData dhtData, SensorData bmpData, SensorData luxData, SensorData rainData) {
  HTTPClient http;
  WiFiClient client;

  if (http.begin(client, "http://younime.my.id/?page=insert&device_id=213")) {
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String data = "password=wadsacwas&suhudht=" + String(dhtData.suhu) + "&kelembapan=" + String(dhtData.kelembapan) + "&suhubmp=" + String(bmpData.suhubmp) + "&tekanan=" + String(bmpData.tekanan)+ "&cahaya=" + String(luxData.cahaya)+ "&air=" + String(rainData.air)+ "&intensitas=" + String(rainData.intensitas);
    int httpResponseCode = http.POST(data);
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);

    http.end();
  } else {
    Serial.println("Gagal terhubung ke server");
  }
}

void loop() {
  static unsigned long previousTime = 0;
  static unsigned long displayDelay = 2000;    // Delay antara tampilan data
  static unsigned long sendDataDelay = 10000;  // Delay antara pengiriman data
  static int displaysensors = 0;
  static unsigned long previousSendTime = 0;

  unsigned long currentTime = millis();

  if (currentTime - previousTime >= displayDelay) {
    previousTime = currentTime;

    SensorData dhtData = SensorDHT();
    SensorData bmpData = SensorBMP();
    SensorData luxData = SensorLUX();
    SensorData rainData = SensorRAIN();

    displaySensorData(dhtData, bmpData, luxData, rainData, displaysensors);
    if(displaysensors<3){
      displaysensors++;
    }else{
      displaysensors = 0;
    }
  }

  if (currentTime - previousSendTime >= sendDataDelay) {
    previousSendTime = currentTime;

    SensorData dhtData = SensorDHT();
    SensorData bmpData = SensorBMP();
    SensorData luxData = SensorLUX();
    SensorData rainData = SensorRAIN();

    kirimdata(dhtData, bmpData, luxData, rainData);
  }
}
