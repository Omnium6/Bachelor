/*
void setupBLE() {
  
  if (!BLE.begin()) {
    Serial.println("BLE kunne ikke starte");
  }

  BLE.setLocalName("ArduinoNano33");
  BLE.setDeviceName("ArduinoNano33");
  BLE.setAdvertisedService(customService);

  customService.addCharacteristic(textCharacteristic);
  BLE.addService(customService);

  BLE.advertise();

  Serial.println("BLE service kører og reklamerer...");
}

void sendMessageBLE() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Forbundet til: ");
    Serial.println(central.address());

    while (central.connected()) {
      if (!messageSent && textCharacteristic.subscribed()) {
        textCharacteristic.writeValue("y");
        Serial.println("Sendte 'Yes' til telefonen");
        messageSent = true;
      }
      if (messageSent) {
        break;
      }
    }

      delay(1000);
  }
  else {
    Serial.println("Error"); // skriv error på display
  }

  delay(500);
}
*/

void setupBLE() {
  Serial.println("Starter BLE setup...");
  
  // TRIN 2A: Opret objekterne dynamisk med 'new'
  // Hukommelsen allokeres nu, mens programmet kører.
  pCustomService = new BLEService("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
  pTextCharacteristic = new BLECharacteristic("6e400003-b5a3-f393-e0a9-e50e24dcca9e", BLERead | BLENotify, 10);
  pTimeSetCharacteristic = new BLECharacteristic(TIME_SET_CHAR_UUID, BLEWrite, 8); 

  // === TILFØJ DISSE TO LINJER ===
  // Fortæl biblioteket, hvilke funktioner der skal kaldes ved hændelser
  BLE.setEventHandler(BLEConnected, onBLEConnected);
  BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);


  // TRIN 2B: Konfigurer BLE
  // Vigtigt: Fordi vi bruger pointere, skal vi bruge '->' til at kalde funktioner
  // og '*' til at give selve objektet til funktioner, der forventer det.
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }
  
  BLE.setLocalName("FallDetector");
  //BLE.setDeviceName("FallDetector"); //Måsle ikke nødvendig fjern hvis du græder
  BLE.setAdvertisedService(*pCustomService); // Giv objektet, ikke pointeren
  pCustomService->addCharacteristic(*pTextCharacteristic); // Tilføj karakteristikken til servicen
   pCustomService->addCharacteristic(*pTimeSetCharacteristic);
  BLE.addService(*pCustomService); // Tilføj servicen til BLE-stack'en

// Sæt den NYE event handler for, når der skrives til tid-karakteristikken
  pTimeSetCharacteristic->setEventHandler(BLEWritten, onTimeSetWritten);


  // Start advertising
  BLE.advertise();
  Serial.println("BLE advertising startet...");
}

void sendMessageBLE() {
  // Tjek først, om en central enhed (din telefon) er forbundet
  BLEDevice central = BLE.central();

  if (central) {
    // En enhed ER forbundet. Print dens adresse.
    Serial.print("Forbundet til central enhed: ");
    Serial.println(central.address());

    // Nu tjekker vi, om enheden rent faktisk lytter på vores karakteristik.
    // Dette er din geniale 'subscribed()' logik.
    // Vi bruger '->' til at kalde funktionen, fordi pTextCharacteristic er en pointer.
    while (central.connected()){

      if (!messageSent && pTextCharacteristic->subscribed()) {
        // Enheden lytter! Send data.
        // Jeg sender et enkelt tegn 'F' (for Fald). Det er mere effektivt end en streng.
        pTextCharacteristic->writeValue((uint8_t)'y');
        Serial.println("Sendte fald-notifikation ('y') til telefonen.");
        messageSent = true;
      } 
      if (messageSent) {
        break;
      }
    } 
    if(!messageSent) {
      Serial.println("Sendte ikke besked :'(");
    }
  }
  else {
    // Ingen enhed er forbundet.
    Serial.println("Kunne ikke sende besked: Ingen central enhed er forbundet.");
  }
}

void onBLEConnected(BLEDevice central) {
  // Denne funktion kører, når en enhed forbinder
  Serial.print("Enhed forbundet med succes: ");
  Serial.println(central.address());
}

