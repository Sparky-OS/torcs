/***************************************************************************

    file                 : simengine_config_test.cpp
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
    Unit tests for SimEngineConfig().
*/

#include <gtest/gtest.h>

#include "engine_test_helpers.h"


class SimEngineConfigTest : public ::testing::Test {
protected:
	EngineFixture fix;

	void SetUp() override
	{
		fix.attachParmHandle(createEngineTestParmHandle());
		ASSERT_NE(fix.hdle, nullptr) << "Failed to create parameter handle";
	}
};


TEST_F(SimEngineConfigTest, ReadsScalarsAndComputesCurveDerivedFields)
{
	GfParmSetNum(fix.hdle, SECT_ENGINE, PRM_REVSLIM, (char*)NULL, 350.0f);
	GfParmSetNum(fix.hdle, SECT_ENGINE, PRM_REVSMAX, (char*)NULL, 400.0f);
	GfParmSetNum(fix.hdle, SECT_ENGINE, PRM_TICKOVER, (char*)NULL, 120.0f);
	GfParmSetNum(fix.hdle, SECT_ENGINE, PRM_INERTIA, (char*)NULL, 0.30f);
	GfParmSetNum(fix.hdle, SECT_ENGINE, PRM_FUELCONS, (char*)NULL, 0.10f);
	GfParmSetNum(fix.hdle, SECT_ENGINE, PRM_ENGBRKCOEFF, (char*)NULL, 0.40f);

	fix.setCurvePoint(1, 100.0f, 50.0f);
	fix.setCurvePoint(2, 200.0f, 80.0f);
	fix.setCurvePoint(3, 300.0f, 60.0f);

	SimEngineConfig(&fix.car);

	EXPECT_FLOAT_EQ(fix.car.engine.revsLimiter, 350.0f);
	EXPECT_FLOAT_EQ(fix.car.engine.revsMax, 400.0f);
	EXPECT_FLOAT_EQ(fix.car.engine.tickover, 120.0f);
	EXPECT_FLOAT_EQ(fix.car.engine.I, 0.30f);
	EXPECT_FLOAT_EQ(fix.car.engine.fuelcons, 0.10f);
	EXPECT_FLOAT_EQ(fix.car.engine.brakeCoeff, 0.40f);

	EXPECT_FLOAT_EQ(fix.car.carElt->_enginerpmRedLine, 350.0f);
	EXPECT_FLOAT_EQ(fix.car.carElt->_enginerpmMax, 400.0f);

	EXPECT_EQ(fix.car.engine.curve.nbPts, 3);
	EXPECT_FLOAT_EQ(fix.car.engine.curve.maxTq, 80.0f);
	EXPECT_FLOAT_EQ(fix.car.engine.curve.rpmMaxPw, 300.0f);
	EXPECT_FLOAT_EQ(fix.car.engine.curve.TqAtMaxPw, 60.0f);
	EXPECT_FLOAT_EQ(fix.car.engine.curve.maxPw, 18000.0f);

	EXPECT_FLOAT_EQ(fix.car.carElt->_engineMaxTq, 80.0f);
	EXPECT_FLOAT_EQ(fix.car.carElt->_enginerpmMaxTq, 200.0f);
	EXPECT_FLOAT_EQ(fix.car.carElt->_engineMaxPw, 18000.0f);
	EXPECT_FLOAT_EQ(fix.car.carElt->_enginerpmMaxPw, 300.0f);

	EXPECT_FLOAT_EQ(fix.car.engine.rads, 120.0f);
}
