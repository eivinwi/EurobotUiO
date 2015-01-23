#include "serial.h"

//TODO: try catch
Serial::Serial(std::string serial_port) {
	char s[20]; 
	strcpy(s, "/dev/");
	strcat(s, serial_port.c_str());
    LOG(INFO) << "[SERIAL]     Opening serial port: " << s;
    serial.Open(s);

    serial.SetBaudRate( SerialStreamBuf::BAUD_38400 );
    serial.SetCharSize( SerialStreamBuf::CHAR_SIZE_8 );
    serial.SetNumOfStopBits( SerialStreamBuf::DEFAULT_NO_OF_STOP_BITS );
    serial.SetParity( SerialStreamBuf::PARITY_NONE );
    serial.SetFlowControl( SerialStreamBuf::FLOW_CONTROL_NONE ); //FLOW_CONTROL_HARD

    LOG(INFO) << "[SERIAL]     Serial port open.";
}


Serial::~Serial() {
    serial.Close();
}


void Serial::write(uint8_t arg) {
    serial << arg;
}


void Serial::write(char arg) {
    serial << arg;
}


void Serial::write(int arg) {
    write((uint8_t) arg); //hack!!!!
}


uint8_t Serial::readNoWait(){
    DBPL("[SERIAL] readNW");
    int r = 0;
    if(available()) {
        r =  serial.get();
    } else {
        DBPL("In ELSE, sleeping for 50ms");
        usleep(SERIAL_DELAY);
        if(available()) {
            r =  serial.get();
        }
    }
    DBPL("[SERIAL] readNW returning " << r);
    return r;
}


uint8_t Serial::read() {
   // usleep(SERIAL_DELAY);
   // return readNoWait();

  /*  long waited = 0;
    while(!available() && waited < MAX_WAIT) {
        usleep(1);
        waited++;
    }
    return readNoWait();*/



    auto t_start = std::chrono::high_resolution_clock::now();
    auto t_end = std::chrono::high_resolution_clock::now();    
    uint8_t byte = 0;
    
    double timepassed = std::chrono::duration<double, std::milli>(t_end-t_start).count();
    while(timepassed < 1000) {
        if(available()) {
            byte = readNoWait();
            break;
        }
        t_end = std::chrono::high_resolution_clock::now();
        timepassed = std::chrono::duration<double, std::milli>(t_end-t_start).count();
    }   
    return byte;
}


long Serial::readLong() {
    DBPL("[SERIAL] readLong")
    auto t_start = std::chrono::high_resolution_clock::now();
    auto t_end = std::chrono::high_resolution_clock::now();    
    uint8_t bytes[4];
    std::fill(std::begin(bytes), std::end(bytes), 0);
    
    DBPL("[SERIAL] readLong starting loop")    
    for(int i = 0; i < 4; i++) {
        double timepassed = std::chrono::duration<double, std::milli>(t_end-t_start).count();
        while(timepassed < 100) {
            if(available()) {
                bytes[i] = readNoWait();
                break;
            }
            t_end = std::chrono::high_resolution_clock::now();
            timepassed = std::chrono::duration<double, std::milli>(t_end-t_start).count();
        }   
    }

    long result = 0;
    result += bytes[0] << 24;
    result += bytes[1] << 16;
    result += bytes[2] << 8;
    result += bytes[3];

    std::bitset<8> r1(bytes[0]);
    std::bitset<8> r2(bytes[1]);
    std::bitset<8> r3(bytes[2]);
    std::bitset<8> r4(bytes[3]);
    
    LOG(DEBUG) << "[SERIAL] read long, bits:[" 
        << r1 << "][" << r2 << "][" << r3 << "][" << r4 << "] result: " << result;
    return result;
}


bool Serial::available() {
	return serial.rdbuf()->in_avail();
}


void Serial::printAll() {
    std::stringstream ss;
    while(serial.rdbuf()->in_avail()) {
        uint8_t b = serial.get();
        ss << (int) b;
        (void) b;
    }
    LOG(DEBUG) << "Flushed serial: [" << ss << "]";
}