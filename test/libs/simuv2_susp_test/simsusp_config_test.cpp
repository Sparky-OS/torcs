/***************************************************************************

    file                 : simsusp_config_test.cpp
    created              : Tue Mar  3 12:00:00 CET 2026
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
    Unit tests for SimSuspConfig().

    SimSuspConfig reads 10 XML parameters, computes derived values
    (x0 = bellcrank * X0, F0 = F0_input / bellcrank, K negated),
    and calls initDamper to compute b2 intercepts.

    Tests use the real GfParmReadFile + GfParmSetNum from tgf.lib
    to create in-memory parameter handles.
*/

#include <gtest/gtest.h>
#include <cmath>
#include <cstdio>
#include "susp_test_helpers.h"


/// Helper: create a parameter handle with a temp file.
/// Uses GfParmReadFile with GFPARM_RMODE_CREAT to create an empty handle,
/// then populates it with GfParmSetNum.
/// Returns the handle; caller must release with GfParmReleaseHandle.
static void* createEmptyParmHandle()
{
	// Use a temp file name that is unlikely to conflict.
	// GFPARM_RMODE_CREAT creates it if it doesn't exist.
	const char* tmpFile = "simuv2_test_susp_config.xml";
	void* hdle = GfParmReadFile(tmpFile, GFPARM_RMODE_CREAT);
	return hdle;
}


/// Helper: set all 10 suspension params in an XML section.
static void setAllSuspParams(
	void* hdle, const char* section,
	tdble spring, tdble course, tdble bellcrank, tdble packers,
	tdble slowBump, tdble slowRebound,
	tdble fastBump, tdble fastRebound,
	tdble bumpThreshold, tdble reboundThreshold
)
{
	GfParmSetNum(hdle, section, PRM_SPR, (char*)NULL, spring);
	GfParmSetNum(hdle, section, PRM_SUSPCOURSE, (char*)NULL, course);
	GfParmSetNum(hdle, section, PRM_BELLCRANK, (char*)NULL, bellcrank);
	GfParmSetNum(hdle, section, PRM_PACKERS, (char*)NULL, packers);
	GfParmSetNum(hdle, section, PRM_SLOWBUMP, (char*)NULL, slowBump);
	GfParmSetNum(hdle, section, PRM_SLOWREBOUND, (char*)NULL, slowRebound);
	GfParmSetNum(hdle, section, PRM_FASTBUMP, (char*)NULL, fastBump);
	GfParmSetNum(hdle, section, PRM_FASTREBOUND, (char*)NULL, fastRebound);
	GfParmSetNum(hdle, section, PRM_BUMPTHRESHOLD, (char*)NULL, bumpThreshold);
	GfParmSetNum(hdle, section, PRM_REBOUNDTHRESHOLD, (char*)NULL, reboundThreshold);
}


/// Test fixture that manages the parameter handle lifetime.
class SimSuspConfigTest : public ::testing::Test {
protected:
	void* hdle;

	void SetUp() override {
		hdle = createEmptyParmHandle();
		ASSERT_NE(hdle, nullptr) << "Failed to create parameter handle";
	}

	void TearDown() override {
		if (hdle) {
			GfParmReleaseHandle(hdle);
			hdle = nullptr;
		}
		// Clean up temp file
		std::remove("simuv2_test_susp_config.xml");
	}
};


TEST_F(SimSuspConfigTest, DefaultValues_EmptySection)
{
	// Empty section: all params get their default values from SimSuspConfig.
	const char* section = "Test Suspension";
	tdble F0 = 5000.0f;
	tdble X0 = 0.15f;
	tSuspension susp{};

	SimSuspConfig(hdle, section, &susp, F0, X0);

	// Defaults: spring=175000 (negated), course=0.5, bellcrank=1.0, packers=0.0
	EXPECT_FLOAT_EQ(susp.spring.K, -175000.0f);
	EXPECT_FLOAT_EQ(susp.spring.xMax, 0.5f);
	EXPECT_FLOAT_EQ(susp.spring.bellcrank, 1.0f);
	EXPECT_FLOAT_EQ(susp.spring.packers, 0.0f);

	// Derived values: x0 = bellcrank * X0, F0 = F0 / bellcrank
	EXPECT_FLOAT_EQ(susp.spring.x0, 1.0f * 0.15f);
	EXPECT_FLOAT_EQ(susp.spring.F0, 5000.0f / 1.0f);

	// Default damper: slowBump=0, slowRebound=0
	// fastBump defaults to bump.C1 (0), fastRebound defaults to rebound.C1 (0)
	EXPECT_FLOAT_EQ(susp.damper.bump.C1, 0.0f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.C1, 0.0f);
	EXPECT_FLOAT_EQ(susp.damper.bump.C2, 0.0f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.C2, 0.0f);

	// Default thresholds
	EXPECT_FLOAT_EQ(susp.damper.bump.v1, 0.5f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.v1, 0.5f);

	// b2 = (C1 - C2) * v1 = 0
	EXPECT_FLOAT_EQ(susp.damper.bump.b2, 0.0f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.b2, 0.0f);
}


