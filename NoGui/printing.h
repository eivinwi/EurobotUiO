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
#include <iostream>

#ifdef DEBUG
#define DBP(x) std::cerr << x;
#define DBPL(x) std::cerr << x << std::endl;
#else
#define DBP(x);
#define DBPL(x);
#endif

#define PRINTLINE(x) std::cout << x << std::endl;
#define PRINT(x) std::cout << x;

#define PRINT_OK() std::cout << "\033[0;31mok\033[0m\n" << std::endl;
#define PRINT_FAIL() std::cout << "\033[0;32mfail\033[0m\n" << std::endl; 

static auto time_start = std::chrono::high_resolution_clock::now();
#define TIMESTAMP(x) std::cout << std::chrono::duration<double, std::milli>( std::chrono::high_resolution_clock::now() - time_start ).count() << ": " << x << std::endl;

#endif	/* PRINTING_H */