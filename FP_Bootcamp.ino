//PLAN
  // Bikin alat untuk deteksi suhu + humidity, kelembapan tanah, jarak, pompa DC, 
      // Intinya ada sensor sensor tertentu disetel ke I/O di ESP32
      // Konsep kerja alatnya:
          // Ketika suhu, kelembapan tanah, kelembapan udara, tinggi air sesuai, alat bakal nyemprotin Air ke tanaman
          // Tinggi air diukur dari sensor SR04
          // Kelembapan diukur dari sensor soil moisture (tancepin ke pot)
          // Udara tetap diukur pake DHT
          // Air bakal disedot dari container air pake pompa DC, terus nyiram ke tanaman jika Humidity, suhu, dan lainnya                   terpenuhi kondisinya
          // Kubikasi airnya bakal kita itung pake rumus pertanian pastinya per sedot nya
          // Sumber tenaga pakai baterai 
          // Semuanya dihubungin ke OLED buat display jadi tau data nya si user
      // Urutan kerja:
          // Alat ready
            // Ketika hasil sensor soil moisture mencapai angka < 40%, siram sampai 50% soil moisture
            // Ketika hasil sensor air humidity mencapai angka < 40%, siram sampai 50% soil moisture
            // Ketika hasil sensor suhu 24 derajat celcius dan lux 10.000-20.000 , siram sampai 60% soil moisture, stop jika >                60% soil moisture
            // Pas nyiram, buzzer idupin ( optional )
            

      
  // Bikin software nya satu satu dulu
  // Sambil bikin software, coba diatur desain HW nya di kicad
  // Pikirin konek ke HTTP, terus sambungin ke mobile Apps
  // "to be continued"

//DESIGN PROJECT
    // HARDWARE ( PCB Perfboard aja biar mudah dan ga ruwet)
    // PIN Data OLED (D22 (SCL), D21 (SDA))
    // PIN Data DHT  (D15)
    // PIN Data SR04 (D23, D19)
    // PIN Data LDR ( AO di D33 )
    // PIN Data Soil Moisture ( )
    // PIN Data Pompa DC ( ) 


    // SOFTWARE
    // Fungsi DHT
    // Fungsi SR04
    // Fungsi LDR
    // Fungsi Soil Moisture
    // Fungsi Pompa DC
    // Fungsi OLED
    




// HEADER

  //Header Arduino
  #include <Arduino.h>
  
  //Header Lib OLED
  #include <Wire.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>

  //Header Lib DHT (Adafruit)
  #include <DHT.h>
  #include <DHT_U.h>

  //Header Wifi
  #include <WiFi.h>
  #include <WiFiClientSecure.h>
  #include <HTTPClient.h>

  //Header MQTT
  #include <EdspertPubSub.h>
  
  

//GLOBAL VAR

  //DHT
  #define PINDHT 15
  #define TipeDHT 22
  DHT Dht(PINDHT, TipeDHT);

  //LDR
  #define LDRPIN 33

  //SR04
  #define TRIG_PIN 19
  #define ECHO_PIN 23

  //OLED
  #define SCREEN_WIDTH 128 //OLED display width, in pixels
  #define SCREEN_HEIGHT 64 //OLED display height, in pixels
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

  //Soil Moisture Sensor
  #define SOIL_PIN 32 //pin soil moisture Analog

  //RELAY Untuk DC PUMP
  #define PIN_RELAY 14

  //Parameter WiFi
  String ssid = "rumahjonan";
  String password = "halobocah12345";

  //Parameter URL
  String accessTokenDHT = "masukinyangdarithingsboard";
  String urlDHT = "https://demo.thingsboard.io/api/v1/" + accessTokenDHT + "/telemetry";

  String accessTokenSoil = "masukinyangdarithingsboard";
  String urlSoil = "https://demo.thingsboard.io/api/v1/" + accessTokenSoil + "/telemetry";

  String accessTokenLDR = "masukinyangdarithingsboard";
  String urlLDR = "https://demo.thingsboard.io/api/v1/" + accessTokenLDR + "/telemetry";

  String accessTokenSR04 = "masukinyangdarithingsboard";
  String urlSR04 = "https://demo.thingsboard.io/api/v1/" + accessTokenSR04 + "/telemetry";

  //Parameter MQTT
  String MQTTServer = "";
  int MQTTPort = ;
  String myClientID = "";
  String topic_1 = "final_project/data/automatic_watering_device";
  extern String callBackPayLoad;
  extern String callBackTopic;

  EdspertPubSub clientMQTT;

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT); // Init Trig SR04
  pinMode(ECHO_PIN, INPUT); // Init Echi SR04
  pinMode(PIN_RELAY, OUTPUT); //init pin relay
  Dht.begin(); //Init DHT
  WiFi.mode(WIFI_STA); // Mode Wifi 
  WiFi.begin(ssid,password); // Init Wifi

  clientMQTT.connect_to_AP(ssid,password); //connect client to WiFi 
  clientMQTT.init_to_broker(MQTTServer, MQTTPort);
  clientMQTT.connect_to_broker(myClientID);
  
  display.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) //kalau ga begin, print
  {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);  // Jika OLED ga jalan, stop
  }

   display.clearDisplay();
   display.display();
}



