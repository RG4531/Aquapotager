#include          <lmic.h>                     
#include          <hal/hal.h>
#include          <SPI.h>                     
#include          <arduino_lmic_hal_boards.h>
#include <DHT.h> 


#define           TX_INTERVAL               30   // seconds // temps entre 2 acquisitions
#define           RX_RSSI_INTERVAL          100   // milliseconds
#define           Null                      0     // '\0'
#define           VBATPIN                   A7 // batterie ext
#define pinDHT 12 //branchement du out    
#define typeDHT DHT22 
float temp, hygro;
DHT dht(pinDHT, typeDHT);  //dht est une variable crée 

String            SiteName                  = "ensat";
String            FrameToSend               = "";
int               sensorValue               = -1;
static uint8_t    mydata[]                  = "";
static            osjob_t sendjob;
uint8_t           MeasuresUint8_tPacket[128];
uint8_t           *PtrUint8;
char              LocalCharArray[128];
char              *PtrChar;
uint8_t           k, i, l, m;
uint8_t           NbrChar;
float             measuredvbat; // mesure tension de batterie

//****************************
//    Pins mapping                                      
//****************************
// Pin mapping: set your pin numbers here. These are for the Dragino shield.
// SEMTECH : https://lora-developers.semtech.com/documentation/tech-papers-and-guides/building-a-lora-based-device-end-to-end-with-arduino/connect-to-a-network-server/
const lmic_pinmap lmic_pins = {
    .nss = 8,                               // 8/A6, (PA06), INT6, AIN6 TCC1[0]
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LMIC_UNUSED_PIN,                 // .rst = 4,
    .dio = {3, 6, LMIC_UNUSED_PIN},
};

//****************************************************
//    OTAA mode                                        
//    AppEUI: neOCampus-Lora test 0xDEAD42DEAD42DEAD                
//****************************************************
// This Eui must be in little-endian format, so least-significant-byte first. 
// When copying an EUI from ttnctl output, this means to reverse the bytes.             
uint8_t devEui[8] PROGMEM = {0x42, 0xFF, 0x13, 0x00, 0xAD, 0xDE, 0x42, 0xFF};     //FF42DEAD0013FF42
uint8_t appEui[8] PROGMEM = {0xAD, 0xDE, 0x42, 0xAD, 0xDE, 0x42, 0xAD, 0xDE};     // your AppEUI provided by your server LoRaWAN application only for OTAA mode (LoRaWAN neOCampus)
/* identification number device in OTAA mode on LoRaWAN application from neOCampus: 0x85BD5B6B */
// This key should be in big endian format (or, since it is not really a number but a block of memory, endianness does not really apply).
uint8_t appKey[16] PROGMEM = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};  // your application key (LoRaWAN neOCampus)

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);  
  delay(4000);                 
  Serial.println(F("Starting"));
  delay(500);
  Serial.println();
  os_init();                                          // LMIC initialization oslmic.c
  LMIC_reset();                                       // lmic.c
  LMIC_setClockError(MAX_CLOCK_ERROR * 4 / 100);      // enum MAX_CLOCK_ERROR = 65536 (lmic.h)
  do_send(&sendjob);
  led_blink(5);
}

void loop() {
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

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
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
            {       // this braces here means the compiler accepts the creation of local variables and it is not mandatory if no variables have to be defined 
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
void do_send(osjob_t *j) {            // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) Serial.println(F("OP_TXRXPEND, not sending"));
  else {                            // Prepare upstream data transmission at the next possible time.
    measuredvbat = analogRead(VBATPIN);
    measuredvbat *= 2;    // we divided by 2, so multiply back
    measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
    measuredvbat /= 1024; // convert to voltage
    FrameToSend = "{\"loc\":\"" + SiteName + "\",\"v_bat\":" + String(measuredvbat) + "}";
    prepare_frame(FrameToSend);
    Serial.println();
    Serial.print("Frame to send: ");
    Serial.println(FrameToSend);
    Serial.println(F("Packet queued"));
    LMIC_setDrTxpow(DR_SF11, 16);                       // Spreading factor forced (void LMIC_setDrTxpow(dr_t dr, s1_t txpow);  // set default/start DR/txpow
    LMIC_setTxData2(1, MeasuresUint8_tPacket, NbrChar, 0);  // lmic_tx_error_t LMIC_setTxData2(u1_t port, xref2u1_t data, u1_t dlen, u1_t confirmed);
  }
  // Next TX is scheduled after TX_COMPLETE event.
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
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(600);                       // wait for a second
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW 
      delay(300);
    }
  }
  else{
    for(int i=1; i<nb+1; i++){
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(100);                       // wait for a second
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW 
      delay(150);
    }
  }
}
