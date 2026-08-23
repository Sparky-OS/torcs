/***************************************************************************

    file                 : simaxle_config_test.cpp
    created              : Sat Mar  8 12:00:00 CET 2026
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
    Unit tests for SimAxleConfig().

    SimAxleConfig reads XML parameters for an axle (xpos, inertia,
    roll center, suspension course) and the anti-roll bar spring.
    It distributes rollCenter to both wheels, distributes half of the
    axle inertia to each wheel's feedBack.I, and calls SimSuspConfig
    for the third element suspension.

    Tests use the real GfParmReadFile + GfParmSetNum from tgf.lib
    to create in-memory parameter handles.
*/

#include <gtest/gtest.h>
#include <cmath>
#include <cstdio>
#include "axle_test_helpers.h"
#include "susp_test_helpers.h"


/// Test fixture that manages the AxleConfigFixture lifetime.
class SimAxleConfigTest : public ::testing::Test {
protected:
	AxleConfigFixture fix;

	void SetUp() override {
		ASSERT_NE(fix.hdle, nullptr) << "Failed to create parameter handle";
	}
};


// ---------------------------------------------------------------------------
// Default value tests
// ---------------------------------------------------------------------------

TEST_F(SimAxleConfigTest, DefaultValues_FrontAxle)
{
	// Empty section: all params get their default values.
	SimAxleConfig(&fix.car, FRNT);

	tAxle& axle = fix.car.axle[FRNT];

	// Defaults: xpos=0.0, I=0.15, rollCenter=0.15
	EXPECT_FLOAT_EQ(axle.xpos, 0.0f);
	EXPECT_FLOAT_EQ(axle.I, 0.15f);

	// Roll center distributed to both front wheels (0 and 1)
	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_RGT].rollCenter, 0.15f);
	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_LFT].rollCenter, 0.15f);

	// ARB spring: default is 0.0 (no PRM_SPR set in SECT_FRNTARB)
	EXPECT_FLOAT_EQ(axle.arbSuspSpringK, 0.0f);

	// feedBack.I: starts at 0 (memset), += I/2 = 0.15/2 = 0.075
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].feedBack.I, 0.075f, 1e-6f);
	EXPECT_NEAR(fix.car.wheel[FRNT_LFT].feedBack.I, 0.075f, 1e-6f);

	// Third element: configured via SimSuspConfig with F0=0, X0=suspCourse(default 0.0)
	// Spring K negated from default 175000
	EXPECT_FLOAT_EQ(axle.thirdSusp.spring.K, -175000.0f);
	// x0 = bellcrank(1.0) * X0(0.0) = 0.0
	EXPECT_FLOAT_EQ(axle.thirdSusp.spring.x0, 0.0f);
}


TEST_F(SimAxleConfigTest, DefaultValues_RearAxle)
{
	SimAxleConfig(&fix.car, REAR);

	tAxle& axle = fix.car.axle[REAR];

	EXPECT_FLOAT_EQ(axle.xpos, 0.0f);
	EXPECT_FLOAT_EQ(axle.I, 0.15f);

	// Roll center distributed to rear wheels (2 and 3)
	EXPECT_FLOAT_EQ(fix.car.wheel[REAR_RGT].rollCenter, 0.15f);
	EXPECT_FLOAT_EQ(fix.car.wheel[REAR_LFT].rollCenter, 0.15f);

	// ARB spring from SECT_REARARB
	EXPECT_FLOAT_EQ(axle.arbSuspSpringK, 0.0f);

	// feedBack.I for rear wheels
	EXPECT_NEAR(fix.car.wheel[REAR_RGT].feedBack.I, 0.075f, 1e-6f);
	EXPECT_NEAR(fix.car.wheel[REAR_LFT].feedBack.I, 0.075f, 1e-6f);
}


// ---------------------------------------------------------------------------
// Individual parameter tests
// ---------------------------------------------------------------------------

TEST_F(SimAxleConfigTest, CustomXpos)
{
	GfParmSetNum(fix.hdle, SECT_FRNTAXLE, PRM_XPOS, (char*)NULL, 1.47f);
	SimAxleConfig(&fix.car, FRNT);

	EXPECT_FLOAT_EQ(fix.car.axle[FRNT].xpos, 1.47f);
}


