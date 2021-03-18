#!/usr/bin/python3
import time
import serial
import binascii
import struct

msgStart = '<ROVERODOMSG>'
msgEnd = '</ROVERODOMSG>'


#def asciiToHex(mostSigChar, leastSigChar):
    



def checkForMessage(data):
    # check for the start of the message
    index1 = data.find('<ROVERODOMSG>')
    if index1 < 0:
        return False
        
    # check for  the end of the message
    index2 = data.find('</ROVERODOMSG>')
    if index2 < 0:
        return False
    
    return True

def decodeIMUValues(data):
    # the data from each sensor containes 6x floating point values
    # each float contains 4 bytes
    
    IMUValueList = []
    
    # remove the message headers
    index1 = data.find(msgStart)
    index2 = data.find(msgEnd)
  
    if index1 < 0 or index2 <0:
        return IMUValueList
  
    data = data[index1 + len(msgStart) : index2]
    
    # convert ascii HEX to actual number
    print('Decoded Data: ' + data)
    dataBytes = binascii.unhexlify(data)
    
    bytesPerSensor = 4*6    
    for itr in range(0,2):
    
        # accelerometer data
        s_ACC_X = struct.unpack('f', dataBytes[0+itr*bytesPerSensor:4+itr*bytesPerSensor])
        #print(s_ACC_X)
        
        s_ACC_Y = struct.unpack('f', dataBytes[4 + itr*bytesPerSensor : 8+itr*bytesPerSensor])
        #print(s_ACC_Y)
        
        s_ACC_Z = struct.unpack('f', dataBytes[8 + itr*bytesPerSensor : 12+itr*bytesPerSensor])
        #print(s_ACC_Z)

        # gyro data
        s_GYRO_X = struct.unpack('f', dataBytes[12+itr*bytesPerSensor:16+itr*bytesPerSensor])
        #print(s_GYRO_X)
        
        s_GYRO_Y = struct.unpack('f', dataBytes[16 + itr*bytesPerSensor : 20+itr*bytesPerSensor])
        #print(s_GYRO_Y)
        
        s_GYRO_Z = struct.unpack('f', dataBytes[20 + itr*bytesPerSensor : 24+itr*bytesPerSensor])
        #print(s_GYRO_Z)
    
        sensorCombined = [s_ACC_X, s_ACC_Y, s_ACC_Z, s_GYRO_X, s_GYRO_Y, s_GYRO_Z]
        IMUValueList.append(sensorCombined)
    
    return IMUValueList
    

if __name__ == "__main__":
   
    print("UART Demonstration Program")
    print("NVIDIA Jetson Nano Developer Kit")


    serial_port = serial.Serial(
        port="/dev/ttyTHS1",
        baudrate=115200,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
    )
    # Wait a second to let the port initialize
    time.sleep(1)

    try:
        # Send a simple header
        serial_port.write("UART Demonstration Program\r\n".encode())
        serial_port.write("NVIDIA Jetson Nano Developer Kit\r\n".encode())
        data = ""
        while True:
            if serial_port.inWaiting() > 0:
                ch = serial_port.read().decode("utf-8")
                data = data + ch
                
                #serial_port.write(data)
                # if we get a carriage return, add a line feed too
                # \r is a carriage return; \n is a line feed
                # This is to help the tty program on the other end 
                # Windows is \r\n for carriage return, line feed
                # Macintosh and Linux use \n
                if ch == "\r":
                    # For Windows boxen on the other end              
                    print("Data: " + data)
                    #check if the data has valid format
                    isValid = checkForMessage(data)
                    print(isValid)
                    if isValid == True:
                        # store returned IMU data
                        IMUValues = decodeIMUValues(data)
                        
                        print("--------Sensor 1 Data--------")
                        sensor1Values = IMUValues[0]
                        print("Acceleration Data (X,Y,Z)")
                        print(sensor1Values[0:3])
                        
                        print("Magnetometer Data (X,Y,Z)")
                        print(sensor1Values[3:])
                        
                        
                        print("--------Sensor 2 Data--------")
                        sensor2Values = IMUValues[1]
                        print("Acceleration Data (X,Y,Z)")
                        print(sensor2Values[0:3])
                        
                        print("Magnetometer Data (X,Y,Z)")
                        print(sensor2Values[3:])
                        print("\n\n\n\n\n\n")
    
                    data=""


    except KeyboardInterrupt:
        print("Exiting Program")

    except Exception as exception_error:
        print("Error occurred. Exiting Program")
        print("Error: " + str(exception_error))

    finally:
        serial_port.close()
        pass
