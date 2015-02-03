## Introduction
Part of the Eurobot project at UiO. 

This part of the project contains the navigation and position control.


## Compilation
While in Program directory:

| Argument | Description |
|----------|---------------------------------------------------------------|
| make     | compiles default program
|make debug| compiles program with extra debug printing	
|make clean| removes compiled files


## Running
While in Program directory:

```
./exec
```


### Execution parameters

| Argument | Description |
|----------|---------------------------------------------------------------|
|    -s    | enable simulation of serial-ports.
|    -t    | enable testing mode (commands finish instantly).
|    -d    | enables extra debug-logging to separate debug.log.
|    -n    | disable the default info-logging to file.
| -m \<port\>| sets argument port as motor serial port (default: ttyUSB0)
|  -l \<port\>| sets argument port as lift serial port (default: ttyUSB1)

### Examples

##### Run program 
```
./exec
```

##### Run program with testing+simulation functionality
```
./exec -s -t
```

##### Run program on different motor serial-port
```
./exec -m ttyACM0
```



## Library installation details

##### Ubuntu packages
```
sudo apt-get install libserial-dev libroot-core-dev libroot-graf2d-postscript-dev libroot-math-physics-dev libroot-graf3d-eve-dev libtool pkg-config build-essential autoconf automake pip python-dev
```

##### ZMQ
http://zeromq.org/intro:get-the-software 
```
wget http://download.zeromq.org/zeromq-4.1.0-rc1.tar.gz
tar xvzf zeromq-4.1.0-rc1.tar.gz
cd zeromq-4.1.0 && ./configure && make && sudo make install && sudo ldconfig
```

##### Cppzmq
https://github.com/zeromq/cppzmq
```
wget https://github.com/zeromq/cppzmq/blob/master/zmq.hpp
sudo mv zmq.hpp /usr/local/include
```

##### Pyzmq (for testing with Eurobot-AI)
http://zeromq.org/bindings:python
```
sudo pip install pyzmq
```


##### Easylogging++
https://github.com/easylogging/easyloggingpp/
```
wget https://github.com/easylogging/easyloggingpp/releases/download/v9.80/easyloggingpp_v9.80.tar.gz
tar xvzf easyloggingpp_v9.80.tar.gz && sudo mv easylogging++.h /usr/local/include
```


## Mechanical/electronical parts

##### Devantech RB-Dev-38 Motors
http://www.robotshop.com/en/devantech-24v-49-1-gear-motor-encoder.html<br>
http://www.robotshop.com/media/files/pdf/datasheet-emg49.pdf<br>


##### Devantech MD49 Motor controller
http://www.robotshop.com/en/devantech-md49-24v-5a-dual-h-bridge-motor-driver.html<br>
http://www.robot-electronics.co.uk/htm/md49tech.htm<br>


##### SpringRC SM-S4303R (gripper servo)
https://www.sparkfun.com/products/9347<br>
https://www.sparkfun.com/datasheets/Robotics/servo-360_e.pdf<br>


##### Stepper
https://www.sparkfun.com/products/9238<br>
https://www.sparkfun.com/datasheets/Robotics/SM-42BYG011-25.pdf<br>

