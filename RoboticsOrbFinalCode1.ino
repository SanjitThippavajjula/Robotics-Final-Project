#include "I2Cdev.h"
#include <Adafruit_NeoPixel.h>
#include "MPU6050_6Axis_MotionApps20.h"

#define NUMPIXELS 8
#define LED_PIN 13
#define OUTPUT_READABLE_EULER

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, 4, NEO_RGB + NEO_KHZ800);

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

MPU6050 mpu;
bool dmpReady = false;
uint8_t devStatus;
uint16_t packetSize;
uint16_t fifoCount;
uint8_t fifoBuffer[64];

Quaternion q;
float euler[3];

void setup() {
    pixels.begin();
    for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(50, 50, 50)); // Initial white
    }
    pixels.show();

    
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        TWBR = 24;  
    #endif

    Serial.begin(38400);

    
    mpu.initialize();

    if (!mpu.testConnection()) {
        Serial.println(F("MPU6050 connection failed"));
        while (1);
    }

    
    devStatus = mpu.dmpInitialize();

    
    mpu.setXGyroOffset(220);
    mpu.setYGyroOffset(76);
    mpu.setZGyroOffset(-85);
    mpu.setZAccelOffset(1788);

    if (devStatus == 0) {
        mpu.setDMPEnabled(true);
        dmpReady = true;
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }

    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    if (!dmpReady) return;

    fifoCount = mpu.getFIFOCount();

    
    if (fifoCount == 1024) {
        mpu.resetFIFO();
        Serial.println(F("FIFO overflow! Resetting..."));
        return;
    }


    if (fifoCount >= packetSize) {
        
        while (fifoCount >= packetSize) {
            mpu.getFIFOBytes(fifoBuffer, packetSize);
            fifoCount -= packetSize;
        }

        #ifdef OUTPUT_READABLE_EULER
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetEuler(euler, &q);

            byte R = abs(euler[0]) * 255 / M_PI;
            byte G = abs(euler[1]) * 511 / M_PI;
            byte B = 255 - (abs(euler[2]) * 255 / M_PI);

            byte cycl = (millis() / 100) % 8;
            for (byte i = 0; i < NUMPIXELS; i++) {
                pixels.setPixelColor((i + cycl) % 8, pixels.Color(
                    constrain(R + 4 * i, 0, 255),
                    constrain(B + 5 * i, 0, 255),
                    constrain(G - 6 * i, 0, 255)
                ));
            }
            pixels.show();

            Serial.print("Euler\t");
            Serial.print(euler[0] * 180 / M_PI);
            Serial.print("\t");
            Serial.print(euler[1] * 180 / M_PI);
            Serial.print("\t");
            Serial.print(euler[2] * 180 / M_PI);
            Serial.print("\tRGB\t");
            Serial.print(R);
            Serial.print("\t");
            Serial.print(G);
            Serial.print("\t");
            Serial.println(B);
        #endif
    }

    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 500) {
        lastBlink = millis();
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
}
