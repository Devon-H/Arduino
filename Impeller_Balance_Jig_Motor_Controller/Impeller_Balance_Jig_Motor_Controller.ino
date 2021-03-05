// Include Wire Library for I2C
#include <Wire.h>
 
// Include Adafruit Graphics & OLED libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Reset pin not used but needed for library
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

//yourmom
//initialize motor pin and PWM value
int mpin = 3;
float mval =  0;

float dutyCycle = 0.00; 

//initalize potentiometer pin and value
int ppin = A3;
int pval = 0;

//initialize hall sensor pin
int hall = 9;

unsigned long initial = 0;
unsigned long final = 0;

float dt;
float rpm;
bool read_rpm;

void setup() {
  // put your setup code here, to run once:

  // Start Wire library for I2C
  Wire.begin();

  // initialize OLED with I2C addr 0x3C
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  
  Serial.begin(115200); //initialize serial by setting serial baud (communication rate)
  pinMode(hall, INPUT_PULLUP); //set hall sensor pin as an input with integrated pullup resistor
  initial = millis(); //set initial to time since program start in milliseconds
   
}


void displayRPMandDC(float rpm, float DC){

  Serial.print(String(rpm) +" ");
  Serial.print(String(DC));
  // Clear the display
  display.clearDisplay();
  //Set the color - always use white despite actual display color
  display.setTextColor(WHITE);
  //Set the font size
  display.setTextSize(1);
  //Set the cursor coordinates
  display.setCursor(0,0); 
  display.print("RPM:         "); 
  display.print(rpm);
  display.setCursor(0,15);
  display.print("Duty Cycle:  "); 
  display.print(DC);
  display.print("%");
}

void loop() {
  // put your main code here, to run repeatedly:
  
  pval = analogRead(ppin); //
  mval = map(pval, 0, 1023, 0, 255); //map the potentiometer position to PWM value
  analogWrite(mpin, mval);

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
     
    //display to OLED
    displayRPMandDC(rpm, dutyCycle);
    display.display();
    
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
  Serial.print("RPM = " + String(rpm) + " "); 
  Serial.print("potentiometer = " + String(pval) + " ");
  Serial.println("motor = " + String(mval));

  //display to OLED
  displayRPMandDC(rpm, dutyCycle);
  display.display();
}
