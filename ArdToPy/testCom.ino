
void setup() {
	Serial.begin(9600);
}

void loop() {
	digitalWrite(12, HIGH); 
	Serial.write("HIGH");
	delay(1000);
	digitalWrite(12, LOW); 
	Serial.write("LOW");
	delay(1000);
}