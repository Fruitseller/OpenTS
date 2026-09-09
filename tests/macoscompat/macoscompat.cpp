/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "wincompat.h"
#include "isotype.h"

#include <cstdio>
#include <cstring>

namespace
{
	int Failures = 0;

	void Check(bool condition, char const * what)
	{
		std::printf("%-68s %s\n", what, condition ? "ok" : "FAILED");
		if (!condition) Failures++;
	}
}

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
		Check(view.Tile_Count() == 0, "empty tile set reports no records");
		Check(view.Fetch_Record_Pointer(0) == nullptr, "empty tile set lookup returns no record");
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
		Check(view.Tile_Count() == 1, "single-tile set reports one record");
		Check(view.Fetch_Record_Pointer(0) == nullptr, "zero tile offset returns no record");
	}
};

int main(void)
{
	char path[MAX_PATH] = {};

	_makepath(path, nullptr, nullptr, "CLEAR01", "TEM");
	Check(std::strcmp(path, "CLEAR01.TEM") == 0, "_makepath adds the missing extension separator");

	_makepath(path, nullptr, nullptr, "POWER", ".SHP");
	Check(std::strcmp(path, "POWER.SHP") == 0, "_makepath preserves an existing extension separator");

	IsoTileSetTestAccess::Check_Empty_Set();
	IsoTileSetTestAccess::Check_Absent_Record();

	std::printf("%s\n", Failures == 0 ? "all checks passed" : "checks FAILED");
	return(Failures == 0 ? 0 : 1);
}
