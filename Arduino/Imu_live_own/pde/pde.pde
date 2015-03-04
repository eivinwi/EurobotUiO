import processing.serial.*;
import org.gwoptics.*;

Serial port;


float[][] gyro = new float[10000][3];
float[][] accel = new float[10000][3];
float[][] magne = new float[10000][3];


void setup() {
    port = new Serial(this, "/dev/ttyUSB0", 115200);  
}


//void draw() { 
//}


void serialEvent(Serial p) {
    if(p.available() > 0) {
        String inputString = p.readStringUntil('\n');

        print("Recieved: ");
        print(inputString);

        if (inputString != null && inputString.length() > 0) {
            String [] strArr = split(inputString, " ");

            char first = strArr[0].charAt(0);

            switch(first) {
                case 'G':
                    //gyro
                    addGyro(strArr);
                    break;
                case 'A':
                    //accel
                    addAccel(strArr);
                    break;
                case 'M':
                    //magnetometer
                    addMagn(strArr);
                    break;
                case 'L':
                    print("Return value: (");
                    print(strArr[1]);
                    print("==");
                    print("0x49D4) =>");
                    print( (str[1]).equals("0x49D4") == 0 );
                    break;
                case 'S':
                    print(inputString);
                    if(strArr.size() == 4) {
                        gRes = String.toInt(strArr[1]);
                        aRes = String.toInt(strArr[2]);
                        mRes = String.toInt(strArr[3]);  
                    }
                    break;
                default:
                    break;
            }   
        }
    }
}


void addGyro(String []arr) {
    if(arr.size() == 4) {
        //draw onto plot
    }
}


void addAccel(String []arr) {
    if(arr.size() == 4) {
        //draw onto plot
    }
}


void addMagn(String []arr) {
    if(arr.size() == 4) {
        //draw onto plot
    }
}













  // BELOW SHOULD BE MOVED TO PC:

  // Here's a fun function to calculate your heading, using Earth's
  // magnetic field.
  // It only works if the sensor is flat (z-axis normal to Earth).
  // Additionally, you may need to add or subtract a declination
  // angle to get the heading normalized to your location.
  // See: http://www.ngdc.noaa.gov/geomag/declination.shtml
  void printHeading(float hx, float hy) {
    float heading;

    if (hy > 0) {
      heading = 90 - (atan(hx / hy) * (180 / PI));
    }
    else if (hy < 0) {
      heading = - (atan(hx / hy) * (180 / PI));
    }
    else {
      if (hx < 0) heading = 180;
      else heading = 0;
    }

    Serial.print("Heading: ");
    Serial.println(heading, 2);
  }

  // Another fun function that does calculations based on the
  // acclerometer data. This function will print your LSM9DS0's
  // orientation -- it's roll and pitch angles.
  void printOrientation(float x, float y, float z) {
    float pitch, roll;

    pitch = atan2(x, sqrt(y * y) + (z * z));
    roll = atan2(y, sqrt(x * x) + (z * z));
    pitch *= 180.0 / PI;
    roll *= 180.0 / PI;

    Serial.print("Pitch, Roll: ");
    Serial.print(pitch, 2);
    Serial.print(", ");
    Serial.println(roll, 2);
  }