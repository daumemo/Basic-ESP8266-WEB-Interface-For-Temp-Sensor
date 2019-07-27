#include "si7006.h"
#include <Wire.h>
#include <Arduino.h>

Si7006 TemperatureHumiditySensor;

/******************************************************************************
                             Register an Commands names
******************************************************************************/
#define SI7006_I2C_ADDRESS 0x40
// 0xF5 measures and Temperature and Relative Humidity in No Hold Mode
// 0xE5 measures the same as 0xF5 but in Master Hold Mode
// Change value below according to your needs
#define COMM_MEASURE_RELATIVE_HUMIDITY 0xF5
// 0xF3 measures Temperature in No Hold Mode
// 0xE3 measures the same as 0xF3 but in Master Hold Mode
// Change value below according to your needs
#define COMM_MEASURE_TEMPERATURE 0xF3
// 0xE0 reads Temperature value, which was measured with RH last measurement
#define COMM_READ_TEMPERATURE_FROM_PREVIOUS_RH 0xE0
#define COMM_RESET 0xFE
#define COMM_WRITE_RHT_USER_REGISTER 0xE6
#define COMM_READ_RHT_USER_REGISTER 0xE7
#define COMM_WRITE_HEATER_CONTROL_REGISTER 0x51
#define COMM_READ_HEATER_CONTROL_REGISTER 0x11
//#define COMM_READ_ELECTRONIC_ID_FIRST_BYTE 0xFA
//#define COMM_READ_ELECTRONIC_ID_SECOND_BYTE 0xFC
/*****************************************************************************/

Si7006::Si7006(){
  useTemperatureUnit = "C";
}

void Si7006::writeData(int address, int data)
{
  Wire.beginTransmission(address);
  Wire.write(data);
  Wire.endTransmission();
}

void Si7006::writeData(int address, int data, int data0)
{
  Wire.beginTransmission(address);
  Wire.write(data);
  Wire.write(data0);
  Wire.endTransmission();
}

int Si7006::readDataByte()
{
  //Wire.requestFrom(si7006Adsress, 1);
  return Wire.read();
}

void Si7006::requestForData(int address, int byteCount)
{
  unsigned int timer = 0;
  while((Wire.available() == 0) && (timer < SI7006_I2C_TIMEOUT_MS)){
  Wire.requestFrom(address, byteCount);
  delay(1);
  timer++;
  }
}

double Si7006::calculateTemperature(int data)
{
  double temp;

  if (useTemperatureUnit == "C")
    temp = (double)(175.72*(data)/65536 - 46.85);
  else if (useTemperatureUnit == "K")
    temp = (double)(175.72*(data)/65536 - 46.85 + 273.15);
  else if (useTemperatureUnit == "F")
    temp = (double)((175.72*(data)/65536 - 46.85)*1.8 + 32);
  else temp = 999;

  return temp;
}

double Si7006::measureTemperature(void)
{
  int data;
  writeData(SI7006_I2C_ADDRESS, COMM_MEASURE_TEMPERATURE);
  //delay(12);
  requestForData(SI7006_I2C_ADDRESS, 2);
  data = readDataByte()<<8;
  data |= readDataByte();
  return calculateTemperature(data);
}

double Si7006::readTemperature(void)
{
  int data;
  writeData(SI7006_I2C_ADDRESS, COMM_READ_TEMPERATURE_FROM_PREVIOUS_RH);
  delay(2);
  requestForData(SI7006_I2C_ADDRESS, 2);
  data = readDataByte()<<8;
  data |= readDataByte();
  return (double)(175.72*(data)/65536 - 46.85);
}

double Si7006::measureRelativeHumidity(void)
{
  int data;
  writeData(SI7006_I2C_ADDRESS, COMM_MEASURE_RELATIVE_HUMIDITY);
  //delay(25);
  requestForData(SI7006_I2C_ADDRESS, 2);
  data = readDataByte()<<8;
  data |= readDataByte();
  return (double)(125.0*data/65536 - 6);
}

