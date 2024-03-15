#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <arduino_lmic_hal_boards.h>
#include <DHT.h>
#include <Adafruit_VEML7700.h>

#define TX_INTERVAL          30    // secondes
#define RX_RSSI_INTERVAL     100   // millisecondes
#define Null                 0     // '\0'
#define VBATPIN              A7    // batterie externe
#define pinDHT               12    // branchement du capteur DHT
#define typeDHT              DHT22

float temp, hygro;
DHT dht(pinDHT, typeDHT);  // capteur DHT
Adafruit_VEML7700 veml;

String SiteName = "ensat";
String FrameToSend = "";
int sensorValue = -1;
static uint8_t mydata[] = "";
static osjob_t sendjob;
uint8_t MeasuresUint8_tPacket[128];
uint8_t *PtrUint8;
char LocalCharArray[128];
char *PtrChar;
uint8_t k, i, l, m;
uint8_t NbrChar;
float measuredvbat;  // mesure de tension de batterie

const lmic_pinmap lmic_pins = {
    .nss = 8,           // broche pour NSS
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LMIC_UNUSED_PIN,  // broche pour RESET
    .dio = {3, 6, LMIC_UNUSED_PIN},
};

uint8_t devEui[8] PROGMEM = {0x42, 0xFF, 0x13, 0x00, 0xAD, 0xDE, 0x42, 0xFF};
uint8_t appEui[8] PROGMEM = {0xAD, 0xDE, 0x42, 0xAD, 0xDE, 0x42, 0xAD, 0xDE};
uint8_t appKey[16] PROGMEM = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  delay(4000);
  Serial.println(F("Starting"));
  delay(500);
  Serial.println();
  
  if (!veml.begin()) {
    Serial.println("Capteur de luminosité non trouvé !");
    while (1);
  }
  Serial.println("Capteur de luminosité trouvé.");

  dht.begin();

  os_init();
  LMIC_reset();
  LMIC_setClockError(MAX_CLOCK_ERROR * 4 / 100);
  // Envoi des données
  do_send(&sendjob);
  led_blink(5);
}

void loop() {  
  // loop_sensors collecte les données des capteurs
  loop_sensors();
  // assure le fonctionnement global du système                                            
  os_runloop_once();
}


//**************************
// Gestion protocole LoRaWAN
//**************************

/********************************************************************************************************************/
/* Functions to collect credential values which have been exchanged with gateway and LoRa application on server.    */
/********************************************************************************************************************/

void os_getArtEui(u1_t *buf) {
  memcpy_P(buf, appEui, 8);
}

void os_getDevEui(u1_t *buf) {
  memcpy_P(buf, devEui, 8);
}

void os_getDevKey(u1_t *buf) {
  memcpy_P(buf, appKey, 16);
}

void printHex2(unsigned v) {
  v &= 0xff;
  if (v < 16) Serial.print('0');
  Serial.print(v, HEX);
}

void onEvent(ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  switch (ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println(F("EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      Serial.println(F("EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      Serial.println(F("EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      Serial.println(F("EV_BEACON_TRACKED"));
      break;
    case EV_JOINING:
      Serial.println(F("EV_JOINING"));
      break;
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));
      {
        u4_t netid = 0;
        devaddr_t devaddr = 0;
        u1_t nwkKey[16];
        u1_t artKey[16];
        LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
        Serial.print("netid: ");
        Serial.println(netid, DEC);
        Serial.print("devaddr: ");
        Serial.println(devaddr, HEX);
        Serial.print("AppSKey: ");
        for (size_t i = 0; i < sizeof(artKey); ++i) {
          if (i != 0) Serial.print("-");
          printHex2(artKey[i]);
        }
        Serial.println("");
        Serial.print("NwkSKey: ");
        for (size_t i = 0; i < sizeof(nwkKey); ++i) {
          if (i != 0) Serial.print("-");
          printHex2(nwkKey[i]);
        }
        Serial.println();
      }
      LMIC_setLinkCheckMode(0);
      break;
    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      break;
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
      if (LMIC.txrxFlags & TXRX_ACK) Serial.println(F("Received ack"));
      if (LMIC.dataLen) {
        Serial.print(F("Received "));
        Serial.print(LMIC.dataLen);
        Serial.println(F(" bytes of payload"));
      }
      // Schedule next transmission
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      led_blink(3);
      break;
    case EV_LOST_TSYNC:
      Serial.println(F("EV_LOST_TSYNC"));
      break;
    case EV_RESET:
      Serial.println(F("EV_RESET"));
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println(F("EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      Serial.println(F("EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      Serial.println(F("EV_LINK_ALIVE"));
      break;
    case EV_TXSTART:
      Serial.println(F("EV_TXSTART"));
      break;
    case EV_TXCANCELED:
      Serial.println(F("EV_TXCANCELED"));
      break;
    case EV_RXSTART:                    /* do not print anything -- it wrecks timing */
      break;
    case EV_JOIN_TXCOMPLETE:
      Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
      led_blink(0);
      break;
    default:
      Serial.print(F("Unknown event: "));
      Serial.println((unsigned) ev);
      break;
  }
}

/********************************************************************************************************************/
/* Function to store measures collected in an array mydata[]. LMIC is a structure declared in oslmic.h using macro: */
/* #define DEFINE_LMIC struct lmic_t LMIC                                                                           */
/* lmic_t is a structure defined in lmic.h                                                                          */
/********************************************************************************************************************/

void do_send(osjob_t *j) {
  if (LMIC.opmode & OP_TXRXPEND) Serial.println(F("OP_TXRXPEND, not sending"));
  else {
    measuredvbat = analogRead(VBATPIN);
    measuredvbat *= 2;    // we divided by 2, so multiply back
    measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
    measuredvbat /= 1024; // convert to voltage
    FrameToSend = "{\"loc\":\"" + SiteName + "\",\"v_bat\":" + String(measuredvbat) + ",\"temp\":" + String(temp) + ",\"hygro\":" + String(hygro) + "}";

    prepare_frame(FrameToSend);
    Serial.println();
    Serial.print("Frame to send: ");
    Serial.println(FrameToSend);
    Serial.println(F("Packet queued"));
    LMIC_setDrTxpow(DR_SF11, 16);                       
    LMIC_setTxData2(1, MeasuresUint8_tPacket, NbrChar, 0);  
  }
}

void prepare_frame(String frame){
  memset(LocalCharArray, Null, sizeof(LocalCharArray));
  NbrChar = frame.length();
  frame.toCharArray(LocalCharArray, NbrChar + 1);
  PtrChar = &LocalCharArray[0];
  memset(MeasuresUint8_tPacket, Null, sizeof(MeasuresUint8_tPacket));
  for (k = 0; k < NbrChar; k++) MeasuresUint8_tPacket[k] = (uint8_t)*(PtrChar++);
}
//******************
// Clignotement LED
//******************

void led_blink(int nb){
  if(nb==0){
    for(int i=1; i<3; i++){
      digitalWrite(LED_BUILTIN, HIGH);   
      delay(600);                       
      digitalWrite(LED_BUILTIN, LOW);    
      delay(300);
    }
  }
  else{
    for(int i=1; i<nb+1; i++){
      digitalWrite(LED_BUILTIN, HIGH);   
      delay(100);                       
      digitalWrite(LED_BUILTIN, LOW);    
      delay(150);
    }
  }
}



void loop_sensors() {
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




