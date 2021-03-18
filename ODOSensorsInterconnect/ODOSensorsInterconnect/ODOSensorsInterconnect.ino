/*
 Name:		ODOSensorsInterconnect.ino
 Created:	2/25/2021 12:02:06 PM
 Author:	hayes
*/

/****************************************************************
 * Example1_Basics.ino
 * ICM 20948 Arduino Library Demo
 * Use the default configuration to stream 9-axis IMU data
 * Owen Lyke @ SparkFun Electronics
 * Original Creation Date: April 17 2019
 *
 * Please see License.md for the license information.
 *
 * Distributed as-is; no warranty is given.
 ***************************************************************/
#include "ICM_20948.h"  // Click here to get the library: http://librarymanager/All#SparkFun_ICM_20948_IMU

#define USE_SPI       // Uncomment this to use SPI

#define SERIAL_PORT Serial

#define SPI_PORT SPI    // Your desired SPI port.       Used only when "USE_SPI" is defined
#define CS_PIN 2     // Which pin you connect CS to. Used only when "USE_SPI" is defined
#define CS_PIN_2 6

#define WIRE_PORT Wire  // Your desired Wire port.      Used when "USE_SPI" is not defined
#define AD0_VAL   1     // The value of the last bit of the I2C address.
 // On the SparkFun 9DoF IMU breakout the default is 1, and when
 // the ADR jumper is closed the value becomes 0

#ifdef USE_SPI
ICM_20948_SPI myICM;  // If using SPI create an ICM_20948_SPI object
ICM_20948_SPI myICM2; 


#else
ICM_20948_I2C myICM;  // Otherwise create an ICM_20948_I2C object
#endif


typedef struct asciihex
{
    char msc;
    char lsc;

}ASCIIHEX;






typedef struct {
    float Acc_X;
    float Acc_Y;
    float Acc_Z;

    float Gyro_X;
    float Gyro_Y;
    float Gyro_Z;

}IMUSensorValues;



typedef struct {

    IMUSensorValues sensor_1;
    IMUSensorValues sensor_2;
   // IMUSensorValues sensor_3;
   // IMUSensorValues sensor_4;
   // IMUSensorValues sensor_5;

} ODODataTransfer;


ODODataTransfer dataToSend;


void setup() {

    SERIAL_PORT.begin(115200);
    while (!SERIAL_PORT) {};

    Serial1.begin(115200);
    while (!Serial1) {};


#ifdef USE_SPI
    SPI_PORT.begin(); // this will initialise the SPI port
#else
    WIRE_PORT.begin();
    WIRE_PORT.setClock(400000);
#endif

    //myICM.enableDebugging(); // Uncomment this line to enable helpful debug messages on Serial

    bool initialized = false;
    while (!initialized) {

#ifdef USE_SPI
        myICM.begin(CS_PIN, SPI_PORT);
#else
        myICM.begin(WIRE_PORT, AD0_VAL);
#endif

        SERIAL_PORT.print(F("Initialization of the sensor returned: "));
        SERIAL_PORT.println(myICM.statusString());
        if (myICM.status != ICM_20948_Stat_Ok) {
            SERIAL_PORT.println("Trying again...");
            delay(500);
        }
        else {
            initialized = true;
            SERIAL_PORT.println("Sensor 1 Initialised");
        }
    }


    //initialising the second IMU
     initialized = false;
    while (!initialized) {

#ifdef USE_SPI
        myICM2.begin(CS_PIN_2, SPI_PORT);
#else
        myICM.begin(WIRE_PORT, AD0_VAL);
#endif

        SERIAL_PORT.print(F("Initialization of the sensor 2 returned: "));
        SERIAL_PORT.println(myICM2.statusString());
        if (myICM2.status != ICM_20948_Stat_Ok) {
            SERIAL_PORT.println("Trying again...");
            delay(500);
        }
        else {
            initialized = true;
            SERIAL_PORT.println("Sensor 2 Initialised");
        }
    }




}

