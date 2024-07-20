# Modul Interaktive Audiosysteme

```bash
# build (after editing the .pro file):
qmake
make

# build (after only editing the code):
make

# run
./ias

# quit application (on Mac): Ctrl + C
```

## 1. Qt Grundlagen
### Basics: Konsolenausgabe & Input
[Tutorial](https://youtu.be/smQms-2yJYc?si=abzgEe0WbT2DSZJf)
- \<QTextStream\> und \<QDebug\> 
- siehe Klasse "BasicLogging.cpp"

### Signals & Slots
[Tutorial](https://www.youtube.com/watch?v=KugPAznC4Yo&list=PLUbFnGajtZlXbrbdlraCe3LMC_YH5abao&index=6)
- Basically Observer Pattern: You can connect signals to slots so that slots (functions) are called when the signal is emitted.
- s. Klassen "Observer" und "Reporter"


## 2. Aufgabenteil
### A. UDP-Transceiver (Transmitter/Receiver)
*C++ mit Qt-Library*
Im Sekundentakt sollen Daten via UDP-Protokoll an einen entfernten Server gesendet werden, welcher als Reflektor fungiert - heißt er schickt die Daten sofort wieder zurück.
Pro Sekunde soll eine Zahl gesendet werden - von 1 bis 10 und dann wieder von vorne.
Der Rückerhalt einer Zahl vom Reflektor soll mit der Konsolenausgabe "Wert \<x\> empfangen" quittiert werden.
IP-Adresse und Port des Reflektors werden als User-Input entgegen genommen.
