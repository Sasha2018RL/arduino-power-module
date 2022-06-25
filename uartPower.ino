#define BATT 11
#define B16_CHARGE 9
#define B12_CHARGE 8
#define SIREN 10
#define ETHERNET 4
#define DISK 3
#define B12_MINUS 5
#define B12_PLUS 6
#define RELAY_RESERV 7

#define ON 0
#define OFF 1

#include "GyverHacks.h"
#include <GyverOS.h>

GyverOS<6> OS;

#include <EEPROM.h>
#define FULL_CHARGE_TIME 32
#define CHARGE_TIME 24

#define B12_ADDITIONAL_CHARGING_TIME 30 // в секундах, нужно переделать в часы дополнительное время дозарядки аккумулятора 12 в
#define B16_ADDITIONAL_CHARGING_TIME 30 // в секундах, нужно переделать в часы дополнительное время дозарядки аккумулятора 12 в
#define DISK_TIMEOUT 5000L
#define ETHERNET_TIMEOUT 10000L

bool was_line = false;
unsigned long charge_timer = 0L;
unsigned long stat_timer = 0L;
unsigned long siren_timer = 0L;
unsigned long ethernet_timer = 900000L;
unsigned long disk_timer = 900000L;
unsigned long whenStop12VCharging = 0L;
unsigned long whenStop16VCharging = 0L;
unsigned long prev12VCharging = 0L;
unsigned long prev16VCharging = 0L;
unsigned long line_disable_timer = 0L;
int line_disable_count = 0;

#define LINE_REBOOT_TIMEOUT 30000

bool is12vCharging = false;
void chargeB12()
{
  if (!hasLine())
  {
    digitalWrite(B12_CHARGE, OFF);
    return;
  }

  if (digitalRead(B12_CHARGE) == OFF)
  {
    digitalWrite(B12_MINUS, ON);
    delay(2000);
  }

  Serial.print(isB12NeedsCharge());
  Serial.print(" ");
  Serial.print(hasLine());
  Serial.print(" ");
  Serial.print(millis());
  Serial.print(" ");
  Serial.print(millis() - prev12VCharging);
  Serial.println(" ");

  if (isB12NeedsCharge() && hasLine() && millis() - prev12VCharging > 30000)
  {
    digitalWrite(B12_MINUS, OFF);
    digitalWrite(B12_PLUS, OFF);
    digitalWrite(B12_CHARGE, ON);
    Serial.println('started 12v charging');

    is12vCharging = true;
    whenStop12VCharging = millis() + B12_ADDITIONAL_CHARGING_TIME * 1000; //todo *3600
  }
}

void checkB12VChargeState()
{
  if (!hasLine())
  {
    digitalWrite(B12_CHARGE, OFF);
    return;
  }

  if (digitalRead(B12_CHARGE) == OFF)
  {
    Serial.println("b12 is npt charging now, in check b12 charge state");
    return;
  }

  digitalWrite(B12_CHARGE, OFF);
  delay(1000);
  digitalWrite(B12_MINUS, ON);
  delay(10000);

  if (isB12Full())
  {
    Serial.println("b12 doesnt need charge, in check b12 charge state");
    digitalWrite(B12_CHARGE, OFF);
    delay(900);
    if (is12vCharging)
    {
      Serial.println('finished 12v charging');
      prev12VCharging = millis();
    }

    is12vCharging = false;
    digitalWrite(B12_MINUS, OFF);
  }
  else
  {
    digitalWrite(B12_MINUS, OFF);
    delay(500);
    digitalWrite(B12_CHARGE, ON);
  }
}

// digitalWrite(B16_CHARGE, OFF);
//     if (is16vCharging)
//     {
//       Serial.println('finished 16v charging');
//       prev16VCharging = millis();
//     }

//     is16vCharging = false;

bool is16vCharging = false;

