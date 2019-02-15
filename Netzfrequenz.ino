#define SDonoff 0
#define LCDonoff 1

#if LCDonoff > 0
#include <LiquidCrystal.h> // includes the library to control the LCD display
#endif

#if SDonoff > 0
#include <SPI.h>
#include <SdFat.h>
#endif

#define BUTTONUP 14   //Button up in on Pin14 of the Arduino Nano
#define BUTTONDOWN 16 //Button down in on Pin16 of the Arduino Nano
#define BUTTONOK 15   //Button OK in on Pin15 of the Arduino Nano
#define BUTTONCE 17   //Button CE in on Pin17 of the Arduino Nano
#define LEDB 12       //green
#define LEDR 13       //red

#if LCDonoff > 0
LiquidCrystal lcd(7, 6, 11, 10, 9, 8); // Defines where the LCD is connected to the Arduino Nano
String lcdtext; //sets variable to string
#endif

// Error messages stored in flash.
#define message(msg) Serial.println(F(msg))

#if SDonoff > 0
// SD chip select pin.  Be sure to disable any other SPI devices such as Enet.
const uint8_t chipSelect = 3;

// SdFat software SPI template
SdFat sd;

// Test file.
SdFile file;

// Log file base name.  Must be six characters or less.
#define FILE_BASE_NAME "Data"

const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
char fileName[13] = FILE_BASE_NAME "00.csv";
#endif

#if LCDonoff > 0
void LCD() {
  lcd.clear();
  lcd.setCursor(0, 0); // Set cursor positon to the upper left
  lcd.print("    Frequenz    ");
  lcd.setCursor(0, 1); //sets cursor to the left of the second line
  lcd.print(""); lcd.print(lcdtext); lcd.print(" Hz  ");
}
#endif

#define PIN 2

#define MESSZEITRAUM 250 // alle 5 sec.

volatile int impulse = -1;
volatile unsigned long zeit = 0;
volatile unsigned long zeitVorher = 0;

void setup() {
#if SDonoff > 0
  pinMode(chipSelect, OUTPUT); //defines the pin on the Arduino as output
#endif
  pinMode(BUTTONUP, INPUT);   //defines the pin on the Arduino as input
  pinMode(BUTTONDOWN, INPUT); //defines the pin on the Arduino as input
  pinMode(BUTTONOK, INPUT); //defines the pin on the Arduino as input
  pinMode(BUTTONCE, INPUT); //defines the pin on the Arduino as input
  pinMode(LEDB, OUTPUT); //defines the pin on the Arduino as output
  pinMode(LEDR, OUTPUT); //defines the pin on the Arduino as output

  Serial.begin(9600);

#if LCDonoff > 0
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0); // Set cursor positon to the upper left
  lcd.print("    Frequenz    "); // prints the text between the brackets on the Display
#endif

#if SDonoff > 0
  message("Start,SD");

  // Initialize at the highest speed supported by the board that is
  // not over 50 MHz. Try a lower speed if SPI errors occur.
  if (!sd.begin(chipSelect, SD_SCK_MHZ(25))) {
    sd.initErrorHalt();
  }

  // Find an unused file name.
  if (BASE_NAME_SIZE > 6) {
    message("Error,FILE_BASE_NAME");
  }
  while (sd.exists(fileName)) {
    if (fileName[BASE_NAME_SIZE + 1] != '9') {
      fileName[BASE_NAME_SIZE + 1]++;
    } else if (fileName[BASE_NAME_SIZE] != '9') {
      fileName[BASE_NAME_SIZE + 1] = '0';
      fileName[BASE_NAME_SIZE]++;
    } else {
      message("Error,FileCreate");
    }
  }

  Serial.print(F("Info,"));
  Serial.println(fileName);

  if (!file.open(fileName, O_WRONLY | O_CREAT | O_EXCL)) {
    message("Error,Open");
  }
  file.close();
#endif

  message("Start,Messung");

  attachInterrupt(digitalPinToInterrupt(PIN), messung, FALLING);
}

void loop()
{
  if (impulse == MESSZEITRAUM)
  {
    detachInterrupt(digitalPinToInterrupt(PIN));
    //Empirische Messzeitraumkorrektur: +0.125 für 5 sec.
    double frequenz = 1e6 / ((zeit - zeitVorher) / ((double)MESSZEITRAUM + 0.125));
    char float_str[10];
    dtostrf(frequenz, 6, 3, float_str);
#if SDonoff > 0
    writeToSD(float_str);
#endif
#if LCDonoff > 0
    lcdtext = float_str; LCD();
#endif
    Serial.print(millis());
    Serial.print(F(","));
    Serial.println(float_str);
    impulse = -1;
    zeit = 0;
    zeitVorher = 0;
    attachInterrupt(digitalPinToInterrupt(PIN), messung, FALLING);
  }
  if (impulse > (MESSZEITRAUM + 10))
  {
    impulse = -1;
    zeit = 0;
    zeitVorher = 0;
#if LCDonoff > 0
    lcdtext = "Fehler"; LCD();
#endif
    message("Fehler,Fehler");
  }
}

void messung()
{
  impulse++;
  if (impulse == 0)
  {
    zeitVorher = micros();
  }
  else if (impulse == MESSZEITRAUM)
  {
    zeit = micros();
  }
}

#if SDonoff > 0
void writeToSD(char float_str[10]) {
  if (!file.open(fileName, O_WRONLY | O_AT_END)) {
    message("Open,Error");
  }
  file.print(millis());
  file.print(',');
  file.println(float_str);
  // Force data to SD and update the directory entry to avoid data loss.
  if (!file.sync() || file.getWriteError()) {
    message("Write,Error");
  }
  file.close();
}
#endif
