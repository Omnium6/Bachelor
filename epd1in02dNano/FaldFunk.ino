void SetupFall() {
  // init IMU sensor
  if (!IMU.begin()) {
    Serial.println("IMU sensor init failed!");
    while (true); // stop program here.
  }

  // print IMU sampling frequencies
  Serial.print("Accelerometer sampling frequency = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.print("Gyroscope sampling frequency = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");

  Serial.println();
  Serial.println("Init model..");
  if (!modelInit(model, tensorArena, tensorArenaSize)){
    Serial.println("Model initialization failed!");
    while(true);
  }
  Serial.println("Model initialization done.");
}

void checkIfFallOccured(){ //Tilføj så den chekker om hændelse er fald og sæt flag tilsvarende
  // reading cycle of all samples for current gesture
  while (samplesRead < numSamples) {
    // check if a sample is available
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      // read acceleration and gyroscope values
      IMU.readAcceleration(aX, aY, aZ);
      IMU.readGyroscope(gX, gY, gZ);

      // normalize sensor data because model was trained using normalized data
      aX = (aX + 4.0) / 8.0;
      aY = (aY + 4.0) / 8.0;
      aZ = (aZ + 4.0) / 8.0;
      gX = (gX + 2000.0) / 4000.0;
      gY = (gY + 2000.0) / 4000.0;
      gZ = (gZ + 2000.0) / 4000.0;
      
      // put the 6 values of current sample in the proper position
      // in the input tensor of the model
      modelSetInput(aX,samplesRead * 6 + 0);
      modelSetInput(aY,samplesRead * 6 + 1);
      modelSetInput(aZ,samplesRead * 6 + 2); 
      modelSetInput(gX,samplesRead * 6 + 3);
      modelSetInput(gY,samplesRead * 6 + 4);
      modelSetInput(gZ,samplesRead * 6 + 5); 
      
      samplesRead++;
      
      // if all samples are got, run inference
      if (samplesRead == numSamples) {
        if(!modelRunInference()){
          Serial.println("RunInference Failed!");
          return;
        }

        // get output values and print as percentage
        for (int i = 0; i < NUM_GESTURES; i++) {
          Serial.print(GESTURES[i]);
          Serial.print(": ");
          Serial.print(modelGetOutput(i)*100, 2);
          Serial.println("%");
        }
        Serial.println();
      }
    }
  }

}