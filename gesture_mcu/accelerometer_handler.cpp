#include <Arduino.h>
#include "LSM6DS3.h"

#include "accelerometer_handler.h"

int begin_index;                    // input 배열에 넣을 index
int x,y,z;                          // 가속도값 x축, y축, z축 담을 변수

LSM6DS3 myIMU(I2C_MODE, 0x6A);      // Create a instance of class LSM6DS3

TfLiteStatus SetupAccelerometer(tflite::ErrorReporter* error_reporter) {
  // 시리얼 포트가 준비될 때까지 대기
  while (!Serial);

  // IMU 활성화
  if (myIMU.begin() != 0) {
    error_reporter->Report("Failed to initialize IMU");
    return kTfLiteError;
  }
  
  myIMU.settings.accelSampleRate = 52;

  float sample_rate = myIMU.settings.accelSampleRate;
  Serial.println(sample_rate);
  error_reporter->Report("Gesture starts!");
  
  return kTfLiteOk;
}

bool ReadAccelerometer(tflite::ErrorReporter* error_reporter, float* input, int length) {
  begin_index = 0;

  // length = 384, 128개의 데이터 받기
  for(int a = 0; a < length/3; a++) {
    if(!read_IMU_Acceleration(x,y,z)) {
      error_reporter->Report("Failed to read data");
      return false;
    }

    input[begin_index++] = x;
    input[begin_index++] = y;
    input[begin_index++] = z;
    delay(5);
  }

/*
  int k = 0;
  int l = 0;

  for (int j = 0; j < 127; j++) {
      Serial.print(k++);
      Serial.print(" ");
      Serial.print(input[l++]);
      Serial.print(",");
      Serial.print(input[l++]);
      Serial.print(",");
      Serial.println(input[l++]);
  }
*/
  return true;
}

bool read_IMU_Acceleration(int& x, int& y, int& z) {
  int16_t data_x;
  int16_t data_y;
  int16_t data_z;

  if (myIMU.readRegisterInt16(&data_x, LSM6DS3_ACC_GYRO_OUTX_L_XL)) {
    x = y = z = 0;
    return false;
  }

  myIMU.readRegisterInt16(&data_y, LSM6DS3_ACC_GYRO_OUTY_L_XL);
  myIMU.readRegisterInt16(&data_z, LSM6DS3_ACC_GYRO_OUTZ_L_XL);

  x = (int)(data_x * 0.061 * (myIMU.settings.accelRange >> 1));
  y = (int)(data_y * 0.061 * (myIMU.settings.accelRange >> 1));
  z = (int)(data_z * 0.061 * (myIMU.settings.accelRange >> 1));

  return true;
}