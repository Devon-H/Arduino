int butt = 4; 
int led = 7;
int flag = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(butt, INPUT_PULLUP);
  pinMode(led, OUTPUT);
}

void loop(){
  // put your main code here, to run repeatedly:
    if(digitalRead(butt) == true){
      flagset();
    }
    
}

void flagset(){
   flag = !flag;
   digitalWrite(led,flag);

   while(digitalRead(butt) == true){
    delay(75);
   }
}
