//EPD%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

#include "GUI_Paint.h"
#include "DEV_Config.h"
#include "EPD_1in02d.h"
#include "fonts.h"
#include "imagedata.h"

//FALD Detektion%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#include <Arduino_BMI270_BMM150.h> // IMU Sensor Library for Arduino Nano 33 BLE Rev.2
#include <ArduTFLite.h>
#include "model.h" //MachineLearning modellen for at se om fald falder wuup skal laves og tilføjes makker

// BlueTooth%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#include <ArduinoBLE.h>

//Farver%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#define EPD_WHITE 0x00
#define EPD_BLACK 0xff

// Gemer den data som skal displayes på displayet nudoi (Onomatopoeia)
unsigned char image_temp[1280]={0};
Paint paith(image_temp, 80, 128);

//Time Handling %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
unsigned long myTime;
unsigned long timeDelay;
unsigned int second;
unsigned int hour;
unsigned int minute;
unsigned int previousMinute;
unsigned long offset; //Used for setting time when program is powered up
unsigned int compileHour;
unsigned int compileMinute;
unsigned int compileSecond;
// Opbevarer tid som char array så den kan printes til display
char hour_str[5];
char minute_str[5];

// BLE variabler%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

//BLEService customService("6e400001-b5a3-f393-e0a9-e50e24dcca9e"); // Brugerdefineret service UUID
//BLECharacteristic textCharacteristic("6e400002-b5a3-f393-e0a9-e50e24dcca9e", BLERead | BLENotify, 1); // Karakteristik med læse- og notifikationsmulighed
// BLE variabler - nu som pointere

BLEService* pCustomService;
BLECharacteristic* pTextCharacteristic;


static bool messageSent = false;

//Fald Detektion%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
const float accelerationThreshold = 2.5; // Threshold (in G values) to detect a "gesture" start %% Skal også konfirmeres %% Tror faktisk ikke vi bruger denne men den bare er et levn
const int numSamples = 119; // Number of samples for a single gesture %% her skal denne måske også overvejes

int samplesRead; // sample counter 
const int inputLength = 714; // dimension of input tensor (6 values * 119 samples)


constexpr int tensorArenaSize = 8 * 1024;
alignas(16) byte tensorArena[tensorArenaSize];

// a simple table to map gesture labels
const char* GESTURES[] = {
  "punch",
  "flex"
};

#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0])) //Tror lidt det her bare er for at gøre tutorial børnevenlig kan nok bare laves 
                                                              //tal når vi er ved at være færdige

volatile bool fallSuspected; // Skal sættes af BMI270 hvis significant movenment er sandt
volatile bool fallConfirmed; // Bruges til at aktivere alarm

const int BMIIntPin = 3;     //Den pin den har på NINA skal måske være noget andet, svarende til D noget på arduino
const int buttonIntPin = 4;  //Den pin den har på NINA skal måske være noget andet
const int buzzerPin = 5;      //Den pin den har på NINA skal måske være noget andet

//TEST %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
const int fallSuspectedPin = 14;
const int fallConfirmedPin = 15;

// Globale variable til at håndtere timing uden delay()
unsigned long alarmStartTime = 0;
bool alarmIsActive = false;


void setup() {
  pinMode(fallSuspectedPin, OUTPUT);
  pinMode(fallConfirmedPin, OUTPUT);
  fallSuspected = false;
  fallConfirmed= false;
  Serial.begin(115200);
  while (!Serial);
  setupEPD();
  //BLEsetup();
  setupBLE();
  setupFall();
  getCompileTime();
  calculateTimeOffset(compileHour, compileMinute, compileSecond);
  pinMode(13, OUTPUT);
}

