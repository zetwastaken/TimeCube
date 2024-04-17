

// int whichWallStationary(gptimer_handle_t gptimer) {
//   u_int8_t wallPositionTab[5] = {0, 0, 0, 0, 0};
//   readAccel(&AccelX, &AccelY, &AccelZ);

//   u_int16_t timerValue = getTimerValueMs(gptimer);
//   bool isWallPositionUnchanged = true;

//   if (timerValue > 5000) {
//     wallPositionTab[4] = checkPosition(AccelX, AccelY, AccelZ);
//     resetTimer(gptimer);
//     for (size_t i = 0; i < 5; i++) {
//       if (wallPositionTab[i] != wallPositionTab[0]) {
//         isWallPositionUnchanged = false;
//       }
//     }
//   } else if (timerValue > 4000) {
//     wallPositionTab[3] = checkPosition(AccelX, AccelY, AccelZ);
//     isWallPositionUnchanged = false;
//   } else if (timerValue > 3000) {
//     wallPositionTab[2] = checkPosition(AccelX, AccelY, AccelZ);
//     isWallPositionUnchanged = false;
//   } else if (timerValue > 2000) {
//     wallPositionTab[1] = checkPosition(AccelX, AccelY, AccelZ);
//     isWallPositionUnchanged = false;
//   } else if (timerValue > 1000) {
//     wallPositionTab[0] = checkPosition(AccelX, AccelY, AccelZ);
//     isWallPositionUnchanged = false;
//   }
// }
