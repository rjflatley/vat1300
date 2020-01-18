/*  Juntek VAT-1300 data logger   
 *  tested on Arduino Uno and ESP32 (Node32s)
 */
#include <SPI.h>
#include "RF24.h"
#include "printf.h"


#if defined(ARDUINO_AVR_UNO)
  // Uno pin assignments
  const int irqPin = 2;
  const int cePin = 7;
  const int csnPin = 8;
  const int mosiPin = 11;
  const int misoPin = 12;
  const int sckPin = 13;
//#elif defined(ARDUINO_SAM_DUE)
//  // Due assignments
//  // SPI pins csn, mosi, miso, sck are wired directly to SPI header 
//  #define RF24_DUE 
//  const int irqPin = 6;
//  const int cePin = 7;
//  const int csnPin = 10;
#elif defined(ARDUINO_ARCH_ESP32)
  const int irqPin = 4;
  const int cePin = 17;
  const int csnPin = 5;
  const int mosiPin = 23;
  const int misoPin = 19;
  const int sckPin = 18;
#else
  #error Unsupported board selection.
#endif

const int address = 27;
const char frequency = 'A';

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 0;

int lpCnt = 0;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus */
RF24 radio(cePin, csnPin);
/**********************************************************/

uint64_t pipes[2] = { 0x8967452300LL, 0x8967452300LL };



/**********************************************************/
//Function to configure the radio
void configureRadio() {

  radio.begin();

  radio.setAutoAck(false);
  radio.setDataRate(RF24_2MBPS);
  for (int i = 0; i < 6; i++)
    radio.closeReadingPipe(i);
  // Open a writing and reading pipe
  pipes[0] |= (uint64_t)address;
  pipes[1] |= (uint64_t)address;
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(0, pipes[1]);
  radio.setAutoAck(0, true);
  radio.setChannel((frequency - 'A') * 4);
  radio.setPALevel(RF24_PA_MAX);

  radio.printDetails();

}

/**********************************************************/

void setup() {
  Serial.begin(115200);
  pinMode(irqPin, INPUT);
  printf_begin();

  configureRadio();
}

uint32_t configTimer =  millis();

