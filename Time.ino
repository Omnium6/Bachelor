void calculateTime() {
  myTime = millis() + offset;

  unsigned long totalSeconds = myTime / 1000;
  second = totalSeconds % 60;

  unsigned long totalMinutes = totalSeconds / 60;
  minute = totalMinutes % 60;

  unsigned long totalHours = totalMinutes / 60;
  hour = totalHours % 24;
}

void calculateTimeOffset(int init_hour, int init_minute, int init_second) {
  // skal regne tiden om fra tid til millisekunder
  unsigned long current_minute = init_hour*60 + init_minute;

  offset = ((current_minute * 60 + init_second) * 1000) - millis();
}
//Er her bare for at teste til phone time virker
void getCompileTime() { 
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

