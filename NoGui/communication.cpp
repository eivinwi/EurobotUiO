#include "communication.h"

Communication::Communication() {
    new_pos_ready = 0;
    
    //for reading position FROM PosControl
   // pthread_mutex_init(read_pos_mutex, NULL);
    
    //pthread_mutex_init(write_pos_mutex, NULL);

    int s1 = 0;
    pthread_t com_thread;
    pthread_create(&com_thread, NULL, readLoop, &s1);

    int s2 = 0;
    pthread_t th;
    pthread_create(&th, NULL, writeLoop, &s2);
}

Communication::~Communication() {
}


/* return true - position added to queue
 * return false - invalid position
*/
/*bool Communication::addToQueue(std::string input) {
	int positions[3];
	int i = 0;

    vector<string> strings;
    istringstream f(input);
    string s;    
    while(getline(f, s, ',')) {
        cout << s << endl;
        //strings.push_back(s);
        positions[i] = atoi(s);
        
        i++;
        if(i >= 3) break;
    }

    if(i != 2) {
    	return false;
    } else {
        if(abs(positions[0]) > 300)) return false;
        else if(abs(positions[1] > 300)) return false;
        else if(positions[2] < 0 || positions[2] > 360) return false;
    	else {
            //TODO: create object and add it to queue
            if(pthread_mutex_trylock() )

            return true;
        }
    }
}

void Communication::dequeuePos() {

}*/