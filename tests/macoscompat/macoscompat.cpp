/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Pins the Win32 replacements the macOS build calls in place of the Windows API.

#include "wincompat.h"
#include "isotype.h"

#include "doctest/doctest.h"

#include <cstdlib>
#include <cstring>
#include <string>

// IsoTileSet keeps its fields for the engine's loaders; only this name may read them.
struct IsoTileSetTestAccess
{
	static void Check_Empty_Set(void)
	{
		IsoTileSet tile_set;
		tile_set.MapWidth = 0;
		tile_set.MapHeight = 0;
		tile_set.Width = 0;
		tile_set.Height = 0;
		tile_set.TileOffsets[0] = 0;

		IsoTileSet const & view = tile_set;
		CHECK(view.Tile_Count() == 0);
		CHECK(view.Fetch_Record_Pointer(0) == nullptr);
	}

	static void Check_Absent_Record(void)
	{
		IsoTileSet tile_set;
		tile_set.MapWidth = 1;
		tile_set.MapHeight = 1;
		tile_set.Width = 60;
		tile_set.Height = 30;
		tile_set.TileOffsets[0] = 0;

		IsoTileSet const & view = tile_set;
		CHECK(view.Tile_Count() == 1);
		CHECK(view.Fetch_Record_Pointer(0) == nullptr);
	}
};


namespace
{

// Sets TZ for one case and restores what the process had when the case ends.
class ScopedTimeZone
{
	public:
		explicit ScopedTimeZone(char const * zone)
		{
			char const * previous = std::getenv("TZ");
			HadPrevious = (previous != nullptr);
			if (HadPrevious) {
				Previous = previous;
			}
			setenv("TZ", zone, 1);
			tzset();
		}

		~ScopedTimeZone(void)
		{
			if (HadPrevious) {
				setenv("TZ", Previous.c_str(), 1);
			} else {
				unsetenv("TZ");
			}
			tzset();
		}

		ScopedTimeZone(ScopedTimeZone const &) = delete;
		ScopedTimeZone & operator=(ScopedTimeZone const &) = delete;

	private:
		bool HadPrevious = false;
		std::string Previous;
};


SYSTEMTIME Utc_Time(int year, int month, int day, int hour, int minute)
{
	SYSTEMTIME time = {};
	time.wYear = static_cast<WORD>(year);
	time.wMonth = static_cast<WORD>(month);
	time.wDay = static_cast<WORD>(day);
	time.wHour = static_cast<WORD>(hour);
	time.wMinute = static_cast<WORD>(minute);
	return(time);
}


// The wall clock the shim reports for a UTC instant, read the way the save list reads it.
SYSTEMTIME Local_Wall_Clock(SYSTEMTIME utc)
{
	FILETIME stamp = {};
	REQUIRE(SystemTimeToFileTime(&utc, &stamp));

	FILETIME local = {};
	REQUIRE(FileTimeToLocalFileTime(&stamp, &local));

	SYSTEMTIME wall = {};
	REQUIRE(FileTimeToSystemTime(&local, &wall));
	return(wall);
}

}


TEST_CASE("_makepath completes a missing extension")
{
	char path[MAX_PATH] = {};

	_makepath(path, nullptr, nullptr, "CLEAR01", "TEM");
	CHECK(std::strcmp(path, "CLEAR01.TEM") == 0);

	_makepath(path, nullptr, nullptr, "POWER", ".SHP");
	CHECK(std::strcmp(path, "POWER.SHP") == 0);
}


TEST_CASE("IsoTileSet reports no record for an empty set")
{
	IsoTileSetTestAccess::Check_Empty_Set();
}


TEST_CASE("IsoTileSet reports no record for a zero tile offset")
{
	IsoTileSetTestAccess::Check_Absent_Record();
}


TEST_CASE("FileTimeToLocalFileTime applies the offset in effect at the instant")
{
	ScopedTimeZone zone("America/New_York");

	SUBCASE("winter keeps standard time")
	{
		SYSTEMTIME wall = Local_Wall_Clock(Utc_Time(2024, 1, 15, 12, 0));

		CHECK(wall.wYear == 2024);
		CHECK(wall.wMonth == 1);
		CHECK(wall.wDay == 15);
		CHECK(wall.wHour == 7);
		CHECK(wall.wMinute == 0);
	}

	SUBCASE("summer adds daylight saving time")
	{
		SYSTEMTIME wall = Local_Wall_Clock(Utc_Time(2024, 7, 15, 12, 0));

		CHECK(wall.wYear == 2024);
		CHECK(wall.wMonth == 7);
		CHECK(wall.wDay == 15);
		CHECK(wall.wHour == 8);
		CHECK(wall.wMinute == 0);
	}
}


TEST_CASE("FileTimeToLocalFileTime keeps an offset that is not a whole hour")
{
	ScopedTimeZone zone("Asia/Kolkata");

	SYSTEMTIME wall = Local_Wall_Clock(Utc_Time(2024, 1, 15, 12, 0));

	CHECK(wall.wDay == 15);
	CHECK(wall.wHour == 17);
	CHECK(wall.wMinute == 30);
}


TEST_CASE("FileTimeToLocalFileTime shifts nothing in a UTC zone")
{
	ScopedTimeZone zone("UTC");

	SYSTEMTIME utc = Utc_Time(2024, 1, 15, 12, 0);

	FILETIME stamp = {};
	REQUIRE(SystemTimeToFileTime(&utc, &stamp));

	FILETIME local = {};
	REQUIRE(FileTimeToLocalFileTime(&stamp, &local));

	CHECK(CompareFileTime(&stamp, &local) == 0);
}
