// pins
#define MicPin A0 // used with analogRead mode only
#define LedPin D2

// consts
#define Sensitivity 92 // 0 - 100
#define AmpMax (1024 / 2)
#define MicSamples (1024*2) // Three of these time-weightings have been internationally standardised, 'S' (1 s) originally called Slow, 'F' (125 ms) originally called Fast and 'I' (35 ms) originally called Impulse.

// modes
#define Use3.3 // use 3.3 voltage. the 5v voltage from usb is not regulated. this is much more stable.
//#define ADCReClock // switch to higher clock, not needed if we are ok with freq between 0 and 4Khz.
//#define ADCFlow // read data from adc with free-run (not interupt). much better data, dc low. hardcoded for A0.

#define VolumeGainFactorBits 0

#define greenPrecent 50
#define yellowPercent 83

// macros
// http://yaab-arduino.blogspot.co.il/2015/02/fast-sampling-from-analog-input.html
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

// LED STRIP
#include <Adafruit_NeoPixel.h>
#define LEDCOUNT 6
#define LEDBRIGHTNESS 100 // 0-255
int ledArray[LEDCOUNT][3];
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDCOUNT, LedPin, NEO_GRB + NEO_KHZ800);


void setup()
{
  //pinMode(MicPin, INPUT); // relevant for digital pins. not relevant for analog. however, don't put into digital OUTPUT mode if going to read analog values.

#ifdef ADCFlow
  // set the adc to free running mode
  // register explanation: http://maxembedded.com/2011/06/the-adc-of-the-avr/
  // 5 => div 32. sample rate 38.4
  // 7 => switch to divider=128, default 9.6khz sampling
  ADCSRA = 0xe0+7; // "ADC Enable", "ADC Start Conversion", "ADC Auto Trigger Enable" and divider.
  ADMUX = 0x0; // use adc0 (hardcoded, doesn't use MicPin). Use ARef pin for analog reference (same as analogReference(EXTERNAL)).
#ifndef Use3.3
  ADMUX |= 0x40; // Use Vcc for analog reference.
#endif
  DIDR0 = 0x01; // turn off the digital input for adc0
#else
#ifdef Use3.3
  // analogReference(EXTERNAL); // 3.3V to AREF
#endif
#endif

#ifdef ADCReClock // change ADC freq divider. default is div 128 9.6khz (bits 111)
  // http://yaab-arduino.blogspot.co.il/2015/02/fast-sampling-from-analog-input.html
  // 1 0 0 = mode 4 = divider 16 = 76.8khz
  //sbi(ADCSRA, ADPS2);
  //cbi(ADCSRA, ADPS1);
  //cbi(ADCSRA, ADPS0);
  // 1 0 1 = mode 5 = divider 32 = 38.4Khz
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);
#endif

  // serial
  Serial.begin(9600);
  while (!Serial); // Wait untilSerial is ready - Leonardo
  Serial.println("Starting mic volume meter");
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // fill led array with colors
  for (int i = 0; i < LEDCOUNT; i++) {
    int currentLedPercent = (i + 1) * 100 / LEDCOUNT;
    if (currentLedPercent <= greenPrecent) {
      ledArray[i][0] = 0;
      ledArray[i][1] = LEDBRIGHTNESS;
      ledArray[i][2] = 0;
    } else if (currentLedPercent <= yellowPercent) {
      ledArray[i][0] = LEDBRIGHTNESS;
      ledArray[i][1] = LEDBRIGHTNESS;
      ledArray[i][2] = 0;
    } else {
      ledArray[i][0] = LEDBRIGHTNESS;
      ledArray[i][1] = 0;
      ledArray[i][2] = 0;
    }
  }
}

void loop()
{
  // what do we want to do?
  MeasureVolume();
}


// calculate volume level of the signal and print to serial and LCD
void MeasureVolume()
{
  long soundVolMax = 0;
  //cli();  // UDRE interrupt slows this way down on arduino1.0
  for (int i = 0; i < MicSamples; i++)
  {
#ifdef ADCFlow
    while (!(ADCSRA & /*0x10*/_BV(ADIF))); // wait for adc to be ready (ADIF)
    sbi(ADCSRA, ADIF); // restart adc
    byte m = ADCL; // fetch adc data
    byte j = ADCH;
    int k = ((int)j << 8) | m; // form into an int
#else
    int k = analogRead(MicPin);
#endif
    long amp = abs(k - AmpMax);
    amp <<= VolumeGainFactorBits;
    soundVolMax = max(soundVolMax, amp);
  }

  // convert from 0 to 100
  soundVolMax = 100 * soundVolMax / AmpMax;
  Serial.println(soundVolMax);

  int displayPeak = map(soundVolMax, 0, 20, 0, LEDCOUNT);
  
  for (int i = 0; i < LEDCOUNT; i++) // update ledstrip
  {
    if (displayPeak <= i)
    {
      strip.setPixelColor(i, 0, 0, 0);
    } else {
      strip.setPixelColor(i, ledArray[i][0], ledArray[i][1], ledArray[i][2]);
    }
  }
  strip.show();
  if (displayPeak >= LEDCOUNT) {
    delay(1000);
  }
}

