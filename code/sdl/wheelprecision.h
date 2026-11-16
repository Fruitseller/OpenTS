/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <cstdint>


/*
 * A trackpad or a Magic Mouse reports fractions of a notch, while a notched wheel reports
 * whole numbers. SDL exposes no flag for the device, so the deltas are the only evidence. A
 * device that reported a fraction stays classified as precise for a short hold after it,
 * because a trackpad that slows to a stop reports whole numbers.
 */

// How long a precise device stays classified after its last fractional delta.
constexpr unsigned int WHEEL_PRECISE_HOLD_MS = 150;


class WheelPrecisionClass
{
	public:
		// The deltas are taken before any direction correction; now is in milliseconds.
		bool Classify(float delta_x, float delta_y, std::uint64_t now);

	private:
		bool IsPrecise = false;
		std::uint64_t LastFractional = 0;
};
