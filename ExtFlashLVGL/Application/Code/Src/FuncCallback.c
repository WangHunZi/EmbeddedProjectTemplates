#include "tim.h"
#include "lvgl.h"

static uint16_t count = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        if (count % 2 == 0) {
            lv_tick_inc(3);
        }
        if (count == 5000) {
            HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
            count = 0;
        }
        count ++;
    }
}
