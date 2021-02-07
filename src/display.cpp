#include "display.h"

Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);


void setupDisplay()
{
    Serial.println("OLED FeatherWing test");
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);

    Serial.println("OLED begun");

    // Show image buffer on the display hardware.
    // Since the buffer is intialized with an Adafruit splashscreen
    // internally, this will display the splashscreen.
    display.display();
    delay(1000);

    // Clear the buffer.
    display.clearDisplay();
    display.display();

    // text display tests
    display.setTextSize(4);
    display.setTextWrap(false);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.display();
}
