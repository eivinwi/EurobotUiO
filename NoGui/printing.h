/*  
 * File: printing.h
 * Author: Eivind Wikheim
 *
 * Debug print macros
 */
#ifndef PRINTING_H
#define	PRINTING_H

#include <chrono>
#include <ctime>
#include <easylogging++.h>
#include <iostream>

#ifndef ELPP_THREAD_SAFE
#define ELPP_THREAD_SAFE
#endif

/* Logging print levels:
Global 		Generic level that represents all levels. Useful when setting global configuration for all levels.
Trace 		Information that can be useful to back-trace certain events - mostly useful than debug logs.
Debug 		Informational events most useful for developers to debug application. Only applicable if NDEBUG is not defined (for non-VC++) or _DEBUG is defined (for VC++).
Fatal 		Very severe error event that will presumably lead the application to abort.
Error 		Error information but will continue application to keep running.
Warning 	Information representing errors in application but application will keep running.
Info 		Mainly useful to represent current progress of application.
Verbose 	Information that can be highly useful and vary with verbose logging level. Verbose logging is not applicable to hierarchical logging.
Unknown 	Only applicable to hierarchical logging and is used to turn off logging completely.
*/


#ifdef DEBUG
#define DBP(x) std::cerr << x;
#define DBPL(x) std::cerr << x << std::endl;
#else
#define DBP(x);
#define DBPL(x);
#endif

#define PRINTLINE(x) std::cout << x << std::endl;
#define PRINT(x) std::cout << x;

static auto time_start = std::chrono::high_resolution_clock::now();
#define TIMESTAMP(x) std::cout << std::chrono::duration<double, std::milli>( std::chrono::high_resolution_clock::now() - time_start ).count() << ": " << x << std::endl;

#endif	/* PRINTING_H */