/* SPDX-License-Identifier: Unlicense */

// -----------------------------------------------------------------------------
// IMPLEMENTATION NOTES
// 
// A 16-bit basic timer is initialized where 1LSB = 1us
// It interrupts on the natural 16-bit rollover (every 65.536 ms)
// During the interrupt a 32-bit software counter is incremented, creating a 48-bit timer.
// The "ticker" system utilizes the lower 32-bits, offering 1us timing
//  resolution with a rollover at 4294.967 sec rollover, or a 2147.483 sec "event window"
// The STM32 Cube Libraries ("HAL") expect a 1ms tick, so HAL_GetTick() offers
//  the upper 32-bits. There's a slight inaccuracy here as this bit position will
//  actually represent 1.024ms. But we'd rather have a slightly longer tick for
//  the STM32 "HAL" than a less accurate 1us for our more critical timing
//
// -----------------------------------------------------------------------------

/* Includes ------------------------------------------------------------------*/
#include "stm32h7rsxx_hal.h"

/** @addtogroup STM32H7RSxx_HAL_Driver
  * @{
  */

/** @addtogroup HAL_TimeBase
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define TIM_CNT_FREQ 1000000U   /* Timer counter frequency : 1 MHz */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static TIM_HandleTypeDef TimHandle;
volatile uint32_t ticks_upper;

/* Private function prototypes -----------------------------------------------*/
void TIM6_IRQHandler(void);
#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1U)
void TimeBase_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
#endif /* USE_HAL_TIM_REGISTER_CALLBACKS */
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  This function configures the TIM6 as a time base source.
  *         The time source is configured  to have 1ms time base with a dedicated
  *         Tick interrupt priority.
  * @note   This function is called  automatically at the beginning of program after
  *         reset by HAL_Init() or at any time when clock is configured, by HAL_RCC_ClockConfig().
  * @param  TickPriority Tick interrupt priority.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  RCC_ClkInitTypeDef    clkconfig;
  uint32_t              uwTimclock;
  uint32_t              uwAPB1Prescaler;
  uint32_t              uwPrescalerValue;
  uint32_t              pFLatency;
  HAL_StatusTypeDef     Status;

  /* Enable TIM6 clock */
  __HAL_RCC_TIM6_CLK_ENABLE();

  /* Get clock configuration */
  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

  /* Get APB1 prescaler */
  uwAPB1Prescaler = clkconfig.APB1CLKDivider;

  /* Compute TIM6 clock */
  if (uwAPB1Prescaler == RCC_APB1_DIV1)
  {
    uwTimclock = HAL_RCC_GetPCLK1Freq();
  }
  else if (uwAPB1Prescaler == RCC_APB1_DIV2)
  {
    uwTimclock = 2UL * HAL_RCC_GetPCLK1Freq();
  }
  else
  {
    if (__HAL_RCC_GET_TIMCLKPRESCALER() == RCC_TIMPRES_DISABLE)
    {
      uwTimclock = 2UL * HAL_RCC_GetPCLK1Freq();
    }
    else
    {
      uwTimclock = 4UL * HAL_RCC_GetPCLK1Freq();
    }
  }

  /* Compute the prescaler value to have TIM6 counter clock equal to TIM_CNT_FREQ */
  uwPrescalerValue = (uint32_t)((uwTimclock / TIM_CNT_FREQ) - 1U);

  /* Initialize TIM6 */
  TimHandle.Instance = TIM6;

  /* Initialize TIMx peripheral as follow:
  + Period = [uwTickFreq * (TIM_CNT_FREQ/TIM_FREQ) - 1]. to have a (uwTickFreq/TIM_FREQ) s time base.
  + Prescaler = (uwTimclock/TIM_CNT_FREQ - 1) to have a TIM_CNT_FREQ counter clock.
  + ClockDivision = 0
  + Counter direction = Up
  */
  TimHandle.Init.Period = 0xFFFF;
  TimHandle.Init.Prescaler = uwPrescalerValue;
  TimHandle.Init.ClockDivision = 0;
  TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
  TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  Status = HAL_TIM_Base_Init(&TimHandle);
  if (Status == HAL_OK)
  {
#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1U)
    HAL_TIM_RegisterCallback(&TimHandle, HAL_TIM_PERIOD_ELAPSED_CB_ID, TimeBase_TIM_PeriodElapsedCallback);
#endif /* USE_HAL_TIM_REGISTER_CALLBACKS */

    /* Start the TIM time Base generation in interrupt mode */
    Status = HAL_TIM_Base_Start_IT(&TimHandle);
    if (Status == HAL_OK)
    {
      if (TickPriority < (1UL << __NVIC_PRIO_BITS))
      {
        /* Configure the TIM6 global Interrupt priority */
        HAL_NVIC_SetPriority(TIM6_IRQn, TickPriority, 0);

        /* Enable the TIM6 global Interrupt */
        HAL_NVIC_EnableIRQ(TIM6_IRQn);

        uwTickPrio = TickPriority;
      }
      else
      {
        Status = HAL_ERROR;
      }
    }
  }

  /* Return function status */
  return Status;
}

