#include "communication.h"

struct read_thread_data {
	int thread_id;
	int x;
	int y;
	int rot;
};

Communication::Communication() {
    new_pos_ready = 0;
    
    //for reading position FROM PosControl

    pthread_mutex_init(&read_pos_mutex, NULL);
    pthread_mutex_init(&write_pos_mutex, NULL);

    struct read_thread_data rd;
    rd.thread_id = 0;
    rd.x = 0;
    rd.y = 0;
    rd.rot = 0;

    pthread_create(&read_pos_thread, NULL, readLoop, &rd);

    int s2 = 1;
    pthread_create(&write_pos_thread, NULL, writeLoop, &s2);
}

Communication::~Communication() {
	pthread_join(read_pos_thread, NULL);
	pthread_join(write_pos_thread, NULL);	
}


/* return true - position added to queue
 * return false - invalid position
*/
bool Communication::addToQueue(std::string input) {
/*	int positions[3];
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
    }*/
    return false;
}

void Communication::dequeuePos() {

}