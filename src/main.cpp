#include <Arduino.h>
#include <EEPROM.h>
#include <ACAN_T4.h>

//Power hold-up system pins
const uint8_t  VS_PIN = 24;

//CAN transceiver pins
const uint8_t  CAN_TX_PIN = 30;
const uint8_t  CAN_RX_PIN = 31;
const uint8_t  SHDN_PIN = 28;
const uint8_t  STBY_PIN = 29;

//H-Bridge pins
const uint8_t  IN1_A_PIN = 3;
const uint8_t  IN1_B_PIN = 36;
const uint8_t  IN1_C_PIN = 14;
const uint8_t  IN1_D_PIN = 13;
const uint8_t  IN2_A_PIN = 15;
const uint8_t  IN2_B_PIN = 18;
const uint8_t  IN2_C_PIN = 2;
const uint8_t  IN2_D_PIN = 37;
const uint8_t  INH_A_PIN = 1;
const uint8_t  INH_B_PIN = 16;
const uint8_t  INH_C_PIN = 0;
const uint8_t  INH_D_PIN = 17;

//Hall effect sensor pins
const uint8_t  HALL1_A_PIN = 26;
const uint8_t  HALL1_B_PIN = 35;
const uint8_t  HALL1_C_PIN = 27;
const uint8_t  HALL1_D_PIN = 38;
const uint8_t  HALL2_A_PIN = 25;
const uint8_t  HALL2_B_PIN = 33;
const uint8_t  HALL2_C_PIN = 32;
const uint8_t  HALL2_D_PIN = 34;

//Current sensing pins
const uint8_t  IS1_A_PIN = A9;
const uint8_t  IS1_B_PIN = A15;
const uint8_t  IS1_C_PIN = A6;
const uint8_t  IS1_D_PIN = A5;
const uint8_t  IS2_A_PIN = A7;
const uint8_t  IS2_B_PIN = A17;
const uint8_t  IS2_C_PIN = A8;
const uint8_t  IS2_D_PIN = A16;

const int n = 10; //Variable for length of EEPROM test message
uint8_t EEPROM_msg[n]; //Buffer to hold EEPROM test message

//State flag for emergency power loss data write
volatile bool ISR_triggered = false; 

uint16_t HALL1_A_count;
uint16_t HALL1_B_count;
uint16_t HALL1_C_count;
uint16_t HALL1_D_count;
uint16_t HALL2_A_count;
uint16_t HALL2_B_count;
uint16_t HALL2_C_count;
uint16_t HALL2_D_count;

//Char to indicate which direction actuator is noving for single-signal HALL
// 'R' for retract, 'E' for extend
char actuator_A_dir;
char actuator_B_dir;
char actuator_C_dir;
char actuator_D_dir;

char actuator_1 = 'A';
char actuator_2 = 'B';
char actuator_3 = 'C';
char actuator_4 = 'D';

// Function declarations
void power_loss_ISR();
void extend_actuator(char actuator);
void retract_actuator(char actuator);
void stop_actuator (char actuator);
void position_save (uint8_t *buffer);

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
  extend_actuator(actuator_1);
  retract_actuator(actuator_2);
  retract_actuator(actuator_3);
  retract_actuator(actuator_4);
  delay(8500); //Full retraction from max extension takes ~7.5 seconds at 2A

  //Turn off all actuators
  stop_actuator(actuator_1);
  stop_actuator(actuator_2);
  stop_actuator(actuator_3);
  stop_actuator(actuator_4);

  //Set HALL sensor count to 0 after homing
  HALL1_A_count = 0;
  HALL1_B_count = 0;
  HALL1_C_count = 0;
  HALL1_D_count = 0;
  HALL2_A_count = 0;
  HALL2_B_count = 0;
  HALL2_C_count = 0;
  HALL2_D_count = 0;

  Serial.begin (9600);
}

