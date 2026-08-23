/***************************************************************************

    file                 : simbrake_system_config_test.cpp
    created              : Tue Mar  4 12:00:00 CET 2026
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
    Unit tests for SimBrakeSystemConfig().

    SimBrakeSystemConfig reads 4 XML parameters from the "Brake System"
    section into the tBrakeSyst struct:
    - rep (front-rear repartition, default 0.5)
    - coeff (max pressure, default 1000000)
    - repCmdClickValue (click value, default 0.0025)
    - repCmdMaxClicks (max clicks, cast to int, default 20)

    Tests use the real GfParmReadFile + GfParmSetNum from tgf.lib.
*/

#include <gtest/gtest.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include "brake_test_helpers.h"


/// Helper: create a parameter handle with a temp file.
static void* createEmptyParmHandle()
{
	const char* tmpFile = "simuv2_test_brake_system_config.xml";
	void* hdle = GfParmReadFile(tmpFile, GFPARM_RMODE_CREAT);
	return hdle;
}


/// Helper: set all 4 brake system params.
static void setBrakeSystemParams(
	void* hdle,
	tdble rep, tdble coeff, tdble clickValue, tdble maxClicks
)
{
	GfParmSetNum(hdle, SECT_BRKSYST, PRM_BRKREP, (char*)NULL, rep);
	GfParmSetNum(hdle, SECT_BRKSYST, PRM_BRKPRESS, (char*)NULL, coeff);
	GfParmSetNum(hdle, SECT_BRKSYST, PRM_BRKREPCMD_CLICKVALUE, (char*)NULL, clickValue);
	GfParmSetNum(hdle, SECT_BRKSYST, PRM_BRKREPCMD_MAXCLICKS, (char*)NULL, maxClicks);
}


/// Test fixture that manages the parameter handle lifetime.
class SimBrakeSystemConfigTest : public ::testing::Test {
protected:
	void* hdle;
	tCar car;

	void SetUp() override {
		hdle = createEmptyParmHandle();
		ASSERT_NE(hdle, nullptr) << "Failed to create parameter handle";
		memset(&car, 0, sizeof(car));
		car.params = hdle;
	}

	void TearDown() override {
		if (hdle) {
			GfParmReleaseHandle(hdle);
			hdle = nullptr;
		}
		std::remove("simuv2_test_brake_system_config.xml");
	}
};


TEST_F(SimBrakeSystemConfigTest, DefaultValues_EmptySection)
{
	SimBrakeSystemConfig(&car);

	EXPECT_FLOAT_EQ(car.brkSyst.rep, 0.5f);
	EXPECT_FLOAT_EQ(car.brkSyst.coeff, 1000000.0f);
	EXPECT_FLOAT_EQ(car.brkSyst.repCmdClickValue, 0.0025f);
	EXPECT_EQ(car.brkSyst.repCmdMaxClicks, 20);
}


TEST_F(SimBrakeSystemConfigTest, CustomValues_AllSet)
{
	setBrakeSystemParams(hdle, 0.6f, 2000000.0f, 0.005f, 30.0f);

	SimBrakeSystemConfig(&car);

	EXPECT_FLOAT_EQ(car.brkSyst.rep, 0.6f);
	EXPECT_FLOAT_EQ(car.brkSyst.coeff, 2000000.0f);
	EXPECT_FLOAT_EQ(car.brkSyst.repCmdClickValue, 0.005f);
	EXPECT_EQ(car.brkSyst.repCmdMaxClicks, 30);
}


TEST_F(SimBrakeSystemConfigTest, PartialParams_RestGetDefaults)
{
	// Only set rep and coeff
	GfParmSetNum(hdle, SECT_BRKSYST, PRM_BRKREP, (char*)NULL, 0.7f);
	GfParmSetNum(hdle, SECT_BRKSYST, PRM_BRKPRESS, (char*)NULL, 1500000.0f);

	SimBrakeSystemConfig(&car);

	EXPECT_FLOAT_EQ(car.brkSyst.rep, 0.7f);
	EXPECT_FLOAT_EQ(car.brkSyst.coeff, 1500000.0f);
	// Click params should get defaults
	EXPECT_FLOAT_EQ(car.brkSyst.repCmdClickValue, 0.0025f);
	EXPECT_EQ(car.brkSyst.repCmdMaxClicks, 20);
}


TEST_F(SimBrakeSystemConfigTest, MaxClicksCastToInt)
{
	// GfParmGetNum returns float; repCmdMaxClicks is cast to int.
	// Test with a float value that has a fractional part.
	GfParmSetNum(hdle, SECT_BRKSYST, PRM_BRKREPCMD_MAXCLICKS, (char*)NULL, 15.7f);

	SimBrakeSystemConfig(&car);

	// (int)15.7f = 15 (truncation toward zero)
	EXPECT_EQ(car.brkSyst.repCmdMaxClicks, 15);
}
