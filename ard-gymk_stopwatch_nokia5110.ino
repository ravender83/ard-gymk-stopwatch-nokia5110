#include <stoper.h>
#include <LedControl.h>
#include <SimpleList.h>
#include <LCD5110_Graph.h>

/* Stoper */
#define sensorPin 4
#define resetPin  3
#define btnMENU   2
#define maxLiczbaCzasow    12
#define czas_nieczulosci_ms 5000

/* Output */
#define outLedPin 1
#define outBuzzer 6
#define czas_buzzera_ms 1000

/* LedControl */
#define LC_dataPin 11
#define LC_clockPin 13
#define LC_csPin 12
#define LC_numDevices 1

/* Nokia 5110 display */
#define NK_SCLK 9
#define NK_SDIN 8
#define NK_DC 7
#define NK_RES 5
#define NK_SCE 0

#define logoTime 3000

/*
  Stoper(unsigned int inSensorPin, unsigned int inResetPin, unsigned long inDelay_ms);
  * inSensorPin - numer pinu do którego podłączona będzie fotokomórka
  * inResetPin - numer pinu do którego podłączony będzie przycisk resetu
  * inDelay_ms - czas w [ms] poniżej którego nie będzie możliwe zatrzymanie odliczania po przerwaniu fotokomórki
*/
Stoper timer(sensorPin, resetPin, czas_nieczulosci_ms);

/* 
  LedControl(int dataPin, int clkPin, int csPin, int numDevices=1);
  * Params :
  * dataPin   pin on the Arduino where data gets shifted out
  * clockPin    pin for the clock
  * csPin   pin for selecting the device 
  * numDevices  maximum number of devices that can be controlled
*/
LedControl lc=LedControl(LC_dataPin,LC_clockPin,LC_csPin,LC_numDevices);

/* This program requires a Nokia 5110 LCD module.
  
  
    SCLK, SDIN, D/C, RES, SCE
  
*/  
LCD5110 myGLCD(NK_SCLK, NK_SDIN, NK_DC, NK_RES, NK_SCE);

extern uint8_t MediumNumbers[];
extern uint8_t SmallFont[];
extern uint8_t TinyFont[];
extern uint8_t puchar[];
extern uint8_t gymkhana[];

SimpleList<long> listaCzasow;

char bufms[3] = "00";
char bufsek[3] = "00";
char bufmin[3] = "00";
char ledbuf[7] = "000000";
char lcdbuf[9] = "00:00:00";
char buf[9] = "00:00:00";

bool wyswietlWyniki = false;
bool wyswietlonoWyniki = false;

int lcd_tryb=0;
int ekran=0;
long best = 3600000; // 1h
bool best_time = false;

void generujStrBuf(unsigned long tim) {
  sprintf(buf, "%02d:%02d:%02d",(int)((tim / 60000) % 60), (int)((tim / 1000) % 60), (int)((tim / 10) % 100));
}

void generujStrBuffory() {
  sprintf(ledbuf, "%02d%02d%02d", timer.Czas_minut(), timer.Czas_sekund(), timer.Czas_2dig_milsekund());
  sprintf(lcdbuf, "%02d:%02d:%02d", timer.Czas_minut(), timer.Czas_sekund(), timer.Czas_2dig_milsekund());
  sprintf(bufmin, "%02d", timer.Czas_minut());
  sprintf(bufsek, "%02d", timer.Czas_sekund());
  sprintf(bufms, "%02d", timer.Czas_2dig_milsekund());
}

void wyswietlNaLed() {
  int znakLed = 0;
  
  bool dot = false;
  if (best_time==true) {lc.setChar(0,7,'b',false);} else {lc.setChar(0,7,' ',false);}
  for (int i = 0; i <6; i++)
  {
    if ((i == 1) || (i == 3)) dot = true; else dot = false;
    znakLed = ledbuf[i] - '0';
    lc.setDigit(0, 5-i, znakLed, dot);
  }
  lc.shutdown(0, false);
}

