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


## Mechanical/Electronical parts
![](http://www.robotshop.com/media/catalog/product/cache/1/image/515x515/9df78eab33525d08d6e5fb8d27136e95/d/e/devantech-24v-49-1-gear-motor-encoder.jpg)


##### Devantech RB-Dev-38 Motors
<img src="http://www.robotshop.com/media/catalog/product/cache/1/image/515x515/9df78eab33525d08d6e5fb8d27136e95/d/e/devantech-24v-49-1-gear-motor-encoder.jpg" style="width: 100px;"/><br>

http://www.robotshop.com/en/devantech-24v-49-1-gear-motor-encoder.html<br>
http://www.robotshop.com/media/files/pdf/datasheet-emg49.pdf<br>


##### Devantech MD49 Motor controller
<img src="http://www.robotshop.com/media/catalog/product/cache/1/image/515x515/9df78eab33525d08d6e5fb8d27136e95/d/e/devantech-md49-24v-5a-dual-h-bridge-motor-driver.jpg" style="width: 150px;"/><br>
http://www.robotshop.com/en/devantech-md49-24v-5a-dual-h-bridge-motor-driver.html<br>
http://www.robot-electronics.co.uk/htm/md49tech.htm<br>


##### SpringRC SM-S4303R (gripper servo)
<img src="https://cdn.sparkfun.com//assets/parts/2/9/1/1/09347-1.jpg" style="width: 150px; height: 150px;"/><br>
https://www.sparkfun.com/products/9347<br>
https://www.sparkfun.com/datasheets/Robotics/servo-360_e.pdf<br>


##### Stepper Motor (lift)
<img src="https://cdn.sparkfun.com//assets/parts/2/7/1/5/09238-01.jpg" style="width: 150px;"/>
<img src="https://cdn.sparkfun.com//assets/parts/1/0/3/8/0/13226-01b.jpg" style="width: 150px;"/><br>
https://www.sparkfun.com/products/9238<br>
https://www.sparkfun.com/datasheets/Robotics/SM-42BYG011-25.pdf<br>
https://www.sparkfun.com/products/13226<br>
http://www.sparkfun.com/datasheets/Robotics/A3967.pdf<br>

##### 14.8V LiPo Batteries
<img src="http://img-europe.electrocomponents.com/largeimages/R7760869-01.jpg" style="width: 150px;"/><br>
http://no.rs-online.com/web/p/lithium-rechargeable-battery-packs/7760869/<br>
http://no.rs-online.com/web/p/lithium-rechargeable-battery-packs/7760869/<br>s

##### LiPo Fuel Gauges
<img src="https://cdn.sparkfun.com//assets/parts/5/2/7/9/10617-01c.jpg" style="width: 150px;"/><br><br>
https://www.sparkfun.com/products/10617<br>
http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Prototyping/MAX17043-MAX17044.pdf<br>

##### DC-DC converters
<img src="http://img-europe.electrocomponents.com/largeimages/R0183679-01.jpg" style="width: 150px;"/><br><br>
<img src="http://img-europe.electrocomponents.com/largeimages/F4414037-01.jpg" style="width: 150px;"/><br><br>
http://no.rs-online.com/web/p/isolated-dc-dc-converters/0183679/<br>
http://no.rs-online.com/web/p/isolated-dc-dc-converters/4414037/<br>
