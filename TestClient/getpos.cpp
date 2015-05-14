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

int main (int argc, char *argv[])
{


	// Prepare our context and socket
	zmq::context_t context (1);
	zmq::socket_t socket (context, ZMQ_REQ);
	std::cout << "Connecting to hello world server..." << std::endl;
//	socket.connect ("tcp://localhost:5900");
	socket.connect(argv[1]);
	//socket.connect("ipc://ai.ipc");

	std::string position = "1,100,300,57\n";

	// Do 10 requests, waiting each time for a response
	for (int request_nbr = 0; request_nbr < 1; request_nbr++) {
		
		zmq::message_t request( position.length()+1 );
	
		memcpy ((void *) request.data (), position.c_str(), position.length());
		std::cout << "Sending position: " << position << std::endl;
		socket.send (request);
		// Get the reply.
		//usleep(2);
		zmq::message_t reply;
		socket.recv (&reply);
		std::string recv_str = std::string(static_cast<char*>(reply.data()), reply.size());
		std::cout << "Received: " << recv_str << std::endl;
	}
	return 0;
}