void wyswietlCzasy() {
    Serial.println("---------------------");
    int i=1;
  int iOfBest = 0;
    for (SimpleList<long>::iterator itr = listaCzasow.begin(); itr != listaCzasow.end(); ++itr)
     {
       if (*itr < best) {best = *itr; iOfBest++;}
     }
   if (iOfBest==1) {best_time=true;}
   if (best_time==true) Serial.println("=== BEST TIME ==="); 
     Serial.print("B) ");
     generujStrBuf(best);
     Serial.println(buf);
     Serial.println();

     for (SimpleList<long>::iterator itr = listaCzasow.begin(); itr != listaCzasow.end(); ++itr)
      { 
      Serial.print(i);
      Serial.print(") ");
      generujStrBuf(*itr);
      Serial.println(buf);
    
      if (i==maxLiczbaCzasow) {itr = listaCzasow.erase(itr); break;}
      i++;
      }
      Serial.println();
}

void wyswietlNaLCD() {
  int x=1;
  int y=16;
  int i=1;
  if (ekran!=lcd_tryb) {
    myGLCD.clrScr();
    lcd_tryb=ekran;
  }
  
  switch (lcd_tryb) {
    // Logo
    case 0:
    myGLCD.drawBitmap(0, 0, gymkhana, 84, 48);
    myGLCD.update();
    delay(logoTime);
    ekran=1;
    break;
    
    // Stoper
    case 1:
    myGLCD.setFont(SmallFont);
    myGLCD.drawBitmap(1,0, puchar,8,8);
    generujStrBuf(best);
    myGLCD.print(buf, 19, 1);
    myGLCD.drawLine(0,9,83,9);
      
    myGLCD.setFont(MediumNumbers);
    myGLCD.print(bufmin, x, y);
    myGLCD.print(bufsek, x+28, y);
    myGLCD.print(bufms, x+28+28, y);
    myGLCD.drawRect(x+25,y+5, x+26 ,y+6);
    myGLCD.drawRect(x+25,y+10, x+26 ,y+11);
    myGLCD.drawRect(x+53,y+5, x+54 ,y+6);
    myGLCD.drawRect(x+53,y+10, x+54 ,y+11);
    myGLCD.setFont(SmallFont);
    myGLCD.print("            ", CENTER, 38);
    if (best_time==true) myGLCD.print("=== BEST ===", CENTER, 38); else
    { 
      if ((timer.Finish()== LOW) && (timer.Working()==LOW)) myGLCD.print("READY", CENTER, 38); 
      if ((timer.Finish()== LOW) && (timer.Working()==HIGH)) myGLCD.print("RUNNING", CENTER, 38);
      if ((timer.Finish()== HIGH) && (timer.Working()==LOW)) myGLCD.print("FINISH", CENTER, 38);
    }
    myGLCD.update();
    break;
    
    case 2:
    myGLCD.setFont(SmallFont);
    myGLCD.drawBitmap(1,0, puchar,8,8);
    generujStrBuf(best);
    myGLCD.print(buf, 19, 1);
    myGLCD.drawLine(0,9,83,9);
    for (SimpleList<long>::iterator itr = listaCzasow.begin(); itr != listaCzasow.end(); ++itr)
    {
      myGLCD.printNumI(i,1,3+(9*i));
      myGLCD.print(") ",7,3+(9*i));
      generujStrBuf(*itr);
      myGLCD.print(buf, 19, 3+(9*i));
      i++;
    }
    myGLCD.drawRect(78,11, 82 ,47);
    myGLCD.drawLine(79,12, 79, 24);
    myGLCD.drawLine(80,12, 80, 24);
    myGLCD.drawLine(81,12, 81, 24);
    myGLCD.update();
    break;
    
    case 3:
    myGLCD.setFont(SmallFont);
    myGLCD.drawBitmap(1,0, puchar,8,8);
    generujStrBuf(best);
    myGLCD.print(buf, 19, 1);
    myGLCD.drawLine(0,9,83,9);
    for (SimpleList<long>::iterator itr = listaCzasow.begin(); itr != listaCzasow.end(); ++itr)
    {
      if (i>=5) {
      myGLCD.printNumI(i,1,3+(9*(i-4)));
      myGLCD.print(") ",7,3+(9*(i-4)));
      generujStrBuf(*itr);
      myGLCD.print(buf, 19, 3+(9*(i-4)));
      }
      i++;
    }
    myGLCD.drawRect(78,11, 82 ,47);
    myGLCD.drawLine(79,25, 79, 36);
    myGLCD.drawLine(80,25, 80, 36);
    myGLCD.drawLine(81,25, 81, 36);
    myGLCD.update();
    break;
    
    case 4:
    myGLCD.setFont(SmallFont);
    myGLCD.drawBitmap(1,0, puchar,8,8);
    generujStrBuf(best);
    myGLCD.print(buf, 19, 1);
    myGLCD.drawLine(0,9,83,9);
    for (SimpleList<long>::iterator itr = listaCzasow.begin(); itr != listaCzasow.end(); ++itr)
    {
      if (i>=9) {
        myGLCD.printNumI(i,1,3+(9*(i-8)));
        myGLCD.print(") ",7,3+(9*(i-8)));
        generujStrBuf(*itr);
        myGLCD.print(buf, 19, 3+(9*(i-8)));
      }
      i++;
    }
    myGLCD.drawRect(78,11, 82 ,47);
    myGLCD.drawLine(79,37, 79, 48);
    myGLCD.drawLine(80,37, 80, 48);
    myGLCD.drawLine(81,37, 81, 48);
    myGLCD.update();
    break;
  }
  
}

