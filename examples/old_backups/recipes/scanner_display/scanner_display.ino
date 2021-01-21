#include <SPI.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_GFX.h>

#define SCREEN_WIDTH 128 // display width in pixels
#define SCREEN_HEIGHT 64 // display height in pixels

#include <Adafruit_SSD1306.h>
#define WHITE SSD1306_WHITE
#define BLACK SSD1306_BLACK
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

#include <Adafruit_ST7789.h>

#define WHITE ST77XX_WHITE
#define BLACK ST77XX_BLACK
#define TFT_CS        10
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8

Adafruit_ST7739 display(TFT_CS, TFT_DC, TFT_RST);

// instantiate an object for the nRF24L01 transceiver
RF24 radio(2, 3); // using pin 7 for the CE pin, and pin 8 for the CSN pin

const uint8_t num_reps = 100;
const uint8_t num_channels = 126;
uint8_t values[num_channels];

void setup(void) {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!"));
    while (1) {} // hold in infinite loop
  }

  // print example's introductory prompt
  Serial.println(F("RF24/examples/recipes/scanner_TFT"));

  display.init(SCREEN_WIDTH, SCREEN_HEIGHT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    while(1) {} // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
  drawFooter();
}

void loop() {

  // Clear measurement values
  memset(values, 0, sizeof(values));

  // Scan all channels num_reps times
  int rep_counter = num_reps;
  while (rep_counter--) {
    int i = num_channels;
    while (i--) {
      radio.setChannel(i);       // Select this channel
      radio.startListening();    // start an RX session
      delayMicroseconds(128);    // Listen for a little
      if (radio.testCarrier()) { // Did we get a carrier?
          ++values[i];           // note the detected signal
      }
      radio.stopListening();     // reset the RPD flag
    }
  }
  drawValues();
}

/**
 * draw the graph of signals' values
 */
void drawValues() {

  // clear section reserved for the graph
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - 10, BLACK);
  display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - 10, BLACK);

  // draw the graph
  const uint8_t diff = (SCREEN_WIDTH - num_channels) / 2;

  for (uint8_t i = diff; i < diff + num_channels; i++) {
    values[i - diff] = scaleValue(values[i - diff]);
    uint8_t val_diff = (SCREEN_HEIGHT - 9) - values[i - diff];
    display.drawFastVLine(i, val_diff, values[i - diff], WHITE);
  }
  display.display();
}

/**
 * scale the signal value to the screen's height (- footer's height)
 */
uint8_t scaleValue(uint8_t val) {
  return val * (SCREEN_HEIGHT - 9) / 0xF;
}

/**
 * draw the x-axis label for chart
 */
void drawFooter() {

  // draw delimiting lines
  display.drawFastHLine(0, SCREEN_HEIGHT - 9, SCREEN_WIDTH, WHITE);
  display.drawFastVLine(0, SCREEN_HEIGHT - 9, 9, WHITE);
  display.drawFastVLine(SCREEN_WIDTH - 1, SCREEN_HEIGHT - 9, 9, WHITE);
  display.drawFastVLine(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 9, 9, WHITE);

  // draw numbers
  display.drawChar(2, SCREEN_HEIGHT - 8, '0', WHITE, BLACK, 1);
  display.drawChar(SCREEN_WIDTH / 2 + 2, SCREEN_HEIGHT - 8, '6', WHITE, BLACK, 1);
  display.drawChar(SCREEN_WIDTH / 2 + 8, SCREEN_HEIGHT - 8, '3', WHITE, BLACK, 1);
  display.drawChar(SCREEN_WIDTH - 17, SCREEN_HEIGHT - 8, '1', WHITE, BLACK, 1);
  display.drawChar(SCREEN_WIDTH - 12, SCREEN_HEIGHT - 8, '2', WHITE, BLACK, 1);
  display.drawChar(SCREEN_WIDTH - 7, SCREEN_HEIGHT - 8, '5', WHITE, BLACK, 1);
  display.display();
}