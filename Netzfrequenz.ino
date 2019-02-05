#include <LiquidCrystal.h> // includes the library to control the LCD display

#define BUTTONUP 14   //Button up in on Pin14 of the Arduino Nano
#define BUTTONDOWN 16 //Button down in on Pin16 of the Arduino Nano
#define BUTTONOK 15   //Button OK in on Pin15 of the Arduino Nano
#define BUTTONCE 17   //Button CE in on Pin17 of the Arduino Nano
#define LEDB 12       //green
#define LEDR 13       //red
LiquidCrystal lcd(7, 6, 11, 10, 9, 8); // Defines where the LCD is connected to the Arduino Nano
String lcdtext; //sets variable to string

// this function is to display the variable "lcdtext" to the second line of the display
void LCD() {
  lcd.setCursor(0, 1); //sets cursor to the left of the second line
  lcd.print("" ); lcd.print(lcdtext); lcd.print(" Hz"); // prints this to the Display
}
// end of the LCD function

#define PIN 2

#define MESSZEITRAUM 250 // alle 5 sec.

volatile int impulse = -1;
volatile unsigned long zeit = 0;
volatile unsigned long zeitVorher = 0;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.clear();      //Clear LCD
  lcd.setCursor(0, 0); // Set cursor positon to the upper left
  lcd.print("Netzfrequenz"); // prints the text between the brackets on the Display


  pinMode(BUTTONUP, INPUT);   //defines the pin on the Arduino as input
  pinMode(BUTTONDOWN, INPUT); //defines the pin on the Arduino as input
  pinMode(BUTTONOK, INPUT); //defines the pin on the Arduino as input
  pinMode(BUTTONCE, INPUT); //defines the pin on the Arduino as input
  pinMode(LEDB, OUTPUT); //defines the pin on the Arduino as output
  pinMode(LEDR, OUTPUT); //defines the pin on the Arduino as output

  Serial.println("Start...");
  attachInterrupt(digitalPinToInterrupt(PIN), messung, FALLING);
}

void loop()
{
  if (impulse == MESSZEITRAUM)
  {
    detachInterrupt(digitalPinToInterrupt(PIN));
    //Empirische Messzeitraumkorrektur: +0.125 für 5 sec.
    double frequenz = 1e6 / ((zeit - zeitVorher) / ((double)MESSZEITRAUM + 0.125));
    digitalWrite(LEDB, HIGH);
    Serial.println(frequenz, 4);
    char float_str[10];
    dtostrf(frequenz, 7, 4, float_str);
    lcdtext = float_str; LCD();
    digitalWrite(LEDB, LOW);
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
    lcdtext = "Fehler"; LCD();
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
