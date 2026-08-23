/***************************************************************************

    file                 : simsusp_reconfig_test.cpp
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
    Unit tests for SimSuspReConfig() and SimSuspThirdReConfig().

    These functions read pit setup values from tCar/tCarElt and
    conditionally update suspension parameters. Each parameter
    is only updated if SimAdjustPitCarSetupParam returns true
    (i.e., min != max within tolerance).
*/

#include <gtest/gtest.h>
#include <cmath>
#include "susp_test_helpers.h"


// ---------------------------------------------------------------------------
// SimSuspReConfig tests
// ---------------------------------------------------------------------------

TEST(SimSuspReConfigTest, AllAdjustable_AllFieldsUpdated)
{
	CarReConfigFixture fix;
	int idx = 0;
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	// Set all pit values as adjustable (min != max)
	fix.setSuspSetupValue(&setup.suspspring[idx],        200000.0f, 100000.0f, 300000.0f);
	fix.setSuspSetupValue(&setup.susppackers[idx],       0.03f,     0.0f,      0.05f);
	fix.setSuspSetupValue(&setup.suspslowbump[idx],      6000.0f,   1000.0f,   10000.0f);
	fix.setSuspSetupValue(&setup.suspslowrebound[idx],   5000.0f,   1000.0f,   10000.0f);
	fix.setSuspSetupValue(&setup.suspfastbump[idx],      4000.0f,   1000.0f,   8000.0f);
	fix.setSuspSetupValue(&setup.suspfastrebound[idx],   3000.0f,   1000.0f,   8000.0f);
	fix.setSuspSetupValue(&setup.suspbumpthreshold[idx], 0.6f,      0.1f,      1.0f);
	fix.setSuspSetupValue(&setup.suspreboundthreshold[idx], 0.7f,   0.1f,      1.0f);

	// Pre-existing suspension state
	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.1f, 0.5f, 1.5f, 0.01f,
		3000.0f, 2000.0f, 0.5f, 2500.0f, 1500.0f, 0.5f
	);

	tdble F0 = 4500.0f;
	tdble X0 = 0.12f;

	SimSuspReConfig(&fix.car, idx, &susp, F0, X0);

	// Spring K is negated
	EXPECT_FLOAT_EQ(susp.spring.K, -200000.0f);
	EXPECT_FLOAT_EQ(susp.spring.packers, 0.03f);
	EXPECT_FLOAT_EQ(susp.damper.bump.C1, 6000.0f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.C1, 5000.0f);
	EXPECT_FLOAT_EQ(susp.damper.bump.C2, 4000.0f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.C2, 3000.0f);
	EXPECT_FLOAT_EQ(susp.damper.bump.v1, 0.6f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.v1, 0.7f);

	// x0 and F0 always recomputed: x0 = bellcrank * X0, F0 = F0 / bellcrank
	EXPECT_NEAR(susp.spring.x0, 1.5f * 0.12f, 1e-6f);
	EXPECT_NEAR(susp.spring.F0, 4500.0f / 1.5f, 1e-2f);

	// b2 recomputed by initDamper
	EXPECT_NEAR(susp.damper.bump.b2, (6000.0f - 4000.0f) * 0.6f, 1e-2f);
	EXPECT_NEAR(susp.damper.rebound.b2, (5000.0f - 3000.0f) * 0.7f, 1e-2f);
}


