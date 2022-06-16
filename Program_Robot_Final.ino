
//Servo defenitions____________________________________________________________
  #include <Servo.h>      //servo library
  Servo myservo;          // create servo object to control servo
  #define servoPin 4      //servo pin
  #define servoDelay 400  //delay for turning the servo (current best 400)

//Servo degree calibration_____________________________________________________
  //(can be imported from the servo test program)
  #define deg0 5           
  #define deg45 50
  #define deg90 93
  #define deg135 135
  #define deg180 180

//Sensor defenitions_______________________________________________________
  int Echo = A1;          //ultrasonic pins
  int Trig = A0;
  #define scanTime 200    //time to scan distance

  int lightSensor = A5;

//Motor defenitions____________________________________________________________
  #define ENA 5               //motor driver pin map
  #define IN1 6
  #define IN2 7
  #define IN3 12
  #define IN4 11
  #define ENB 13
  #define carSpeed 255        //speed of the motors (0 - 255)
  #define turnTime 210        //time to activate 90 degree turn function (original= 360,class= 300, class2= 210, home= 265)
  #define turnAroundTime 400  //time to activate 180 degree turn function (original= 580, class= 600, class2=400, home= 540)
  #define reverseTime 800     //time to activate "back()" turn function (original is 800)
  #define zigTime 1500        //time to cuntinue before turning around corner (class2=1500)
  #define motorTrim 2         //trim value for driving stright (speed - trim). (home= 7, class= 3, class2= 2)

  class Motor {

    int enablePin;
    int directionPin1;
    int directionPin2;

    public:

      //Method to define the motor pins
      Motor(int ENPin, int dPin1, int dPin2) {
        enablePin = ENPin;
        directionPin1 = dPin1;
        directionPin2 = dPin2;
      };

      //drive the motor 0~255 driving forward | -1~-255 driving backward
    void Drive(int speed) {
      if (speed >= 0) {
         digitalWrite(directionPin1, LOW);
         digitalWrite(directionPin2, HIGH);
      }
      else {
        digitalWrite(directionPin1, HIGH);
        digitalWrite(directionPin2, LOW);
        speed = - speed;
      }
      analogWrite(enablePin, speed);
      }
    };
    Motor rightMotor = Motor(ENA, IN2, IN1);
    Motor leftMotor = Motor(ENB, IN3, IN4);

//Variables____________________________________________________________________
  int rightDistance = 0, leftDistance = 0, middleDistance = 0;  //distance Variables
  int diagonalDistance = 0, controlDistance = 0, maxDist = 420, corner = 0;    //distance Variables and corner count
  int lightLevel = 0, controlLight = 0, lightDeg = 0;           //light sensor Variables
  
  enum Directions { Forward, TurnLeft, TurnRight, TurnAround, Brake, Back}; //custom variable
  Directions goDirection = Forward;                                   //define of goDirection
