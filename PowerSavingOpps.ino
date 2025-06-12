// Deaktiver ubrugte enheder for at spare strøm
  // Eksempel: Sluk for USB CDC for at spare strøm, hvis du ikke bruger seriel kommunikation til debug.
  // Dette er ofte et af de største strømforbrug i 'idle' tilstand.
  // NRF_UARTE0->TASKS_STOPRX = 1;
  // NRF_UARTE0->TASKS_STOPTX = 1;
  // NRF_UARTE0->ENABLE = 0;
  //
  // Husk at sæt EPD i sleep / off 

/* TIl EPD 
  Du fpr et gær til at gætte hvad de gør
  EPD_Sleep();
  EPD_TurnOnDisplay();
*/

/*
Slukker LED på boardet
pinMode(LED_PWR, OUTPUT);
digitalWrite(LED_PWR, LOW);

Skal bruge Arduino Low Power bib
LowPower.deepSleep(); // Eller LowPower.sleep() for en lettere dvale
*/

void wakeUp_isr1() { // Når knappen trykkes
  // Sæt event flaget fra ISR-kontekst.
  // Det er vigtigt at bruge ISR-sikre funktioner, hvis de findes,
  // men event_flags.set() er generelt sikker.
  event_flags.set(D3_INTERRUPT_FLAG);
}