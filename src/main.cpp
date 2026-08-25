#include <Arduino.h>
#include <EEPROM.h>
#include <ACAN_T4.h>

//Power hold-up system pins
const uint8_t  VS_PIN = 24

//CAN transceiver pins
const uint8_t  CAN_TX_PIN = 30
const uint8_t  CAN_RX_PIN = 31
const uint8_t  SHDN_PIN = 28
const uint8_t  STBY_PIN = 29

//H-Bridge pins
const uint8_t  IN1_A_PIN = 3
const uint8_t  IN1_B_PIN = 36
const uint8_t  IN1_C_PIN = 14
const uint8_t  IN1_D_PIN = 13
const uint8_t  IN2_A_PIN = 15
const uint8_t  IN2_B_PIN = 18
const uint8_t  IN2_C_PIN = 2
const uint8_t  IN2_D_PIN = 37
const uint8_t  INH_A_PIN = 1
const uint8_t  INH_B_PIN = 16
const uint8_t  INH_C_PIN = 0
const uint8_t  INH_D_PIN = 17

//Hall effect sensor pins
const uint8_t  HALL1_A_PIN = 26
const uint8_t  HALL1_B_PIN = 35
const uint8_t  HALL1_C_PIN = 27
const uint8_t  HALL1_D_PIN = 38
const uint8_t  HALL2_A_PIN = 25
const uint8_t  HALL2_B_PIN = 33
const uint8_t  HALL2_C_PIN = 32
const uint8_t  HALL2_D_PIN = 34

//Current sensing pins
const uint8_t  IS1_A_PIN = A9
const uint8_t  IS1_B_PIN = A15
const uint8_t  IS1_C_PIN = A6
const uint8_t  IS1_D_PIN = A5
const uint8_t  IS2_A_PIN = A7
const uint8_t  IS2_B_PIN = A17
const uint8_t  IS2_C_PIN = A8
const uint8_t  IS2_D_PIN = A16

uint8_t n = 10; //Variable for length of EEPROM test message
uint8_t EEPROM_msg [n]; //Buffer to hold EEPROM test message

//State flag for emergency power loss data write
volatile uint8_t ISR_triggered = 0; 

// Function declarations
void power_loss_ISR();
void drive_actuator(int actuator, double current);

void setup() {
  pinMode(VS_PIN, INPUT);

  pinMode(IN1_A_PIN, OUTPUT);
  pinMode(IN1_B_PIN, OUTPUT);
  pinMode(IN1_C_PIN, OUTPUT);
  pinMode(IN1_D_PIN, OUTPUT);
  pinMode(IN2_A_PIN, OUTPUT);
  pinMode(IN2_B_PIN, OUTPUT);
  pinMode(IN2_C_PIN, OUTPUT);
  pinMode(IN2_D_PIN, OUTPUT);
  pinMode(INH_A_PIN, OUTPUT);
  pinMode(INH_B_PIN, OUTPUT);
  pinMode(INH_C_PIN, OUTPUT);
  pinMode(INH_D_PIN, OUTPUT);

  pinMode(HALL1_A_PIN, INPUT);
  pinMode(HALL1_B_PIN, INPUT);
  pinMode(HALL1_C_PIN, INPUT);
  pinMode(HALL1_D_PIN, INPUT);
  pinMode(HALL2_A_PIN, INPUT);
  pinMode(HALL2_B_PIN, INPUT);
  pinMode(HALL2_C_PIN, INPUT);
  pinMode(HALL2_D_PIN, INPUT);

  //Configure interrupt for power loss detection
  attachInterrupt(digitalPinToInterrupt(VS_PIN), power_loss_ISR, FALLING);

  //Set INH pins HIGH to activate H-bridges
  digitalWrite(INH_A_PIN, HIGH);
  digitalWrite(INH_B_PIN, HIGH);
  digitalWrite(INH_C_PIN, HIGH);
  digitalWrite(INH_D_PIN, HIGH);

  //Homing actuators. Full retraction is 0 position
  digitalWrite(IN1_A_PIN, HIGH);
  digitalWrite(IN2_A_PIN, LOW);
  digitalWrite(IN1_B_PIN, HIGH);
  digitalWrite(IN2_B_PIN, LOW);
  digitalWrite(IN1_C_PIN, HIGH);
  digitalWrite(IN2_C_PIN, LOW);
  digitalWrite(IN1_D_PIN, HIGH);
  digitalWrite(IN2_D_PIN, LOW);

  delay(8500); //Full retraction from max extension takes ~7.5 seconds at 2A

  //Turn off all actuators
  digitalWrite(IN1_A_PIN, LOW);
  digitalWrite(IN1_B_PIN, LOW);
  digitalWrite(IN1_B_PIN, LOW);
  digitalWrite(IN1_B_PIN, LOW);

  Serial.begin (9600);
}

void loop() {
  if(ISR_triggered){
    ISR_triggered = false;

    for (int i = 0; i < EEPROM.length(); i++) {
      EEPROM.write(i, 0); //Clear EEPROM by setting all registers to 0
    }

  }
}

// Function definitions
void power_loss_ISR() {
  ISR_triggerd = true ;
}

void drive_actuator(int actuator, double current) {
  return x + y;
}