void checkB16ChargeState()
{
  if (!hasLine())
  {
    digitalWrite(B16_CHARGE, OFF);
    return;
  }

  if (digitalRead(B16_CHARGE) == OFF)
  {
    Serial.println("b16 is not charging now, in check b16 charge state");
    return;
  }

  digitalWrite(B16_CHARGE, OFF);
  delay(10000);

  if (isB16Full())
  {
    Serial.println("b16 doesnt need charge, in check b16 charge state");

    if (digitalRead(B16_CHARGE) == ON)
    {
      Serial.println('finished 16v charging');
      prev16VCharging = millis();
    }
    digitalWrite(B16_CHARGE, OFF);

    is16vCharging = false;
  }
  else
  {
    digitalWrite(B16_CHARGE, ON);
  }
}
void chargeB16()
{
  if (!hasLine())
  {
    digitalWrite(B16_CHARGE, OFF);
    return;
  }

  if (isB16NeedsCharge() && hasLine() && millis() - prev16VCharging > 30000)
  {
    digitalWrite(B16_CHARGE, ON);
    if (!is16vCharging)
    {
      Serial.println('started 16v charging');
    }

    is16vCharging = true;
    prev16VCharging = millis();
  }
}

bool registered_all_critical = false;
void controlBatteryLevel()
{
  if (!hasLine())
  {
    digitalWrite(B12_MINUS, ON);
    if (isB12Available())
    {
      Serial.println("b12available");
      changeBattery(true);
      registered_all_critical = false;
    }
    else if (isB16Available())
    {
      Serial.println("b16available");
      changeBattery(false);
      registered_all_critical = false;
    }
    else
    {
      if (registered_all_critical)
      {
        Serial.println("Critical batteries!");
        changeBattery(false);
        digitalWrite(BATT, OFF);
        Serial.println('If you see this - power is still here!');
      }
      else
      {
        Serial.println("Registered critical batteries once");
        registered_all_critical = true;
      }
    }
  }
  else
  {
    changeBattery(false);
  }
}

void reportState()
{
  Serial.print("STAT:U24=");
  Serial.print(get24V());
  Serial.print(":U16=");
  Serial.print(getB16Voltage());
  Serial.print(":U12=");
  Serial.print(getB12Voltage());
  Serial.print(":B16anal=");
  Serial.print(analogRead(A4));
  Serial.print(":B12anal=");
  Serial.print(analogRead(A5));
  Serial.print(":VCC=");
  Serial.print(getVCC());
  Serial.print(":c12v=");
  Serial.print(is12vCharging);
  Serial.print(":c16v=");
  Serial.print(is16vCharging);
  Serial.println(";");
}

//bool current_battery = false;
void changeBattery(bool battery)
{ // false - 16v, true - 12v
  if (!hasLine())
  {
    if (battery)
    {
      digitalWrite(B12_PLUS, ON);
      digitalWrite(B12_MINUS, ON);
    }
    else
    {
      digitalWrite(B12_PLUS, OFF);
      digitalWrite(B12_MINUS, ON);
      digitalWrite(BATT, ON);
    }
  }
  else
  {
    digitalWrite(B12_PLUS, OFF);

    digitalWrite(BATT, ON);
  }
}

void setup()
{
  pinMode(BATT, OUTPUT);
  pinMode(B16_CHARGE, OUTPUT);
  pinMode(B12_CHARGE, OUTPUT);
  pinMode(SIREN, OUTPUT);
  pinMode(ETHERNET, OUTPUT);
  pinMode(DISK, OUTPUT);
  pinMode(B12_MINUS, OUTPUT);
  pinMode(B12_PLUS, OUTPUT);
  pinMode(RELAY_RESERV, OUTPUT);

  digitalWrite(BATT, ON);
  digitalWrite(DISK, OFF);
  digitalWrite(ETHERNET, OFF);
  digitalWrite(B16_CHARGE, OFF);
  digitalWrite(B12_CHARGE, OFF);
  digitalWrite(B12_MINUS, OFF);
  digitalWrite(B12_PLUS, OFF);
  digitalWrite(SIREN, 0);
  digitalWrite(RELAY_RESERV, ON);

  Serial.begin(9600);

  was_line = hasLine();

  // подключаем задачи (порядковый номер, имя функции, период в мс)
  OS.attach(0, chargeB12, 60000);
  OS.attach(1, chargeB16, 60000);
  OS.attach(2, controlBatteryLevel, 60000);
  OS.attach(3, reportState, 5000);
  OS.attach(4, checkB12VChargeState, 300000); // must be big interval
  OS.attach(5, checkB16ChargeState, 300000);  // must be big interval
}

