

void setup() {
	Serial.begin(9600);
}

void loop() {
	
	for(int i = 0; i < 100; i++) {
		Serial.print("Heyhey");
		Serial.println(i);
		delay(100);
	}
	delay(2000);
}