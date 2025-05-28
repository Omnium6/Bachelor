

void suspectFall() {
  fallSuspected = true;
}

void buttonPressed() {
  fallConfirmed = !fallConfirmed;
}


void setupFall() {
  
  pinMode(BMIIntPin, INPUT_PULLUP);
  pinMode(buttonIntPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BMIIntPin), suspectFall, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonIntPin), buttonPressed, FALLING);

  // Initialiser IMU-sensoren
  if (!IMU.begin()) {
    Serial.println("IMU sensor initialisering fejlede!");
    while (true); // Stop programmet her.
  }

  // UDSKRIV SAMPLING FREKVENS (VIGTIGT FOR FEJLSØGNING)
  // Hvis disse værdier er 0, så ved vi, at IMU'en ikke sampler korrekt.
  Serial.println("-----------------------------------");
  Serial.print("Accelerometer sampling frekvens = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.print("Gyroskop sampling frekvens = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");
  Serial.println("-----------------------------------");

  // Initialiser TensorFlow modellen
  if (!modelInit(model, tensorArena, tensorArenaSize)) {
    Serial.println("Model initialisering fejlede!");
    while (true);
  }

  Serial.println("Model initialisering er færdig.");
}


void checkIfFallOccured() {
  Serial.println("Starter fald-tjek og dataindsamling...");
  
  // Nulstil sample-tælleren FØR vi begynder at indsamle nye samples
  samplesRead = 0; 
  
  float aX, aY, aZ, gX, gY, gZ;

  // Løkken kører, indtil det nødvendige antal samples er indsamlet.
  while (samplesRead < numSamples) {
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      Serial.println("IMU i gang"); // Tilføjet for at se, om IMU faktisk giver data
      
      // Aflæs data
      IMU.readAcceleration(aX, aY, aZ);
      IMU.readGyroscope(gX, gY, gZ);

      // Normaliser data
      aX = (aX + 4.0) / 8.0;
      aY = (aY + 4.0) / 8.0;
      aZ = (aZ + 4.0) / 8.0;
      gX = (gX + 2000.0) / 4000.0;
      gY = (gY + 2000.0) / 4000.0;
      gZ = (gZ + 2000.0) / 4000.0;
      
      // Læg data i tensoren
      modelSetInput(aX, samplesRead * 6 + 0);
      modelSetInput(aY, samplesRead * 6 + 1);
      modelSetInput(aZ, samplesRead * 6 + 2); 
      modelSetInput(gX, samplesRead * 6 + 3);
      modelSetInput(gY, samplesRead * 6 + 4);
      modelSetInput(gZ, samplesRead * 6 + 5); 
      
      samplesRead++;
      Serial.print("Læser samples: "); // Tilføjet for at følge med i sample-læsningen
      Serial.println(samplesRead); 
    }
    // Overvej at tilføje et lille delay her, hvis IMU.accelerationAvailable() 
    // og IMU.gyroscopeAvailable() sjældent er true, for at undgå at spinde for hurtigt.
    // Men vær forsigtig, da det kan påvirke sample rate.
    // delay(1); // Kun hvis nødvendigt
  }

  // Når alle samples er indsamlet, kør modellen.
  if (samplesRead == numSamples) {
  Serial.println("Alle samples indsamlet. Kører model...");
  if (!modelRunInference()) {
    Serial.println("RunInference Fejlede!");
    fallSuspected = false; // Nulstil, da der ikke kunne køres inference
    return;
  }

  // Hent og udskriv resultater
  Serial.println("Model resultater:");
  for (int i = 0; i < NUM_GESTURES; i++) {
    Serial.print(GESTURES[i]);
    Serial.print(": ");
    Serial.print(modelGetOutput(i) * 100, 2);
    Serial.println("%");
  }
  Serial.println();

  // Opdater flag baseret på output
  // Jeg har brugt din oprindelige tærskel (0.5), men du nævnte også 0.8 i den kommenterede kode.
  // Vælg den tærskel, der passer bedst til din model.
  if (modelGetOutput(1) > 0.5) { // Fald bekræftet
    fallConfirmed = true;
    fallSuspected = false;
    Serial.println("Fald bekræftet af model.");
  } else {
    fallSuspected = false; // Ikke et fald, nulstil 'fallSuspected'
    Serial.println("Fald IKKE bekræftet af model.");
  }
  }
  else {
    Serial.println("samplesRead != numSamples");
  }
}