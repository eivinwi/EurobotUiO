#ifndef GOALPOSITION_H
#define GOALPOSITION_H

#include "rotation.h"
#include "md49.h"
#include "printing.h"
#include <unistd.h>
#include <time.h>
#include <string>
#include <sstream>
#include <math.h>
#include <atomic>


/* curPos - current estimated position based on dead reckoning with encoders. Is updated regularly to be Exactpos
 * ExactPos   - input position from SENS
 * GoalPos    - input from main or AI 
*/
class GoalPosition {
public:
	GoalPosition();
	~GoalPosition();

	void reset();
	void set(int n_x, int n_y, int n_rotation);
	double timeSinceUpdate();
	float getRotation();
	float getX();
	float getY();
	std::string getPosString();

	void print();

private:
	std::atomic<int> x;
	std::atomic<int> y;
	std::atomic<int> rotation;
	time_t changed;

};

#endif /* GOALPOSITION_H */