/*
FHT library for frequency spectrum analyser http://wiki.openmusiclabs.com/wiki/ArduinoFHT

Using GreyGnome's PinChangeInt to change detect button push and change number of 
bars displayed: https://github.com/GreyGnome/PinChangeInt

Also found some code by the-fuchs on Github to help with the peak hold display. I believe his
code was working with an MSGEQ7 7 band chip, but I was able to use some of his magic in my
sketch.  https://github.com/the-fuchs/AVR-7-BandAudioSpectrum check out main.c and his
drawDouble or drawSingle functions.
*/

// input on A0
// #define LOG_OUT 1 // use the log output function
#define LIN_OUT8 1
#define SCALE 256
#define WINDOW 1
// #define OCT_NORM 1 // 0: no normalisation, more high freq 1: divided by number of bins, less high freq
// #define OCTAVE 1
#define FHT_N 256 // set to 256 point fht

#include "PinChangeInt.h"
#include <FHT.h> // include the library

#include "U8glib.h"
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST); // Fast I2C / TWI

const int oled_rst = 4;

const int noise_level = 0;

#define ARDUINOPIN 8 // PinChangeInt Pin

// Notice that values that get modified inside an interrupt, that I wish to access
// outside the interrupt, are marked "volatile". It tells the compiler not to optimize
// the variable.
volatile uint16_t style = 0; // 1 is 12 bars, 2 is 8 bars, 3 is 6 bars

// Do not use any Serial.print() in interrupt subroutines. Serial.print() uses interrupts,
// and by default interrupts are off in interrupt subroutines. Interrupt routines should also
// be as fast as possible. Here we just increment a counter.
void interruptFunction()
{
  style++;
}

void setup()
{

  pinMode(oled_rst, OUTPUT); // reset OLED display
  digitalWrite(oled_rst, LOW);
  delay(100);
  digitalWrite(oled_rst, HIGH);
  pinMode(ARDUINOPIN, INPUT_PULLUP); // Configure the pin as an input, and turn on the pullup resistor.
                                     // See http://arduino.cc/en/Tutorial/DigitalPins

  attachPinChangeInterrupt(ARDUINOPIN, interruptFunction, FALLING);

  drawIntro();
  delay(1000);

  Serial.begin(9600); // use the serial port
  TIMSK0 = 0;         // turn off timer0 for lower jitter // no for TIMING
  ADCSRA = 0xe5;      // set the adc to free running mode
  ADMUX = 0x40;       // use adc0
  DIDR0 = 0x01;       // turn off the digital input for adc0
}

void loop()
{

  while (1) // reduces jitter
  {

    cli(); // no for TIMING

    //unsigned long start_time = micros(); // yes for TIMING

    for (int i = 0; i < FHT_N; i++)
    { // save 256 samples
      while (!(ADCSRA & 0x10))
        ;            // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200;          // form into a signed int
      k <<= 6;              // form into a 16b signed int
      fht_input[i] = k;     // put real data into bins
    }

    //Serial.println(micros()-start_time); // yes for TIMING

    fht_window();  // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run();     // process the data in the fht
    fht_mag_lin8();
    //fht_mag_log();
    //fht_mag_octave();
    sei(); // no for TIMING

    //Serial.print("Pin was interrupted: ");
    //Serial.print(interruptCount, DEC);      // print the interrupt co

    // everytime we push the button, cycle through the displays
    if (style == 1)
    {
      draw12Bars();
    }
    else if (style == 2)
    {
      draw8Bars();
    }
    else if (style == 3)
    {
      draw6Bars();
    }
    else if (style > 3)
    {
      draw12Bars();
      style = 1;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////

void draw12Bars()
{
  static uint8_t maxPeak[14] = {0};
  uint8_t currentPeak = 0;
  static uint8_t cntSlow = 0;

  u8g.firstPage(); // draw 12 bars
  do
  {
    for (int x = 2; x < 14; x++)
    {

      int currentPeak = max((fht_lin_out8[x] - noise_level) / 4, 0); // scale the height fht_oct_out

      u8g.drawBox((x - 2) * 10, 63 - maxPeak[x], 8, 3); // maxpeak

      u8g.drawBox((x - 2) * 10, 63 - currentPeak, 8, currentPeak + 1);

      //calcultate the maxPeak for each bar
      if (currentPeak >= maxPeak[x])
      {
        maxPeak[x] = currentPeak;
      }
      else
      {
        if (cntSlow % 8 == 0)
        {
          (maxPeak[x]--);
        }
      }
    }
  } while (u8g.nextPage());
  cntSlow++;
}

///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////

void draw8Bars()
{
  static uint8_t maxPeak[10] = {0};
  uint8_t currentPeak = 0;
  static uint8_t cntSlow = 0;

  u8g.firstPage(); // draw 8 bars
  do
  {
    for (int x = 2; x < 10; x++)
    {

      int currentPeak = max((fht_lin_out8[x] - noise_level) / 4, 0); // scale the height fht_oct_out

      u8g.drawBox((x - 2) * 16, 63 - maxPeak[x], 12, 3); // maxpeak

      u8g.drawBox((x - 2) * 16, 63 - currentPeak, 12, currentPeak + 1);

      //calcultate the maxPeak for each bar
      if (currentPeak >= maxPeak[x])
      {
        maxPeak[x] = currentPeak;
      }
      else
      {
        if (cntSlow % 8 == 0)
        {
          (maxPeak[x]--);
        }
      }
    }
  } while (u8g.nextPage());
  cntSlow++;
}

///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////

void draw6Bars()
{
  static uint8_t maxPeak[8] = {0};
  uint8_t currentPeak = 0;
  static uint8_t cntSlow = 0;

  u8g.firstPage(); // draw 6 bars
  do
  {
    for (int x = 2; x < 8; x++)
    {

      int currentPeak = max((fht_lin_out8[x] - noise_level) / 4, 0); // scale the height fht_oct_out

      u8g.drawBox((x - 2) * 20, 63 - maxPeak[x], 16, 3); // maxpeak

      u8g.drawBox((x - 2) * 20, 63 - currentPeak, 16, currentPeak + 1);

      //calcultate the maxPeak for each bar
      if (currentPeak >= maxPeak[x])
      {
        maxPeak[x] = currentPeak;
      }
      else
      {
        if (cntSlow % 8 == 0)
        {
          (maxPeak[x]--);
        }
      }
    }
  } while (u8g.nextPage());
  cntSlow++;
}

void drawIntro()
{
  u8g.firstPage(); // draw intro
  do
  {
    u8g.setFont(u8g_font_courB12);
    u8g.setPrintPos(10, 12);
    u8g.print("Spectrum");
    u8g.setPrintPos(30, 27);
    u8g.print("Analyzer");
    u8g.setFont(u8g_font_profont10);
    u8g.setPrintPos(2, 40);
    u8g.print("Press button to cycle");
    u8g.setPrintPos(2, 47);
    u8g.print("through spectrum modes.");
    u8g.setPrintPos(10, 60); 
    u8g.setFont(u8g_font_tpss);
    u8g.print("github.com/uxking");
  } while (u8g.nextPage());
}
