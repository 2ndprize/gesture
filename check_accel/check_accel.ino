#include "LSM6DS3.h"

int interruptPin = A0;
volatile bool buttonPressed = false;
unsigned long lastDebounceTime = 0;  // 마지막 상태 변경 시간
const unsigned long debounceDelay = 75; // 디바운싱 지연(ms)
int x, y, z;
int cnt = 0;

LSM6DS3 myIMU(I2C_MODE, 0x6A); // I2C를 사용하여 LSM6DS3 객체 생성

void setup() {
  Serial.begin(9600);

  if (myIMU.begin() != 0) {
    Serial.println("LSM6DS3 실패");
    while (1);
  }
/*
  // FIFO 설정
  myIMU.writeRegister(LSM6DS3_ACC_GYRO_FIFO_CTRL1, 0x00);  // FIFO 임계치 설정 (512개 데이터)
  myIMU.writeRegister(LSM6DS3_ACC_GYRO_FIFO_CTRL2, 0x02);  // 상위 4비트는 0으로 설정
  myIMU.writeRegister(LSM6DS3_ACC_GYRO_FIFO_CTRL3, 0x01);  // Gyro센서는 FIFO X, Accel센서만 FIFO 사용 (No decimation)
  myIMU.writeRegister(LSM6DS3_ACC_GYRO_FIFO_CTRL5, 0x36);  // FIFO 모드 설정 (Continuous Mode), ODR 416Hz
  
  myIMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL1_XL, 0x30); // ODR(Output Data Rate) 52Hz 설정, 0011 : 52Hz
  myIMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL3_C, 0x44);  // FIFO data output register를 읽기 위해 BDU를 1로 설정
  myIMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL4_C, 0x01);  // FIFO threshold level 사용
*/
  myIMU.settings.accelSampleRate = 52;

  float sample_rate = myIMU.settings.accelSampleRate;
  Serial.println(sample_rate);

  pinMode(interruptPin, INPUT_PULLDOWN);
  attachInterrupt(interruptPin, digital_callback, RISING);
}

void loop() {
  if (buttonPressed) {
    buttonPressed = false; // 플래그 초기화

    while (digitalRead(interruptPin)) {
    read_IMU_Acceleration();
    }

  }

  cnt = 0;
}

void digital_callback() {
  unsigned long currentTime = millis();  // millis() : 프로그램이 실행된 후 경과된 시간을 밀리초 단위로 제공

  if ((currentTime - lastDebounceTime) > debounceDelay)
  {
    lastDebounceTime = currentTime; // 마지막 변경 시간 업데이트
    buttonPressed = true;          // 플래그 설정
  }
}

int read_IMU_Acceleration() {
  int16_t data_x;
  int16_t data_y;
  int16_t data_z;

  if (myIMU.readRegisterInt16(&data_x, LSM6DS3_ACC_GYRO_OUTX_L_XL)) {
    x = NAN;
    y = NAN;
    z = NAN;

    return 0;
  }

  myIMU.readRegisterInt16(&data_y, LSM6DS3_ACC_GYRO_OUTY_L_XL);
  myIMU.readRegisterInt16(&data_z, LSM6DS3_ACC_GYRO_OUTZ_L_XL);

  x = (int)(data_x * 0.061 * (myIMU.settings.accelRange >> 1));
  y = (int)(data_y * 0.061 * (myIMU.settings.accelRange >> 1));
  z = (int)(data_z * 0.061 * (myIMU.settings.accelRange >> 1));

  //Serial.print(cnt++);
  Serial.print(x);
  Serial.print(",");
  Serial.print(y);
  Serial.print(",");
  Serial.println(z);
  
  delay(8);

  return 1;
}
