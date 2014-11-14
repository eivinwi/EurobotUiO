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

#ifdef DEBUG
#define DBP(x) std::cerr << x;
#define DBPL(x) std::cerr << x << std::endl;
#else
#define DBP(x);
#define DBPL(x);
#endif

#define PRINTLINE(x) std::cout << x << std::endl;
#define PRINT(x) std::cout << x;


#define TIMESTAMP(x) std::cout << std::chrono::duration<double, std::milli>( std::chrono::high_resolution_clock::now() - time_start ).count() << ": " << x << std::endl;

static auto time_start = std::chrono::high_resolution_clock::now();


#endif	/* PRINTING_H */