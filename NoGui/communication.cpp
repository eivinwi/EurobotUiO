#include "communication.h"

Communication::Communication() {
	int s = 0;
    pthread_t com_thread;
    pthread_create(&com_thread, NULL, readLoop, &s);


    pthread_t th;
    pthread_create(&th, NULL, writeLoop, &s);
}

Communication::~Communication() {
}