void loop()
{
  OS.tick();
  bool has_line = hasLine();
  if (!has_line)
  {
    digitalWrite(B16_CHARGE, OFF);
    //БАТАРЕЯ
    if (was_line)
    {
      if (millis() - line_disable_timer > LINE_REBOOT_TIMEOUT)
      {
        line_disable_timer = millis();
        line_disable_count = 0;
      }
      else
      {
        line_disable_count++;
      }
      if (line_disable_count == 4)
      {

        digitalWrite(SIREN, 1);
        delay(1000);
        digitalWrite(SIREN, 0);

        changeBattery(false);

        digitalWrite(BATT, OFF);
      }
      Serial.println("BATT");
      digitalWrite(SIREN, 1);
      delay(100);
      digitalWrite(SIREN, 0);
    }
  }
  else
  {
    // СЕТЬ
    if (!was_line)
    {
      Serial.println("LINE");
      digitalWrite(SIREN, 1);
      delay(100);
      digitalWrite(SIREN, 0);
      delay(100);
      digitalWrite(SIREN, 1);
      delay(100);
      digitalWrite(SIREN, 0);
    }
  }

  was_line = has_line;

  if (millis() > siren_timer)
  {
    digitalWrite(SIREN, 0);
  }
  else
  {
    digitalWrite(SIREN, 1);
  }

  if (millis() > ethernet_timer)
  {
    Serial.print("et:");
    Serial.print("ethernet_timer: ");
    Serial.println(ethernet_timer);
    Serial.print("millis: ");
    Serial.println(millis());
    digitalWrite(ETHERNET, ON);
    delay(10000);
    digitalWrite(ETHERNET, OFF);
    ethernet_timer = millis() + 900000L;
  }

  if (millis() > disk_timer)
  {
    Serial.print("dt:");
    Serial.println(disk_timer);
    Serial.print("millis:");
    Serial.println(millis());
    digitalWrite(DISK, ON);
    delay(10000);
    digitalWrite(DISK, OFF);
    disk_timer = millis() + 900000L;
  }

  if (has_line)
  {
    digitalWrite(RELAY_RESERV, ON);
  }
  else if (was_line)
  {
    digitalWrite(RELAY_RESERV, OFF);
  }

  if (Serial.available())
  {
    byte input = Serial.read();
    if (input == 49)
    {
      siren_timer = millis() + 50L;
    }
    if (input == 50)
    {
      siren_timer = millis() + 100L;
    }
    if (input == 51)
    {
      siren_timer = millis() + 200L;
    }
    if (input == 52)
    {
      siren_timer = millis() + 500L;
    }
    if (input == 53)
    {
      siren_timer = millis() + 1000L;
    }
    if (input == 54)
    {
      siren_timer = millis() + 5000L;
    }
    if (input == 55)
    {
      siren_timer = millis() + 10000L;
    }
    if (input == 56)
    {
      siren_timer = millis() + 60000L;
    }
    if (input == 57)
    {
      siren_timer = millis() + 300000L;
    }
    if (input == 101)
    { // (e)thernet
      ethernet_timer = millis() + 900000L;
    }
    if (input == 100)
    { // (d)isk
      disk_timer = millis() + 900000L;
    }
    // a - enable out 12v line
    if (input == 97)
    {
      digitalWrite(RELAY_RESERV, ON);
    }
    // b - disable 12v line if powered from battery
    if (input == 98)
    {
      if (!has_line)
      {
        digitalWrite(RELAY_RESERV, OFF);
      }
    }
  }
}