void onBLEDisconnected(BLEDevice central) {
  // Denne funktion kører, når en enhed afbryder forbindelsen
  Serial.print("Enhed afbrudt: ");
  Serial.println(central.address());
  
  // Start advertising igen, så enheden er synlig
  Serial.println("Starter advertising igen...");
  BLE.advertise();
  messageSent = false; // VIGTIGT: Nulstil flaget her!
}

// Funktion der kaldes, når tid modtages fra telefonen
void onTimeSetWritten(BLEDevice central, BLECharacteristic characteristic) {
  Serial.print("Tid modtaget fra telefon: ");
  timeRecived = true; // Dit flag for modtaget tid
  Serial.println(central.address());

  // Hent den værdi (data), der blev skrevet
  const uint8_t* data = characteristic.value();
  int len = characteristic.valueLength();
  
  // Konverter data til en tekststreng
  // Sørg for, at bufferen er stor nok til "HH.MM.SS" (8 tegn) + null-terminator ('\0') = 9 tegn
  char timeStr[len + 1]; // Eksempel: hvis len er 8, bliver arrayet timeStr[9]
  memcpy(timeStr, data, len);
  timeStr[len] = '\0'; // Vigtigt at null-terminere strengen

  Serial.print("Modtaget tid-streng: '");
  Serial.print(timeStr);
  Serial.println("'");

  // Forsøg at parse "HH.MM.SS" formatet
  int receivedHour = -1, receivedMinute = -1, receivedSecond = -1;
  // sscanf returnerer antallet af succesfuldt konverterede items. Vi forventer nu 3.
  // Formatstrengen er ændret til at bruge '.' som skilletegn.
  if (sscanf(timeStr, "%d.%d.%d", &receivedHour, &receivedMinute, &receivedSecond) == 3) {
    
    // Tjek om de parsede værdier er gyldige
    if (receivedHour >= 0 && receivedHour <= 23 && 
        receivedMinute >= 0 && receivedMinute <= 59 &&
        receivedSecond >= 0 && receivedSecond <= 59) {
      
      Serial.print("Parsed tid: ");
      Serial.print(receivedHour);
      Serial.print("."); // Brug '.' for konsistens med input
      Serial.print(receivedMinute);
      Serial.print(".");
      Serial.println(receivedSecond);

      // Opdater Arduinoens interne tid ved at bruge din eksisterende funktion
      calculateTimeOffset(receivedHour, receivedMinute, receivedSecond); 
      
      // Efter at have sat nyt offset, skal vi straks genberegne den aktuelle tid
      calculateTime(); 
      previousMinute = minute; // Synkroniser for at undgå unødig display-opdatering
      
      // Opdater displayet med den nye tid (hvis fald ikke er aktivt)
      if (!fallConfirmed && !alarmIsActive) { 
          updateDisplayTime();
      }
      Serial.println("Arduino tid er nu opdateret med timer, minutter og sekunder.");

    } else {
      Serial.println("Fejl: Ugyldigt tid format (tal udenfor gyldigt interval). Forventer HH.MM.SS");
    }
  } else {
    Serial.println("Fejl: Kunne ikke parse tid-streng. Forventer \"HH.MM.SS\" format (f.eks. \"14.40.44\").");
  }
}

/*
void BLEsetup() {
  Serial.begin(9600);

  if (!BLE.begin()) {
    Serial.println("BLE kunne ikke starte");
  }

  BLE.setLocalName("FallDetector");
  BLE.setDeviceName("FallDetector");
  BLE.setAdvertisedService(customService);

  customService.addCharacteristic(textCharacteristic);
  BLE.addService(customService);

  BLE.advertise();

  Serial.println("BLE service kører og reklamerer...");
}


void BLEloop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Forbundet til: ");
    Serial.println(central.address());

    while (central.connected()) {
      if (!messageSent && textCharacteristic.subscribed()) {
        textCharacteristic.writeValue("y");
        Serial.println("Sendte 'Yes' til telefonen");
        messageSent = true;
      }
      if (messageSent) {
        break;
      }
    }

      delay(1000);
  }
  else {
    Serial.println("Error"); // skriv error på display
  }

  delay(500);
}
*/