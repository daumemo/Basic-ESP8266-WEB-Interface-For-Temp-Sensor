/******************************************************************************
This file is Silicon Labs Temp/Humidity sensor's library header file
*****************************************************************************/

#ifndef SI7006_H
#define SI7006_H

#include <Wire.h>
#include <Arduino.h>

#define SI7006_I2C_TIMEOUT_MS 200
//enum temperatureUnits { "K", "C", "F"};

class Si7006
{
  private:
    int readDataByte(void);
    void requestForData(int address, int byteCount);
    void writeData(int address, int data);
    void writeData(int address, int data, int data0);
    double calculateTemperature(int data);
    String useTemperatureUnit;
  public:
    Si7006();
    /**Measures and returns Temperature*/
    double measureTemperature(void);
    /**Gets temperature from last RH measurement*/
    double readTemperature(void);
    /**Measures and returns RH value*/
    double measureRelativeHumidity(void);
    /**Sets Temperature measurement units: string unit "C", "K" or "F"*/
    void setTemperatureUnits(String unit);
    /**Resets IC*/
    void reset(void);
    /**Sets Heater current (4 LSB)*/
    void setHeaterCurrent(int data);
    /**Reads Heater current 4 bits*/
    int readHeaterCurrent(void);
    /**Reads 32 MSB bits of Part ID*/
    uint32_t readId32MSB(void);
    /**Reads 32 LSB bits of Part ID*/
    uint32_t readId32LSB(void);
    /**Reads Firmware Revision number: 0xFF - v1.0; 0x20 - v2.0*/
    int readFwRevision(void);
    /**False - heater off; True - heater on*/
    void setHeaterState(bool state);
    /**Returns True if Vdd is OK, and false if Vdd is too low*/
    bool isVddsStatusOk(void);
    /**Reads Measurement Resolution bits and returns in 0bXX format*/
    int readMeasurementResolutionBits(void);

    /***********************************************************************
      Resolution bits values and measurement resolutions:
      0b00 - 12bit (RH) 14bit (Temp);
      0b01 - 8bit (RH) 12bit (Temp);
      0b10 - 10bit (RH) 13bit (Temp);
      0b11 - 11bit (RH) 11bit (Temp);
    ************************************************************************/
    void writeMeasurementResolutionBits(int bits); //writes lsb to D0 and 2nd lsb bit to D7 of user register;
};

extern Si7006 TemperatureHumiditySensor;

#endif
