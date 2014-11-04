#include "communication.h"

Communication::Communication() {
	int s = 0;
    pthread_t com_thread;
    pthread_create(&com_thread, NULL, readLoop, &s);
}

Communication::~Communication() {
}


