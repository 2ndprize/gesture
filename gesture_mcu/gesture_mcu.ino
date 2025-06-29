#include <TensorFlowLite.h>

#include "main_functions.h"

#include "gesture_model_data.h"
#include "accelerometer_handler.h"
#include "gesture_predictor.h"
#include "output_handler.h"
#include "bleuart.h"

#include "tensorflow/lite/experimental/micro/kernels/micro_ops.h"             // 필요한 연산자(Ops)만 등록하기 위한 헤더파일
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"          // 디버깅을 위해 오류와 출력을 기록하는 헤더파일
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"             // 모델을 실행할 텐서플로 라이트
#include "tensorflow/lite/experimental/micro/micro_mutable_op_resolver.h"     // 선택적 연산자 등록을 위한 클래스 제공
#include "tensorflow/lite/schema/schema_generated.h"                          // 모델 데이터를 이해하는데 사용되는 tensorflowlite 플랫버퍼 데이터 구조를 정의하는 스키마
#include "tensorflow/lite/version.h"                                          // 스키마의 현재 버전 번호, 모델이 호환가능한 버전으로 정의되어 있는지 확인할 수 있음

// 전역 변수 : 아두이노 스타일 스케치와의 호환성에 필요
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* model_input = nullptr;
int input_length;

// 입력, 출력, 중간 배열에 사용할 메모리 영역을 생성
// 배열의 크기는 사용 중인 모델에 따라 다르며, 실험을 통해 결정해야 할 수도 있다.
constexpr int kTensorArenaSize = 60 * 1024;
uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

int interruptPin = A0;                    // External Interrupt를 위한 버튼
volatile bool buttonPressed = false;
unsigned long lastDebounceTime = 0;       // 마지막 상태 변경 시간
const unsigned long debounceDelay = 75;   // 디바운싱 지연(ms)

void setup() {
  // 로깅 설정, 구글의 스타일은 변수의 수명(lifetime)이 언제 끝나는지 명확하지 않아서 프로그램의 동작이 예측하기 어려울 수 있기 때문에
  // 전역 또는 정적 변수를 피하지만, 이 경우에는 소멸자가 있으므로 괜찮다.
  static tflite::MicroErrorReporter micro_error_reporter;  // NOLINT, Lint 도구의 특정 경고를 무시하도록 지시하는 주석
  error_reporter = &micro_error_reporter;
  
  // 모델을 사용 가능한 데이터 구조에 매핑함
  // 복사나 파싱(parsing, 문자열이나 코드 같은 데이터를 해석하여, 특정 구조로 변환하는 과정)을 포함하지 않는 가벼운 작업
  model = tflite::GetModel(g_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report(
        "Model provided is schema version %d not equal "
        "to supported version %d.",
        model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }
  
  // 필요한 Op 구현만 가져옴
  // AllOpsResolver를 사용하면 더 쉽지만, 코드 공간과 메모리를 절약하기 위해 이 방법을 사용
  static tflite::MicroMutableOpResolver micro_mutable_op_resolver;  // NOLINT
  micro_mutable_op_resolver.AddBuiltin(
      tflite::BuiltinOperator_DEPTHWISE_CONV_2D,
      tflite::ops::micro::Register_DEPTHWISE_CONV_2D());
  micro_mutable_op_resolver.AddBuiltin(
      tflite::BuiltinOperator_MAX_POOL_2D,
      tflite::ops::micro::Register_MAX_POOL_2D());
  micro_mutable_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D,
                                       tflite::ops::micro::Register_CONV_2D());
  micro_mutable_op_resolver.AddBuiltin(
      tflite::BuiltinOperator_FULLY_CONNECTED,
      tflite::ops::micro::Register_FULLY_CONNECTED());
  micro_mutable_op_resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX,
                                       tflite::ops::micro::Register_SOFTMAX());

  // 모델을 실행하기 위해 인터프리터를 빌드
  static tflite::MicroInterpreter static_interpreter(
      model, micro_mutable_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // 모델의 텐서를 위해 tensor_arena로 부터 메모리 할당
  interpreter->AllocateTensors();

  // 모델의 입력 텐서에 대한 포인터를 획득
  model_input = interpreter->input(0);
  if ((model_input->dims->size != 4) || (model_input->dims->data[0] != 1) ||
      (model_input->dims->data[1] != 128) ||
      (model_input->dims->data[2] != kChannelNumber) ||
      (model_input->type != kTfLiteFloat32)) {
    error_reporter->Report("Bad input tensor parameters in model");
    return;
  }
  
  input_length = model_input->bytes / sizeof(float);

  TfLiteStatus setup_status = SetupAccelerometer(error_reporter);
  if (setup_status != kTfLiteOk) {
    error_reporter->Report("Set up failed\n");
  }

  //bleuart 초기 설정
  Bluefruit.autoConnLed(true);                                    // 연결 시 BLE LED를 활성화 설정, 연결 가능 상태면 BLUE LED 점멸
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);                   // 최대 대역폭으로 주변 장치 연결 설정, SoftDevice는 더 많은 SRAM이 필요함

  Bluefruit.begin();
  Bluefruit.setTxPower(4);                                        // 지원되는 값은 bluefruit.h에서 확인 가능
  Bluefruit.Periph.setConnectCallback(connect_callback);          // 연결 콜백 설정
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);    // 연결 해제 콜백 설정

  bledfu.begin();                                                 // 일관성을 위해 OTA DFU가 있을 경우 먼저 시작

  bledis.setManufacturer("Adafruit Industries");                  // 제조사 정보 설정 및 장치 정보 서비스 시작
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();

  bleuart.begin();                                                // BLE UART 서비스 시작

  blebas.begin();                                                 // BLE 배터리 서비스 시작
  blebas.write(100);                                              // 배터리 상태 100으로 설정

  startAdv();                                                     // 광고 설정 및 시작

  // External Interrupt 버튼 초기 설정
  pinMode(interruptPin, INPUT_PULLDOWN);
  attachInterrupt(interruptPin, digital_callback, RISING);
}

void loop() {
  if (buttonPressed) {
    buttonPressed = false; // 플래그 초기화

    while (digitalRead(interruptPin)) {
      // 가속도계에서 데이터를 읽음
      bool got_data = ReadAccelerometer(error_reporter, model_input->data.f, input_length);
      if (!got_data) return;                                                    // 만약 새로운 데이터가 없으면, 다음 시간까지 대기
      
      // 추론을 실행, 오류를 보고
      TfLiteStatus invoke_status = interpreter->Invoke();
      if (invoke_status != kTfLiteOk) {
        error_reporter->Report("Invoke failed on index: %d\n", begin_index);
        return;
      }
      
      int gesture_index = PredictGesture(interpreter->output(0)->data.f);       // 결과를 분석하여 예측을 획득
      HandleOutput(error_reporter, gesture_index);                              // 출력을 생성
      bleUART(gesture_index);                                                   // BLE로 송신
    }
  }
}

void digital_callback() {
  unsigned long currentTime = millis();   // millis() : 프로그램이 실행된 후 경과된 시간을 밀리초 단위로 제공

  if ((currentTime - lastDebounceTime) > debounceDelay)
  {
    lastDebounceTime = currentTime;       // 마지막 변경 시간 업데이트
    buttonPressed = true;                 // 플래그 설정
  }
}