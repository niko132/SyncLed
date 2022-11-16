#include "EspTest.h"
#include <user_interface.h>
#include <Esp.h>

void EspTestBegin(const char* filename)
{
    delay(100);
    checkReset();
    UnityBegin(filename);
}

// auto restart at flashing to submit the test results
void checkReset() {
    rst_info *resetInfo = ESP.getResetInfoPtr();
    if (resetInfo->reason == 0) {
        ESP.restart();
    }
}