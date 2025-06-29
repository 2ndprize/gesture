#include "gesture_predictor.h"
#include <Arduino.h>

// 0: left, 1: right, 2: circle, 3: unknown
int PredictGesture(float* output) {
  int this_predict = 3;        // 예측 결과 index
  // 확률이 0.8보다 큰 출력을 탐색 (전체 확률의 합은 1)
  for (int i = 0; i < 3; i++) {
    if (output[i] > 0.8) this_predict = i;
  }

   return this_predict;
}