//=====================================DRIVING FUNCTIONS=================================================================================
void drive() {
  //General motor drive by pre-defined direction
  switch (goDirection) {
    case Forward:
      leftMotor.Drive(carSpeed);
      rightMotor.Drive(carSpeed-motorTrim); 
      Serial.println("Forward");
      break;
    
    case TurnLeft:
      leftMotor.Drive(-carSpeed);
      rightMotor.Drive(carSpeed);
      Serial.println("TurnLeft");
      delay(turnTime);
      break;
    
    case TurnRight:
      leftMotor.Drive(carSpeed);
      rightMotor.Drive(-carSpeed);
      Serial.println("TurnRight");
      delay(turnTime);
      break;
    
    case TurnAround:
      leftMotor.Drive(carSpeed);
      rightMotor.Drive(-carSpeed);
      Serial.println("TurnAround");
      delay(turnAroundTime);
      break;

    case Brake:
      leftMotor.Drive(0);
      rightMotor.Drive(0);
      Serial.println("Stopped");
    
    /*case Back:
      leftMotor.Drive(-carSpeed);
      rightMotor.Drive(-(carSpeed-motorTrim));
      Serial.println("Backward");
      delay(reverseTime);
      break;*/
  }
}
void normal2Light(){
  //turn the car faceing normal to light source
  myservo.write(deg90);                   //strighten the head
  delay(servoDelay);
  //goDirection =TurnLeft;                  //turn to initial scan direction (optional)
  //drive();                 
  controlLight= analogRead(lightSensor);  //take a control light sample
  lightLevel = controlLight;
  delay(1000);
  while (lightLevel<=controlLight)        //turn the car until max light level is reached
  {
    leftMotor.Drive(carSpeed);
    //rightMotor.Drive(-carSpeed);
    delay(50);
    goDirection = Brake;
    drive();
    delay(1000);
    lightLevel= analogRead(lightSensor);
  } 
}
void drive_along_right(){
  //driving along the right side of an obstacle
  Serial.println("Ride along right");
  myservo.write(deg45);                            //turn head to 45 degrees
  delay(servoDelay/2);
  controlDistance = Distance_test();               //take a control distance sample
  delay(scanTime);
  diagonalDistance = controlDistance;
  goDirection = Forward;
  drive();
  while (diagonalDistance <= controlDistance+7){  //drive until distance is open ahead
    controlDistance = Distance_test();
    delay(scanTime);
    if (controlDistance<=10 || diagonalDistance == maxDist){                      //condition for reaching a corner
      goDirection = Brake;
      drive();
      corner_hit();
      corner = 1;                              
    }
    else{
      diagonalDistance = Distance_test();
    }  
  }
  if (corner==0){
    goDirection = Forward;
    drive();
    delay(zigTime);
    goDirection = Brake;
    drive();
    delay(200);
    goDirection = TurnLeft;
    drive();
    goDirection = Brake;
    drive();
  }
  corner = 0;
  myservo.write(deg90);
  delay(servoDelay);
}
void drive_along_left(){
  Serial.println("Ride along left");
  myservo.write(deg135);
  delay(servoDelay/2);
  controlDistance = Distance_test();
  delay(scanTime);
  diagonalDistance = controlDistance;
  goDirection = Forward;
  drive();
  while (diagonalDistance <=controlDistance+7){
    controlDistance = Distance_test();
    delay(scanTime);
    if (controlDistance<=10 || diagonalDistance == maxDist)
    {
      goDirection = Brake;
      drive();
      corner_hit();
      corner = 1;
    }
    else
    {
      diagonalDistance = Distance_test();
    }  
  }
  if (corner==0)
  {
    goDirection = Forward;
    drive();
    delay(zigTime);
    goDirection = Brake;
    drive();
    delay(200);
    goDirection = TurnRight;
    drive();
    goDirection = Brake;
    drive();
  }
  corner = 0;
  myservo.write(deg90);
  delay(servoDelay);
}
int corner_hit(){
  Serial.println("arived to a corner");
  checkDirection();
  if (rightDistance > leftDistance) {    //ride along obstacle (to the left)
    goDirection= TurnRight;
    drive();
    goDirection= Brake;
    drive();
    drive_along_right();
    lightDirection();
      if (lightDeg<45){                     
        goDirection= TurnLeft;
        drive();
        goDirection= Brake;
        drive();
      }
      else if (lightDeg<135 )
      {
        goDirection= Forward;
      }
      else
      {
        goDirection= TurnRight;
        drive();
        goDirection= Brake;
        drive();
      }
  }
  else if (rightDistance < leftDistance) {   //ride along obstacle (to the right)
    goDirection= TurnLeft;
    drive();
    goDirection= Brake;
    drive();
    drive_along_left();
  }
  return 1;
}
int Distance_test() {
  //Ultrasonic distance measurement function
  // annything over 420cm is "out of range"
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  float Fdistance = pulseIn(Echo, HIGH);
  Fdistance = Fdistance / 58;
  
  if ( Fdistance > maxDist ) {
    Serial.println("Out of range");
    //Fdistance = maxDist;
    }
  else {
    Serial.print(Fdistance);
    Serial.print(" cm\n");}
  return (int)Fdistance;
}
void checkDirection() {
  Serial.println("checking direction:");
  myservo.write(deg180);             // tell servo to go to right
  delay(servoDelay);                 // waits 15ms for the servo to reach the position
  rightDistance = Distance_test();   //measure the distance
  Serial.println("Right distance:");
  Serial.println(rightDistance);
  delay(scanTime);
    
  myservo.write(deg90);
  delay(servoDelay);
  myservo.write(deg0);                // tell servo to go to right
  delay(servoDelay);                  // waits 15ms for the servo to reach the position
  leftDistance = Distance_test();     //measure the distance
  Serial.println("Left distance:");
  Serial.println(leftDistance);
  delay(scanTime);
  
  myservo.write(deg90);
  delay(servoDelay);
}
int lightDirection(){
  Serial.println("checking light direction:");
  double lightLevels[36], lightDegs[36];
  int i = 0, maxlight = 0;
  for (lightDeg = 0; lightDeg <=180; lightDeg+=5)
  {
    myservo.write(lightDeg);
    lightLevel= analogRead(lightSensor);
    lightLevels[i]=lightLevel;
    lightDegs[i]=lightDeg;
    Serial.println(lightLevel);
    i++;
    delay(80);
  }

  for (i = 0; i < 36; i++)
  {
    if (lightLevels[i] > maxlight)
    {
      maxlight = lightLevels[i];
      lightDeg = lightDegs[i];
    }
  }
  Serial.println("lightDeg is: ");
  Serial.println(lightDeg);
  return lightDeg;
  }

