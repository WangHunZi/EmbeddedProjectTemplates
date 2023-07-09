#include "lvgl.h"
#include "tim.h"

static uint16_t count = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        if (count % 3 == 0) {
            lv_tick_inc(1);
        }
        if (count == 5000) {
            HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
            count = 0;
        }
        count++;
    }
}