// ===== NY, IKKE-BLOKERENDE LOOP-FUNKTION =====
void loop() {
  // TRIN 1: SERVICÉR ALTID BLE FØRST!
  // Dette er det vigtigste. Denne funktion skal kaldes så ofte som muligt.
  BLE.poll();

  // ----- Logik til at starte fald-tjek -----
  if (fallSuspected == true) {
    digitalWrite(fallSuspectedPin, HIGH);
    Serial.println("Fall Suspected");
    Serial.println("Running ml model");
    checkIfFallOccured(); // Denne funktion indeholder nu den eneste blokering
    Serial.println("Model done");

    if (!fallConfirmed) {
      Serial.println("Fall not confirmed");
      fallSuspected = false;
      Serial.println("fallSuspected is set to false");
    }
  } else {
    digitalWrite(fallSuspectedPin, LOW);
  }

  // ----- Logik til at håndtere en bekræftet alarm (IKKE-BLOKERENDE) -----
  if (fallConfirmed && !alarmIsActive) {
    // Et fald er lige blevet bekræftet. Start alarm-sekvensen.
    digitalWrite(fallConfirmedPin, HIGH);
    EPDAlarm();
    Serial.println("Alarm is alarming");
    digitalWrite(buzzerPin, HIGH);
    
    alarmIsActive = true;         // Sæt et flag for at vise, at alarmen er i gang
    alarmStartTime = millis();    // Notér starttidspunktet for alarmen
  }

  // ----- Logik til at sende besked efter 5 sekunders alarm -----
  if (alarmIsActive) {
    // Tjek om der er gået 5 sekunder siden alarmen startede
    if (millis() - alarmStartTime > 5000) {
      
      // 5 sekunder er gået. Tjek om alarmen stadig er bekræftet.
      if (fallConfirmed) { 
        sendMessageBLE(); // Forsøg at sende beskeden
        //BLEloop();
        Serial.println("Har prøvet at sende besked");
      }
      // Uanset hvad, stop alarm-sekvensen nu for at undgå at den kører igen
      alarmIsActive = false; 
    }
  }

  // ----- Logik for at annullere alarmen med knappen -----
  // Hvis brugeren trykker på knappen, vil din interrupt sætte fallConfirmed = false
  if (!fallConfirmed && alarmIsActive) {
    fallSuspected = false;
    alarmIsActive = false; // Stop alarm-sekvensen
    digitalWrite(buzzerPin, LOW);
    digitalWrite(fallConfirmedPin, LOW);
    Serial.println("Fall canceled by user");
    messageSent = false;
    updateDisplayTime();
  }


  // ----- Logik til at opdatere tid og display (IKKE-BLOKERENDE) -----
  calculateTime(); 
  if (previousMinute != minute) {
    Serial.println("Tid er gået opdaterer display");
    if(!fallConfirmed) {
      updateDisplayTime();
    }
    Serial.println("Display opdateret");
    previousMinute = minute;
  } 
}

/*
void loop() {
  //skal være i sleep mode indtil den modtager int fra IMU som så skal vække
  //den og køre følgende loop, men tænker lige at starte uden
  if (fallSuspected == true) {
    digitalWrite(fallSuspectedPin, HIGH);
    Serial.println("Fall Suspected");
    Serial.println("Running ml model");
    checkIfFallOccured();
    Serial.println("Model done");
  // Kør modellen som kontrollerer for fald
  // Altså skal sættes ind (og laves) lol
    if (!fallConfirmed) {
      Serial.println("Fall not confirmed");
      fallSuspected = false;
      Serial.println("fallSuspected is set to false");
    }
  }
  else {
    digitalWrite(fallSuspectedPin, LOW); 
  }
  if (millis() > timeDelay + 15000) {
  Serial.println("Regner tid");
  calculateTime(); // Burde nok ligge et andet sted
  Serial.println("Tid regnet");
  timeDelay = millis();
  }
  if (fallConfirmed) {
    digitalWrite(fallConfirmedPin, HIGH);
    EPDAlarm();
    Serial.println("Alarm is alarming");
    digitalWrite(buzzerPin, HIGH);// dette er hvis den er aktiv
    delay(5000); //delay for at brugeren har tid til at annulere alarmen
                 // Kan / burde nok laves om til noget med millis
    if(fallConfirmed) { //Ser fjollet ud men interrupts, giver mulighed for at ændre den under delay
      digitalWrite(13, HIGH);
      BLEloop();
      //sendMessageBLE();
      Serial.println("Fall confirmed message sent to phone");
    }
    else { //tanken er brugeren har afbrudt alarm med knappen hvorfor fallConfirmed ikke længere er true
      fallSuspected = false;
      Serial.println("fall canceled by user");
      digitalWrite(buzzerPin, LOW);
      // slå alarm fra på en måde
    }
  }
  if(!fallConfirmed) {
    digitalWrite(fallConfirmedPin, LOW);
  }

  if (previousMinute != minute){
    Serial.println("Tid er gået opdaterer display");
    updateDisplayTime();
    Serial.println("Display opdateret");
    previousMinute = minute;
    // Gør ting 
  }  
  //delay(500);
}
*/