//=====================================TESTS==================================================================================
void servo_calibration(){
  int calibrationDelay =6000;
  myservo.write(deg0);
  delay(calibrationDelay);
  myservo.write(deg45);
  delay(calibrationDelay);
  myservo.write(deg90);
  delay(calibrationDelay);
  myservo.write(deg135);
  delay(calibrationDelay);
  myservo.write(deg180);
  delay(calibrationDelay);
}
void servo_verify(){
  myservo.write(deg90);
  delay(200);
  myservo.write(deg90 -10);
  delay(200);
  myservo.write(deg90 +10);
  delay(200);
  myservo.write(deg90 -10);
  delay(200);
  myservo.write(deg90);
  delay(1000);
}
void turnTime_calibration(){
  delay(2000);
  //leftMotor.Drive(-carSpeed);
  //rightMotor.Drive(-(carSpeed-motorTrim));
  goDirection = Forward;
  drive();
  delay(2000);
  goDirection = Brake;
  drive();
  delay(2000);
  goDirection = TurnAround;
  drive();
  goDirection = Brake;
  drive();
  delay(2000);
  goDirection = Forward;
  drive();
  delay(1000);
  goDirection = Brake;
  drive();
  delay(2000);
  goDirection = TurnRight;
  drive();
  goDirection = Brake;
  drive();
  delay(1000);
  goDirection = TurnLeft;
  drive();
  goDirection = Brake;
  drive();
  delay(1000);
  goDirection = TurnLeft;
  drive();
  goDirection = Brake;
  drive();
  delay(1000);          
  delay(10000);
}
void motorTest(){
  normal2Light();
}
void lightTest(){
  for (lightDeg = 0; lightDeg < 180; lightDeg+=5)
  {
    myservo.write(lightDeg);
    lightLevel= analogRead(lightSensor);
    Serial.println(lightLevel);
    delay(100); 
  }
}
//=====================================MAIN==========================================================================
void setup() {
  myservo.attach(servoPin, 700, 2400); // attach servo on pin 3 to servo object
  Serial.begin(9600);
  pinMode(Echo, INPUT);
  pinMode(Trig, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  // Set initial direction and speed
  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  

  //______test selection___________
  //lightDirection();
  //turnTime_calibration();
  //servo_calibration();
  //servo_verify();
  //motorTest();
  //playMusic();
  myservo.write(deg90);
  delay(3000);
}

void loop() {
  //*
  middleDistance = Distance_test();

  if (middleDistance <= 25) {       //robot encountered an obstacle
    goDirection= Brake;
    drive();
    checkDirection();
    //_________________work on this case with the flow chart_________________________________________
    if (rightDistance > leftDistance) {        //ride along obstacle (to the left)
      goDirection= TurnRight;
      drive();
      goDirection= Brake;
      drive();
      drive_along_right();
    }
    else if (rightDistance < leftDistance) {   //ride along obstacle (to the right)
      goDirection= TurnLeft;
      drive();
      goDirection= Brake;
      drive();
      drive_along_left();
    }
    else if ((rightDistance <= 20) || (leftDistance <= 20)) {   //exit out of if statments
      goDirection=Back;
    }
    else {
    goDirection= Forward;
    drive();
    }
  }
  else if (goDirection== Back) {                 //drive back
    leftMotor.Drive(-carSpeed);
    rightMotor.Drive(-(carSpeed-motorTrim));
    delay(reverseTime);
    goDirection= Brake;
    drive();
    rightDistance=leftDistance=100;
    }
  else {

    goDirection= Forward;
    drive();
  }                                            // */
}