TEST(SimSuspReConfigTest, NoneAdjustable_NoFieldsChanged)
{
	CarReConfigFixture fix;
	int idx = 1;
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	// All pit values non-adjustable (min == max == 0, the default from memset)
	// They stay at 0; SimAdjustPitCarSetupParam will return false.

	// Pre-existing suspension with known values
	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.1f, 0.5f, 1.0f, 0.01f,
		3000.0f, 2000.0f, 0.5f, 2500.0f, 1500.0f, 0.5f
	);

	tdble F0 = 4000.0f;
	tdble X0 = 0.1f;

	SimSuspReConfig(&fix.car, idx, &susp, F0, X0);

	// Fields should NOT have changed (SimAdjustPitCarSetupParam returned false)
	EXPECT_FLOAT_EQ(susp.spring.K, -175000.0f);
	EXPECT_FLOAT_EQ(susp.spring.packers, 0.01f);
	EXPECT_FLOAT_EQ(susp.damper.bump.C1, 3000.0f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.C1, 2500.0f);
	EXPECT_FLOAT_EQ(susp.damper.bump.C2, 2000.0f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.C2, 1500.0f);
	EXPECT_FLOAT_EQ(susp.damper.bump.v1, 0.5f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.v1, 0.5f);

	// x0 and F0 are ALWAYS recomputed regardless
	EXPECT_FLOAT_EQ(susp.spring.x0, 1.0f * 0.1f);
	EXPECT_FLOAT_EQ(susp.spring.F0, 4000.0f / 1.0f);
}


TEST(SimSuspReConfigTest, PartialAdjustment_OnlySomeFields)
{
	CarReConfigFixture fix;
	int idx = 2;
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	// Only spring and slow bump adjustable
	fix.setSuspSetupValue(&setup.suspspring[idx],   180000.0f, 100000.0f, 250000.0f);
	fix.setSuspSetupValue(&setup.suspslowbump[idx], 7000.0f,   2000.0f,   12000.0f);
	// Everything else stays at default (min == max == 0 => non-adjustable)

	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.1f, 0.5f, 1.0f, 0.01f,
		3000.0f, 2000.0f, 0.5f, 2500.0f, 1500.0f, 0.5f
	);

	SimSuspReConfig(&fix.car, idx, &susp, 5000.0f, 0.1f);

	// Updated fields
	EXPECT_FLOAT_EQ(susp.spring.K, -180000.0f);
	EXPECT_FLOAT_EQ(susp.damper.bump.C1, 7000.0f);

	// Unchanged fields
	EXPECT_FLOAT_EQ(susp.spring.packers, 0.01f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.C1, 2500.0f);
	EXPECT_FLOAT_EQ(susp.damper.bump.C2, 2000.0f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.C2, 1500.0f);
	EXPECT_FLOAT_EQ(susp.damper.bump.v1, 0.5f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.v1, 0.5f);
}


TEST(SimSuspReConfigTest, ValueClampedToMinMax)
{
	CarReConfigFixture fix;
	int idx = 0;
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	// Value outside [min, max] range -- should be clamped
	fix.setSuspSetupValue(&setup.suspspring[idx], 500000.0f, 100000.0f, 300000.0f);

	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.1f, 0.5f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);

	SimSuspReConfig(&fix.car, idx, &susp, 5000.0f, 0.1f);

	// Value was 500000, max is 300000 => clamped to 300000, then negated
	EXPECT_FLOAT_EQ(susp.spring.K, -300000.0f);
}


TEST(SimSuspReConfigTest, X0F0AlwaysRecomputed)
{
	CarReConfigFixture fix;
	int idx = 0;

	// No adjustable fields at all
	tSuspension susp = makeSuspension(
		-175000.0f, 999.0f, 999.0f, 0.5f, 2.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);

	tdble F0 = 8000.0f;
	tdble X0 = 0.2f;

	SimSuspReConfig(&fix.car, idx, &susp, F0, X0);

	// x0 and F0 are recomputed even when no pit params changed
	EXPECT_FLOAT_EQ(susp.spring.x0, 2.0f * 0.2f);      // bellcrank * X0
	EXPECT_FLOAT_EQ(susp.spring.F0, 8000.0f / 2.0f);   // F0 / bellcrank
}


// ---------------------------------------------------------------------------
// Parameterized test: verify correct array indexing for wheel indices 0-3
// ---------------------------------------------------------------------------

struct ReConfigIndexCase {
	const char* name;
	int index;
};

class SimSuspReConfigIndexTest : public ::testing::TestWithParam<ReConfigIndexCase> {};

