/***************************************************************************

    file                 : simsteer_reconfig_test.cpp
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
    Unit tests for SimSteerReConfig().
*/

#include <gtest/gtest.h>

#include "steer_test_helpers.h"


static const float kEps = 1e-5f;


class SimSteerReConfigTest : public ::testing::Test {
protected:
	SteerFixture fix;

	void SetUp() override
	{
		fix.car.steer.steerLock = 0.40f;
		fix.car.carElt->_steerLock = 0.40f;
	}
};


TEST_F(SimSteerReConfigTest, AdjustableSetupUpdatesSteerLock)
{
	fix.setSteerPitSetup(0.60f, 0.20f, 0.80f);

	SimSteerReConfig(&fix.car);

	EXPECT_NEAR(fix.car.steer.steerLock, 0.60f, kEps);
	EXPECT_NEAR(fix.car.carElt->_steerLock, 0.60f, kEps);
}


TEST_F(SimSteerReConfigTest, AdjustableSetupClampsOutOfRangeValue)
{
	fix.setSteerPitSetup(1.20f, 0.10f, 0.70f);

	SimSteerReConfig(&fix.car);

	EXPECT_NEAR(fix.car.steer.steerLock, 0.70f, kEps);
	EXPECT_NEAR(fix.car.carElt->_steerLock, 0.70f, kEps);
}


TEST_F(SimSteerReConfigTest, DisabledRangeDoesNotUpdateSteerLock)
{
	fix.setSteerPitSetup(0.65f, 0.30f, 0.30f);

	SimSteerReConfig(&fix.car);

	EXPECT_NEAR(fix.car.steer.steerLock, 0.40f, kEps);
	EXPECT_NEAR(fix.car.carElt->_steerLock, 0.40f, kEps);
	EXPECT_NEAR(fix.car.carElt->pitcmd.setup.steerLock.value, 0.30f, kEps);
}
