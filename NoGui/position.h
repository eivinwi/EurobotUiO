#ifndef POSITION_H
#define POSITION_H
#include "rotation.h"
#include "md49.h"
#include "printing.h"
#include <unistd.h>
#include <time.h>
#include <string>
#include <sstream>
#include <math.h>


/* curPos - current estimated position based on dead reckoning with encoders. Is updated regularly to be Exactpos
 * ExactPos   - input position from SENS
 * GoalPos    - input from main or AI 
*/
class Position {
public:
	Position();
	~Position();
	void reset();
	void set(int n_x, int n_y, int n_rotation);
	void updateEncoder();
	float distanceX(float to);
	float distanceY(float to);
	float distanceRot(float to);
	double timeSinceUpdate();
	float getRotation();
	float getX();
	float getY();
	void updateAngle(float leftDiff, float rightDiff);
	std::string getPosString();
	void incrX(float dist);
	void decrX(float dist);
	void incrY(float dist);
	void decrY(float dist);
	void print();

private:
	float x;
	float y;
	Rotation *rotation;
	time_t changed;

};

#endif /* POSITION_H */