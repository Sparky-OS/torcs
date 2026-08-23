/***************************************************************************

    file                 : simsteer_update_test.cpp
    created              : Thu Mar 12 00:00:00 CET 2026
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
    Unit tests for SimSteerUpdate().
*/

#include <gtest/gtest.h>

#include <cmath>

#include "steer_test_helpers.h"


static const float kEps = 1e-5f;


static tdble computeSteer2(tdble steer, tdble wheelbase, tdble wheeltrack)
{
	const tdble tanSteer = fabs(tan(steer));
	return atan2((wheelbase * tanSteer), (wheelbase - tanSteer * wheeltrack));
}


class SimSteerUpdateTest : public ::testing::Test {
protected:
	SteerFixture fix;
	tdble prevDt;

	void SetUp() override
	{
		prevDt = SimDeltaTime;
		SimDeltaTime = 0.1f;

		fix.car.wheelbase = 2.5f;
		fix.car.wheeltrack = 1.5f;
		fix.car.steer.steerLock = 0.6f;
		fix.car.steer.maxSpeed = 2.0f;
		fix.car.steer.steer = 0.0f;
	}

	void TearDown() override
	{
		SimDeltaTime = prevDt;
	}
};


TEST_F(SimSteerUpdateTest, PositiveSteerWithoutRateLimitAssignsWheelAngles)
{
	fix.car.ctrl->steer = 0.20f;

	SimSteerUpdate(&fix.car);

	const tdble expectedSteer = 0.12f;
	const tdble expectedSteer2 = computeSteer2(expectedSteer, fix.car.wheelbase, fix.car.wheeltrack);

	EXPECT_NEAR(fix.car.steer.steer, expectedSteer, kEps);
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].steer, expectedSteer2, kEps);
	EXPECT_NEAR(fix.car.wheel[FRNT_LFT].steer, expectedSteer, kEps);
}


TEST_F(SimSteerUpdateTest, PositiveDeltaIsRateLimited)
{
	fix.car.ctrl->steer = 1.0f;
	fix.car.steer.maxSpeed = 1.0f;

	SimSteerUpdate(&fix.car);

	const tdble expectedSteer = 0.1f;
	const tdble expectedSteer2 = computeSteer2(expectedSteer, fix.car.wheelbase, fix.car.wheeltrack);

	EXPECT_NEAR(fix.car.steer.steer, expectedSteer, kEps);
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].steer, expectedSteer2, kEps);
	EXPECT_NEAR(fix.car.wheel[FRNT_LFT].steer, expectedSteer, kEps);
}


TEST_F(SimSteerUpdateTest, NegativeSteerUsesNegativeBranchAndRateLimit)
{
	fix.car.ctrl->steer = -1.0f;
	fix.car.steer.maxSpeed = 2.0f;

	SimSteerUpdate(&fix.car);

	const tdble expectedSteer = -0.2f;
	const tdble expectedSteer2 = computeSteer2(expectedSteer, fix.car.wheelbase, fix.car.wheeltrack);

	EXPECT_NEAR(fix.car.steer.steer, expectedSteer, kEps);
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].steer, expectedSteer, kEps);
	EXPECT_NEAR(fix.car.wheel[FRNT_LFT].steer, -expectedSteer2, kEps);
}
