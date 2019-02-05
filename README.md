# Netzfrequenz
Einfache Netzfrequenzmessung mit einem Arduino Nano

Messprinzip: 250 x Interrupt auf fallende Flanke, währenddessen Zeit messen und dann die Frequenz errechnen.

Basishardware: 

- https://github.com/awall9999/Arduino-Quick-Project

- Trafo Vpeak 23V an Spannungsteiler 1:10
- Offsetspannung 2.5 V mit Spannungsteiler 1:1, damit die Sinuskurve komplett im positiven Bereich liegt

--> Messsignal 2.5 V +- 2.1 V an PIN 2