void Si7006::setTemperatureUnits(String unit)
{
  if((unit == "K") || (unit == "F")) useTemperatureUnit = unit;
  else useTemperatureUnit = "C";
}

int Si7006::readHeaterCurrent(void)
{
  int data;
  writeData(SI7006_I2C_ADDRESS, COMM_READ_HEATER_CONTROL_REGISTER);
  requestForData(SI7006_I2C_ADDRESS, 1);
  data = readDataByte() & 0b00001111; // 4 msb are not needed;
  return data;
}

void Si7006::setHeaterCurrent(int data)
{
  int oldData;
  writeData(SI7006_I2C_ADDRESS, COMM_READ_HEATER_CONTROL_REGISTER);
  requestForData(SI7006_I2C_ADDRESS, 1);
  oldData = readDataByte() & 0b11110000; //clearing used bits;
  data = (data & 0b00001111) | oldData;
  writeData(SI7006_I2C_ADDRESS, COMM_WRITE_HEATER_CONTROL_REGISTER, data);
}

void Si7006::reset(void)
{
  writeData(SI7006_I2C_ADDRESS, COMM_RESET);
}

int Si7006::readFwRevision(void)
{
  int data;
  writeData(SI7006_I2C_ADDRESS, 0x84, 0xB8);
  requestForData(SI7006_I2C_ADDRESS, 1);
  data = readDataByte();
  return data;
}

uint32_t Si7006::readId32MSB(void)
{
  uint32_t data = 0;
  writeData(SI7006_I2C_ADDRESS, 0xFA, 0x0F);
  requestForData(SI7006_I2C_ADDRESS, 4);
  data |= readDataByte();
  data = data<<8;
  data |= readDataByte();
  data = data<<8;
  data |= readDataByte();
  data = data<<8;
  data |= readDataByte();

  return data;
}

uint32_t Si7006::readId32LSB(void)
{
  uint32_t data = 0;
  writeData(SI7006_I2C_ADDRESS, 0xFC, 0xC9);
  requestForData(SI7006_I2C_ADDRESS, 4);
  data |= readDataByte();
  data = data<<8;
  data |= readDataByte();
  data = data<<8;
  data |= readDataByte();
  data = data<<8;
  data |= readDataByte();

  return data;
}

void Si7006::setHeaterState(bool state)
{
  int data;
  writeData(SI7006_I2C_ADDRESS, COMM_READ_RHT_USER_REGISTER);
  requestForData(SI7006_I2C_ADDRESS, 1);
  data = readDataByte();
  if(state == true)
  {
    data |= (1<<2); //turn on heater;
  }
  else
  {
    data &=~(1<<2); //turn heater off;
  }
  writeData(SI7006_I2C_ADDRESS, COMM_WRITE_RHT_USER_REGISTER, data);
}

bool Si7006::isVddsStatusOk(void)
{
  int data;
  writeData(SI7006_I2C_ADDRESS, COMM_READ_RHT_USER_REGISTER);
  requestForData(SI7006_I2C_ADDRESS, 1);
  data = readDataByte() & 0b01000000;
  if (data > 0) return false;
  else return true;
}

int Si7006::readMeasurementResolutionBits(void)
{
  int data;
  writeData(SI7006_I2C_ADDRESS, COMM_READ_RHT_USER_REGISTER);
  requestForData(SI7006_I2C_ADDRESS, 1);
  data = readDataByte() & 0b10000001;
  data = (data >> 6) | (data & 0b1);
  return data;
}

void Si7006::writeMeasurementResolutionBits(int bits)
{
  int data;
  writeData(SI7006_I2C_ADDRESS, COMM_READ_RHT_USER_REGISTER);
  requestForData(SI7006_I2C_ADDRESS, 1);
  data = readDataByte();
  bits = ((bits & 0b10) << 6) | (bits & 0b1);
  data = (data & 0b01111110) | bits;
  writeData(SI7006_I2C_ADDRESS, COMM_WRITE_RHT_USER_REGISTER, data);
}
