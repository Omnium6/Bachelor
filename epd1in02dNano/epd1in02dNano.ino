//EPD%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#include "GUI_Paint.h"
#include "DEV_Config.h"
#include "EPD_1in02d.h"
#include "fonts.h"
#include "imagedata.h"

//FALD Detektion%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#include <Arduino_BMI270_BMM150.h> // IMU Sensor Library for Arduino Nano 33 BLE Rev.2
#include <ArduTFLite.h>
//#include "model.h" //MachineLearning modellen for at se om fald falder wuup skal laves og tilføjes makker

//Bluetooth Fobindelse %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#include <ArduinoBLE.h>

// Definer UUIDs for din BLE service og characteristic
// Du kan generere dine egne på f.eks. www.uuidgenerator.net
const char* DEVICE_NAME = "Nano_33_BLE_Sender";
const char* SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214";
const char* CHARACTERISTIC_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214";

// Opret BLE service og characteristic
BLEService ledService(SERVICE_UUID);
BLEStringCharacteristic messageCharacteristic(CHARACTERISTIC_UUID, BLERead | BLENotify | BLEWrite, 20); // Max 20 bytes for beskeden

// Besked der skal sendes
String beskedTilESP32LEDOn = "SÆT_PIN_HØJ";
String beskedTilESP32LEDOff = "SÆT_PIN_LAV";
bool LEDOn = false;

unsigned long previousMillis = 0;
const long interval = 5000; // Send besked hvert 5. sekund som eksempel

//Farver%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#define EPD_WHITE 0x00
#define EPD_BLACK 0xff

// Gemer den data som skal displayes på displayet nudoi (Onomatopoeia)
unsigned char image_temp[1280]={0};
Paint paith(image_temp, 80, 128);

//Time Handling %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
unsigned long myTime;
unsigned int second;
unsigned int hour;
unsigned int minute;
unsigned int previousMinute;
unsigned long offset; //Used for setting time when program is powered up

//Fald Detektion%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
const float accelerationThreshold = 2.5; // Threshold (in G values) to detect a "gesture" start
const int numSamples = 119; // Number of samples for a single gesture

int samplesRead; // sample counter 
const int inputLength = 714; // dimension of input tensor (6 values * 119 samples)

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// Opbevarer tid som char array så den kan printes til display
char hour_str[5];
char minute_str[5];


void setup() {
  Serial.begin(115200);
  SetupEPDandIMU();
}

void loop() {
  if (previousMinute != minute) {
    //Ryd billedbufferen til baggrundsfarven (HVID)
    paith.Clear(EPD_WHITE);

    Serial.print("Buffer cleared. Hour = ");
    Serial.print(hour);
    Serial.print("  Minute: ");
    Serial.println(minute);

    UpdateDisplayTimeBuffer();
   
    //ConnectionState();


    // 4. Send bufferens indhold til E-Paper displayet
    EPD_Display(image_temp);
    Serial.println("Display opdateret");
    previousMinute = minute;
  }

  CalculateTime();
}