/**
  * @brief  Suspend Tick increment.
  * @note   Disable the tick increment by disabling TIM6 update interrupt.
  * @retval None
  */
void HAL_SuspendTick(void)
{
  /* Disable TIM6 update interrupt */
  __HAL_TIM_DISABLE_IT(&TimHandle, TIM_IT_UPDATE);
}

/**
  * @brief  Resume Tick increment.
  * @note   Enable the tick increment by enabling TIM6 update interrupt.
  * @retval None
  */
void HAL_ResumeTick(void)
{
  /* Enable TIM6 update interrupt */
  __HAL_TIM_ENABLE_IT(&TimHandle, TIM_IT_UPDATE);
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM6_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim TIM handle
  * @retval None
  */
#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1U)
void TimeBase_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
#else
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
#endif /* USE_HAL_TIM_REGISTER_CALLBACKS */
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(htim);

  ticks_upper++;
}

/**
  * @brief  This function handles TIM6 interrupt request.
  * @retval None
  */
void TIM6_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&TimHandle);
}

//------------------------------------------------------------------------------
// Description: Provides the raw ticker count (a timestamp)
//     Returns: See above
//      Inputs: none
//------------------------------------------------------------------------------
uint32_t ticks(void)
{
	// Getting L, U, L allows us to detect and account for roll-over happening
	// by an IRQ firing within this routine
	uint32_t lower1 = TimHandle.Instance->CNT;
	uint32_t upper1 = ticks_upper;
	uint32_t lower2 = TimHandle.Instance->CNT;

	// Account for roll-over, creating a 32-bit counter of 1us ticks
	// by taking all 16-bits of the timer register and the lower
	// 16-bits of the soft-counter
	if(lower2 < lower1)
	{
		// Rollover happened, lower2 is post-rollover. Unsure of upper1 validity, re-get.
		return (ticks_upper << 16)+(lower2 & 0xFFFF);
	}
	else
	{
		// No rollover
		return (upper1 << 16)+(lower2 & 0xFFFF);
	}
}

// -----------------------------------------------------------------------------
// Description: Provides a tick value in millisecond.
//     Returns: none
//      Inputs: The one millisecond tick count
// -----------------------------------------------------------------------------
uint32_t HAL_GetTick(void)
{
	// Getting L, U, L allows us to detect and account for roll-over happening
	// by an IRQ firing within this routine
	uint32_t lower1 = TimHandle.Instance->CNT;
	uint32_t upper1 = ticks_upper;
	uint32_t lower2 = TimHandle.Instance->CNT;

	// Account for roll-over, creating a 32-bit counter of ~1ms ticks
	// by taking the upper 6-bits of the timer register and the lower
	// 26-bits of the soft-counter
	if(lower2 < lower1)
	{
		// Rollover happened, lower2 is post-rollover. Unsure of upper1 validity, re-get.
		return (ticks_upper << 6)+( (lower2 >> 10) & 0x003F);
	}
	else
	{
		// No rollover
		return (upper1 << 6)+( (lower2 >> 10) & 0x003F);
	}
}

/**
  * @}
  */

/**
  * @}
  */
