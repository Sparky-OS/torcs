/***************************************************************************

    file                 : simaero_config_test.cpp
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
    Unit tests for SimAeroConfig().
*/

#include <gtest/gtest.h>

#include "aero_test_helpers.h"


static const float kEps = 1e-5f;


class SimAeroConfigTest : public ::testing::Test {
protected:
	AeroFixture fix;

	void SetUp() override
	{
		fix.attachParmHandle(createAeroTestParmHandle());
		ASSERT_NE(fix.hdle, nullptr) << "Failed to create parameter handle";
	}
};


TEST_F(SimAeroConfigTest, ReadsConfiguredAeroValuesAndComputesScx2AndCd)
{
	fix.car.aero.Cd = 0.8f;
	fix.setAeroParam(PRM_CX, 0.30f);
	fix.setAeroParam(PRM_FRNTAREA, 1.80f);
	fix.setAeroParam(PRM_FCL, 2.20f);
	fix.setAeroParam(PRM_RCL, 3.30f);

	SimAeroConfig(&fix.car);

	const tdble expectedScx2 = 0.645f * 0.30f * 1.80f;
	EXPECT_NEAR(fix.car.aero.Clift[0], 2.20f, kEps);
	EXPECT_NEAR(fix.car.aero.Clift[1], 3.30f, kEps);
	EXPECT_NEAR(fix.car.aero.SCx2, expectedScx2, kEps);
	EXPECT_NEAR(fix.car.aero.Cd, 0.8f + expectedScx2, kEps);
}


TEST_F(SimAeroConfigTest, UsesDefaultsWhenParametersAreMissing)
{
	SimAeroConfig(&fix.car);

	const tdble expectedScx2 = 0.645f * 0.4f * 2.5f;
	EXPECT_NEAR(fix.car.aero.Clift[0], 0.0f, kEps);
	EXPECT_NEAR(fix.car.aero.Clift[1], 0.0f, kEps);
	EXPECT_NEAR(fix.car.aero.SCx2, expectedScx2, kEps);
	EXPECT_NEAR(fix.car.aero.Cd, expectedScx2, kEps);
}
