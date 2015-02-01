## Introduction
Part of the Eurobot project at UiO. 

This part of the project contains the navigation and position control.


To compile/run the main program, go to directory Program. 


### Compilation
While standing in directory Program:

* make        - compiles default program   
* make debug  - compiles program with extra debug printing
* make clean  - removes compiled files


### Running
While standing in directory Program:

* ./exec


### Execution parameters
```
-s: enable simulation of serial
-t: enable test-mode
-d: enable debug-file
-n: disable logging to file
-m <port>: set motor serial port (ex: ttyUSB0)
-l <port>: set lift serial port (ex: ttyUSB1)
```

### Examples

##### Run program 
./exec

##### Run program with testing+simulation functionality
./exec -s -t 

##### Run program on different motor serial-port
./exec -m ttyACM0 