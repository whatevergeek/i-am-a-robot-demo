//Make sure the dip switches are turned ON, and none of your shields are using pins A0,A1,A2,A3 or D4

#include <Shieldbot.h>  //includes the Shieldbot Library
#include <IRremote.h>

Shieldbot shieldbot = Shieldbot();  //decares a Shieldbot object
int S1,S2,S3,S4,S5; //values to store state of sensors

#define RECV_PIN A4

int LED1 = 2;

long cmd_go  = 0x00FD08F7;
long cmd_stop = 0x00FD8877;
long cmd_random_stop = 0x00FD48B7;

int on = 0;
unsigned long last = millis();

long max_move_cycles = 100;
long remaining_move_cycles = max_move_cycles;

IRrecv irrecv(RECV_PIN);
decode_results results;
void dump(decode_results *results) {
  int count = results->rawlen;
  if (results->decode_type == UNKNOWN) 
  {
   Serial.println("Could not decode message");
  } 
  else 
  {
  if (results->decode_type == NEC) 
    {
     Serial.print("Decoded NEC: ");
    } 
  else if (results->decode_type == SONY) 
    {
     Serial.print("Decoded SONY: ");
    } 
  else if (results->decode_type == RC5) 
    {
     Serial.print("Decoded RC5: ");
    } 
  else if (results->decode_type == RC6) 
    {
     Serial.print("Decoded RC6: ");
    }
   Serial.print(results->value, HEX);
   Serial.print(" (");
   Serial.print(results->bits, DEC);
   Serial.println(" bits)");
  }
  Serial.print("Raw (");
  Serial.print(count, DEC);
  Serial.print("): ");

  for (int i = 0; i < count; i++) 
  {
    if ((i % 2) == 1) {
      Serial.print(results->rawbuf[i]*USECPERTICK, DEC);
    } 
    else  
    {
      Serial.print(-(int)results->rawbuf[i]*USECPERTICK, DEC);
    }
    Serial.print(" ");
  }
Serial.println("");
}


void setup(){
  Serial.begin(9600);//Begin serial comm for debugging
  shieldbot.setMaxSpeed(50,50);//255 is max, if one motor is faster than another, adjust values 

  pinMode(RECV_PIN, INPUT);   
  pinMode(LED1, OUTPUT);
  pinMode(13, OUTPUT);
  
  irrecv.enableIRIn(); // Start the receiver
}

void loop(){
  Serial.print("Remaining move cycles: ");
  Serial.print(remaining_move_cycles);
  Serial.println();
  if(remaining_move_cycles > 0)
  {
    robot_move();
    if(remaining_move_cycles < max_move_cycles)
    {
      remaining_move_cycles--;
    }
  }
  else
  {
    shieldbot.stop();
    digitalWrite(LED1, LOW);     
    Serial.println("LED: Off"); 
  }
  check_ir();
}

void check_ir()
{
   if (irrecv.decode(&results)) 
   {
    // If it's been at least 1/4 second since the last
    // IR received, toggle the relay
    if (millis() - last > 250) 
      {
       on = !on;
       digitalWrite(13, on ? HIGH : LOW);
       dump(&results);
      }
    if (results.value == cmd_go )
    {
       digitalWrite(LED1, HIGH);
       Serial.println("LED: On");

       remaining_move_cycles = max_move_cycles;
    }
    if (results.value == cmd_stop )
    {
       remaining_move_cycles = 0;
    }
    if (results.value == cmd_random_stop )
    {
       remaining_move_cycles = random(max_move_cycles-1);
    }
    last = millis();      
    irrecv.resume(); // Receive the next value
  }
}

void robot_move(){
  //Read all the sensors 
  S1 = shieldbot.readS1();
  S2 = shieldbot.readS2();
  S3 = shieldbot.readS3();
  S4 = shieldbot.readS4();
  S5 = shieldbot.readS5();

  //Print the status of each sensor to the Serial console
  Serial.print("S5: ");
  Serial.print(S5);
  Serial.print(" S4: ");
  Serial.print(S4);
  Serial.print(" S3: ");
  Serial.print(S3);
  Serial.print(" S2: ");
  Serial.print(S2);
  Serial.print(" S1: ");
  Serial.print(S1);
  Serial.println();
 
  //Note about IR sensors  
  //if a sensor sees HIGH, it means that it just sees a reflective surface background(ex. whie)
  //if a sensor sees LOW, it means that it sees a non-reflective surface or empty space (ex. black tape line, or empty space off ledge)
 
  if(S1 == HIGH && S5 == HIGH){ //if the two outer IR line sensors see background, go forward
    shieldbot.forward(); 
  Serial.println("Forward");
  }else if(S1 == LOW && S5 == LOW){ //if either of the two outer IR line sensors see empty space (like edge of a table) stop moving
    shieldbot.stop();
  Serial.println("Stop");
    delay(100);
  }else if((S1 == LOW) || (S2 == LOW)){ //if the two most right IR line sensors see black tape, turn right
    shieldbot.drive(127,-128);// to turn right, left motor goes forward and right motor backward
  Serial.println("Right");
    delay(100);
  }else if((S5 == LOW) || (S4 == LOW)){ //if either of the two most left IR line sensors see black , turn left
    shieldbot.drive(-128,127);// to turn right, left motor goes backward and right motor forward
  Serial.println("Left");
    delay(100);
  }else //otherwise just go forward
    shieldbot.forward();
}


