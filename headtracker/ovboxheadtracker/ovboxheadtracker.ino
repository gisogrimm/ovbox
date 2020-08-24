#include <SLIPEncodedUSBSerial.h>
#include <SLIPEncodedSerial.h>
#include <OSCBoards.h>
#include <OSCData.h>
#include <OSCTiming.h>
#include <OSCMatch.h>
#include <OSCMessage.h>

#include <Wire.h>

#define DEBUG
#include "MPU6050_6Axis_MotionApps20.h"

MPU6050 mpu;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
int16_t ax, ay, az;
int16_t gx, gy, gz;
double gx0 = 0;
double gy0 = 0;
double gz0 = 0;
double rotx = 0;
double roty = 0;
double rotz = 0;
double mrotx = 0;
double mroty = 0;
double mrotz = 0;
double rotscale = 0.061771;
double mrotcoeff = 0.0002;

double rt = 0;
double rtp = 0;
double dt = 0;

bool b_calibrating = false;
bool b_calibinit = true;

uint16_t calib_cnt = 0;


void setup() {
  pinMode(15, INPUT);
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock (200kHz if CPU is 8MHz). Comment this line if having compilation difficulties
  mpu.initialize();
  devStatus = mpu.dmpInitialize();

  // supply your own gyro offsets here, scaled for min sensitivity
  mpu.setXGyroOffset(187);
  mpu.setYGyroOffset(25);
  mpu.setZGyroOffset(-3);
  mpu.setZAccelOffset(1688); // 1688 factory default for my test chip

  // make sure it worked (returns 0 if so)
  if (devStatus == 0) {
    // turn on the DMP, now that it's ready
    mpu.setDMPEnabled(true);
    // set our DMP Ready flag so the main loop() function knows it's okay to use it
    dmpReady = true;
    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
  }
}

void loop() {
  if (!dmpReady) return;
  mpuIntStatus = mpu.getIntStatus();
  // get current FIFO count
  fifoCount = mpu.getFIFOCount();
  // check for overflow (this should never happen unless our code is too inefficient)
  if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
    // reset so we can continue cleanly
    mpu.resetFIFO();
    // otherwise, check for DMP data ready interrupt (this should happen frequently)
  } else if (mpuIntStatus & 0x02) {
    // wait for correct available data length, should be a VERY short wait
    while (fifoCount < packetSize)
      fifoCount = mpu.getFIFOCount();
    // read a packet from FIFO, then clear the buffer
    mpu.getFIFOBytes(fifoBuffer, packetSize);
    // track FIFO count here in case there is > 1 packet available
    // (this lets us immediately read more without waiting for an interrupt)
    fifoCount -= packetSize;
    rt = 0.001 * millis();
    dt = rt-rtp;
    rtp = rt;
    if ( b_calibinit || (digitalRead(15) && (!b_calibrating)) ) {
      b_calibinit = false;
      b_calibrating = true;
      Serial.println("C1");
      calib_cnt = 400;
      mpu.setXGyroOffset(0);
      mpu.setYGyroOffset(0);
      mpu.setZGyroOffset(0);
      gx0 = 0;
      gy0 = 0;
      gz0 = 0;
    }
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    if ( calib_cnt ) {
      gx0 += gx;
      gy0 += gy;
      gz0 += gz;
      calib_cnt--;
      if ( calib_cnt == 0 ) {
        b_calibrating = false;
        gx0 /= 400.0;
        gy0 /= 400.0;
        gz0 /= 400.0;
        rotx = 0;
        roty = 0;
        rotz = 0;
        mrotx = 0;
        mroty = 0;
        mrotz = 0;
        Serial.println("C0");
      }
    } else {
      if( dt > 0 ){
        rotx += (gx-gx0)*dt*rotscale;
        roty += (gy-gy0)*dt*rotscale;
        rotz += (gz-gz0)*dt*rotscale;
        Serial.print('G');
        Serial.print(rotx-mrotx);
        Serial.print(',');
        Serial.print(roty-mroty);
        Serial.print(',');
        Serial.println(rotz-mrotz);
        Serial.print('A');
        Serial.print(ax);
        Serial.print(',');
        Serial.print(ay);
        Serial.print(',');
        Serial.println(az);
        mrotx = (1.0-mrotcoeff)*mrotx+mrotcoeff*rotx;
        mroty = (1.0-mrotcoeff)*mroty+mrotcoeff*roty;
        mrotz = (1.0-mrotcoeff)*mrotz+mrotcoeff*rotz;
      }
    }
  }
}

/*
   Local Variables:
   mode: c++
   c-basic-offset: 2
   End:
*/
