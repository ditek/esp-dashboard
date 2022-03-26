#include "display.h"
#include "i2c_mux.h"

Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);


void setupSingleDisplay(){
    Serial.println("OLED FeatherWing test");
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);

    Serial.println("OLED begun");

    // Show image buffer on the display hardware.
    // Since the buffer is intialized with an Adafruit splashscreen
    // internally, this will display the splashscreen.
    display.display();
    delay(500);

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

// Initialize the displays 
void setupDisplay() {
    for (uint8_t i = 0; i < NUM_DISPLAYS; i++) {
        i2c_select(i);   // Loop through each connected displays on the I2C buses  
        setupSingleDisplay();
    }
    i2c_select(0);   // Loop through each connected displays on the I2C buses  
}

void resetDisplay() {
    display.clearDisplay();
    display.setCursor(0, 0);
}

void selectDisplay(uint8_t i){
    if (i < NUM_DISPLAYS){
        i2c_select(i);
    }
}

void displayDataVector(std::vector<dataItem> dataVector, bool withSuffix) {
    for (uint8_t i = 0; i < NUM_DISPLAYS; i++) {
        i2c_select(i);
        resetDisplay();
        if (i >= dataVector.size()) {
            display.display();
            continue;
        }
        auto data = dataVector.at(i);
        display.print(data.value.c_str());
        if (withSuffix) {
            display.setTextSize(3);
            display.print(' ');
            display.print(data.suffix.c_str());
            display.setTextSize(4);
        }
        display.display();
    }
}