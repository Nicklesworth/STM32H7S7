/* SPDX-License-Identifier: Unlicense */

#ifndef TIMEBASE_H_
#define TIMEBASE_H_

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "common.h"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------
#define TICKS_PER_US				1
#define TICKS_PER_MS				1000
#define TICKS_PER_SEC				1000000

// -----------------------------------------------------------------------------
// Macros
// -----------------------------------------------------------------------------
#define msToTicks(ms)				(uint32_t)(ms * TICKS_PER_MS)
#define usToTicks(us)				(uint32_t)((us * TICKS_PER_MS) / 1000L)
#define ticksToMS(t)				(uint32_t)(t / TICKS_PER_MS)
#define ticksToUS(t)				(uint32_t)(t / TICKS_PER_US)

// -----------------------------------------------------------------------------
// Public Functions
// -----------------------------------------------------------------------------
uint32_t ticks(void);

//------------------------------------------------------------------------------
// Description: Calculates the delta between two timestamps
//     Returns: See above
//      Inputs: Two different timestamps 
//------------------------------------------------------------------------------
static inline int32_t ticksDiff(uint32_t then, uint32_t now)
{
	return(now - then);
}

//------------------------------------------------------------------------------
// Description: Provides the # of ticks that have elapsed w.r.t. a previous timestamp
//     Returns: See above
//      Inputs: The previous timestamp in question
//------------------------------------------------------------------------------
static inline uint32_t ticksElapsed(uint32_t timestamp)
{
	return(ticksDiff(timestamp, ticks()));
}

//------------------------------------------------------------------------------
// Description: Blocking delay by a specified amount of ticks
//     Returns: none
//      Inputs: The amount of ticks to delay 
//------------------------------------------------------------------------------
static inline void ticksDelay(uint32_t t)
{
	uint32_t now = ticks();
	while(ticksElapsed(now) < t);
}

//------------------------------------------------------------------------------
// Description: Blocking delay by a specified amount of milliseconds
//     Returns: none
//      Inputs: The amount of milliseconds to delay 
//------------------------------------------------------------------------------
static inline void delayMS(uint32_t ms)
{
	ticksDelay(msToTicks(ms));
}

//------------------------------------------------------------------------------
// Description: Blocking delay by a specified amount of microseconds
//     Returns: none
//      Inputs: The amount of microseconds to delay 
//------------------------------------------------------------------------------
static inline void delayUS(uint32_t us)
{
	ticksDelay(usToTicks(us));
}

#ifdef __cplusplus
}
#endif

#endif