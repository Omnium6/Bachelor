import serial
import csv
import time

# --- Konfiguration ---
SERIAL_PORT = 'COM5'  # Sørg for at dette matcher porten for din Arduino
BAUD_RATE = 9600      # Sørg for at dette matcher din Arduino's Serial.begin() hastighed
OUTPUT_FILENAME = 'flex.csv'
# ---

def log_arduino_data():
    """
    Forbinder til Arduino, læser seriel data og gemmer i en CSV-fil.
    """
    try:
        # Initialiserer seriel forbindelse
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"Forbundet til Arduino på {SERIAL_PORT} med baudrate {BAUD_RATE}")
        time.sleep(2) # Giv Arduino tid til at initialisere efter seriel forbindelse

        # Åbner CSV-filen i append-tilstand ('a')
        # Hvis filen ikke findes, oprettes den.
        with open(OUTPUT_FILENAME, 'a', newline='') as csvfile:
            csv_writer = csv.writer(csvfile)

            print(f"Data vil blive gemt i '{OUTPUT_FILENAME}'")
            print("Tryk Ctrl+C for at stoppe datalogging.")

            while True:
                if ser.in_waiting > 0:
                    # Læs en linje fra den serielle port
                    line = ser.readline().decode('utf-8').strip()

                    if line: # Sørg for at linjen ikke er tom
                        print(f"Modtaget: {line}")
                        # Del linjen op i datafelter.
                        # Dette antager at dine data er adskilt af f.eks. kommaer.
                        # Hvis din Arduino sender én værdi pr. linje, vil 'data' være en liste med ét element.
                        data = line.split(',') # Tilpas separator hvis din Arduino bruger en anden

                        # Skriv data til CSV-filen
                        csv_writer.writerow(data)

    except serial.SerialException as e:
        print(f"Fejl ved forbindelse til seriel port: {e}")
        print(f"Tjek om {SERIAL_PORT} er korrekt og ikke er i brug af et andet program.")
    except KeyboardInterrupt:
        print("\nDatalogging stoppet af bruger.")
    except Exception as e:
        print(f"Der opstod en uventet fejl: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Seriel forbindelse lukket.")

if __name__ == "__main__":
    log_arduino_data()