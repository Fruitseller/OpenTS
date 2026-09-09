/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#ifndef _ReturnAddress
#define _ReturnAddress() __builtin_return_address(0)
#endif

#ifndef _AddressOfReturnAddress
#define _AddressOfReturnAddress() __builtin_frame_address(0)
#endif
