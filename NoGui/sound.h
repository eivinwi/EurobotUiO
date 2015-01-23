/*  
 * File: serial.h
 * Author: Eivind Wikheim
 *
 * TODO: implement a interface for playing audio files to use for feedback. 
 *
 */

#ifndef SOUND_H
#define	SOUND_H

//#include "printing.h"
#include <cassert>
#include <iostream>
#include <pthread.h>
#include <pwd.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <string>
#include <sys/types.h>
#include <thread>
#include <unistd.h>


#define MARIO_PLZ 1
#define MARIO_PLZ_EXT 1


/* TODO:
- implement queue
- play sounds from queue
- install libraries on all pcs
*/

class Sound {
public:
    Sound();
    ~Sound();

    void addToQueue();
    void loadFiles();
    void playFileThread(int nr);
    void playSound(int nr);
private:
	bool ready_to_play;

	struct passwd *pw;
	char* LOCATION;


	Mix_Music* files[20];
};

#endif /* SOUND_H */
