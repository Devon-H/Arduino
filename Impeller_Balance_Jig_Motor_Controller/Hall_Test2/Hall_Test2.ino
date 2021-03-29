
//initialize motor pin and PWM value
int mpin = 3;
float mval =  0;

float dutyCycle = 0; 

//initalize potentiometer pin and value
int ppin = A1;
int pval = 0;

//initialize hall sensor pin
int hall = 5;

unsigned long initial = 0;
unsigned long final = 0;

float dt;
float rpm;
bool read_rpm;

void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200); //initialize serial by setting serial baud (communication rate)
  
  pinMode(hall, INPUT_PULLUP); //set hall sensor pin as an input with integrated pullup resistor
  initial = millis(); //set initial to time since program start in milliseconds
}

void loop() {
  // put your main code here, to run repeatedly:

    pval = analogRead(ppin); //read potentiometer value
    mval = map(pval, 0, 1023, 0, 255); //map the potentiometer position to PWM value
    analogWrite(mpin, mval); //write PWM value to speed control mosfet

    dutyCycle = mval/255*100; //calculate duty cycle percentage
  
   while(!digitalRead(hall)){ // If this loop is entered, we encountered a magnet. 
    //keep mapping
    pval = analogRead(ppin);
    mval = map(pval, 0, 1023, 0, 255);
    analogWrite(mpin, mval);

    dutyCycle = mval/255*100; //calculate duty cycle percentage
    
    //print to serial monitor for debugging
    Serial.print("RPM = " + String(rpm)); 
    Serial.print("potentiometer = " + String(pval) + " ");
    Serial.println("motor = " + String(mval)+ " ");
    
    read_rpm = true;
    final = millis();
  }
  
  if(read_rpm){
     dt = final-initial; //change in time in milliseconds
     initial = final; 
     read_rpm = false;
  }
  
  rpm = 60000/dt; //calculate RPM

      //print to serial monitor for debugging
    Serial.print("RPM = " + String(rpm)); 
    Serial.print("potentiometer = " + String(pval) + " ");
    Serial.println("motor = " + String(mval)+ " ");
}
