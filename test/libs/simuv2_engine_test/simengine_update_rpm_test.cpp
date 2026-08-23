/***************************************************************************

    file                 : simengine_update_rpm_test.cpp
    created              : Sun Mar  8 22:00:00 CET 2026
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
    Unit tests for SimEngineUpdateRpm().
*/

#include <gtest/gtest.h>

#include "engine_test_helpers.h"


class SimEngineUpdateRpmTest : public ::testing::Test {
protected:
	EngineFixture fix;
	tdble prevDt;

	void SetUp() override
	{
		prevDt = SimDeltaTime;
		SimDeltaTime = 0.2f;

		fix.car.fuel = 10.0f;
		fix.car.engine.rads = 100.0f;
		fix.car.engine.Tq = 0.0f;
		fix.car.engine.I = 10.0f;
		fix.car.engine.tickover = 100.0f;
		fix.car.engine.revsMax = 1000.0f;

		fix.car.transmission.gearbox.gear = 1;
		fix.car.transmission.curOverallRatio = 4.0f;
		fix.car.transmission.clutch.transferValue = 1.0f;
	}

	void TearDown() override
	{
		SimDeltaTime = prevDt;
	}
};


TEST_F(SimEngineUpdateRpmTest, NoFuel_ForcesClutchAppliedAndReturnsZero)
{
	fix.car.fuel = 0.0f;
	fix.car.transmission.clutch.state = CLUTCH_RELEASED;
	fix.car.transmission.clutch.transferValue = 0.7f;

	const tdble reaction = SimEngineUpdateRpm(&fix.car, 200.0f);

	EXPECT_FLOAT_EQ(reaction, 0.0f);
	EXPECT_FLOAT_EQ(fix.car.engine.rads, 0.0f);
	EXPECT_EQ(fix.car.transmission.clutch.state, CLUTCH_APPLIED);
	EXPECT_FLOAT_EQ(fix.car.transmission.clutch.transferValue, 0.0f);
}


TEST_F(SimEngineUpdateRpmTest, CoupledOverRev_ClampsAndReturnsAxleReaction)
{
	const tdble reaction = SimEngineUpdateRpm(&fix.car, 300.0f);

	EXPECT_FLOAT_EQ(fix.car.engine.rads, 1000.0f);
	EXPECT_FLOAT_EQ(reaction, 250.0f);
}


TEST_F(SimEngineUpdateRpmTest, NoCoupling_UsesFreeRpmPath)
{
	fix.car.transmission.clutch.transferValue = 0.0f;
	fix.car.engine.Tq = 50.0f;

	const tdble reaction = SimEngineUpdateRpm(&fix.car, 300.0f);

	// freerads = 100 + 50 / 10 * 0.2 = 101
	EXPECT_NEAR(fix.car.engine.rads, 101.0f, 1e-6f);
	EXPECT_FLOAT_EQ(reaction, 0.0f);
}


TEST_F(SimEngineUpdateRpmTest, CoupledUnderTickover_ClampsToTickover)
{
	const tdble reaction = SimEngineUpdateRpm(&fix.car, 10.0f);

	EXPECT_FLOAT_EQ(fix.car.engine.rads, 100.0f);
	EXPECT_FLOAT_EQ(reaction, 0.0f);
}
