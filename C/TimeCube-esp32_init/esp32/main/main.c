
#include "driver/gpio.h"
#include "driver/i2c.h"

#include "checkIfCubeChangedPosition.c"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ic2Initialize.c"
#include "readAndDisplayAccelValues.c"
#include "readAndDisplayTimerValue.c"
#include "variables.c"
#include "wifi_starter.c"
#include <esp_log.h>
#include <stdio.h>

static const char *TAGm = "Accel";

void MMA8452Q_init() {
  init_Ic2_With_Given_Parameters(MMA8452Q_ADDR, 0x2A, 0x00);
  init_Ic2_With_Given_Parameters(MMA8452Q_ADDR, 0x2A, 0x01);
  init_Ic2_With_Given_Parameters(MMA8452Q_ADDR, 0x0E, 0x00);
}

void app_main() {
  main_i2c_init(MMA8452Q_SDA, MMA8452Q_SCL);
  MMA8452Q_init();
  // startWifi();

  gptimer_handle_t gptimer = timer_init();
  gptimer_handle_t gptimerGlobal = timer_init();

  u_int8_t wallPositionTab[5] = {0, 0, 0, 0, 0};
  u_int8_t wallTime1 = 0, wallTime2 = 0, wallTime3 = 0, wallTime4 = 0,
           wallTime5 = 0, wallTime6 = 0;

  int16_t AccelX, AccelY, AccelZ;
  while (1) {
    readAccel(&AccelX, &AccelY, &AccelZ);

    u_int16_t timerValue = getTimerValueMs(gptimer);
    bool isWallPositionUnchanged = true;

    if (timerValue > 5000) {
      wallPositionTab[4] = checkPosition(AccelX, AccelY, AccelZ);
      resetTimer(gptimer);
      for (size_t i = 0; i < 5; i++) {
        if (wallPositionTab[i] != wallPositionTab[0]) {
          isWallPositionUnchanged = false;
        }
      }
    } else if (timerValue > 4000) {
      wallPositionTab[3] = checkPosition(AccelX, AccelY, AccelZ);
      isWallPositionUnchanged = false;
    } else if (timerValue > 3000) {
      wallPositionTab[2] = checkPosition(AccelX, AccelY, AccelZ);
      isWallPositionUnchanged = false;
    } else if (timerValue > 2000) {
      wallPositionTab[1] = checkPosition(AccelX, AccelY, AccelZ);
      isWallPositionUnchanged = false;
    } else if (timerValue > 1000) {
      wallPositionTab[0] = checkPosition(AccelX, AccelY, AccelZ);
      isWallPositionUnchanged = false;
    }

    bool wallSend = false;
    if (isWallPositionUnchanged == true && wallPositionTab[0] != 0) {

      wallSend = true;
      resetTimer(gptimerGlobal);
      if (wallPositionTab[0] == 1) {
        wallTime1 += 5;
      } else if (wallPositionTab[0] == 2) {
        wallTime2 += 5;
      } else if (wallPositionTab[0] == 3) {
        wallTime3 += 5;
      } else if (wallPositionTab[0] == 4) {
        wallTime4 += 5;
      } else if (wallPositionTab[0] == 5) {
        wallTime5 += 5;
      } else if (wallPositionTab[0] == 6) {
        wallTime6 += 5;
      }
      ESP_LOGI(TAGm, "Wall 6 time: %d\n", wallTime6);
      printf("--------------------\n");
    }
    if (wallSend == true) {
      for (size_t i = 0; i < 5; i++) {
        wallPositionTab[i] = 0;
      }
      wallSend = false;
    }
  }
}
