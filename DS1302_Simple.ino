#include <LiquidCrystal_I2C.h>
#include <DHT.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // LCD 주소


#define DHTPIN 8       
#define DHTTYPE DHT11   

DHT dht(DHTPIN, DHTTYPE);  

bool showTemperature = true;  

unsigned long lastPrintTime = 0; 
const unsigned long updateInterval = 4000; 

void setup() {
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
    lcd.begin(16, 2);
    dht.begin(); 

}

void loop() {

    if (millis() - lastPrintTime >= updateInterval) {
        float temperature = dht.readTemperature(); 
        float humidity = dht.readHumidity();      

      
        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            lcd.clear();
            lcd.print("Sensor Error");
        } else {
           
            lcd.clear();

            if (showTemperature) {
                
                lcd.print("Temperature: ");
                lcd.setCursor(0, 1);
                lcd.print(temperature, 1); 
                lcd.print(" C");
            } else {
                
                lcd.print("Humidity: ");
                lcd.setCursor(0, 1);
                lcd.print(humidity, 1);
                lcd.print(" %");
            }
        }

        lastPrintTime = millis(); 
    }
}
