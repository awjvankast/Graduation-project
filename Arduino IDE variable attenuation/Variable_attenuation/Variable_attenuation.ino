void setup() {
  // put your setup code here, to run once:
pinMode(0,OUTPUT);
pinMode(1,OUTPUT);
pinMode(2,OUTPUT);
pinMode(3,OUTPUT);
pinMode(4,OUTPUT);
pinMode(5,OUTPUT);
Serial.begin(9600);
}

int number =-1;

void loop() {
  // put your main code here, to run repeatedly:

number++;
 //     Serial.println(number);
  //    Serial.println();
for(int i = 0; i<6; i++){
  //    Serial.println(number,BIN);
     // Serial.println(pow(2,i));
      //  Serial.println(round(pow(2,i)));
  //  Serial.println(round(pow(2,float(i))) ,BIN);
  //  Serial.println(number & round(pow(2,i)) ,BIN);

  if( round(number & round(pow(2,i))) !=0 ){
  // Serial.print("Setting pin high:");
  // Serial.println(i);
    digitalWrite(i,HIGH);
  }
  else{
    digitalWrite(i,LOW);
  }
      //Serial.println();
}
//Serial.println("NUMBER INCREASE");
if(number == 63) number = -1;
delay(12.5);
}