TEST_F(SimAxleConfigTest, CustomInertia)
{
	GfParmSetNum(fix.hdle, SECT_FRNTAXLE, PRM_INERTIA, (char*)NULL, 0.30f);
	SimAxleConfig(&fix.car, FRNT);

	EXPECT_FLOAT_EQ(fix.car.axle[FRNT].I, 0.30f);
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].feedBack.I, 0.15f, 1e-6f);
	EXPECT_NEAR(fix.car.wheel[FRNT_LFT].feedBack.I, 0.15f, 1e-6f);
}


TEST_F(SimAxleConfigTest, InertiaAddedToExistingFeedbackI)
{
	// Pre-set feedBack.I to simulate existing value from prior config
	fix.car.wheel[FRNT_RGT].feedBack.I = 0.10f;
	fix.car.wheel[FRNT_LFT].feedBack.I = 0.10f;

	GfParmSetNum(fix.hdle, SECT_FRNTAXLE, PRM_INERTIA, (char*)NULL, 0.40f);
	SimAxleConfig(&fix.car, FRNT);

	// feedBack.I += I/2 = 0.10 + 0.20 = 0.30
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].feedBack.I, 0.30f, 1e-6f);
	EXPECT_NEAR(fix.car.wheel[FRNT_LFT].feedBack.I, 0.30f, 1e-6f);
}


TEST_F(SimAxleConfigTest, RollCenterDistributedToBothWheels)
{
	GfParmSetNum(fix.hdle, SECT_FRNTAXLE, PRM_ROLLCENTER, (char*)NULL, 0.05f);
	SimAxleConfig(&fix.car, FRNT);

	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_RGT].rollCenter, 0.05f);
	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_LFT].rollCenter, 0.05f);
}


TEST_F(SimAxleConfigTest, RearRollCenterDistributedToRearWheels)
{
	GfParmSetNum(fix.hdle, SECT_REARAXLE, PRM_ROLLCENTER, (char*)NULL, 0.08f);
	SimAxleConfig(&fix.car, REAR);

	EXPECT_FLOAT_EQ(fix.car.wheel[REAR_RGT].rollCenter, 0.08f);
	EXPECT_FLOAT_EQ(fix.car.wheel[REAR_LFT].rollCenter, 0.08f);
}


TEST_F(SimAxleConfigTest, FrontArbSpringRead)
{
	GfParmSetNum(fix.hdle, SECT_FRNTARB, PRM_SPR, (char*)NULL, 50000.0f);
	SimAxleConfig(&fix.car, FRNT);

	EXPECT_FLOAT_EQ(fix.car.axle[FRNT].arbSuspSpringK, 50000.0f);
}


TEST_F(SimAxleConfigTest, RearArbSpringRead)
{
	GfParmSetNum(fix.hdle, SECT_REARARB, PRM_SPR, (char*)NULL, 40000.0f);
	SimAxleConfig(&fix.car, REAR);

	EXPECT_FLOAT_EQ(fix.car.axle[REAR].arbSuspSpringK, 40000.0f);
}


TEST_F(SimAxleConfigTest, ThirdSuspConfigured)
{
	// Set suspension course and a custom spring in the axle section.
	// SimSuspConfig will read spring params from the same section.
	GfParmSetNum(fix.hdle, SECT_FRNTAXLE, PRM_SUSPCOURSE, (char*)NULL, 0.08f);
	GfParmSetNum(fix.hdle, SECT_FRNTAXLE, PRM_SPR, (char*)NULL, 100000.0f);
	GfParmSetNum(fix.hdle, SECT_FRNTAXLE, PRM_BELLCRANK, (char*)NULL, 1.0f);
	GfParmSetNum(fix.hdle, SECT_FRNTAXLE, PRM_SLOWBUMP, (char*)NULL, 2000.0f);
	GfParmSetNum(fix.hdle, SECT_FRNTAXLE, PRM_SLOWREBOUND, (char*)NULL, 1500.0f);

	SimAxleConfig(&fix.car, FRNT);

	tSuspension& ts = fix.car.axle[FRNT].thirdSusp;

	// K negated, course used as xMax
	EXPECT_FLOAT_EQ(ts.spring.K, -100000.0f);
	EXPECT_FLOAT_EQ(ts.spring.xMax, 0.08f);
	// x0 = bellcrank(1.0) * X0(0.08) = 0.08
	EXPECT_FLOAT_EQ(ts.spring.x0, 0.08f);
	// F0 = 0.0 / bellcrank = 0.0
	EXPECT_FLOAT_EQ(ts.spring.F0, 0.0f);
	// Damper values
	EXPECT_FLOAT_EQ(ts.damper.bump.C1, 2000.0f);
	EXPECT_FLOAT_EQ(ts.damper.rebound.C1, 1500.0f);
}