void loop() {
  if(ISR_triggered){
    ISR_triggered = false;

    for (int i = 0; i < EEPROM.length(); i++) {
      EEPROM.write(i, 0); //Clear EEPROM by setting all registers to 0
    }
    Serial.print("Power disconnected");
  }
}

// Function definitions
void power_loss_ISR() {
  ISR_triggered = true;
}

void extend_actuator(char actuator) {
  switch (actuator){
    case 'A':
    digitalWrite(IN1_A_PIN, LOW);
    digitalWrite(IN2_A_PIN, HIGH);
    actuator_A_dir = 'E';
    break;

    case 'B':
    digitalWrite(IN1_B_PIN, LOW);
    digitalWrite(IN2_B_PIN, HIGH);
    actuator_B_dir = 'E';
    break;

    case 'C':
    digitalWrite(IN1_C_PIN, LOW);
    digitalWrite(IN2_C_PIN, HIGH);
    actuator_C_dir = 'E';
    break;

    case 'D' :
    digitalWrite(IN1_D_PIN, LOW);
    digitalWrite(IN2_D_PIN, HIGH);
    actuator_D_dir = 'E';
    break;
  }
}

void retract_actuator(char actuator) {
  switch (actuator){
    case 'A':
    digitalWrite(IN1_A_PIN, HIGH);
    digitalWrite(IN2_A_PIN, LOW);
    actuator_A_dir = 'R';
    break;

    case 'B':
    digitalWrite(IN1_B_PIN, HIGH);
    digitalWrite(IN2_B_PIN, LOW);
    actuator_B_dir = 'R';
    break;

    case 'C':
    digitalWrite(IN1_C_PIN, HIGH);
    digitalWrite(IN2_C_PIN, LOW);
    actuator_C_dir = 'R';
    break;

    case 'D' :
    digitalWrite(IN1_D_PIN, HIGH);
    digitalWrite(IN2_D_PIN, LOW);
    actuator_D_dir = 'R';
    break;
  }
}

void stop_actuator(char actuator){
  switch (actuator){
    case 'A':
    digitalWrite(IN1_A_PIN, LOW);
    digitalWrite(IN2_A_PIN, LOW);
    break;

    case 'B':
    digitalWrite(IN1_B_PIN, LOW);
    digitalWrite(IN2_B_PIN, LOW);
    break;

    case 'C':
    digitalWrite(IN1_C_PIN, LOW);
    digitalWrite(IN2_C_PIN, LOW);
    break;

    case 'D' :
    digitalWrite(IN1_D_PIN, LOW);
    digitalWrite(IN2_D_PIN, LOW);
    break;
  }
}

void read_hall(char actuator) {
  switch (actuator) {
    case 'A' :
    if( digitalRead (HALL2_A_PIN) ) {
      if (actuator_A_dir == 'E') 
        HALL2_A_count++;
      else if (actuator_A_dir == 'R') {
        HALL2_A_count--;
      }
    }
    break;

    case 'B' :
    if( digitalRead (HALL2_B_PIN) ) {
      if (actuator_B_dir == 'E') {
        HALL2_B_count++;
      }
      else if (actuator_B_dir == 'R') {
        HALL2_B_count--;

        if (HALL2_B_count < 0){
          HALL2_B_count = 0;
        }
      }
    }
    break;

    case 'C' :
    if( digitalRead (HALL2_C_PIN) ) {
      if (actuator_C_dir == 'E') {
        HALL2_C_count++;
      }
      else if (actuator_C_dir == 'R') {
        HALL2_C_count--;
        
        if (HALL2_C_count < 0) {
          HALL2_C_count = 0;
        }
      }
    }
    break;

    case 'D' :
    if( digitalRead (HALL2_D_PIN) ) {
      if (actuator_D_dir == 'E') {
        HALL2_D_count++;
      }
      else if (actuator_D_dir == 'R') {
        HALL2_D_count--;

        if (HALL2_D_count < 0){
          HALL2_D_count = 0;
        }
      }
    }
    break;
  }
}

void position_save (uint8_t *buffer) {

}