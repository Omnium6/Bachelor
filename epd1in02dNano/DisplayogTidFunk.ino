
//FUNKTIONER%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void getCompileTime(int& compileHour, int& compileMinute) {
  // __TIME__ er en streng-literal i formatet "HH:MM:SS"
  const char* timeStr = __TIME__;

  // Udtræk timer (de første to tegn)
  char hourStr[3]; // Plads til to cifre + null-terminator
  hourStr[0] = timeStr[0];
  hourStr[1] = timeStr[1];
  hourStr[2] = '\0'; // Null-terminer strengen
  compileHour = atoi(hourStr); // Konverter streng til integer

  // Udtræk minutter (tegnene ved indeks 3 og 4)
  char minuteStr[3]; // Plads til to cifre + null-terminator
  minuteStr[0] = timeStr[3];
  minuteStr[1] = timeStr[4];
  minuteStr[2] = '\0'; // Null-terminer strengen
  compileMinute = atoi(minuteStr); // Konverter streng til integer
}

void getPhoneTime(int& compileHour, int& compileMinute) { //Brug den her når denne bliver tilgængelig

}

void SetupEPDandIMU() {
  int compileHour;
  int compileMinute;
  Config_Init();
  if(EPD_Init() != 0) {
      Serial.print("e-Paper init failed\r\n");
  }
  if (!IMU.begin()) {
    Serial.println("IMU kunne ikke initialiseres");
    while (1);
  }
  Serial.println("IMU påbegyndt");

  Serial.print("1.02inch e-Paper Module1\r\n");
  EPD_Clear();
  DEV_Delay_ms(500);
  getCompileTime(compileHour, compileMinute);
  CalculateTimeOffset(compileHour, compileMinute);
}


void UpdateDisplayTimeBuffer() {
 // 2. Formatér tiden som en streng (f.eks. "05", "12")
  sprintf(hour_str, "%02u", hour); // %02u sikrer to cifre med foranstillet nul
  sprintf(minute_str, "%02u", minute); // %02u sikrer to cifre med foranstillet nul
  paith.DrawStringAt(25, 35, hour_str, &Font24, EPD_BLACK);
  paith.DrawStringAt(25, 65, minute_str, &Font24, EPD_BLACK);
  //paith.DrawRectangle(15, 10, 60, 68, EPD_BLACK);
}

void CalculateTime() {
  myTime = millis() + offset;

  unsigned long totalSeconds = myTime / 1000;
  second = totalSeconds % 60;

  unsigned long totalMinutes = totalSeconds / 60;
  minute = totalMinutes % 60;

  unsigned long totalHours = totalMinutes / 60;
  hour = totalHours % 24;
}

void CalculateTimeOffset(int init_hour, int init_minute) {
  // skal regne tiden om fra tid til millisekunder
  unsigned long current_minute = init_hour*60 + init_minute;

  offset = current_minute * 60 * 1000;
}