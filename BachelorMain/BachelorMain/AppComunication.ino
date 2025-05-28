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
  pTextCharacteristic = new BLECharacteristic("6e400002-b5a3-f393-e0a9-e50e24dcca9e", BLERead | BLENotify, 10);

  // TRIN 2B: Konfigurer BLE
  // Vigtigt: Fordi vi bruger pointere, skal vi bruge '->' til at kalde funktioner
  // og '*' til at give selve objektet til funktioner, der forventer det.
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }
  
  BLE.setLocalName("FallDetector");
  BLE.setAdvertisedService(*pCustomService); // Giv objektet, ikke pointeren
  pCustomService->addCharacteristic(*pTextCharacteristic); // Tilføj karakteristikken til servicen
  BLE.addService(*pCustomService); // Tilføj servicen til BLE-stack'en

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
    if (pTextCharacteristic->subscribed()) {
      
      // Enheden lytter! Send data.
      // Jeg sender et enkelt tegn 'F' (for Fald). Det er mere effektivt end en streng.
      pTextCharacteristic->writeValue((uint8_t)'y');
      Serial.println("Sendte fald-notifikation ('y') til telefonen.");

    } else {
      Serial.println("Enhed er forbundet, men lytter ikke (har ikke subscribed) endnu.");
    }
  }
  else {
    // Ingen enhed er forbundet.
    Serial.println("Kunne ikke sende besked: Ingen central enhed er forbundet.");
  }
}