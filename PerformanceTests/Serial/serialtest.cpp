#include <SerialStream.h>
#include <iostream>
using namespace LibSerial ;





int main() {
    serial.Open("/dev/ttyACM0");
    serial.SetBaudRate( SerialStreamBuf::BAUD_38400 );
    serial.SetCharSize( SerialStreamBuf::CHAR_SIZE_8 );
    serial.SetNumOfStopBits( SerialStreamBuf::DEFAULT_NO_OF_STOP_BITS);
    serial.SetParity( SerialStreamBuf::PARITY_NONE );
    serial.SetFlowControl( SerialStreamBuf::FLOW_CONTROL_HARD );




    serial.Close();
}


