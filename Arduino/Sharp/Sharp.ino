#define INPUT_PIN A5

void setup() {
	Serial.begin(38400);
	pinMode(INPUT_PIN, INPUT);

}


void loop() {
	int val = analogRead(INPUT_PIN);
	int result = calculate(val);
	Serial.print("Voltage(");
	Serial.print(val);
	Serial.print(") = ");
	Serial.print(result);
	Serial.println("cm.");
	delay(1000);
}



double calculate(int x) {
	if(x < 78) {
		return 0;
	}
   return  1.6250568241692588e+002 * pow(x,0)
        + -1.5201470521433809e+000 * pow(x,1)
        +  6.0156917251002716e-003 * pow(x,2)
        + -1.0418167954007743e-005 * pow(x,3)
        +  6.4526747802433565e-009 * pow(x,4);
}
