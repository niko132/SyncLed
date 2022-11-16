#ifndef ESP_TEST_H
#define ESP_TEST_H

#include <unity.h>

#define ESP_TEST_BEGIN() EspTestBegin(__FILE__)
#define ESP_TEST_END() UNITY_END()

void EspTestBegin(const char* filename);
void checkReset();

#endif