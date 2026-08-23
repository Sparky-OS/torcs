/***************************************************************************

    file                 : simwing_reconfig_test.cpp
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
    Unit tests for SimWingReConfig().
*/

#include <cmath>

#include <gtest/gtest.h>

#include "aero_test_helpers.h"


static const float kEps = 1e-5f;


class SimWingReConfigTest : public ::testing::Test {
protected:
	AeroFixture fix;

	void SetUp() override
	{
		fix.car.aero.Cd = 0.9f;
		fix.car.wing[0].Kx = -1.0f;
		fix.car.wing[1].Kx = -1.0f;
		fix.car.wing[0].angle = 0.2f;
		fix.car.wing[1].angle = 0.2f;
	}

	void setWingSetup(int index, tdble value, tdble min, tdble max)
	{
		tCarPitSetupValue* v = &fix.car.carElt->pitcmd.setup.wingangle[index];
		v->value = value;
		v->min = min;
		v->max = max;
	}
};


TEST_F(SimWingReConfigTest, RearWingReconfigUpdatesAngleAndCd)
{
	setWingSetup(1, 0.4f, 0.0f, 1.0f);

	const tdble oldCdPart = fix.car.wing[1].Kx * sin(fix.car.wing[1].angle);
	SimWingReConfig(&fix.car, 1);

	const tdble expectedCd = 0.9f + oldCdPart - fix.car.wing[1].Kx * sin(0.4f);
	EXPECT_NEAR(fix.car.wing[1].angle, 0.4f, kEps);
	EXPECT_NEAR(fix.car.aero.Cd, expectedCd, kEps);
}


TEST_F(SimWingReConfigTest, FrontWingReconfigDoesNotChangeCd)
{
	setWingSetup(0, 0.5f, 0.0f, 1.0f);

	SimWingReConfig(&fix.car, 0);

	EXPECT_NEAR(fix.car.wing[0].angle, 0.5f, kEps);
	EXPECT_NEAR(fix.car.aero.Cd, 0.9f, kEps);
}


TEST_F(SimWingReConfigTest, ReconfigClampsOutOfRangeValue)
{
	setWingSetup(1, 2.0f, 0.1f, 0.8f);

	SimWingReConfig(&fix.car, 1);

	EXPECT_NEAR(fix.car.wing[1].angle, 0.8f, kEps);
}


TEST_F(SimWingReConfigTest, ReconfigIgnoredWhenRangeDisabled)
{
	setWingSetup(1, 0.6f, 0.4f, 0.4f);

	SimWingReConfig(&fix.car, 1);

	EXPECT_NEAR(fix.car.wing[1].angle, 0.2f, kEps);
	EXPECT_NEAR(fix.car.aero.Cd, 0.9f, kEps);
}
