/***************************************************************************

    file                 : simengine_update_tq_test.cpp
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
    Unit tests for SimEngineUpdateTq().
*/

#include <gtest/gtest.h>

#include "engine_test_helpers.h"


static void setDefaultCurveForUpdate(EngineFixture& fix)
{
	fix.allocateCurve(2);
	fix.setCurveElem(0, 200.0f, 0.5f, 10.0f);
	fix.setCurveElem(1, 400.0f, 0.0f, 0.0f);
}


class SimEngineUpdateTqTest : public ::testing::Test {
protected:
	EngineFixture fix;
	tdble prevDt;

	void SetUp() override
	{
		prevDt = SimDeltaTime;
		SimDeltaTime = 0.01f;

		setDefaultCurveForUpdate(fix);
		fix.car.engine.tickover = 100.0f;
		fix.car.engine.revsMax = 500.0f;
		fix.car.engine.revsLimiter = 450.0f;
		fix.car.engine.brakeCoeff = 0.2f;
		fix.car.engine.fuelcons = 1.0f;
		fix.car.fuel = 10.0f;
		fix.car.carElt->_state = 0;
		fix.car.ctrl->accelCmd = 1.0f;
	}

	void TearDown() override
	{
		SimDeltaTime = prevDt;
	}
};


TEST_F(SimEngineUpdateTqTest, NoFuel_SetsRpmAndTorqueToZero)
{
	fix.car.fuel = 0.0f;
	fix.car.engine.rads = 200.0f;
	fix.car.engine.Tq = 42.0f;

	SimEngineUpdateTq(&fix.car);

	EXPECT_FLOAT_EQ(fix.car.engine.rads, 0.0f);
	EXPECT_FLOAT_EQ(fix.car.engine.Tq, 0.0f);
}


TEST_F(SimEngineUpdateTqTest, AboveLimiter_ClampsRpmAndSetsTorqueZero)
{
	fix.car.engine.rads = 500.0f;
	fix.car.engine.Tq = 42.0f;

	SimEngineUpdateTq(&fix.car);

	EXPECT_FLOAT_EQ(fix.car.engine.rads, 450.0f);
	EXPECT_FLOAT_EQ(fix.car.engine.Tq, 0.0f);
}


TEST_F(SimEngineUpdateTqTest, ComputesTorqueAndConsumesFuelInCurveSegment)
{
	fix.car.engine.rads = 150.0f;

	SimEngineUpdateTq(&fix.car);

	// Tmax = 150 * 0.5 + 10 = 85
	// EngBrkK = 0.2 * (150 - 100) / (500 - 100) = 0.025
	// Tq = 85 * (1 * (1 + 0.025) - 0.025) = 85
	EXPECT_NEAR(fix.car.engine.Tq, 85.0f, 1e-5f);

	// fuel -= abs(Tq) * rads * fuelcons * 1e-7 * dt
	const tdble expectedFuel = 10.0f - 85.0f * 150.0f * 1.0f * 0.0000001f * 0.01f;
	EXPECT_NEAR(fix.car.fuel, expectedFuel, 1e-6f);
}
