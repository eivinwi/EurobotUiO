
void setup() {
	Serial.begin(9600);
}

void loop() {
	digitalWrite(12, HIGH); 
	Serial.write("HIGH");
        Serial.write('\n');
        if(Serial.available()) {
          char line = Serial.read();
          if(line == '-') {
              digitalWrite(10, HIGH);
          }
        }
        
	delay(200);
	digitalWrite(12, LOW); 
        digitalWrite(10, LOW);
	delay(200);
}
