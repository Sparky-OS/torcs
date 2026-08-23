/***************************************************************************

    file                 : simwing_update_test.cpp
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
    Unit tests for SimWingUpdate().
*/

#include <cmath>

#include <gtest/gtest.h>

#include "aero_test_helpers.h"


static const float kEps = 1e-5f;


class SimWingUpdateTest : public ::testing::Test {
protected:
	AeroFixture fix;

	void SetUp() override
	{
		fix.car.airSpeed2 = 400.0f;
		fix.car.dammage = 0;
		fix.car.wing[0].Kx = -1.5f;
		fix.car.wing[0].Kz = -6.0f;
		fix.car.wing[0].angle = 0.0f;
	}
};


TEST_F(SimWingUpdateTest, ForwardSpeedComputesWingForces)
{
	fix.car.DynGC.vel.x = 20.0f;
	fix.car.DynGC.vel.z = 5.0f;
	fix.car.wing[0].angle = 0.1f;
	fix.car.dammage = 3000;

	SimWingUpdate(&fix.car, 0, nullptr);

	const tdble aoa = atan2(5.0f, 20.0f) + 0.1f;
	const tdble sinaoa = sin(aoa);
	const tdble expectedFx = -1.5f * 400.0f * 1.3f * sinaoa;
	const tdble expectedFz = -6.0f * 400.0f * sinaoa;
	EXPECT_NEAR(fix.car.wing[0].forces.x, expectedFx, kEps);
	EXPECT_NEAR(fix.car.wing[0].forces.z, expectedFz, kEps);
}


TEST_F(SimWingUpdateTest, BackwardOrZeroForwardSpeedZeroesWingForces)
{
	fix.car.DynGC.vel.z = 4.0f;

	fix.car.DynGC.vel.x = 0.0f;
	SimWingUpdate(&fix.car, 0, nullptr);
	EXPECT_NEAR(fix.car.wing[0].forces.x, 0.0f, kEps);
	EXPECT_NEAR(fix.car.wing[0].forces.z, 0.0f, kEps);

	fix.car.DynGC.vel.x = -15.0f;
	SimWingUpdate(&fix.car, 0, nullptr);
	EXPECT_NEAR(fix.car.wing[0].forces.x, 0.0f, kEps);
	EXPECT_NEAR(fix.car.wing[0].forces.z, 0.0f, kEps);
}


TEST_F(SimWingUpdateTest, JumpPositiveAngleOfAttackMatchesFormula)
{
	fix.car.DynGC.vel.x = 20.0f;
	fix.car.DynGC.vel.z = 20.0f;

	SimWingUpdate(&fix.car, 0, nullptr);

	const tdble aoa = atan2(20.0f, 20.0f);
	const tdble expectedFz = -6.0f * 400.0f * sin(aoa);
	EXPECT_NEAR(fix.car.wing[0].forces.z, expectedFz, kEps);
}


TEST_F(SimWingUpdateTest, JumpNegativeAngleOfAttackMatchesFormula)
{
	fix.car.DynGC.vel.x = 20.0f;
	fix.car.DynGC.vel.z = -20.0f;

	SimWingUpdate(&fix.car, 0, nullptr);

	const tdble aoa = atan2(-20.0f, 20.0f);
	const tdble expectedFz = -6.0f * 400.0f * sin(aoa);
	EXPECT_NEAR(fix.car.wing[0].forces.z, expectedFz, kEps);
}


TEST_F(SimWingUpdateTest, SteepAngleOfAttackStillFollowsFormula)
{
	fix.car.DynGC.vel.x = 1.0f;
	fix.car.DynGC.vel.z = 30.0f;

	SimWingUpdate(&fix.car, 0, nullptr);

	const tdble aoa = atan2(30.0f, 1.0f);
	const tdble expectedFx = -1.5f * 400.0f * sin(aoa);
	EXPECT_NEAR(fix.car.wing[0].forces.x, expectedFx, kEps);
}
