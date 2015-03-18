#define INPUT_PIN A5

void setup() {
	Serial.begin(38400);
	pinMode(INPUT_PIN, INPUT);

}


void loop() {
	int val = analogRead(INPUT_PIN);
	Serial.print("Voltage: ");
	Serial.println(val);
	delay(500);
}
