#include "sound.h"

Sound::Sound() {
	// Get user homedir
	pw = getpwuid(getuid());	
	LOCATION = pw->pw_dir;
	strcat(LOCATION, "/EurobotUiO/Sound/");

	ready_to_play = true;

	// Init
	if (SDL_Init(SDL_INIT_AUDIO) != 0) {
	  std::cerr << "SDL_Init ERROR: " << SDL_GetError() << std::endl;
	  ready_to_play = false;
	}

	// Open Audio device
	if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 2048) != 0) {
	  std::cerr << "Mix_OpenAudio ERROR: " << Mix_GetError() << std::endl;
	  ready_to_play = false;
	}

	// Set Volume
	Mix_VolumeMusic(100);

	if(ready_to_play) {
		loadFiles();
	}
}

Sound::~Sound() {
	//possibly free files
	Mix_CloseAudio();
}


void Sound::loadFiles() {
	files[0] = Mix_LoadMUS((std::string(LOCATION) + std::string("/Mario/mario_plz.wav")).c_str());
	files[1] = Mix_LoadMUS((std::string(LOCATION) + std::string("Defective_Turret/DF_come_on.wav")).c_str());
	files[2] = Mix_LoadMUS((std::string(LOCATION) + std::string("Defective_Turret/DF_come_on_you_guys.wav")).c_str());
	files[3] = Mix_LoadMUS((std::string(LOCATION) + std::string("Defective_Turret/DF_blind.wav")).c_str());
	files[4] = Mix_LoadMUS((std::string(LOCATION) + std::string("Defective_Turret/DF_fantastic.wav")).c_str());
}

//to be run in thread by main
void Sound::playFileThread(int nr) {
	if(ready_to_play) {
		if(nr < 20) {
			if (files[nr]) {
				// Start Playback
				if (Mix_PlayMusic(files[nr], 1) == 0) {
					unsigned int startTime = SDL_GetTicks();

					// Wait
					while (Mix_PlayingMusic()) {
					   SDL_Delay(100);
					   std::cout << "Time: " << (SDL_GetTicks() - startTime) / 1000 << std::endl;
					}
				}
				else {
					std::cerr << "Mix_PlayMusic ERROR: " << Mix_GetError() << std::endl;
				}

				// Free File
				//         Mix_FreeMusic(music);
				//         music = 0;
			}
			else {
				std::cerr << "Mix_LoadMuS ERROR: " << Mix_GetError() << std::endl;
			}
		} else {
			std::cerr << "Number too high: " << nr << std::endl;
		}
	} else {
		std::cerr << "Audio device not ready" << std::endl;		
	}
}


void Sound::playSound(int nr) {
//	std::thread read_thread(playFileThread, &nr);

 //   std::thread sound_thread(&Sound::playFileThread, 0);

	//if(sound_thread.joinable()) {
//		sound_thread.join();
//	}
}