void loop() {


   // digitalWrite(CS_PIN_2, HIGH);
    if (myICM.dataReady()) {
        myICM.getAGMT();                // The values are only updated when you call 'getAGMT'
    //    printRawAGMT( myICM.agmt );     // Uncomment this to see the raw values, taken directly from the agmt structure

        SERIAL_PORT.println("Printing from Sensor 1");

        printScaledAGMT(&myICM);   // This function takes into account the scale settings from when the measurement was made to calculate the values with units
        fillSensorValues(&dataToSend.sensor_1, &myICM);

        delay(30);
    }
    else {
        SERIAL_PORT.println("Waiting for data");
        delay(500);
    }


   // digitalWrite(CS_PIN, HIGH);
    if (myICM2.dataReady()) {
        myICM2.getAGMT();                // The values are only updated when you call 'getAGMT'
       // printRawAGMT( myICM.agmt );     // Uncomment this to see the raw values, taken directly from the agmt structure

        SERIAL_PORT.println("Printing from Sensor 2");

        printScaledAGMT(&myICM2);   // This function takes into account the scale settings from when the measurement was made to calculate the values with units
        fillSensorValues(&dataToSend.sensor_2, &myICM2);
        delay(30);
    }
    else {
        SERIAL_PORT.println("Waiting for data 2");
        delay(500);
    }


    // send the data through serial port
    sendDataToJetson(&dataToSend, sizeof(dataToSend));
    //delay(500);

}


// Below here are some helper functions to print the data nicely!

void printPaddedInt16b(int16_t val) {
    if (val > 0) {
        SERIAL_PORT.print(" ");
        if (val < 10000) { SERIAL_PORT.print("0"); }
        if (val < 1000) { SERIAL_PORT.print("0"); }
        if (val < 100) { SERIAL_PORT.print("0"); }
        if (val < 10) { SERIAL_PORT.print("0"); }
    }
    else {
        SERIAL_PORT.print("-");
        if (abs(val) < 10000) { SERIAL_PORT.print("0"); }
        if (abs(val) < 1000) { SERIAL_PORT.print("0"); }
        if (abs(val) < 100) { SERIAL_PORT.print("0"); }
        if (abs(val) < 10) { SERIAL_PORT.print("0"); }
    }
    SERIAL_PORT.print(abs(val));
}


void sendDataToJetson(ODODataTransfer * dfer, size_t dataSize) {

    #define BUFFER_SIZE 1000

    //uint8_t numbers[] = { 2,4,6,8,254,255 };

    
    void* tempPtr = dfer;
    uint8_t* ptr = (uint8_t*)tempPtr;

    const char* msgStart = "<ROVERODOMSG>";
    const char* msgEnd = "</ROVERODOMSG>";

    //define the buffer to store the message to be sent
    char msgBuf[BUFFER_SIZE];
    char* msgbufptr = msgBuf;
    
    //reset the buffer
    memset(msgbufptr, 0, BUFFER_SIZE);

    memcpy(msgbufptr, msgStart, strlen(msgStart));
    msgbufptr += strlen(msgStart); //point to the next available location in the buffer


    ASCIIHEX tempASCII;

    for (unsigned int i = 0; i < dataSize; i++) {

        tempASCII = hextoascii(*(ptr+i));
        *msgbufptr = tempASCII.msc;
        msgbufptr++;
        *msgbufptr = tempASCII.lsc;
        msgbufptr++;
    }

    memcpy(msgbufptr, msgEnd, strlen(msgEnd));

    Serial1.println(msgBuf); //Print to the serial port connected to Jetson
}



void printRawAGMT(ICM_20948_AGMT_t agmt) {
    SERIAL_PORT.print("RAW. Acc [ ");
    printPaddedInt16b(agmt.acc.axes.x);
    SERIAL_PORT.print(", ");
    printPaddedInt16b(agmt.acc.axes.y);
    SERIAL_PORT.print(", ");
    printPaddedInt16b(agmt.acc.axes.z);
    SERIAL_PORT.print(" ], Gyr [ ");
    printPaddedInt16b(agmt.gyr.axes.x);
    SERIAL_PORT.print(", ");
    printPaddedInt16b(agmt.gyr.axes.y);
    SERIAL_PORT.print(", ");
    printPaddedInt16b(agmt.gyr.axes.z);
    SERIAL_PORT.print(" ], Mag [ ");
    printPaddedInt16b(agmt.mag.axes.x);
    SERIAL_PORT.print(", ");
    printPaddedInt16b(agmt.mag.axes.y);
    SERIAL_PORT.print(", ");
    printPaddedInt16b(agmt.mag.axes.z);
    SERIAL_PORT.print(" ], Tmp [ ");
    printPaddedInt16b(agmt.tmp.val);
    SERIAL_PORT.print(" ]");
    SERIAL_PORT.println();
}

