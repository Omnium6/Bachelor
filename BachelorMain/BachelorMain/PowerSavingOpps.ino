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
