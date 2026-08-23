/***************************************************************************

    file                 : simwing_config_test.cpp
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
    Unit tests for SimWingConfig().
*/

#include <cmath>

#include <gtest/gtest.h>

#include "aero_test_helpers.h"


static const float kEps = 1e-5f;


struct WingConfigCase {
	int index;
	tdble expectedCdDelta;
};


class SimWingConfigParamTest : public ::testing::TestWithParam<WingConfigCase> {
protected:
	AeroFixture fix;

	void SetUp() override
	{
		fix.attachParmHandle(createAeroTestParmHandle());
		ASSERT_NE(fix.hdle, nullptr) << "Failed to create parameter handle";

		fix.car.aero.Cd = 0.7f;
		fix.car.statGC.x = 1.5f;
	}
};


TEST_P(SimWingConfigParamTest, ReadsWingParametersAndComputesCoefficients)
{
	const WingConfigCase p = GetParam();
	const tdble area = 1.2f;
	const tdble angle = 0.3f;

	fix.setWingParam(p.index, PRM_WINGAREA, area);
	fix.setWingParam(p.index, PRM_WINGANGLE, angle);
	fix.setWingParam(p.index, PRM_XPOS, 5.0f);
	fix.setWingParam(p.index, PRM_ZPOS, 1.8f);

	SimWingConfig(&fix.car, p.index);

	const tWing& wing = fix.car.wing[p.index];
	const tdble expectedKx = -1.23f * area;
	const tdble expectedKz = 4.0f * expectedKx;
	EXPECT_NEAR(wing.angle, angle, kEps);
	EXPECT_NEAR(wing.staticPos.x, 5.0f - 1.5f, kEps);
	EXPECT_NEAR(wing.staticPos.z, 1.8f, kEps);
	EXPECT_NEAR(wing.Kx, expectedKx, kEps);
	EXPECT_NEAR(wing.Kz, expectedKz, kEps);
	EXPECT_NEAR(fix.car.aero.Cd, 0.7f + p.expectedCdDelta, kEps);
}


INSTANTIATE_TEST_SUITE_P(
	FrontAndRearWing,
	SimWingConfigParamTest,
	::testing::Values(
		WingConfigCase{0, 0.0f},
		WingConfigCase{1, -(-1.23f * 1.2f) * sin(0.3f)}
	)
);
