#ifndef TENSORFLOW_LITE_EXPERIMENTAL_MICRO_EXAMPLES_MAGIC_WAND_OUTPUT_HANDLER_H_
#define TENSORFLOW_LITE_EXPERIMENTAL_MICRO_EXAMPLES_MAGIC_WAND_OUTPUT_HANDLER_H_

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"

void HandleOutput(tflite::ErrorReporter* error_reporter, int kind);

#endif  // TENSORFLOW_LITE_EXPERIMENTAL_MICRO_EXAMPLES_MAGIC_WAND_OUTPUT_HANDLER_H_