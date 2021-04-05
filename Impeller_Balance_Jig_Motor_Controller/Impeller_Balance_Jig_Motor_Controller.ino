// Include Wire Library for I2C
#include <Wire.h>

// Load Cell Library
#include <HX711_ADC.h>
 
// Adafruit Graphics & OLED libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Reset pin not used but needed for library
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

int prev = 0;

// Load pins
const int HX711_dout_1 = 9;   // mcu > HX711 no 1 dout pin
const int HX711_sck_1 = 10;   // mcu > HX711 no 1 sck pin
const int HX711_dout_2 = 11;  // mcu > HX711 no 2 dout pin
const int HX711_sck_2 = 12;   // mcu > HX711 no 2 sck pin

// HX711 constructor
HX711_ADC LoadCell_1(HX711_dout_1, HX711_sck_1); // HX711 1
HX711_ADC LoadCell_2(HX711_dout_2, HX711_sck_2); // HX711 2

const float calibrationValue_1 = 42069;  // Calibration value load cell 1
const float calibrationValue_2 = 277680; // Calibration value load cell 2

unsigned long t = 0;

// Initialize motor pin and PWM value
int mpin = 3;
float mval =  0;

float dutyCycle = 0;

// Initalize potentiometer pin and value
int pot_pin = A1;
int pot_val = 0;

// Initialize hall sensor pin
int hall = 5;

// Supporting variables to calculate rpm
float dt;
float rpm;
int curr_leading_edge;
int prev_leading_edge;
bool on_magnet;

void setup() {

  // Start Wire library for I2C
  Wire.begin();

  // Initialize OLED with I2C addr 0x3C
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  Serial.begin(115200); // Initialize serial by setting serial baud (communication rate)
  
  pinMode(hall, INPUT_PULLUP);  // Set hall sensor pin as an input with integrated pullup resistor

  // Load cell setup
  LoadCell_1.begin();
  LoadCell_2.begin();

  unsigned long stabilizingtime = 2000; // Tare preciscion can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; // Set this to false if you don't want tare to be performed in the next step
  byte loadcell_1_rdy = 0;
  byte loadcell_2_rdy = 0;
  while ((loadcell_1_rdy + loadcell_2_rdy) < 2) { // Run startup, stabilization and tare, both modules simultaniously
    if (!loadcell_1_rdy) loadcell_1_rdy = LoadCell_1.startMultiple(stabilizingtime, _tare);
    if (!loadcell_2_rdy) loadcell_2_rdy = LoadCell_2.startMultiple(stabilizingtime, _tare);
  }
  if (LoadCell_1.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 no.1 wiring and pin designations");
  }
  if (LoadCell_2.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 no.2 wiring and pin designations");
  }
  LoadCell_1.setCalFactor(calibrationValue_1); // User set calibration value (float)
  LoadCell_2.setCalFactor(calibrationValue_2); // User set calibration value (float)
}

void displayRPMandDC(float rpm, float DC){
  
  // Set up the display parameters
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0); 

  // Print data
  display.print("RPM:         " + String(rpm));
  display.setCursor(0,15);
  display.print("Duty Cycle:  " + String(DC) + "%");

  display.display();
}

void do_loadcell(){
  // get smoothed value from data set
  if (LoadCell_1.update() || LoadCell_2.update()) {
    float a = LoadCell_1.getData();
    float b = LoadCell_2.getData();
    Serial.println(String(a)+","+String(b)); 
  }
}

void set_motor_speed(){
  pot_val = analogRead(pot_pin);      // Read potentiometer value
  mval = map(pot_val, 0, 1023, 0, 255);  // Map the potentiometer position to PWM value
  analogWrite(mpin, mval);            // Write PWM value to speed control mosfet
  dutyCycle = mval/255*100;           // calculate duty cycle percentage
}

void get_rpm(){

  // Check if the hall sensor encountered a magnet, 
  // and calc the last time since it encountered a magnet

  // Get the leading edge of the magnet
  if(!digitalRead(hall) && !on_magnet){
    prev_leading_edge = curr_leading_edge;
    curr_leading_edge = millis();
    on_magnet = true;
    dt = curr_leading_edge - prev_leading_edge;
  }

  if(on_magnet && digitalRead(hall))
    on_magnet = false;

    rpm = 60000/dt;
}

void loop() {
  
  // Get load cell values
  do_loadcell();

  // Read potentiometer value and set motor speed
  set_motor_speed();

  // Display rpm and duty cycle to OLED
  if(millis() - prev > 200){
    displayRPMandDC(rpm, dutyCycle);
    prev = millis();
  }

  // Check the hall sensor and calculate the rpm of the motor
  get_rpm();  
}
