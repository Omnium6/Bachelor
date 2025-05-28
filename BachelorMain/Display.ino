
void setupEPD() {
  int compileHour;
  int compileMinute;
  pinMode(buzzerPin, OUTPUT);
  Config_Init();
  if(EPD_Init() != 0) {
    Serial.print("Display init failed");
  }
  EPD_Clear();
  DEV_Delay_ms(500);
}

void updateDisplayTime() {
  paith.Clear(EPD_WHITE);
  sprintf(hour_str, "%02u", hour); // %02u sikrer to cifre med foranstillet nul
  sprintf(minute_str, "%02u", minute); // %02u sikrer to cifre med foranstillet nul
  paith.DrawStringAt(25, 35, hour_str, &Font24, EPD_BLACK);
  paith.DrawStringAt(25, 65, minute_str, &Font24, EPD_BLACK);
  EPD_Display(image_temp);
}
void EPDAlarm() {
  //tone(buzzerPin, 500, 500); //pin, frek, duration; til hvis den er passiv
                             //skal måske bare sættes til high pinen hvis den er aktiv
  //Gør noget alarmerende
  Serial.println("FALL FALL FALL FALL");
}
