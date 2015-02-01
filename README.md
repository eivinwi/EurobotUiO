## Introduction
Part of the Eurobot project at UiO. 

This part of the project contains the navigation and position control.



### Compilation
While standing in Program directory:
```
make      :  compiles default program   
make debug:  compiles program with extra debug printing
make clean:  removes compiled files
```

### Running
While standing in Program directory:

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
| -m 'port'| sets argument port as motor serial port (default: ttyUSB0)
| -n 'port'| sets argument port as lift serial port (default: ttyUSB1)

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