#include <EspTest.h>
#include <Arduino.h>

void setUp() {

}

void tearDown() {

}

void test_math_is_correct() {
    TEST_ASSERT_TRUE(3 - 2 == 1);
    TEST_ASSERT_TRUE(5 * 2 == 10);
}

void some_useless_test() {
    TEST_ASSERT_TRUE(true);
}

void setup() {
    ESP_TEST_BEGIN();
    RUN_TEST(test_math_is_correct);
    RUN_TEST(some_useless_test);
    ESP_TEST_END();
}

void loop() {
    
}