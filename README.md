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
|   Argument    |         Description 											|
|---------------|---------------------------------------------------------------|
|  --help       |  print help message
|  --testing    |  enable testing (instant results)
|  --sim        |  enable simulation of motors
|  --debug      |  enable detailed debug logging
|  --nolog      |  disable all logging
|  --mport arg  |  Set MD49 serial port (ex: /dev/ttyUSB0)
|  --dport arg  |  Set Dynamixel serial port (ex: /dev/ttyUSB1
|  --ai arg     |  Set AI ZMQ server port (ex: 5900)
|  --pos arg    |  Set POS ZMQ client port (ex: 5555)
|  --config arg |  Set YAML config file



### Examples

##### Run program 
```
./exec
```

##### Run program with testing+simulation functionality
```
./exec --sim --testing
```

##### Run program on different motor serial-port
```
./exec -mport /dev/ttyUSB2
```



## Library installation details

##### Ubuntu packages
```
sudo apt-get install libserial-dev libroot-core-dev libroot-graf2d-postscript-dev libroot-math-physics-dev libroot-graf3d-eve-dev libtool pkg-config build-essential autoconf automake python-pip python-dev libboost-all-dev
```

##### ZMQ
http://zeromq.org/intro:get-the-software 
```
wget http://download.zeromq.org/zeromq-4.1.0-rc1.tar.gz
tar xvzf zeromq-4.1.0-rc1.tar.gz && cd zeromq-4.1.0 && ./configure && make && sudo make install && sudo ldconfig
```

##### Cppzmq
https://github.com/zeromq/cppzmq
```
git clone https://github.com/zeromq/cppzmq
sudo mv cppzmq/zmq.hpp /usr/local/include
```

##### Pyzmq (for testing with Eurobot-AI)
Not currently neccessary
http://zeromq.org/bindings:python
```
sudo pip install pyzmq
```

##### Easylogging++
https://github.com/easylogging/easyloggingpp/
```
git clone https://github.com/easylogging/easyloggingpp/
sudo mv easyloggingpp/src/easylogging++.h /usr/local/include
```

##### YAML
https://github.com/jbeder/yaml-cpp.git
```
git clone https://github.com/jbeder/yaml-cpp.git
cd yaml-cpp && mkdir build && cd build && cmake .. && make && sudo make install
```

## Mechanical/Electronical parts

##### Devantech RB-Dev-38 Motors
<img src="http://www.robotshop.com/media/catalog/product/cache/1/image/515x515/9df78eab33525d08d6e5fb8d27136e95/d/e/devantech-24v-49-1-gear-motor-encoder.jpg" width="150px"/><br>

http://www.robotshop.com/en/devantech-24v-49-1-gear-motor-encoder.html<br>
http://www.robotshop.com/media/files/pdf/datasheet-emg49.pdf<br>


##### Devantech MD49 Motor controller
<img src="http://www.robotshop.com/media/catalog/product/cache/1/image/515x515/9df78eab33525d08d6e5fb8d27136e95/d/e/devantech-md49-24v-5a-dual-h-bridge-motor-driver.jpg" width="150px"/><br>
http://www.robotshop.com/en/devantech-md49-24v-5a-dual-h-bridge-motor-driver.html<br>
http://www.robot-electronics.co.uk/htm/md49tech.htm<br>


##### SpringRC SM-S4303R (gripper servo)
<img src="https://cdn.sparkfun.com//assets/parts/2/9/1/1/09347-1.jpg" width="150px;"/><br>
https://www.sparkfun.com/products/9347<br>
https://www.sparkfun.com/datasheets/Robotics/servo-360_e.pdf<br>


##### Stepper Motor (lift)
<img src="https://cdn.sparkfun.com//assets/parts/2/7/1/5/09238-01.jpg" width="150px"/>
<img src="https://cdn.sparkfun.com//assets/parts/1/0/3/8/0/13226-01b.jpg" width="150px"/><br>
https://www.sparkfun.com/products/9238<br>
https://www.sparkfun.com/datasheets/Robotics/SM-42BYG011-25.pdf<br>
https://www.sparkfun.com/products/13226<br>
http://www.sparkfun.com/datasheets/Robotics/A3967.pdf<br>

##### 14.8V LiPo Batteries
<img src="http://img-europe.electrocomponents.com/largeimages/R7760869-01.jpg" width="150px"/><br>
http://no.rs-online.com/web/p/lithium-rechargeable-battery-packs/7760869/<br>
http://no.rs-online.com/web/p/lithium-rechargeable-battery-packs/7760869/<br>

##### LiPo Fuel Gauges
<img src="https://cdn.sparkfun.com//assets/parts/5/2/7/9/10617-01c.jpg" width="150px"/><br><br>
https://www.sparkfun.com/products/10617<br>
http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Prototyping/MAX17043-MAX17044.pdf<br>

##### DC-DC converters
<img src="http://img-europe.electrocomponents.com/largeimages/R0183679-01.jpg" width="150px"/>
<img src="http://img-europe.electrocomponents.com/largeimages/F4414037-01.jpg" width="150px"/><br><br>
http://no.rs-online.com/web/p/isolated-dc-dc-converters/0183679/<br>
http://no.rs-online.com/web/p/isolated-dc-dc-converters/4414037/<br>
