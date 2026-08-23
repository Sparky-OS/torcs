/***************************************************************************

    file                 : simaero_update_aspiration_test.cpp
    created              : Tue Mar 10 00:00:00 CET 2026
    copyright            : (C) 2026 by Bernhard Wymann
    email                : berniw@bluewin.ch

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
    @file
    Aspiration unit tests for SimAeroUpdate().
*/

#include <cmath>
#include <cstring>

#include <gtest/gtest.h>

#include "aero_test_helpers.h"


static const float kEps = 1e-5f;


class SimAeroUpdateAspirationTest : public ::testing::Test {
protected:
	AeroFixture fix;
	tSituation s;
	tCar table[3];
	tCarElt tableElt[3];

	void SetUp() override
	{
		memset(&s, 0, sizeof(s));
		memset(table, 0, sizeof(table));
		memset(tableElt, 0, sizeof(tableElt));

		for (int i = 0; i < 3; i++) {
			table[i].carElt = &tableElt[i];
			tableElt[i].index = i;
			table[i].aero.Cd = 1.0f;
			table[i].DynGC.vel.x = 20.0f;
			table[i].DynGCg.pos.az = 0.0f;
		}

		SimCarTable = table;
		s._ncars = 2;

		fix.car.carElt->index = 0;
		fix.car.aero.SCx2 = 2.0f;
		fix.car.aero.Cd = 1.0f;
		fix.car.aero.Clift[0] = 0.0f;
		fix.car.aero.Clift[1] = 0.0f;
		fix.car.speed = 20.0f;
		fix.car.DynGC.vel.x = 20.0f;
		fix.car.DynGCg.vel.x = 20.0f;
		fix.car.DynGCg.vel.y = 0.0f;
		fix.car.DynGCg.pos.az = 0.0f;
		fix.car.DynGCg.pos.x = 0.0f;
		fix.car.DynGCg.pos.y = 0.0f;
	}

	void TearDown() override
	{
		SimCarTable = nullptr;
	}
};


TEST_F(SimAeroUpdateAspirationTest, NoAspirationWhenOwnAirSpeedAtOrBelowThreshold)
{
	fix.car.DynGC.vel.x = 10.0f;
	table[1].DynGCg.pos.x = 10.0f;
	table[1].DynGCg.pos.y = 0.0f;

	SimAeroUpdate(&fix.car, &s);

	const tdble expectedDrag = -fix.car.aero.SCx2 * 100.0f; // vel.x squared
	EXPECT_NEAR(fix.car.aero.drag, expectedDrag, kEps);
}


TEST_F(SimAeroUpdateAspirationTest, BehindAnotherCarReducesDrag)
{
	table[1].DynGCg.pos.x = 10.0f;
	table[1].DynGCg.pos.y = 0.0f;

	SimAeroUpdate(&fix.car, &s);

	const tdble dragK = 1.0f - exp(-2.0f * 10.0f / (table[1].aero.Cd * table[1].DynGC.vel.x));
	const tdble expectedDrag = -fix.car.aero.SCx2 * 400.0f * dragK * dragK;
	EXPECT_NEAR(fix.car.aero.drag, expectedDrag, kEps);
}


TEST_F(SimAeroUpdateAspirationTest, BeforeAnotherCarReducesDrag)
{
	table[1].DynGCg.pos.x = -10.0f;
	table[1].DynGCg.pos.y = 0.0f;

	SimAeroUpdate(&fix.car, &s);

	const tdble dragK = 1.0f - 0.15f * exp(-8.0f * 10.0f / (fix.car.aero.Cd * fix.car.DynGC.vel.x));
	const tdble expectedDrag = -fix.car.aero.SCx2 * 400.0f * dragK * dragK;
	EXPECT_NEAR(fix.car.aero.drag, expectedDrag, kEps);
}


TEST_F(SimAeroUpdateAspirationTest, UsesMinimumDragFactorFromMultipleCars)
{
	s._ncars = 3;
	table[1].DynGCg.pos.x = 5.0f;
	table[1].DynGCg.pos.y = 0.0f;
	table[2].DynGCg.pos.x = 20.0f;
	table[2].DynGCg.pos.y = 0.0f;

	SimAeroUpdate(&fix.car, &s);

	const tdble dragK1 = 1.0f - exp(-2.0f * 5.0f / (table[1].aero.Cd * table[1].DynGC.vel.x));
	const tdble dragK2 = 1.0f - exp(-2.0f * 20.0f / (table[2].aero.Cd * table[2].DynGC.vel.x));
	const tdble dragK = (dragK1 < dragK2) ? dragK1 : dragK2;
	const tdble expectedDrag = -fix.car.aero.SCx2 * 400.0f * dragK * dragK;
	EXPECT_NEAR(fix.car.aero.drag, expectedDrag, kEps);
}
