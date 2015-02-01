/*
 *	File: testclient.cpp
 *	Author: Eivind Wikheim
 *
 *	Modification of ZMQ "Hello World" client example.
 *  Acts as a ZMQ client and sends request to the main program, expects correct answer.
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


//  TODO: use actual protocol
#include <zmq.hpp>
#include <string>
#include <sstream>
#include <iostream>

int main (int argc, char *argv[])
{
	//default testing:

	// Prepare our context and socket
	zmq::context_t context (1);
	zmq::socket_t socket (context, ZMQ_REQ);
	std::cout << "Connecting to hello world server..." << std::endl;
	socket.connect ("tcp://localhost:5555");
	// Do 10 requests, waiting each time for a response
	for (int request_nbr = 0; request_nbr < 1; request_nbr++) {
		
		zmq::message_t request( 6 );
	
		memcpy ((void *) request.data (), "getpos", 6);
		//std::cout << "Sending position: [" << position << "]" << std::endl;
		std::cout << "Asking for position";
		socket.send (request);
		// Get the reply.
		zmq::message_t reply;
		socket.recv (&reply);
		std::string recv_str = std::string(static_cast<char*>(reply.data()), reply.size());
		std::cout << "Received: " << recv_str << std::endl;
	}
	return 0;
}