// function to copy sensor values into the structure instant
void fillSensorValues(IMUSensorValues* theSensorValue, ICM_20948_SPI* sensor) {
    
    theSensorValue->Acc_X = sensor->accX();
    theSensorValue->Acc_Y = sensor->accY();
    theSensorValue->Acc_Z = sensor->accZ();
    theSensorValue->Gyro_X = sensor->gyrX();
    theSensorValue->Gyro_Y = sensor->gyrY();
    theSensorValue->Gyro_Z = sensor->gyrZ();

}


void printFormattedFloat(float val, uint8_t leading, uint8_t decimals) {
    float aval = abs(val);
    if (val < 0) {
        SERIAL_PORT.print("-");
    }
    else {
        SERIAL_PORT.print(" ");
    }
    for (uint8_t indi = 0; indi < leading; indi++) {
        uint32_t tenpow = 0;
        if (indi < (leading - 1)) {
            tenpow = 1;
        }
        for (uint8_t c = 0; c < (leading - 1 - indi); c++) {
            tenpow *= 10;
        }
        if (aval < tenpow) {
            SERIAL_PORT.print("0");
        }
        else {
            break;
        }
    }
    if (val < 0) {
        SERIAL_PORT.print(-val, decimals);
    }
    else {
        SERIAL_PORT.print(val, decimals);
    }
}

#ifdef USE_SPI
void printScaledAGMT(ICM_20948_SPI* sensor) {
#else
void printScaledAGMT(ICM_20948_I2C * sensor) {
#endif
    SERIAL_PORT.print("Scaled. Acc (mg) [ ");
    printFormattedFloat(sensor->accX(), 5, 2);
    SERIAL_PORT.print(", ");
    printFormattedFloat(sensor->accY(), 5, 2);
    SERIAL_PORT.print(", ");
    printFormattedFloat(sensor->accZ(), 5, 2);
    SERIAL_PORT.print(" ], Gyr (DPS) [ ");
    printFormattedFloat(sensor->gyrX(), 5, 2);
    SERIAL_PORT.print(", ");
    printFormattedFloat(sensor->gyrY(), 5, 2);
    SERIAL_PORT.print(", ");
    printFormattedFloat(sensor->gyrZ(), 5, 2);
    SERIAL_PORT.print(" ], Mag (uT) [ ");
    printFormattedFloat(sensor->magX(), 5, 2);
    SERIAL_PORT.print(", ");
    printFormattedFloat(sensor->magY(), 5, 2);
    SERIAL_PORT.print(", ");
    printFormattedFloat(sensor->magZ(), 5, 2);
    SERIAL_PORT.print(" ], Tmp (C) [ ");
    printFormattedFloat(sensor->temp(), 5, 2);
    SERIAL_PORT.print(" ]");
    SERIAL_PORT.println();
}



uint8_t asciitohex(char mostSigChar, char leastSigChar)
{
    uint8_t num = 0;
    if (mostSigChar >= '0' && mostSigChar <= '9')
    {
        num = mostSigChar - '0';
        num = num * 16;
    }
    else
    {
        num = (mostSigChar - 55);
        num = num * 16;
    }


    if (leastSigChar >= '0' && leastSigChar <= '9')
    {
        num += (leastSigChar - '0');
    }
    else
    {
        num += (leastSigChar - 55);
    }

    return num;
}



ASCIIHEX hextoascii(uint8_t the_byte)
{
    uint8_t msnibble = 0;
    uint8_t lsnibble = 0;
    ASCIIHEX returndata;

    lsnibble = (the_byte & 0x0F);
    msnibble = (the_byte >> 4);
    msnibble = (msnibble & 0x0F);

    if (lsnibble < 10)
    {
        (returndata.lsc) = ('0' + lsnibble);
    }
    else
    {
        (returndata.lsc) = ('A' - 10 + lsnibble);
    }

    if (msnibble < 10)
    {
        (returndata.msc) = ('0' + msnibble);
    }
    else
    {
        (returndata.msc) = ('A' - 10 + msnibble);
    }

    return returndata;
}