void loop() {

  if (radio.failureDetected) {
    radio.failureDetected = false;
    delay(250);
    Serial.println("Radio failure detected, restarting radio");
    configureRadio();
  }
  //Every 5 seconds, verify the configuration of the radio. This can be done using any
  //setting that is different from the radio defaults.
  if (millis() - configTimer > 5000) {
    configTimer = millis();
    if (radio.getDataRate() != RF24_2MBPS) {
      radio.failureDetected = true;
      Serial.print("Radio configuration error detected");
    }
  }


  radio.stopListening();                                    // First, stop listening so we can talk.


  //Serial.println(F("Now sending"));
  byte aTX_PAYLOAD[] = { 0xAA, 0x04, 0x02, address, address + 0xB0, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00 }; 


  aTX_PAYLOAD[4] = address + 0xB0;
/* first time in loop set TX_Payload to fetch setup data */
  if (lpCnt == 0)
  {
    aTX_PAYLOAD[2] = 2;
    aTX_PAYLOAD[4] = address + 0xB0;
  }
/* second time onward in loop set TX_Payload to fetch run data */
  if (lpCnt == 1)
  {
    aTX_PAYLOAD[2] = 1;
    aTX_PAYLOAD[4] = address + 0xAF;
  }
/* special recovery of base unit for power failures */
  if (lpCnt < 0)
  {
    aTX_PAYLOAD[2] = 0xFF;
    aTX_PAYLOAD[4] = address + 0xAD;
    lpCnt = 0;
  }


  unsigned long start_time = micros();                             // Take the time, and send it.  This will block until complete
  if (!radio.write(&aTX_PAYLOAD, sizeof(aTX_PAYLOAD))) {
    Serial.println(F("failed"));
  }

  radio.startListening();                                    // Now, continue listening

  unsigned long started_waiting_at = micros();               // Set up a timeout period, get the current microseconds
  boolean timeout = false;                                   // Set up a variable to indicate if a response was received or not
  byte aRX_PAYLOAD[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00 };

  while ( ! radio.available() ) {                            // While nothing is received
    if (micros() - started_waiting_at > 200000 ) {           // If waited longer than 200ms, indicate timeout and exit while loop
      timeout = true;
      break;
    }
  }

  if ( timeout ) {                                            // Describe the results
    Serial.println(F("Failed, response timed out."));
    lpCnt = -2;
  } else {
    unsigned long got_time;                                 // Grab the response, compare, and send to debugging spew

    //Failure Handling:
    uint32_t failTimer = millis();
    while (radio.available()) {                             //If available always returns true, there is a problem
      if (millis() - failTimer > 250) {
        radio.failureDetected = true;
        Serial.println("Radio available failure detected");
        break;
      }
      radio.read( &aRX_PAYLOAD, sizeof(aRX_PAYLOAD) );             // Get the payload
    }
    unsigned long end_time = micros();

    // Spew it
    /*Serial.print(F("Sent "));
      Serial.print(start_time);
      Serial.print(F(", Got response "));
      Serial.print(got_time);
      Serial.print(F(", Round-trip delay "));
      Serial.print(end_time - start_time);
      Serial.println(F(" microseconds"));*/
    /*for (int i = 0; i < 32; i++)
      {
      Serial.print(aRX_PAYLOAD[i], HEX);
      Serial.print(",");
      }
      Serial.println("");*/
      
/* Print headers and setup data  */
    if (lpCnt == 0)
    {
      float battAmpHours  = (uint16_t)(aRX_PAYLOAD[6] << 8 | aRX_PAYLOAD[7]) / 10.0;
      float ovp           = (int16_t)(aRX_PAYLOAD[8]  << 8 | aRX_PAYLOAD[9]) / 100.0;
      float lvp           = (int16_t)(aRX_PAYLOAD[10] << 8 | aRX_PAYLOAD[11]) / 100.0;
      float ocp           = (int16_t)(aRX_PAYLOAD[12] << 8 | aRX_PAYLOAD[13]);
      float ncp           = (int16_t)(aRX_PAYLOAD[14] << 8 | aRX_PAYLOAD[15]);
      int rDelay          = aRX_PAYLOAD[17];
      int calVoltage      = aRX_PAYLOAD[18] << 8 | aRX_PAYLOAD[19];
      int calCurrentIeq0  = aRX_PAYLOAD[22] << 8 | aRX_PAYLOAD[23];
      int calCurrentIgt0  = aRX_PAYLOAD[24] << 8 | aRX_PAYLOAD[25];

      Serial.println("Battary\t\t\t\t\t\tcal\tcal Currrent");
      Serial.println("AmpHour\tOVP\tLVP\tOCP\tNCP\tDelay\tVoltage\tI=0\tI>0");
      Serial.print(battAmpHours);
      Serial.print("\t");
      Serial.print(ovp);
      Serial.print("\t");
      Serial.print(lvp);
      Serial.print("\t");
      Serial.print(ocp, 0);
      Serial.print("\t");
      Serial.print(ncp, 0);
      Serial.print("\t");
      Serial.print(rDelay);
      Serial.print("\t");
      Serial.print(calVoltage);
      Serial.print("\t");
      Serial.print(calCurrentIeq0);
      Serial.print("\t");
      Serial.println(calCurrentIgt0);
      Serial.println("\t\t\tAmp\tWatt\tTime\t\t");
      Serial.println("Voltage\tCurrent\tWatts\thours\thours\tseconds\tTemp\tOutput");
    }
    else
/* print run data */    
    {
      float voltage   = (int16_t)(aRX_PAYLOAD[4]  << 8  | aRX_PAYLOAD[5]) / 100.0;
      float amperage  = (int16_t)(aRX_PAYLOAD[6]  << 8  | aRX_PAYLOAD[7]) / 10.0;
      float wattage   = (int32_t)(aRX_PAYLOAD[8]  << 24 | aRX_PAYLOAD[9]  << 16 | aRX_PAYLOAD[10] << 8 |  aRX_PAYLOAD[11]) / 1000.0;
      float ampHours  = (int32_t)(aRX_PAYLOAD[12] << 24 | aRX_PAYLOAD[13] << 16 | aRX_PAYLOAD[14] < 8 |  aRX_PAYLOAD[15])  / 1000.0;
      float wattHours = (int32_t)(aRX_PAYLOAD[16] << 24 | aRX_PAYLOAD[17] << 16 | aRX_PAYLOAD[18] << 8 | aRX_PAYLOAD[19])  / 1000.0;
      int32_t seconds = (int32_t)(aRX_PAYLOAD[20] << 24 + aRX_PAYLOAD[21] << 16 | aRX_PAYLOAD[22] << 8 | aRX_PAYLOAD[23]);
      int8_t temperature = (int8_t)aRX_PAYLOAD[25];
      bool outputOn;
      if (aRX_PAYLOAD[24] == 0)
        outputOn = true;
      else
        outputOn = false;

      Serial.print(voltage, 2);
      Serial.print("\t");
      Serial.print(amperage, 1);
      Serial.print("\t");
      Serial.print(wattage, 3);
      Serial.print("\t");
      Serial.print(ampHours, 3);
      Serial.print("\t");
      Serial.print(wattHours, 3);
      Serial.print("\t");
      Serial.print(seconds);
      Serial.print("\t");
      Serial.print(temperature);
      Serial.print("\t");
      if (outputOn == true)
        Serial.println("On ");
      else
        Serial.println("Off");
    }
    lpCnt = 1;
  }

  //    delay(1000);

} // Loop
