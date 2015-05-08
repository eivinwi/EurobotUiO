/*
 *  File: sound.h
 *  Author: Eivind Wikheim
 *
 *  TODO: implement a interface for playing audio files to use for feedback. 
 *
 *  Copyright (c) 2015 Eivind Wikheim <eivinwi@ifi.uio.no>. All Rights Reserved.
 *
 *  This file is part of EurobotUiO.
 *
 *  EurobotUiO is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EurobotUiO is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EurobotUiO.  If not, see <http://www.gnu.org/licenses/>.
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
