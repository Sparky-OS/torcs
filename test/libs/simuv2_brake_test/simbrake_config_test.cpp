/***************************************************************************

    file                 : simbrake_config_test.cpp
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
    Unit tests for SimBrakeConfig().

    SimBrakeConfig reads 4 XML parameters (disk diameter, piston area,
    friction coefficient mu, inertia), computes derived values
    (coeff = diam * 0.5 * area * mu, radius = diam / 2), and stores
    the inertia I directly.

    Tests use the real GfParmReadFile + GfParmSetNum from tgf.lib
    to create in-memory parameter handles.
*/

#include <gtest/gtest.h>
#include <cmath>
#include <cstdio>
#include "brake_test_helpers.h"


/// Helper: create a parameter handle with a temp file.
static void* createEmptyParmHandle()
{
	const char* tmpFile = "simuv2_test_brake_config.xml";
	void* hdle = GfParmReadFile(tmpFile, GFPARM_RMODE_CREAT);
	return hdle;
}


/// Helper: set all 4 brake params in an XML section.
static void setAllBrakeParams(
	void* hdle, const char* section,
	tdble diam, tdble area, tdble mu, tdble inertia
)
{
	GfParmSetNum(hdle, section, PRM_BRKDIAM, (char*)NULL, diam);
	GfParmSetNum(hdle, section, PRM_BRKAREA, (char*)NULL, area);
	GfParmSetNum(hdle, section, PRM_MU, (char*)NULL, mu);
	GfParmSetNum(hdle, section, PRM_INERTIA, (char*)NULL, inertia);
}


/// Test fixture that manages the parameter handle lifetime.
class SimBrakeConfigTest : public ::testing::Test {
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
		std::remove("simuv2_test_brake_config.xml");
	}
};


TEST_F(SimBrakeConfigTest, DefaultValues_EmptySection)
{
	// Empty section: all params get their default values from SimBrakeConfig.
	const char* section = "Test Brake";
	tBrake brake{};

	SimBrakeConfig(hdle, section, &brake);

	// Defaults: diam=0.2, area=0.002, mu=0.30
	// coeff = 0.2 * 0.5 * 0.002 * 0.30 = 0.00006
	EXPECT_NEAR(brake.coeff, 0.00006f, 1e-8f);
	EXPECT_FLOAT_EQ(brake.I, 0.13f);
	EXPECT_FLOAT_EQ(brake.radius, 0.1f);
}


TEST_F(SimBrakeConfigTest, CustomValues_AllParamsSet)
{
	const char* section = SECT_FRNTRGTBRAKE;

	setAllBrakeParams(hdle, section, 0.3f, 0.004f, 0.40f, 0.20f);

	tBrake brake{};
	SimBrakeConfig(hdle, section, &brake);

	// coeff = 0.3 * 0.5 * 0.004 * 0.40 = 0.00024
	EXPECT_NEAR(brake.coeff, 0.00024f, 1e-8f);
	EXPECT_FLOAT_EQ(brake.I, 0.20f);
	EXPECT_FLOAT_EQ(brake.radius, 0.15f);
}


TEST_F(SimBrakeConfigTest, CoeffFormula_DiamHalf_Times_Area_Times_Mu)
{
	const char* section = SECT_FRNTRGTBRAKE;

	// Use distinctive values to verify the formula
	tdble diam = 0.36f;
	tdble area = 0.005f;
	tdble mu = 0.35f;
	setAllBrakeParams(hdle, section, diam, area, mu, 0.15f);

	tBrake brake{};
	SimBrakeConfig(hdle, section, &brake);

	tdble expectedCoeff = diam * 0.5f * area * mu;
	EXPECT_NEAR(brake.coeff, expectedCoeff, 1e-8f);
}


TEST_F(SimBrakeConfigTest, RadiusIsHalfDiam)
{
	const char* section = SECT_FRNTRGTBRAKE;

	GfParmSetNum(hdle, section, PRM_BRKDIAM, (char*)NULL, 0.38f);

	tBrake brake{};
	SimBrakeConfig(hdle, section, &brake);

	EXPECT_FLOAT_EQ(brake.radius, 0.19f);
}


