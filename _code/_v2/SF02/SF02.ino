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

//----------------------------------------------------------------------------
//                            INITIALISATION
//----------------------------------------------------------------------------
#define EN 21
#define MS1 7
#define MS2 6
#define MS3 5
#define STEP 3
#define DIR 2
#define MAXSPEED 1000 //Set maximum speed
#define ACCEL 450 //Set acceleration
#define DEFAULT_STP 135
AccelStepper stepper(AccelStepper::DRIVER, STEP, DIR);
int count = 0; //initialisation du nombre de flaps passé sur le programme


//----------------------------------------------------------------------------
//                           Variable à modifier
//----------------------------------------------------------------------------
int n_flaps_pass = 13;
int pause_standard = 989;
int stp = DEFAULT_STP;  //nb de steps/flap
int init_flaps = 30; //nb de flaps passés lors du démarrage
int pause_after = 10; //pass plus de flaps après n flaps
int timing = pause_standard; //temps entre deux flaps → l.65-69


//----------------------------------------------------------------------------
//                                SETUP INIT
//----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  pinMode(EN, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);

  digitalWrite(EN, 0);  //0=ON ; 1=OFF
  digitalWrite(MS1, 1);
  digitalWrite(MS2, 1);
  digitalWrite(MS3, 0);

  stepper.setMaxSpeed(MAXSPEED);
  stepper.setAcceleration(ACCEL);
  stepper.setCurrentPosition(0);
  stepper.moveTo(DEFAULT_STP);
  stepper.setCurrentPosition(0);

  delay(1000); // Pause au démarrage
  if (stepper.distanceToGo() == 0) {
    stepper.moveTo(stepper.currentPosition() + DEFAULT_STP * init_flaps);  //To get flaps aligned
  }
  stepper.run();
  delay(2000); // Pause après l'initialisation
}


//----------------------------------------------------------------------------
//                                PINCIPAL LOOP
//----------------------------------------------------------------------------
void loop() {
  //--------------------------------------
  //Randomly add more steps to skip flaps
  //--------------------------------------
  if (count == pause_after) {
    stp = DEFAULT_STP*n_flaps_pass;
  } else {
    stp=DEFAULT_STP;
  }

  //--------------
  //  MAIN LOOP
  //  DO NOT MODIFY
  //  AFTER THIS LINE
  //--------------
  if (stepper.distanceToGo() == 0) {
    stepper.moveTo(stepper.currentPosition() + stp);
    delay(timing);
    count++;
  }
  if (count == pause_after+1) {
    count = 0;
  }
  stepper.run();
}
