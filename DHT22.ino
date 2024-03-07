
#include <DHT.h> 
#define pinDHT 12 //branchement du out    
#define typeDHT DHT22 

float temp, hygro;
DHT dht(pinDHT, typeDHT);  //dht est une variable crée 

void setup() {
  Serial.begin(115200);
  dht.begin(); 
}

void loop() {
 temp = dht.readTemperature();
 hygro = dht.readHumidity();
 Serial.print("Il fait une température de ");
 Serial.print(temp);
 Serial.print("°C et ");
 Serial.print(hygro);
 Serial.println("% d'humidité relative");
 delay(1000);
}