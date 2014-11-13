/*  
 * File: serial.h
 * Author: Eivind Wikheim
 *
 * TODO: implement a interface for playing audio files to use for feedback. 
 *
 */

#ifndef SOUND_H
#define	SOUND_H

#include "printing.h"
#include <string>
#include <iostream>
#include <thread>
#include <pthread.h>
#include <unistd.h>
#include <cassert>

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
private:
	
};

#endif /* SOUND_H */
