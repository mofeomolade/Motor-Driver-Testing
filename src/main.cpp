#include <Arduino.h>
#include <EEPROM.h>
#include <ACAN_T4.h>

//Power hold-up system pins
const uint8_t  VS_PIN = 24;

//CAN transceiver pins
const uint8_t  CAN_TX_PIN = 31;
const uint8_t  CAN_RX_PIN = 30;
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

volatile int16_t HALL1_A_count;
volatile int16_t HALL1_B_count;
volatile int16_t HALL1_C_count;
volatile int16_t HALL1_D_count;
volatile int16_t HALL2_A_count;
volatile int16_t HALL2_B_count;
volatile int16_t HALL2_C_count;
volatile int16_t HALL2_D_count;

char actuator_A = 'A';
char actuator_B = 'B';
char actuator_C = 'C';
char actuator_D = 'D';

//Char to indicate which direction actuator is noving for single-signal HALL
// 'R' for retract, 'E' for extend, 'S' for stopped
volatile char actuator_A_dir = 'S';
volatile char actuator_B_dir = 'S';
volatile char actuator_C_dir = 'S';
volatile char actuator_D_dir = 'S';

volatile bool power_lost = false;

// Function declarations
void extend_actuator(char actuator);
void retract_actuator(char actuator);
void stop_actuator (char actuator);
void position_save (uint8_t *buffer);
void hall_ISR_A (void);
void hall_ISR_B (void);
void hall_ISR_C (void);
void hall_ISR_D (void);
void power_loss_ISR(void);

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

  pinMode(HALL1_A_PIN, INPUT_PULLUP);
  pinMode(HALL1_B_PIN, INPUT_PULLUP);
  pinMode(HALL1_C_PIN, INPUT_PULLUP);
  pinMode(HALL1_D_PIN, INPUT_PULLUP);
  pinMode(HALL2_A_PIN, INPUT_PULLUP);
  pinMode(HALL2_B_PIN, INPUT_PULLUP);
  pinMode(HALL2_C_PIN, INPUT_PULLUP);
  pinMode(HALL2_D_PIN, INPUT_PULLUP);

  //Digital interrupt to trigger an EEPROM save when power is disconnected
  attachInterrupt(digitalPinToInterrupt(VS_PIN), power_loss_ISR, FALLING);

  attachInterrupt(digitalPinToInterrupt(HALL1_A_PIN), hall_ISR_A, RISING);
  attachInterrupt(digitalPinToInterrupt(HALL1_B_PIN), hall_ISR_B, RISING);
  attachInterrupt(digitalPinToInterrupt(HALL1_C_PIN), hall_ISR_C, RISING);
  attachInterrupt(digitalPinToInterrupt(HALL1_D_PIN), hall_ISR_D, RISING);
  attachInterrupt(digitalPinToInterrupt(HALL2_A_PIN), hall_ISR_A, RISING);
  attachInterrupt(digitalPinToInterrupt(HALL2_B_PIN), hall_ISR_B, RISING);
  attachInterrupt(digitalPinToInterrupt(HALL2_C_PIN), hall_ISR_C, RISING);
  attachInterrupt(digitalPinToInterrupt(HALL2_D_PIN), hall_ISR_D, RISING);

  //Set INH pins HIGH to activate H-bridges
  digitalWrite(INH_A_PIN, HIGH);
  digitalWrite(INH_B_PIN, HIGH);
  digitalWrite(INH_C_PIN, HIGH);
  digitalWrite(INH_D_PIN, HIGH);

  //Homing actuators. Full retraction is 0 position
  retract_actuator(actuator_A);
  retract_actuator(actuator_B);
  retract_actuator(actuator_C);
  retract_actuator(actuator_D);
  delay(10000);

  //Turn off all actuators
  stop_actuator(actuator_A);
  stop_actuator(actuator_B);
  stop_actuator(actuator_C);
  stop_actuator(actuator_D);

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
  extend_actuator(actuator_A);
  extend_actuator(actuator_B);
  extend_actuator(actuator_C);
  extend_actuator(actuator_D);
  delay(4000);
 
  Serial.print("Actuator A Hall Count = ");
  Serial.println(HALL2_A_count);
  Serial.print("Actuator B Hall Count = ");
  Serial.println(HALL2_B_count);
  Serial.print("Actuator C Hall Count = ");
  Serial.println(HALL2_C_count);
  Serial.print("Actuator D Hall Count = ");
  Serial.println(HALL2_D_count);

  Serial.println();

  retract_actuator(actuator_A);
  retract_actuator(actuator_B);
  retract_actuator(actuator_C);
  retract_actuator(actuator_D);
  delay(4000);

  Serial.print("Actuator A Hall Count = ");
  Serial.println(HALL2_A_count);
  Serial.print("Actuator B Hall Count = ");
  Serial.println(HALL2_B_count);
  Serial.print("Actuator C Hall Count = ");
  Serial.println(HALL2_C_count);
  Serial.print("Actuator D Hall Count = ");
  Serial.println(HALL2_D_count);

  Serial.println();
}

// Function definitions
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
    digitalWrite(IN1_C_PIN, HIGH);
    digitalWrite(IN2_C_PIN, LOW);
    actuator_C_dir = 'E';
    break;

    case 'D' :
    digitalWrite(IN1_D_PIN, HIGH);
    digitalWrite(IN2_D_PIN, LOW);
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
    digitalWrite(IN1_C_PIN, LOW);
    digitalWrite(IN2_C_PIN, HIGH);
    actuator_C_dir = 'R';
    break;

    case 'D' :
    digitalWrite(IN1_D_PIN, LOW);
    digitalWrite(IN2_D_PIN, HIGH);
    actuator_D_dir = 'R';
    break;
  }
}

void stop_actuator(char actuator){
  switch (actuator){
    case 'A':
    digitalWrite(IN1_A_PIN, LOW);
    digitalWrite(IN2_A_PIN, LOW);
    actuator_A_dir = 'S';
    break;

    case 'B':
    digitalWrite(IN1_B_PIN, LOW);
    digitalWrite(IN2_B_PIN, LOW);
    actuator_B_dir = 'S';
    break;

    case 'C':
    digitalWrite(IN1_C_PIN, LOW);
    digitalWrite(IN2_C_PIN, LOW);
    actuator_C_dir = 'S';
    break;

    case 'D' :
    digitalWrite(IN1_D_PIN, LOW);
    digitalWrite(IN2_D_PIN, LOW);
    actuator_D_dir = 'S';
    break;
  }
}

void hall_ISR_A (void){
  if (actuator_A_dir == 'E') 
    HALL2_A_count++;
  else if (actuator_A_dir == 'R') {
    if (HALL2_A_count > 0){
      HALL2_A_count--;
    }
  }
}

void hall_ISR_B (void){
  if (actuator_B_dir == 'E') 
    HALL2_B_count++;
  else if (actuator_B_dir == 'R') {
    if (HALL2_B_count > 0){
      HALL2_B_count--;
    }
  }
}
void hall_ISR_C (void){
  if (actuator_C_dir == 'E') 
    HALL2_C_count++;
  else if (actuator_C_dir == 'R') {
    if (HALL2_C_count > 0){
      HALL2_C_count--;
    }
  }
}

void hall_ISR_D (void){
  if (actuator_D_dir == 'E') 
    HALL2_D_count++;
  else if (actuator_D_dir == 'R') {
    if (HALL2_D_count > 0){
      HALL2_D_count--;
    }
  }
}

void power_loss_ISR(void) {
  power_lost = true;
}

void position_save (uint8_t *buffer) {
}