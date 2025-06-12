
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
  EPD_Sleep();
}

void updateDisplayTime() {
  EPD_TurnOnDisplay();
  EPD_Init();
  paith.Clear(EPD_WHITE);
  sprintf(hour_str, "%02u", hour); // %02u sikrer to cifre med foranstillet nul
  sprintf(minute_str, "%02u", minute); // %02u sikrer to cifre med foranstillet nul
  paith.DrawStringAt(25, 35, hour_str, &Font24, EPD_BLACK);
  paith.DrawStringAt(25, 65, minute_str, &Font24, EPD_BLACK);
  EPD_Display(image_temp);
  EPD_Sleep();
}
void EPDAlarm() {
  //tone(buzzerPin, 500, 500); //pin, frek, duration; til hvis den er passiv
                             //skal måske bare sættes til high pinen hvis den er aktiv
  //Gør noget alarmerende
  EPD_TurnOnDisplay();
  EPD_Init();
  paith.Clear(EPD_BLACK);
  paith.DrawStringAt(10,0, "FALL", &Font24, EPD_WHITE);//Hvis der komme flere linjer med end denne dør fald algorithmen :(
  paith.DrawStringAt(0,30, "Dete", &Font24, EPD_WHITE);
  paith.DrawStringAt(0,60, "cted", &Font24, EPD_WHITE);
  EPD_Display(image_temp);
  Serial.println("FALL FALL FALL FALL");
  EPD_Sleep();
}
