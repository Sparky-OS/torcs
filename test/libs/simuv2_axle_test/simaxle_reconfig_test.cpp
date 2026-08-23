/***************************************************************************

    file                 : simaxle_reconfig_test.cpp
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
    Unit tests for SimAxleReConfig().

    SimAxleReConfig reads 2 pit setup values:
    - arbspring[index]: anti-roll bar spring constant
    - thirdX0[index]: third element initial travel (passed to SimSuspThirdReConfig)

    Each parameter is only updated if SimAdjustPitCarSetupParam returns true
    (i.e., min != max within tolerance). The thirdX0 value is always passed
    through to SimSuspThirdReConfig regardless of whether it was adjusted.
*/

#include <gtest/gtest.h>
#include <cmath>
#include "axle_test_helpers.h"
#include "susp_test_helpers.h"


// ---------------------------------------------------------------------------
// SimAxleReConfig tests
// ---------------------------------------------------------------------------

TEST(SimAxleReConfigTest, BothAdjustable_BothUpdated)
{
	AxleReConfigFixture fix;
	int idx = FRNT;
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	// Pre-populate axle with known values
	tSuspension thirdSusp = makeSuspension(
		-50000.0f, 0.0f, 0.1f, 0.3f, 1.0f, 0.0f,
		2000.0f, 1500.0f, 0.5f, 1800.0f, 1200.0f, 0.5f
	);
	fix.setAxle(idx, 30000.0f, thirdSusp);

	// Set both pit values as adjustable
	fix.setSetupValue(&setup.arbspring[idx], 45000.0f, 10000.0f, 80000.0f);
	fix.setSetupValue(&setup.thirdX0[idx], 0.25f, 0.05f, 0.40f);

	SimAxleReConfig(&fix.car, idx);

	// ARB spring updated
	EXPECT_FLOAT_EQ(fix.car.axle[idx].arbSuspSpringK, 45000.0f);

	// Third element: SimSuspThirdReConfig called with X0 = 0.25
	// xMax is set to X0 in SimSuspThirdReConfig
	EXPECT_FLOAT_EQ(fix.car.axle[idx].thirdSusp.spring.xMax, 0.25f);
	// x0 = bellcrank(1.0) * X0(0.25) = 0.25
	EXPECT_FLOAT_EQ(fix.car.axle[idx].thirdSusp.spring.x0, 0.25f);
}


TEST(SimAxleReConfigTest, NoneAdjustable_NoArbChange)
{
	AxleReConfigFixture fix;
	int idx = REAR;

	// Pre-populate axle
	tSuspension thirdSusp = makeSuspension(
		-60000.0f, 0.0f, 0.15f, 0.35f, 1.0f, 0.0f,
		2000.0f, 1500.0f, 0.5f, 1800.0f, 1200.0f, 0.5f
	);
	fix.setAxle(idx, 35000.0f, thirdSusp);

	// All pit values at default (min == max == 0 => non-adjustable)
	SimAxleReConfig(&fix.car, idx);

	// ARB spring unchanged
	EXPECT_FLOAT_EQ(fix.car.axle[idx].arbSuspSpringK, 35000.0f);

	// Third element: SimSuspThirdReConfig still called with v->value = 0
	// (thirdX0 non-adjustable, value set to max = 0)
	// xMax, x0, F0 are always recomputed by SimSuspThirdReConfig
	EXPECT_FLOAT_EQ(fix.car.axle[idx].thirdSusp.spring.xMax, 0.0f);
}


TEST(SimAxleReConfigTest, OnlyArbAdjustable)
{
	AxleReConfigFixture fix;
	int idx = FRNT;
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	tSuspension thirdSusp = makeSuspension(
		-50000.0f, 0.0f, 0.1f, 0.3f, 1.0f, 0.0f,
		2000.0f, 1500.0f, 0.5f, 1800.0f, 1200.0f, 0.5f
	);
	fix.setAxle(idx, 30000.0f, thirdSusp);

	// Only ARB adjustable
	fix.setSetupValue(&setup.arbspring[idx], 55000.0f, 20000.0f, 80000.0f);
	// thirdX0 stays non-adjustable (default 0)

	SimAxleReConfig(&fix.car, idx);

	// ARB updated
	EXPECT_FLOAT_EQ(fix.car.axle[idx].arbSuspSpringK, 55000.0f);

	// Third element: thirdX0 non-adjustable => v->value = 0 (max)
	// xMax set to 0 by SimSuspThirdReConfig
	EXPECT_FLOAT_EQ(fix.car.axle[idx].thirdSusp.spring.xMax, 0.0f);
}


