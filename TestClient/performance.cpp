/*
 *	File: testclient.cpp
 *	Author: Eivind Wikheim
 *
 *	Modification of ZMQ "Hello World" client example.
 *  Acts as a ZMQ client and sends a command to the main program.
 *
 *	Copyright (c) 2015 Eivind Wikheim <eivinwi@ifi.uio.no>. All Rights Reserved.
 *
 *	This file is part of EurobotUiO.
 *
 *	EurobotUiO is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 * 	(at your option) any later version.
 *
 *	EurobotUiO is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with EurobotUiO.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <zmq.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <chrono>

int main (int argc, char *argv[])
{
	// Prepare our context and socket
	zmq::context_t context (1);
	zmq::socket_t socket (context, ZMQ_REQ);
	std::cout << "Connecting to hello world server..." << std::endl;
	socket.connect ("tcp://localhost:5555");

	int itr = 10000;

    auto t_start = std::chrono::high_resolution_clock::now();

    std::string p = "2,10,1,1";
	for (int request_nbr = 0; request_nbr < itr; request_nbr++) {
		
		std::stringstream ss;
		ss << "2," << (int) request_nbr << "," << "1,1";
		std::string p = ss.str();
		zmq::message_t request( p.length()+1 );
	

		memcpy ((void *) request.data (), p.c_str(), p.length());
	//	std::cout << "Sending position: [" << p << "]" << std::endl;
		socket.send (request);
		// Get the reply.
		zmq::message_t reply;
		socket.recv (&reply);
		std::string recv_str = std::string(static_cast<char*>(request.data()), request.size());
	//	std::cout << "Received: " << recv_str << std::endl;
	}

    auto t_end = std::chrono::high_resolution_clock::now();    
	double timepassed = std::chrono::duration<double, std::milli>(t_end-t_start).count();	

	zmq::message_t request( 3 );
	memcpy ((void *) request.data (), "ok", 2);
	socket.send (request);

	std::cout << std::endl << "Packet size       : " << sizeof(p) << " B" << std::endl;
	std::cout << "Number of packets : " << itr << std::endl;
	std::cout << "Total time 	  : " << timepassed << " ms" << std::endl;
	std::cout << "Average time      : " << (timepassed / itr) << " ms" << std::endl;

	return 0;
}
