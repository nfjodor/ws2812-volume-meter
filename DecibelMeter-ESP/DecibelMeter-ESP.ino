const int sensitivity = 50;

const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

// pins
#define MicPin A0 // used with analogRead mode only
#define LedPin D2

// LED STRIP
#define greenPrecent 50
#define yellowPercent 83
#include <Adafruit_NeoPixel.h>
#define LEDCOUNT 6
#define LEDBRIGHTNESS 100 // 0-255
int ledArray[LEDCOUNT][3];
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDCOUNT, LedPin, NEO_GRB + NEO_KHZ800);

void setup()
{
    // serial
    Serial.begin(9600);
    while (!Serial)
        ; // Wait untilSerial is ready - Leonardo
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
        }
        else if (currentLedPercent <= yellowPercent) {
            ledArray[i][0] = LEDBRIGHTNESS;
            ledArray[i][1] = LEDBRIGHTNESS;
            ledArray[i][2] = 0;
        }
        else {
            ledArray[i][0] = LEDBRIGHTNESS;
            ledArray[i][1] = 0;
            ledArray[i][2] = 0;
        }
    }
}

void loop()
{
    unsigned long startMillis = millis(); // Start of sample window
    unsigned int peakToPeak = 0; // peak-to-peak level

    unsigned int signalMax = 0;
    unsigned int signalMin = 1024;

    while (millis() - startMillis < sampleWindow) {
        sample = analogRead(0);
        if (sample < 1024) // toss out spurious readings
        {
            if (sample > signalMax) {
                signalMax = sample; // save just the max levels
            }
            else if (sample < signalMin) {
                signalMin = sample; // save just the min levels
            }
        }
    }
    peakToPeak = signalMax - signalMin;

    // map 1v p-p level to the max scale of the display
    int displayPeak = map(peakToPeak, 0, (10.23 * (100 - sensitivity)), 0, LEDCOUNT);
    Serial.println(displayPeak);

    for (int i = 0; i < LEDCOUNT; i++) // update ledstrip
    {
        if (displayPeak <= i) {
            strip.setPixelColor(i, 0, 0, 0);
        }
        else {
            strip.setPixelColor(i, ledArray[i][0], ledArray[i][1], ledArray[i][2]);
        }
    }
    strip.show();
    delay(10);
    if (displayPeak >= LEDCOUNT) {
        delay(1000);
    }
}
