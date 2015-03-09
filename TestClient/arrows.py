#
# Hello World client in Python
# Connects REQ socket to tcp://localhost:5555
# Sends "Hello" to server, expects "World" back
#

import zmq
import curses

context = zmq.Context()
angle = 0
# get the curses screen window
screen = curses.initscr()

# turn off input echoing
curses.noecho()

# respond to keys immediately (don't wait for enter)
curses.cbreak()

# map arrow keys to special values
screen.keypad(True)


# Socket to talk to server
print("Connecting to eurobot server...")
socket = context.socket(zmq.REQ)
socket.connect("tcp://localhost:5555")
print("Waiting for keyboard input (arrows)");

def sendMsg(key):
	print("Sending message: %s" % key)
	socket.send(key)

	message = socket.recv()
	print("Received reply %s" % message)

try:
	while True:
		char = screen.getch()
		if char == ord('q'):
			break
		elif char == curses.KEY_RIGHT:
			# print doesn't work with curses, use addstr instead
			angle -= 20
			sendMsg("3,0,%d" % angle) #turn right
		elif char == curses.KEY_LEFT:
			angle += 20
			sendMsg("3,0,%d" % angle) #turn left
		elif char == curses.KEY_UP:
			sendMsg("7,0,50") #go forward       
		elif char == curses.KEY_DOWN:
			sendMsg("7,0,-50") #go backward
finally:
	# shut down cleanly
	curses.nocbreak(); screen.keypad(0); curses.echo()
	curses.endwin()
