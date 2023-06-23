#include <AccelStepper.h>

//****************************************
//**************** A4988 *****************
// MS1    MS2   MS3   Microstep Resolution
//****************************************
// Low  | Low  | Low  |  Full step
// High | Low  | Low  |  Half step
// Low  | High | Low  |	 Quarter step
// High |	High | Low  |  Eighth step
// High |	High | High |	 Sixteenth step
//****************************************

//PCB MAPPING
#define EN 21
#define MS1 7
#define MS2 6
#define MS3 5
#define STEP 3
#define DIR 2
#define MAXSPEED 10000

AccelStepper stepper(AccelStepper::DRIVER, STEP, DIR);

int timing = 1111;
int stp = 34;
int count = 0;

void setup() {
  Serial.begin(115200);

  pinMode(EN, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);

  digitalWrite(EN, 0);  //0=ON ; 1=OFF
  digitalWrite(MS1, 1);
  digitalWrite(MS2, 0);
  digitalWrite(MS2, 0);

  stepper.setMaxSpeed(MAXSPEED);
  stepper.setAcceleration(150);
  stepper.setCurrentPosition(0);
  stepper.moveTo(stp);
  stepper.setCurrentPosition(0);

  // Initialisation when motor is not supply
  delay(1000);
  if (stepper.distanceToGo() == 0) {
    stepper.moveTo(stepper.currentPosition() + stp * 40);  //To get flaps aligned
  }
  stepper.run();
  delay(2000);
}

void loop() {
  //--------------------------------------
  //Randomly add more steps to skip flaps
  //--------------------------------------
  if (count == 13) {
    timing = 5019;
  } else {
    timing = 1111;
  }
  //--------------
  //  Main loop
  //--------------
  if (stepper.distanceToGo() == 0) {
    stepper.moveTo(stepper.currentPosition() + stp);
    delay(timing);
    count++;
  }
  if (count == 14) {
    count = 0;
  }
  stepper.run();
}