/*
 *  File: printing.h
 *  Author: Eivind Wikheim
 *
 *	Contains various debug printing macros.
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

#ifndef PRINTING_H
#define	PRINTING_H

#include <chrono>
#include <easylogging++.h>
#include <iostream>

// Definition to make Easylogging++ thread-safe
#ifndef ELPP_THREAD_SAFE
#define ELPP_THREAD_SAFE
#endif

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