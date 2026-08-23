/***************************************************************************

    file                 : simaero_update_basic_test.cpp
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
    Basic unit tests for SimAeroUpdate().
*/

#include <cmath>
#include <cstring>

#include <gtest/gtest.h>

#include "aero_test_helpers.h"


static const float kEps = 1e-5f;


static tdble computeHm(const AeroFixture& fix)
{
	tdble hm = 1.5f * (fix.car.wheel[FRNT_RGT].rideHeight +
		fix.car.wheel[FRNT_LFT].rideHeight +
		fix.car.wheel[REAR_RGT].rideHeight +
		fix.car.wheel[REAR_LFT].rideHeight);
	hm = hm * hm;
	hm = hm * hm;
	hm = 2.0f * exp(-3.0f * hm);
	return hm;
}


class SimAeroUpdateBasicTest : public ::testing::Test {
protected:
	AeroFixture fix;
	tSituation s;

	void SetUp() override
	{
		memset(&s, 0, sizeof(s));
		s._ncars = 0;

		fix.car.aero.SCx2 = 1.5f;
		fix.car.aero.Cd = 1.0f;
		fix.car.aero.Clift[0] = 2.0f;
		fix.car.aero.Clift[1] = 3.0f;
		fix.car.dammage = 0;
		fix.car.carElt->index = 0;
		fix.setRideHeights(0.05f, 0.05f, 0.06f, 0.06f);
		fix.setPosition(0.0f, 0.0f, 0.0f);
		fix.setVelocities(0.0f, 0.0f, 30.0f, 0.0f);
	}
};


TEST_F(SimAeroUpdateBasicTest, UpdatesAirSpeedAndDragForForwardMotion)
{
	fix.car.speed = 30.0f;
	fix.car.DynGC.vel.x = 30.0f;

	SimAeroUpdate(&fix.car, &s);

	const tdble expectedV2 = 900.0f;
	const tdble expectedDrag = -fix.car.aero.SCx2 * expectedV2;
	EXPECT_NEAR(fix.car.airSpeed2, expectedV2, kEps);
	EXPECT_NEAR(fix.car.aero.drag, expectedDrag, kEps);
}


TEST_F(SimAeroUpdateBasicTest, GoingBackwardDragOpposesBackwardMotion)
{
	fix.car.speed = 40.0f;
	fix.car.DynGC.vel.x = -40.0f;

	SimAeroUpdate(&fix.car, &s);

	const tdble expectedV2 = 1600.0f;
	const tdble expectedDrag = fix.car.aero.SCx2 * expectedV2;
	EXPECT_NEAR(fix.car.aero.drag, expectedDrag, kEps);
}


TEST_F(SimAeroUpdateBasicTest, HighSpeedSidewaysReducesLiftWithSmallCosa)
{
	fix.car.speed = 50.0f;
	fix.car.DynGC.vel.x = 2.0f;

	SimAeroUpdate(&fix.car, &s);

	const tdble expectedV2 = 4.0f;
	const tdble expectedCosa = 2.0f / 50.0f;
	const tdble expectedHm = computeHm(fix);
	EXPECT_NEAR(fix.car.aero.lift[0], -2.0f * expectedV2 * expectedHm * expectedCosa, kEps);
	EXPECT_NEAR(fix.car.aero.lift[1], -3.0f * expectedV2 * expectedHm * expectedCosa, kEps);
}


TEST_F(SimAeroUpdateBasicTest, BackwardFlowClampsCosaToZeroAndZeroesBodyLift)
{
	fix.car.speed = 30.0f;
	fix.car.DynGC.vel.x = -20.0f;

	SimAeroUpdate(&fix.car, &s);

	EXPECT_NEAR(fix.car.aero.lift[0], 0.0f, kEps);
	EXPECT_NEAR(fix.car.aero.lift[1], 0.0f, kEps);
}


TEST_F(SimAeroUpdateBasicTest, DamageScalesDrag)
{
	fix.car.speed = 20.0f;
	fix.car.DynGC.vel.x = 20.0f;
	fix.car.dammage = 5000;

	SimAeroUpdate(&fix.car, &s);

	const tdble expectedDrag = -fix.car.aero.SCx2 * 400.0f * 1.5f;
	EXPECT_NEAR(fix.car.aero.drag, expectedDrag, kEps);
}