TEST_F(SimAxleConfigTest, SuspCoursePassedAsX0ToSimSuspConfig)
{
	// Set course to 0.12, bellcrank to 2.0
	GfParmSetNum(fix.hdle, SECT_REARAXLE, PRM_SUSPCOURSE, (char*)NULL, 0.12f);
	GfParmSetNum(fix.hdle, SECT_REARAXLE, PRM_BELLCRANK, (char*)NULL, 2.0f);

	SimAxleConfig(&fix.car, REAR);

	tSuspension& ts = fix.car.axle[REAR].thirdSusp;

	// xMax = course = 0.12
	EXPECT_FLOAT_EQ(ts.spring.xMax, 0.12f);
	// x0 = bellcrank * course = 2.0 * 0.12 = 0.24
	EXPECT_NEAR(ts.spring.x0, 0.24f, 1e-6f);
	// bellcrank stored
	EXPECT_FLOAT_EQ(ts.spring.bellcrank, 2.0f);
}


// ---------------------------------------------------------------------------
// Parameterized test: systematic sweep of axle configurations
// ---------------------------------------------------------------------------

struct AxleConfigCase {
	const char* name;
	int index;
	const char* axleSect;
	const char* arbSect;
	tdble xpos;
	tdble inertia;
	tdble rollCenter;
	tdble arbSpringK;
	tdble suspCourse;
};

class SimAxleConfigParamTest : public ::testing::TestWithParam<AxleConfigCase> {};

TEST_P(SimAxleConfigParamTest, AllFieldsConfigured)
{
	const AxleConfigCase& c = GetParam();

	AxleConfigFixture fix;
	ASSERT_NE(fix.hdle, nullptr);

	GfParmSetNum(fix.hdle, c.axleSect, PRM_XPOS, (char*)NULL, c.xpos);
	GfParmSetNum(fix.hdle, c.axleSect, PRM_INERTIA, (char*)NULL, c.inertia);
	GfParmSetNum(fix.hdle, c.axleSect, PRM_ROLLCENTER, (char*)NULL, c.rollCenter);
	GfParmSetNum(fix.hdle, c.axleSect, PRM_SUSPCOURSE, (char*)NULL, c.suspCourse);
	GfParmSetNum(fix.hdle, c.arbSect, PRM_SPR, (char*)NULL, c.arbSpringK);

	SimAxleConfig(&fix.car, c.index);

	tAxle& axle = fix.car.axle[c.index];
	int rWheel = c.index * 2;
	int lWheel = c.index * 2 + 1;

	EXPECT_FLOAT_EQ(axle.xpos, c.xpos) << "case: " << c.name;
	EXPECT_FLOAT_EQ(axle.I, c.inertia) << "case: " << c.name;
	EXPECT_FLOAT_EQ(axle.arbSuspSpringK, c.arbSpringK) << "case: " << c.name;
	EXPECT_FLOAT_EQ(fix.car.wheel[rWheel].rollCenter, c.rollCenter) << "case: " << c.name;
	EXPECT_FLOAT_EQ(fix.car.wheel[lWheel].rollCenter, c.rollCenter) << "case: " << c.name;
	EXPECT_NEAR(fix.car.wheel[rWheel].feedBack.I, c.inertia / 2.0f, 1e-6f) << "case: " << c.name;
	EXPECT_NEAR(fix.car.wheel[lWheel].feedBack.I, c.inertia / 2.0f, 1e-6f) << "case: " << c.name;

	// Third element xMax = suspCourse
	EXPECT_FLOAT_EQ(axle.thirdSusp.spring.xMax, c.suspCourse) << "case: " << c.name;
}

INSTANTIATE_TEST_SUITE_P(AxleConfigCases, SimAxleConfigParamTest, ::testing::Values(
	AxleConfigCase{"front_typical", FRNT, SECT_FRNTAXLE, SECT_FRNTARB,  1.2f, 0.15f, 0.10f, 30000.0f, 0.05f},
	AxleConfigCase{"rear_typical",  REAR, SECT_REARAXLE, SECT_REARARB, -1.4f, 0.20f, 0.08f, 25000.0f, 0.06f},
	AxleConfigCase{"zero_values",   FRNT, SECT_FRNTAXLE, SECT_FRNTARB,  0.0f, 0.0f,  0.0f,      0.0f, 0.0f},
	AxleConfigCase{"high_inertia",  REAR, SECT_REARAXLE, SECT_REARARB,  1.0f, 1.0f,  0.20f, 100000.0f, 0.10f}
));
