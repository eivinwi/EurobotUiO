int pwmPin = 11;

void setup() {
	Serial.begin(38400);
	pinMode(pwmPin, OUTPUT);	


}

void loop() {
	analogWrite(pwmPin, 250);
	Serial.println("250");
	delay(2000);
	analogWrite(pwmPin, 100);
	Serial.println("100");
	delay(2000);
}