/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "sdl/wheelprecision.h"

#include <cmath>


/// <summary>
/// Classifies the wheel event that reported these deltas at this time.
/// </summary>
/// <param name="delta_x">The horizontal delta, before any direction correction.</param>
/// <param name="delta_y">The vertical delta, before any direction correction.</param>
/// <param name="now">The current time in milliseconds.</param>
/// <returns>bool; Did the event come from a precise device?</returns>
bool WheelPrecisionClass::Classify(float delta_x, float delta_y, std::uint64_t now)
{
	bool const fractional = delta_x != std::trunc(delta_x) || delta_y != std::trunc(delta_y);

	if (fractional) {
		IsPrecise = true;
		LastFractional = now;
	} else if (IsPrecise && now - LastFractional > WHEEL_PRECISE_HOLD_MS) {
		IsPrecise = false;
	}

	return(IsPrecise);
}