TEST(SimAxleReConfigTest, OnlyThirdX0Adjustable)
{
	AxleReConfigFixture fix;
	int idx = REAR;
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	tSuspension thirdSusp = makeSuspension(
		-50000.0f, 0.0f, 0.1f, 0.3f, 1.0f, 0.0f,
		2000.0f, 1500.0f, 0.5f, 1800.0f, 1200.0f, 0.5f
	);
	fix.setAxle(idx, 30000.0f, thirdSusp);

	// Only thirdX0 adjustable
	fix.setSetupValue(&setup.thirdX0[idx], 0.20f, 0.05f, 0.35f);

	SimAxleReConfig(&fix.car, idx);

	// ARB unchanged
	EXPECT_FLOAT_EQ(fix.car.axle[idx].arbSuspSpringK, 30000.0f);

	// Third element updated: xMax = X0 = 0.20
	EXPECT_FLOAT_EQ(fix.car.axle[idx].thirdSusp.spring.xMax, 0.20f);
}


TEST(SimAxleReConfigTest, ArbValueClampedToRange)
{
	AxleReConfigFixture fix;
	int idx = FRNT;
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	fix.setAxle(idx, 30000.0f, tSuspension{});

	// Value 150000 exceeds max of 80000 => clamped to 80000
	fix.setSetupValue(&setup.arbspring[idx], 150000.0f, 20000.0f, 80000.0f);

	SimAxleReConfig(&fix.car, idx);

	EXPECT_FLOAT_EQ(fix.car.axle[idx].arbSuspSpringK, 80000.0f);
}


TEST(SimAxleReConfigTest, ThirdX0ValueClampedToRange)
{
	AxleReConfigFixture fix;
	int idx = FRNT;
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	tSuspension thirdSusp = makeSuspension(
		-50000.0f, 0.0f, 0.1f, 0.3f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	fix.setAxle(idx, 30000.0f, thirdSusp);

	// Value 0.50 exceeds max of 0.35 => clamped to 0.35
	fix.setSetupValue(&setup.thirdX0[idx], 0.50f, 0.05f, 0.35f);

	SimAxleReConfig(&fix.car, idx);

	// SimSuspThirdReConfig called with clamped X0 = 0.35
	EXPECT_FLOAT_EQ(fix.car.axle[idx].thirdSusp.spring.xMax, 0.35f);
}


// ---------------------------------------------------------------------------
// Parameterized test: verify correct array indexing for FRNT/REAR
// ---------------------------------------------------------------------------

struct AxleReConfigIndexCase {
	const char* name;
	int index;
};

class SimAxleReConfigIndexTest : public ::testing::TestWithParam<AxleReConfigIndexCase> {};

TEST_P(SimAxleReConfigIndexTest, CorrectArrayElementRead)
{
	const AxleReConfigIndexCase& c = GetParam();

	AxleReConfigFixture fix;
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	// Pre-populate both axles with different values
	fix.setAxle(FRNT, 10000.0f, tSuspension{});
	fix.setAxle(REAR, 20000.0f, tSuspension{});

	// Only set the specific index as adjustable with a unique value
	tdble uniqueK = 50000.0f + c.index * 15000.0f;
	fix.setSetupValue(&setup.arbspring[c.index], uniqueK, 10000.0f, 100000.0f);

	SimAxleReConfig(&fix.car, c.index);

	EXPECT_FLOAT_EQ(fix.car.axle[c.index].arbSuspSpringK, uniqueK) << "case: " << c.name;

	// The other axle should be unchanged
	int other = (c.index == FRNT) ? REAR : FRNT;
	tdble otherExpected = (other == FRNT) ? 10000.0f : 20000.0f;
	EXPECT_FLOAT_EQ(fix.car.axle[other].arbSuspSpringK, otherExpected)
		<< "case: " << c.name << " (other axle should be unchanged)";
}

INSTANTIATE_TEST_SUITE_P(AxleIndices, SimAxleReConfigIndexTest, ::testing::Values(
	AxleReConfigIndexCase{"Front", FRNT},
	AxleReConfigIndexCase{"Rear",  REAR}
));
