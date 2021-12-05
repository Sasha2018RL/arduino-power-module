#define BATT 8
#define CHARGE 9
#define SIREN 10

#define ON 0
#define OFF 1


#include "GyverHacks.h"

#include <EEPROM.h>
#define FULL_CHARGE_TIME 32
#define CHARGE_TIME 24

bool was_line = false;
unsigned long charge_timer = 0L;
unsigned long stat_timer = 0L;
unsigned long siren_timer = 0L;

void setup() {
  pinMode(BATT, OUTPUT);
  pinMode(CHARGE, OUTPUT);
  pinMode(SIREN, OUTPUT);
  digitalWrite(BATT, ON);
  digitalWrite(CHARGE, OFF);

  Serial.begin(9600);

  was_line = getU() > 18;

  if (EEPROM.read(9) == 17) {
    if (EEPROM.read(10) == 1) {
      Serial.println("full charging!");
      charge_timer = FULL_CHARGE_TIME * 3600 * 1000;
      EEPROM.write(10, 0);
    }
  } else {
    Serial.println("First run");
    EEPROM.write(9, 17);
  }
}

void loop() {
  float U = getU();
  if (U < 18) {
    digitalWrite(CHARGE, OFF);
    //БАТАРЕЯ
    if (was_line) {
      Serial.println("BATT");
      digitalWrite(CHARGE, OFF);
      digitalWrite(SIREN, 1);
      delay(100);
      digitalWrite(SIREN, 0);
    }


    if (U < 11) {
      for (int i = 0; i < 10; i++) {
        if (getU() > 11) {
          break;
        }

        if (i == 9) {
          Serial.println("OFF");
          EEPROM.write(10, 1); // сохраняем, что села батарея
          delay(10000); // даем время на передачу
          digitalWrite(BATT, OFF); // режем питание сами себе
        }
        Serial.print("LOW_BATT:");
        Serial.println(i);
        delay(1000);
      }
    }
  } else {
    // СЕТЬ
    if (!was_line ) {
      Serial.println("LINE");
      charge_timer = millis() + (CHARGE_TIME * 3600000L);
      Serial.println(CHARGE_TIME * 3600000L);
      digitalWrite(CHARGE, ON);
      digitalWrite(SIREN, 1);
      delay(100);
      digitalWrite(SIREN, 0);
      delay(100);
      digitalWrite(SIREN, 1);
      delay(100);
      digitalWrite(SIREN, 0);
    }

    if (millis() > charge_timer) {
      charge_timer = 0;
      digitalWrite(CHARGE, OFF);
    }else{
      digitalWrite(CHARGE, ON);
    }
  }

  was_line = U > 18;

  if (millis() > stat_timer) {
    Serial.print("STAT:");
    Serial.print(U);
    Serial.print(":");
    if (charge_timer != 0) {
      Serial.print((charge_timer - millis()) / 1000);
    } else {
      Serial.print(0);
    }
    Serial.println(";");
    stat_timer = 5000 + millis();
  }

  if(millis() > siren_timer){
    digitalWrite(SIREN, 0);
  }else{
    digitalWrite(SIREN, 1);
  }

  if (Serial.available()) {
    byte input = Serial.read();
        if(input == 49){
          siren_timer = millis() + 50L;
        }
        if(input == 50){
          siren_timer = millis() + 100L;
        }
        if(input == 51){
          siren_timer = millis() + 200L;
        }
        if(input == 52){
          siren_timer = millis() + 500L;
        }
        if(input == 53){
          siren_timer = millis() + 1000L;
        }
        if(input == 54){
          siren_timer = millis() + 5000L;
        }
        if(input == 55){
          siren_timer = millis() + 10000L;
        }
        if(input == 56){
          siren_timer = millis() + 60000L;
        }
        if(input == 57){
          siren_timer = millis() + 300000L;
        }
  }
}
