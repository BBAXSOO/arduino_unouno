#include <DS1302.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <LiquidCrystal_I2C_Hangul.h>


LiquidCrystal_I2C_Hangul lcd(0x27, 16, 2);

// DHT 센서 핀 정의 및 타입 설정
#define DHTPIN 8        // DHT 센서 핀
#define DHTTYPE DHT11   // DHT 센서 타입 (DHT11)
#define RED_LED_PIN 13  // Red LED 핀
#define SOUND_SENSOR_PIN A0 // 사운드 센서 아날로그 핀
#define SOUND_THRESHOLD 500 // 소리를 감지하는 일정 기준 -> 이거 넘어가면 이제 "warning!"" 문구 출력 예정


int buzzerPin = 9;
// DHT 객체 선언
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastPrintTime = 0; // 마지막 출력 시간을 설정
const unsigned long updateInterval = 4000; // 시간 업데이트 간격 설정
unsigned long lastTimeUpdate = 0;  // 시간 업데이트 타이머
const unsigned long timeUpdateInterval = 1000; //1초 단위로 새로운 시간을 받는다.

//RTC 디지털 핀 설정해놓기!
const int CLK = 5;
const int DAT = 6;
const int RST = 7;

DS1302 myrtc(RST, DAT, CLK);

unsigned long intrusionTime = 0;  // 침입 감지 시간
const unsigned long warningDuration = 3000; // warning 문구 출력 시간 설정

//LED
unsigned long lastBlinkTime = 0;      // 마지막 LED 상태 변경 시간
unsigned long blinkInterval = 300;    // LED 켜기/끄기 간격 (300ms)
int blinkCount = 0;                   // LED 깜빡인 횟수
bool ledState = LOW;                  // 현재 LED 상태 (LOW: 꺼짐, HIGH: 켜짐)
bool isBlinking = false;  

// 알람용 부저
unsigned long lastAlarmTime = 0;
const unsigned long alarmInterval = 60000;
const unsigned long alarmDuration = 1000;



void setup() {
    Serial.begin(9600);  // 시리얼 모니터 초기화
    lcd.init();
    lcd.backlight();
    lcd.printHangul(L"안녕하세요", 0, 5); // 한글로 출력할 수 있어서 한글 라이브러리 불러오기
    pinMode(RED_LED_PIN, OUTPUT);

    dht.begin(); 

    myrtc.halt(false);
    myrtc.writeProtect(false);

    //시간만 출력할 예정이니까 날짜는 설정하지 않아도 된다. 
    myrtc.setTime(22, 30, 20); // 시간 설정 (14시 30분 20초)
}




void loop() {
    // 침입자 감지
    int soundValue = analogRead(SOUND_SENSOR_PIN);
    if (soundValue > SOUND_THRESHOLD) {
        lcd.clear();
        Serial.println("침입자 감지!");
        lcd.print("Warning!");
        tone(buzzerPin, 131);
        delay(1000);
        noTone(buzzerPin);
        intrusionTime = millis();
        delay(1000);  // 잠시 대기 후 다시 출력
    }

    if (millis() - intrusionTime < warningDuration) {
        // 경고 중, 다른 작업을 하지 않음
        return;
    }
    if (millis() - lastTimeUpdate >= timeUpdateInterval) {
        lcd.setCursor(0, 0);
        lcd.print(myrtc.getTimeStr());  // 시간 표시 (시:분:초)
        lastTimeUpdate = millis();  // 시간 갱신 후 타이머 리셋
    }

    // 온도 및 습도 값 읽기
    if (millis() - lastPrintTime >= updateInterval) {
        float temperature = dht.readTemperature(); // 섭씨 온도
        float humidity = dht.readHumidity();       // 습도 (%)

        // 읽기 오류 확인
        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            lcd.clear();
            lcd.print("Sensor Error");
            digitalWrite(RED_LED_PIN, LOW); // LED 끄기
            delay(2000); // 잠시 대기 후 오류 메시지 지우기
            lcd.clear();
        } else {
            // LCD 화면 갱신
            lcd.clear();

            // 시간 출력
            lcd.setCursor(0, 0); // 시간은 상단에 출력하기, 나중에 필요에 따라 옮기자!
            lcd.print(myrtc.getTimeStr());  // 시간 표시 (시:분:초)

            // 온도와 습도 출력
            lcd.setCursor(0, 1);  // 하단에 정보를 출력하기
            lcd.print("T:");
            lcd.print(temperature, 1); // 소수점 1자리 까지 출력하기
            lcd.print("C ");
            lcd.print(" H:");
            lcd.print(humidity, 1); // 소수점 1자리 까지 출력하기
            lcd.print("%");

            // 습도가 60% 이하일 때 빨간색 LED 켜기
            if (humidity <= 60) {
                digitalWrite(RED_LED_PIN, HIGH); // LED 켜기
            } else {
                digitalWrite(RED_LED_PIN, LOW); // LED 끄기
            }


            //과습인 경우
            if(humidity > 80 && !isBlinking) {
              isBlinking = true;
              blinkCount = 0;
            }

        if (isBlinking) {
            unsigned long currentTime = millis();
            if (currentTime - lastBlinkTime >= blinkInterval) {
                lastBlinkTime = currentTime; 
                if (ledState == LOW) {
                    ledState = HIGH; 
                } else {
                    ledState = LOW; 
                }
                digitalWrite(RED_LED_PIN, ledState);  

                blinkCount++;  

                // 4번 깜빡였으면 멈추기
                if (blinkCount >= 8) {  
                    isBlinking = false;
                    digitalWrite(RED_LED_PIN, LOW);
                }
            }
        }
            // 시리얼 모니터 출력
            Serial.print("Temperature: ");
            Serial.print(temperature);
            Serial.print(" °C, Humidity: ");
            Serial.print(humidity);
            Serial.println(" %");
        }

        lastPrintTime = millis(); // 마지막 업데이트 시간 갱신
    }
    if(millis() - lastAlarmTime >= alarmInterval) {
      lastAlarmTime = millis();
      triggerBuzzer();
    }
}


void triggerBuzzer() {

  Serial.println("알람 시간 ");
  lcd.clear();
  lcd.print("ALARM!!!");
  delay(1000);
  tone(buzzerPin, 400);
  delay(alarmDuration);
  noTone(buzzerPin);
  lcd.clear();
}