TEST_P(SimSuspReConfigIndexTest, CorrectArrayElementRead)
{
	const ReConfigIndexCase& c = GetParam();

	CarReConfigFixture fix;
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	// Only set the spring for the specific index as adjustable
	// Use a unique value per index so we can verify the right one was read
	tdble uniqueK = 150000.0f + c.index * 10000.0f;
	fix.setSuspSetupValue(&setup.suspspring[c.index], uniqueK, 100000.0f, 300000.0f);

	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.1f, 0.5f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);

	SimSuspReConfig(&fix.car, c.index, &susp, 5000.0f, 0.1f);

	EXPECT_FLOAT_EQ(susp.spring.K, -uniqueK) << "case: " << c.name;
}

INSTANTIATE_TEST_SUITE_P(WheelIndices, SimSuspReConfigIndexTest, ::testing::Values(
	ReConfigIndexCase{"FR", 0},
	ReConfigIndexCase{"FL", 1},
	ReConfigIndexCase{"RR", 2},
	ReConfigIndexCase{"RL", 3}
));


// ---------------------------------------------------------------------------
// SimSuspThirdReConfig tests
// ---------------------------------------------------------------------------

TEST(SimSuspThirdReConfigTest, AllAdjustable_FieldsUpdated)
{
	CarReConfigFixture fix;
	int idx = 0;  // Front axle
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	fix.setSuspSetupValue(&setup.thirdspring[idx],  80000.0f, 50000.0f, 150000.0f);
	fix.setSuspSetupValue(&setup.thirdbump[idx],    3000.0f,  1000.0f,  6000.0f);
	fix.setSuspSetupValue(&setup.thirdrebound[idx], 2500.0f,  1000.0f,  6000.0f);

	tSuspension susp = makeSuspension(
		-50000.0f, 0.0f, 0.1f, 0.3f, 1.0f, 0.0f,
		2000.0f, 1500.0f, 0.5f, 1800.0f, 1200.0f, 0.5f
	);

	tdble F0 = 0.0f;
	tdble X0 = 0.25f;

	SimSuspThirdReConfig(&fix.car, idx, &susp, F0, X0);

	// Spring K negated
	EXPECT_FLOAT_EQ(susp.spring.K, -80000.0f);

	// Third element: bump C1 and C2 set to same value
	EXPECT_FLOAT_EQ(susp.damper.bump.C1, 3000.0f);
	EXPECT_FLOAT_EQ(susp.damper.bump.C2, 3000.0f);

	// Third element: rebound C1 and C2 set to same value
	EXPECT_FLOAT_EQ(susp.damper.rebound.C1, 2500.0f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.C2, 2500.0f);

	// xMax = X0 (unique to third reconfig)
	EXPECT_FLOAT_EQ(susp.spring.xMax, 0.25f);

	// x0 = bellcrank * X0, F0 = F0_input / bellcrank
	EXPECT_FLOAT_EQ(susp.spring.x0, 1.0f * 0.25f);
	EXPECT_FLOAT_EQ(susp.spring.F0, 0.0f / 1.0f);

	// b2: since C1 == C2 for both bump and rebound, b2 = 0
	EXPECT_FLOAT_EQ(susp.damper.bump.b2, 0.0f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.b2, 0.0f);
}


TEST(SimSuspThirdReConfigTest, BumpSetsC1andC2Equal)
{
	CarReConfigFixture fix;
	int idx = 1;  // Rear axle
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	fix.setSuspSetupValue(&setup.thirdbump[idx], 5000.0f, 1000.0f, 10000.0f);

	tSuspension susp = makeSuspension(
		-50000.0f, 0.0f, 0.1f, 0.3f, 1.0f, 0.0f,
		2000.0f, 1500.0f, 0.5f, 1800.0f, 1200.0f, 0.5f
	);

	SimSuspThirdReConfig(&fix.car, idx, &susp, 0.0f, 0.2f);

	// Both C1 and C2 set to the same value
	EXPECT_FLOAT_EQ(susp.damper.bump.C1, 5000.0f);
	EXPECT_FLOAT_EQ(susp.damper.bump.C2, 5000.0f);

	// Rebound unchanged (non-adjustable)
	EXPECT_FLOAT_EQ(susp.damper.rebound.C1, 1800.0f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.C2, 1200.0f);
}