void setup()
{ 
  pinMode(outLedPin, OUTPUT);
  pinMode(outBuzzer, OUTPUT); 
  pinMode(btnMENU, INPUT_PULLUP); 
  digitalWrite(outLedPin, HIGH);
  digitalWrite(outBuzzer, HIGH);
  
  Serial.begin(9600);
  Serial.println("Gymkhana Stoper Ready");
 
  /* Wyświetlacz LED po SPI */
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);  
  lc.clearDisplay(0);
  
  myGLCD.InitLCD();
  
  listaCzasow.clear();

}

void loop()
{
  timer.Update();
  
  generujStrBuffory();
  if ((timer.Finish()== LOW) && (timer.Working()==HIGH)) {ekran=1; best_time=false;}
  if ((timer.Working()==LOW) && (digitalRead(btnMENU) == LOW)) {ekran++; if (ekran>=5) ekran=1; delay(500);}
  
  if ((timer.Finish() == HIGH) && (wyswietlonoWyniki == LOW)) {wyswietlWyniki = HIGH;}
    
  if (wyswietlWyniki == HIGH) {
    listaCzasow.push_front(timer.Czas());
    wyswietlCzasy();
    wyswietlonoWyniki = HIGH;
    wyswietlWyniki= LOW;
  }
    
  if ((timer.Finish()== LOW) && (timer.Working()==LOW)) {wyswietlonoWyniki=LOW; wyswietlWyniki= LOW; best_time=false;}
  
  /* Załączenie wyswietlacza LED - po zatrzymaniu liczenia lub po wcisnieciu przycisku reset */
  if ((timer.Finish() == HIGH)||(digitalRead(resetPin)==LOW)) {lc.shutdown(0, false); wyswietlNaLed();} else {lc.shutdown(0, true);}
    
  // Zapalenie zielonej diody  
  if ((timer.Finish()== LOW) && (timer.Working()==LOW)) digitalWrite(outLedPin, LOW); else digitalWrite(outLedPin, HIGH);
  
  if ((timer.Working()==HIGH) && (timer.Czas()<czas_buzzera_ms)) digitalWrite(outBuzzer, LOW); else digitalWrite(outBuzzer, HIGH);
   
  wyswietlNaLCD();
  delay(2);
}

