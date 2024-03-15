#include <Adafruit_VEML7700.h>
#include <DHT.h> 

#define pinDHT 12 //branchement du out    
#define typeDHT DHT22 

Adafruit_VEML7700 veml = Adafruit_VEML7700();
DHT dht(pinDHT, typeDHT);  //dht est une variable crée 

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  Serial.println("Initialisation...");

  if (!veml.begin()) {
    Serial.println("Capteur de luminosité non trouvé !");
    while (1);
  }
  Serial.println("Capteur de luminosité trouvé.");

  dht.begin();
}

void loop() {
  float lux = veml.readLux(VEML_LUX_AUTO);
  float temp = dht.readTemperature();
  float hygro = dht.readHumidity();

  Serial.println("------------------------------------");
  Serial.print("Luminosité (lux) = "); Serial.println(lux);
  Serial.println("Paramètres utilisés pour la lecture de la luminosité :");
  Serial.print(F("Gain : "));
  switch (veml.getGain()) {
    case VEML7700_GAIN_1: Serial.println("1"); break;
    case VEML7700_GAIN_2: Serial.println("2"); break;
    case VEML7700_GAIN_1_4: Serial.println("1/4"); break;
    case VEML7700_GAIN_1_8: Serial.println("1/8"); break;
  }
  Serial.print(F("Temps d'intégration (ms) : "));
  switch (veml.getIntegrationTime()) {
    case VEML7700_IT_25MS: Serial.println("25"); break;
    case VEML7700_IT_50MS: Serial.println("50"); break;
    case VEML7700_IT_100MS: Serial.println("100"); break;
    case VEML7700_IT_200MS: Serial.println("200"); break;
    case VEML7700_IT_400MS: Serial.println("400"); break;
    case VEML7700_IT_800MS: Serial.println("800"); break;
  }

  Serial.print("Température (°C) : "); Serial.println(temp);
  Serial.print("Humidité relative (%) : "); Serial.println(hygro);

  delay(1000);
}