TEST(SimSuspThirdReConfigTest, ReboundSetsC1andC2Equal)
{
	CarReConfigFixture fix;
	int idx = 0;
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	fix.setSuspSetupValue(&setup.thirdrebound[idx], 4000.0f, 1000.0f, 8000.0f);

	tSuspension susp = makeSuspension(
		-50000.0f, 0.0f, 0.1f, 0.3f, 1.0f, 0.0f,
		2000.0f, 1500.0f, 0.5f, 1800.0f, 1200.0f, 0.5f
	);

	SimSuspThirdReConfig(&fix.car, idx, &susp, 0.0f, 0.2f);

	// Rebound: both C1 and C2 set to same value
	EXPECT_FLOAT_EQ(susp.damper.rebound.C1, 4000.0f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.C2, 4000.0f);

	// Bump unchanged (non-adjustable)
	EXPECT_FLOAT_EQ(susp.damper.bump.C1, 2000.0f);
	EXPECT_FLOAT_EQ(susp.damper.bump.C2, 1500.0f);
}


TEST(SimSuspThirdReConfigTest, XMaxSetFromX0)
{
	CarReConfigFixture fix;
	int idx = 0;

	tSuspension susp = makeSuspension(
		-50000.0f, 0.0f, 0.1f, 0.3f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);

	tdble X0 = 0.35f;
	SimSuspThirdReConfig(&fix.car, idx, &susp, 0.0f, X0);

	// xMax is set to X0 in SimSuspThirdReConfig
	EXPECT_FLOAT_EQ(susp.spring.xMax, 0.35f);
}


TEST(SimSuspThirdReConfigTest, NoneAdjustable_NoChange)
{
	CarReConfigFixture fix;
	int idx = 1;
	// All pit values default (min == max == 0)

	tSuspension susp = makeSuspension(
		-60000.0f, 0.0f, 0.1f, 0.3f, 1.0f, 0.0f,
		2000.0f, 1500.0f, 0.5f, 1800.0f, 1200.0f, 0.5f
	);

	SimSuspThirdReConfig(&fix.car, idx, &susp, 0.0f, 0.2f);

	// Fields unchanged
	EXPECT_FLOAT_EQ(susp.spring.K, -60000.0f);
	EXPECT_FLOAT_EQ(susp.damper.bump.C1, 2000.0f);
	EXPECT_FLOAT_EQ(susp.damper.bump.C2, 1500.0f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.C1, 1800.0f);
	EXPECT_FLOAT_EQ(susp.damper.rebound.C2, 1200.0f);

	// xMax, x0, F0 are always recomputed
	EXPECT_FLOAT_EQ(susp.spring.xMax, 0.2f);
	EXPECT_FLOAT_EQ(susp.spring.x0, 1.0f * 0.2f);
	EXPECT_FLOAT_EQ(susp.spring.F0, 0.0f);
}


// ---------------------------------------------------------------------------
// Parameterized test: verify correct array indexing for axle indices 0-1
// ---------------------------------------------------------------------------

struct ThirdIndexCase {
	const char* name;
	int index;
};

class SimSuspThirdReConfigIndexTest : public ::testing::TestWithParam<ThirdIndexCase> {};

TEST_P(SimSuspThirdReConfigIndexTest, CorrectArrayElementRead)
{
	const ThirdIndexCase& c = GetParam();

	CarReConfigFixture fix;
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	tdble uniqueK = 70000.0f + c.index * 10000.0f;
	fix.setSuspSetupValue(&setup.thirdspring[c.index], uniqueK, 50000.0f, 150000.0f);

	tSuspension susp = makeSuspension(
		-50000.0f, 0.0f, 0.1f, 0.3f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);

	SimSuspThirdReConfig(&fix.car, c.index, &susp, 0.0f, 0.2f);

	EXPECT_FLOAT_EQ(susp.spring.K, -uniqueK) << "case: " << c.name;
}

INSTANTIATE_TEST_SUITE_P(AxleIndices, SimSuspThirdReConfigIndexTest, ::testing::Values(
	ThirdIndexCase{"Front", 0},
	ThirdIndexCase{"Rear",  1}
));
