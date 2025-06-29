#include "output_handler.h"
#include "Arduino.h"

void HandleOutput(tflite::ErrorReporter* error_reporter, int kind) {
  // 출력 시작할 때, RED LED 점등
  static bool is_initialized = false;
  if (!is_initialized) {
    pinMode(LED_BUILTIN, OUTPUT);
    is_initialized = true;
  }
  // 추론이 수행될 때, LED 점멸
  static int count = 0;
  ++count;
  if (count & 1) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
  // 각 제스처에 대한 결과를 출력
  if (kind == 0) {
    error_reporter->Report("LEFT");
  } else if (kind == 1) {
    error_reporter->Report("RIGHT");
  } else if (kind == 2) {
    error_reporter->Report("CIRCLE");
  } else {
    error_reporter->Report("negative");
  }
}