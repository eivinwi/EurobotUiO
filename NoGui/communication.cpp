//FILE NOT NEEDED: multithreaded communication moved to main.cpp

#include "communication.h"

struct read_thread_data {
	int thread_id;
	int x;
	int y;
	int rot;
};

Communication::Communication() {
 //   new_pos_ready = 0;
    
    //for reading position FROM PosControl

    //pthread_mutex_init(&read_pos_mutex, NULL);
    //pthread_mutex_init(&write_pos_mutex, NULL);

/*    struct read_thread_data rd;
    rd.thread_id = 0;
    rd.x = 0;
    rd.y = 0;
    rd.rot = 0;
*/
    //pthread_create(&read_pos_thread, NULL, readLoop, &rd);


//    thread1 = std::thread(&Communication::readLoop, this);

    //read_pos_thread.join()
//;
  //  int s2 = 1;
  //  pthread_create(&write_pos_thread, NULL, writeLoop, &s2);
}

Communication::~Communication() {
	if(thread1.joinable()) {
		thread1.join();
	}
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


void Communication::readLoop() {
	// Prepare our context and socket
	zmq::context_t context (1);
	zmq::socket_t socket (context, ZMQ_REP);
	socket.bind ("tcp://*:5555");
	while (true) {
		zmq::message_t request;
		// Wait for next request from client
		socket.recv (&request);
		std::string recv_str = std::string(static_cast<char*>(request.data()), request.size());

		PRINTLINE("READTHREAD: received" << recv_str);
		// Do some 'work'
		
		sleep(1);

		//check arguments
		int pos[3];
		bool isPosition = getArguments(recv_str, pos);
		
		zmq::message_t reply(2);
		if(isPosition) {
			//set position in struct

			//if(pthread_mutex_trylock(&read_pos_mutex) != 0) {
			if(false) {
				usleep(5000);
				//if(pthread_mutex_trylock(&read_pos_mutex) != 0) {
					//error: mutex should not still be locked
				//} else {
					//rd.x = pos[0];
					//rd.y = pos[1];
					//rd.rot = pos[2];
				//}
			} else {
				//rd.x = pos[0];
				//rd.y = pos[1];
				//rd.rot = pos[2];
			}

			//return OK to client
			memcpy ((void *) reply.data (), "ok", 2);
		} else {
			//recv_str is invalid, return negative to client
			memcpy ((void *) reply.data (), "no", 2);
		}
		socket.send (reply);
	}
	//return 0;
}	

void Communication::readLoop() {


bool getArguments(std::string input, int *pos) {
	int i = 0;

    std::istringstream f(input);
    std::string s;    
    while(getline(f, s, ',')) {
        //std::cout << s << endl;
        pos[i] = atoi(s.c_str());
        
        i++;
        if(i >= 3) break;
    }

    if(i != 2) {
    	return false;
    } else {
        if(abs(pos[0]) > 300) return false;
        else if(abs(pos[1]) > 300) return false;
        else if(pos[2] < 0 || pos[2] > 360) return false;
    	else return true;
    }