void loop() 
{
  OLEDdanMekanisme();
  DataToThingsboard();
  clientMQTT.mqtt_publish(topic_1, HasilFungsi());
}



// LIST FUNGSI ALAT

float SR04()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

 
  long duration = pulseIn(ECHO_PIN, HIGH); // Baca durasi pulsa dari ECHO_PIN
  float distance = duration * 0.034 / 2; // Hitung jarak dalam cm
  delay(500);
  return distance;
  
}

float LDR()
{
  int ADCVal = analogRead(LDRPIN); // Baca nilai ADC dari pin LDR
  const float GAMMA = 0.7; // Konstanta Gamma untuk perhitungan lux
  const float RL10 = 33; // Resistansi LDR pada 10 lux (dalam kilo-ohm)

  // Perhitungan voltage
  float voltage = ADCVal / 4096.0 * 3.3;

  // Perhitungan resistansi LDR
  float resistance = 2000 * voltage / (1 - voltage / 3.3);

  // Perhitungan nilai lux
  float luxValue = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));
  
  delay(500);
  return luxValue;

  
}


int SoilMoisture ()
{
  int Soil = analogRead(SOIL_PIN);
  delay(100);
  return Soil;
}


String HasilFungsi()
{
    float luxValue = LDR();  // Alias LDR
    float distance = SR04(); // Alias SR04
    float soil = SoilMoisture(); // Alias Soil Moisture Sensor
    float degree = Dht.readTemperature(); // baca suhu
    float humidity = Dht.readHumidity(); // baca kelembapan

}

void OLEDdanMekanisme()
{ 

  float luxValue = LDR();  // Alias LDR
  float distance = SR04(); // Alias SR04
  float soil = SoilMoisture(); // Alias Soil Moisture Sensor
  float degree = Dht.readTemperature(); // baca suhu
  float humidity = Dht.readHumidity(); // baca kelembapan

  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  display.print("Cahaya: ");
  display.print(luxValue);
  display.println("lux");

  display.print("Jarak: ");
  display.print(distance); //print jarak dari SR04
  display.println(" cm");

  display.print("Suhu: ");
  display.print(degree); // print suhu dari DHT
  display.println(" C");

  display.print("Kelembapan Udara: ");
  display.print(humidity); //print kelembapan dari DHT
  display.println(" %");

  display.print("Kelembapan Tanah: ");
  display.print(soil);
  display.println("%");


  display.display();

  if ((luxValue >= 10000) && (luxValue <= 20000) && (degree >=24) && (degree <= 27)) // Pastikan pagi hari, suhu dan intensitas cahaya terpenuhi pompa nyala
  {
    digitalWrite(PIN_RELAY,HIGH);
    if((soil >= 60) && (humidity >= 60)) // Jika humidity > 60%, berhenti
    {
      digitalWrite(PIN_RELAY,LOW);
    }
  }
  
}

void DataToThingsboard()
{
  WiFiClientSecure *clientDHT = new WiFiClientSecure; //DHT
  WiFiClientSecure *clientSoil = new WiFiClientSecure; //Soil
  WiFiClientSecure *clientLDR = new WiFiClientSecure; //LDR
  WiFiClientSecure *clientSR04 = new WiFiClientSecure; //SR04

  if(clientDHT)
  {
      float degree = Dht.readTemperature(); // baca suhu
      float humidity = Dht.readHumidity(); // baca kelembapan
  }

  if(clientSoil)
  {
    SoilMoisture();
  }

  if(clientLDR)
  {
    LDR();
  }

  if(clientSR04)
  {
    SR04();
  }


  clientDHT -> setInsecure();
  clientSoil -> setInsecure();
  clientLDR -> setInsecure();
  clientSR04 -> setInsecure();

  HTTPClient httpsDHT;
  HTTPClient httpsSoil;
  HTTPClient httpsLDR;
  HTTPClient httpsSR04;

  httpsDHT.begin(*clientDHT, urlDHT);
  httpsSoil.begin(*clientSoil, urlSoil);
  httpsLDR.begin(*clientLDR, urlLDR);
  httpsSR04.begin(*clientSR04, urlSR04);

  String payloadDHT = "{\"temperature\":" + String(degree) + ",\"humidity\":" + String(humidity) + "}";
  String payloadSoil = "{\"soilMoisture\":" + String(clientSoil) + "}";
  String payloadLDR = "{\"lux\":" + String(clientLDR) + "}";
  String payloadSR04 = "{\"distance\":" + String(clientSR04) + "}";

  int httpResponseCodeDHT = httpsDHT.POST(payloadDHT);
  int httpResponseCodeSoil = httpsSoil.POST(payloadSoil);
  int httpResponseCodeLDR = httpsLDR.POST(payloadLDR);
  int httpResponseCodeSR04 = httpsSR04.POST(payloadSR04);

  httpResponseCodeDHT.end();
  httpResponseCodeSoil.end();
  httpResponseCodeLDR.end();
  httpResponseCodeSR04.end():

  delay(10000);


}