TEST_F(SimBrakeConfigTest, ZeroDiam_ZeroCoeffAndRadius)
{
	const char* section = SECT_FRNTRGTBRAKE;

	setAllBrakeParams(hdle, section, 0.0f, 0.004f, 0.40f, 0.15f);

	tBrake brake{};
	SimBrakeConfig(hdle, section, &brake);

	EXPECT_FLOAT_EQ(brake.coeff, 0.0f);
	EXPECT_FLOAT_EQ(brake.radius, 0.0f);
}


TEST_F(SimBrakeConfigTest, ZeroMu_ZeroCoeff)
{
	const char* section = SECT_FRNTRGTBRAKE;

	setAllBrakeParams(hdle, section, 0.3f, 0.004f, 0.0f, 0.15f);

	tBrake brake{};
	SimBrakeConfig(hdle, section, &brake);

	EXPECT_FLOAT_EQ(brake.coeff, 0.0f);
	// radius should still be valid
	EXPECT_FLOAT_EQ(brake.radius, 0.15f);
}


TEST_F(SimBrakeConfigTest, ZeroArea_ZeroCoeff)
{
	const char* section = SECT_FRNTRGTBRAKE;

	setAllBrakeParams(hdle, section, 0.3f, 0.0f, 0.40f, 0.15f);

	tBrake brake{};
	SimBrakeConfig(hdle, section, &brake);

	EXPECT_FLOAT_EQ(brake.coeff, 0.0f);
	// radius should still be valid
	EXPECT_FLOAT_EQ(brake.radius, 0.15f);
}


// ---------------------------------------------------------------------------
// Parametrized tests for SimBrakeConfig
// ---------------------------------------------------------------------------

struct BrakeConfigCase {
	const char* name;
	tdble diam, area, mu, inertia;
	tdble expectedCoeff, expectedI, expectedRadius;
};

class SimBrakeConfigParamTest : public ::testing::TestWithParam<BrakeConfigCase> {
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
		std::remove("simuv2_test_brake_config.xml");
	}
};


TEST_P(SimBrakeConfigParamTest, ConfigProducesExpectedValues)
{
	const BrakeConfigCase& c = GetParam();
	const char* section = SECT_FRNTRGTBRAKE;

	setAllBrakeParams(hdle, section, c.diam, c.area, c.mu, c.inertia);

	tBrake brake{};
	SimBrakeConfig(hdle, section, &brake);

	EXPECT_NEAR(brake.coeff, c.expectedCoeff, 1e-8f) << "case: " << c.name;
	EXPECT_FLOAT_EQ(brake.I, c.expectedI) << "case: " << c.name;
	EXPECT_FLOAT_EQ(brake.radius, c.expectedRadius) << "case: " << c.name;
}


INSTANTIATE_TEST_SUITE_P(BrakeConfigCases, SimBrakeConfigParamTest, ::testing::Values(
	// Typical front brake: large disk, high mu
	BrakeConfigCase{"large_front", 0.38f, 0.005f, 0.45f, 0.18f,
		0.38f * 0.5f * 0.005f * 0.45f, 0.18f, 0.19f},
	// Typical rear brake: smaller disk
	BrakeConfigCase{"small_rear", 0.28f, 0.003f, 0.35f, 0.10f,
		0.28f * 0.5f * 0.003f * 0.35f, 0.10f, 0.14f},
	// Very small brake
	BrakeConfigCase{"tiny_brake", 0.10f, 0.001f, 0.20f, 0.05f,
		0.10f * 0.5f * 0.001f * 0.20f, 0.05f, 0.05f},
	// Large area, high friction
	BrakeConfigCase{"high_perf", 0.40f, 0.008f, 0.50f, 0.25f,
		0.40f * 0.5f * 0.008f * 0.50f, 0.25f, 0.20f},
	// Unit values for easy verification
	BrakeConfigCase{"unit_values", 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f * 0.5f * 1.0f * 1.0f, 1.0f, 0.5f},
	// All zeros
	BrakeConfigCase{"all_zero", 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f}
));
