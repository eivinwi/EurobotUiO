#include "communication.h"

Communication::Communication() {
	// Prepare our context and sockets
	
	std::cout << "u\n";

	zmq::context_t context (1);
	zmq::socket_t clients (context, ZMQ_ROUTER);
	clients.bind ("tcp://*:5555");
	zmq::socket_t workers (context, ZMQ_DEALER);
	workers.bind ("inproc://workers");

	std::cout << "wot\n";

	// Launch pool of worker threads
	pthread_t worker;
	pthread_create (&worker, NULL, worker_routine, (void *) &context);
	// Connect work threads to client threads via a queue

	std::cout << "m8\n";
	zmq::proxy (clients, workers, NULL);

	std::cout << "gr8 b8\n";
}

Communication::~Communication() {
}

void* worker_routine(void *arg) {
	zmq::context_t *context = (zmq::context_t *) arg;

	zmq::socket_t socket (*context, ZMQ_REP);
	socket.connect ("inproc://workers");

	while (true) {
		// Wait for next request from client
		zmq::message_t request;
		socket.recv (&request);
		std::cout << "Received request: [" << (char*) request.data() << "]" << std::endl;

		// Do some 'work'
		sleep (1);

		// Send reply back to client
		zmq::message_t reply (6);
		memcpy ((void *) reply.data (), "World", 6);
		socket.send (reply);
	}
	return (NULL);
}
