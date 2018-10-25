//
// Created by Jocelyn Girard on 25/10/2018.
//

#include <Adafruit_TSL2591.h>
#include <ArduinoJson.h>

Adafruit_TSL2591 luxMeter = Adafruit_TSL2591(2591);

int incomingByte = 0;

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        // wait serial port initialization
    }

    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["a"] = "i";
    root["b"] = 0;
    root["v"] = "0.1.0";
    root["n"] = "Pose";
    Serial.print("^");
    root.printTo(Serial);
    Serial.println();
}

void readSerialCommand() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');

        const size_t bufferSize = JSON_OBJECT_SIZE(3) + 30;
        DynamicJsonBuffer jsonBuffer(bufferSize);

        JsonObject& root = jsonBuffer.parseObject(command);

        // {"a": "cs", "g": "48", "t": "3"}
        const char* action = root["a"];
        int gain = root["g"];
        int timing = root["t"];

        if (strcmp ("s", action) == 0) {

            if (gain != (int) luxMeter.getGain()) {
                switch (gain) {
                    case TSL2591_GAIN_LOW:
                    case TSL2591_GAIN_MED:
                    case TSL2591_GAIN_HIGH:
                    case TSL2591_GAIN_MAX:
                        luxMeter.setGain((tsl2591Gain_t) gain);
                        break;
                    default:
                        luxMeter.setGain(TSL2591_GAIN_MED);
                        break;
                }
            }

            if (timing != (int) luxMeter.getTiming()) {
                switch (timing) {
                    case TSL2591_INTEGRATIONTIME_100MS:
                    case TSL2591_INTEGRATIONTIME_200MS:
                    case TSL2591_INTEGRATIONTIME_300MS:
                    case TSL2591_INTEGRATIONTIME_400MS:
                    case TSL2591_INTEGRATIONTIME_500MS:
                    case TSL2591_INTEGRATIONTIME_600MS:
                        luxMeter.setTiming((tsl2591IntegrationTime_t) timing);
                        break;
                    default:
                        luxMeter.setTiming(TSL2591_INTEGRATIONTIME_300MS);
                        break;
                }
            }

            JsonObject& rootNewSettings = jsonBuffer.createObject();
            rootNewSettings["a"] = "c";
            rootNewSettings["g"] = (int) luxMeter.getGain();
            rootNewSettings["t"] = (int) luxMeter.getTiming();
            Serial.print("^");
            rootNewSettings.printTo(Serial);
            Serial.println();
        }
    }
}

void loop() {
    uint32_t lum = luxMeter.getFullLuminosity();
    uint16_t ir, full;
    ir = lum >> 16;
    full = lum & 0xFFFF;
    float lux = luxMeter.calculateLux(full, ir);

    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["a"] = (lux >= 0.000118 && lux <= 88000.0) ? "d" : "e";
    root["L"] = lux;
    root["i"] = ir;
    root["f"] = full;
    root["l"] = lum;
    Serial.print("^");
    root.printTo(Serial);
    Serial.println();

    readSerialCommand();
}