TEST_F(SimSuspConfigTest, CustomSpringValues)
{
	const char* section = SECT_FRNTRGTSUSP;
	tdble F0 = 4000.0f;
	tdble X0 = 0.12f;

	setAllSuspParams(hdle, section,
		200000.0f, 0.4f, 1.5f, 0.02f,
		3000.0f, 2500.0f, 1500.0f, 1200.0f,
		0.6f, 0.7f
	);

	tSuspension susp{};
	SimSuspConfig(hdle, section, &susp, F0, X0);

	EXPECT_FLOAT_EQ(susp.spring.K, -200000.0f);
	EXPECT_FLOAT_EQ(susp.spring.xMax, 0.4f);
	EXPECT_FLOAT_EQ(susp.spring.bellcrank, 1.5f);
	EXPECT_FLOAT_EQ(susp.spring.packers, 0.02f);

	// Derived: x0 = 1.5 * 0.12 = 0.18, F0 = 4000 / 1.5
	EXPECT_NEAR(susp.spring.x0, 0.18f, 1e-6f);
	EXPECT_NEAR(susp.spring.F0, 4000.0f / 1.5f, 1e-2f);
}


TEST_F(SimSuspConfigTest, CustomDamperValues)
{
	const char* section = SECT_FRNTRGTSUSP;

	setAllSuspParams(hdle, section,
		175000.0f, 0.5f, 1.0f, 0.0f,
		5000.0f, 4000.0f, 3000.0f, 2000.0f,
		0.5f, 0.6f
	);

	tSuspension susp{};
	SimSuspConfig(hdle, section, &susp, 5000.0f, 0.15f);

	EXPECT_FLOAT_EQ(susp.damper.bump.C1, 5000.0f);
	EXPECT_FLOAT_EQ(susp.damper.bump.C2, 3000.0f);
	EXPECT_FLOAT_EQ(susp.damper.bump.v1, 0.5f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.C1, 4000.0f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.C2, 2000.0f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.v1, 0.6f);

	// b2 = (C1 - C2) * v1
	EXPECT_NEAR(susp.damper.bump.b2, (5000.0f - 3000.0f) * 0.5f, 1e-2f);
	EXPECT_NEAR(susp.damper.rebound.b2, (4000.0f - 2000.0f) * 0.6f, 1e-2f);
}


TEST_F(SimSuspConfigTest, BellcrankAffectsX0andF0)
{
	const char* section = SECT_FRNTRGTSUSP;

	setAllSuspParams(hdle, section,
		175000.0f, 0.5f, 2.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f
	);

	tdble F0 = 6000.0f;
	tdble X0 = 0.1f;
	tSuspension susp{};
	SimSuspConfig(hdle, section, &susp, F0, X0);

	// x0 = bellcrank * X0 = 2.0 * 0.1 = 0.2
	EXPECT_FLOAT_EQ(susp.spring.x0, 0.2f);
	// F0 = F0_input / bellcrank = 6000 / 2.0 = 3000
	EXPECT_FLOAT_EQ(susp.spring.F0, 3000.0f);
}


TEST_F(SimSuspConfigTest, KisNegated)
{
	const char* section = SECT_FRNTRGTSUSP;

	// Set spring K to 250000 (positive in XML)
	GfParmSetNum(hdle, section, PRM_SPR, (char*)NULL, 250000.0f);

	tSuspension susp{};
	SimSuspConfig(hdle, section, &susp, 5000.0f, 0.1f);

	// K should be stored as negative
	EXPECT_FLOAT_EQ(susp.spring.K, -250000.0f);
}


TEST_F(SimSuspConfigTest, FastBumpDefaultsToBumpC1)
{
	const char* section = SECT_FRNTRGTSUSP;

	// Only set slow bump, not fast bump
	GfParmSetNum(hdle, section, PRM_SLOWBUMP, (char*)NULL, 5000.0f);
	// Don't set PRM_FASTBUMP -- it should default to bump.C1 (5000)

	tSuspension susp{};
	SimSuspConfig(hdle, section, &susp, 5000.0f, 0.1f);

	EXPECT_FLOAT_EQ(susp.damper.bump.C1, 5000.0f);
	EXPECT_FLOAT_EQ(susp.damper.bump.C2, 5000.0f);  // Defaults to C1
}


TEST_F(SimSuspConfigTest, FastReboundDefaultsToReboundC1)
{
	const char* section = SECT_FRNTRGTSUSP;

	// Only set slow rebound, not fast rebound
	GfParmSetNum(hdle, section, PRM_SLOWREBOUND, (char*)NULL, 4000.0f);

	tSuspension susp{};
	SimSuspConfig(hdle, section, &susp, 5000.0f, 0.1f);

	EXPECT_FLOAT_EQ(susp.damper.rebound.C1, 4000.0f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.C2, 4000.0f);  // Defaults to C1
}


TEST_F(SimSuspConfigTest, InitDamperComputes_b2)
{
	const char* section = SECT_FRNTRGTSUSP;

	setAllSuspParams(hdle, section,
		175000.0f, 0.5f, 1.0f, 0.0f,
		8000.0f, 6000.0f, 4000.0f, 3000.0f,
		0.4f, 0.3f
	);

	tSuspension susp{};
	SimSuspConfig(hdle, section, &susp, 5000.0f, 0.1f);

	// b2 = (C1 - C2) * v1
	// bump: (8000 - 4000) * 0.4 = 1600
	EXPECT_NEAR(susp.damper.bump.b2, 1600.0f, 1e-2f);
	// rebound: (6000 - 3000) * 0.3 = 900
	EXPECT_NEAR(susp.damper.rebound.b2, 900.0f, 1e-2f);
}
