/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
 
// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0
// give it a name:
const int led = 13;

int state = LOW;
String command = "";
char commandStart = '<';
char commandEnd = '>';
unsigned long lastMessageTime = 0;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);

  Serial.begin(115200);  
}

// the loop routine runs over and over again forever:
void loop() {
  while (Serial.available() > 0) {
    int input = Serial.read();

    if (input == '\n') {
      handle(command);

      command = "";
    } else {
      command += (char)input;
    }
  }
  
  //Serial.println("<speeds:0:0:0:0:0>");
  
  //delay(16);
}

void handle(String command) {
  digitalWrite(led, state);
  state = state == LOW ? HIGH  : LOW;
  
  unsigned long currentTime = millis();
  unsigned long dt = currentTime - lastMessageTime;
  
  Serial.print("<got:");
  Serial.print(command);
  Serial.print(":");
  Serial.print(dt);
  Serial.print(">\n");
  
  lastMessageTime = currentTime;
}
