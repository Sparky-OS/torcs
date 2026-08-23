/***************************************************************************

    file                 : simsteer_config_test.cpp
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
    Unit tests for SimSteerConfig().
*/

#include <gtest/gtest.h>

#include "steer_test_helpers.h"


static const float kEps = 1e-5f;


class SimSteerConfigTest : public ::testing::Test {
protected:
	SteerFixture fix;

	void SetUp() override
	{
		fix.attachParmHandle(createSteerTestParmHandle());
		ASSERT_NE(fix.hdle, nullptr) << "Failed to create parameter handle";
	}
};


TEST_F(SimSteerConfigTest, ReadsConfiguredValuesAndMirrorsSteerLock)
{
	GfParmSetNum(fix.hdle, SECT_STEER, PRM_STEERLOCK, (char*)NULL, 0.55f);
	GfParmSetNum(fix.hdle, SECT_STEER, PRM_STEERSPD, (char*)NULL, 1.75f);

	SimSteerConfig(&fix.car);

	EXPECT_NEAR(fix.car.steer.steerLock, 0.55f, kEps);
	EXPECT_NEAR(fix.car.steer.maxSpeed, 1.75f, kEps);
	EXPECT_NEAR(fix.car.carElt->_steerLock, 0.55f, kEps);
}


TEST_F(SimSteerConfigTest, UsesDefaultsWhenParametersAreMissing)
{
	SimSteerConfig(&fix.car);

	EXPECT_NEAR(fix.car.steer.steerLock, 0.43f, kEps);
	EXPECT_NEAR(fix.car.steer.maxSpeed, 1.0f, kEps);
	EXPECT_NEAR(fix.car.carElt->_steerLock, 0.43f, kEps);
}
