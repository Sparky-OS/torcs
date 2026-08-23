/***************************************************************************

    file                 : simsusp_update_test.cpp
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
    Unit tests for SimSuspUpdate().

    SimSuspUpdate computes the total suspension force as
    (springForce + damperForce) * bellcrank, clamped to >= 0.

    springForce and damperForce are static functions in susp.cpp
    and are tested indirectly through SimSuspUpdate:
    - Isolate spring by setting damper coefficients to 0.
    - Isolate damper by using a known spring force contribution.

    Spring model:  f = K * (x - x0) + F0, clamped to >= 0
    Damper model:  two-slope piecewise linear, velocity clamped to [-10, 10]
      |v| < v1:  f = C1 * |v|          (slow region)
      |v| >= v1: f = C2 * |v| + b2     (fast region, b2 = (C1 - C2) * v1)
      Sign of force follows sign of velocity.
*/

#include <gtest/gtest.h>
#include <cmath>
#include "susp_test_helpers.h"


// ---------------------------------------------------------------------------
// Spring-only tests (damper coefficients = 0)
// ---------------------------------------------------------------------------

TEST(SimSuspUpdateSpringTest, CompressedSpring_PositiveForce)
{
	// K=-175000, x0=0.2, x=0.1 (compressed below x0)
	// springForce = -175000 * (0.1 - 0.2) + 5000 = 17500 + 5000 = 22500
	// force = 22500 * bellcrank(1.0) = 22500
	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.2f, 0.5f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	susp.x = 0.1f;
	susp.v = 0.0f;

	SimSuspUpdate(&susp);

	EXPECT_NEAR(susp.force, 22500.0f, 1e-1f);
}


TEST(SimSuspUpdateSpringTest, AtRestPosition_OnlyPreload)
{
	// x == x0: springForce = K * 0 + F0 = F0
	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.2f, 0.5f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	susp.x = 0.2f;
	susp.v = 0.0f;

	SimSuspUpdate(&susp);

	EXPECT_NEAR(susp.force, 5000.0f, 1e-1f);
}


TEST(SimSuspUpdateSpringTest, ExtendedBeyondRest_SpringForceClampedToZero)
{
	// x > x0 enough that K*(x-x0)+F0 < 0
	// K=-175000, x0=0.1, F0=5000, x=0.2
	// springForce = -175000 * (0.2 - 0.1) + 5000 = -17500 + 5000 = -12500 < 0 => clamped to 0
	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.1f, 0.5f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	susp.x = 0.2f;
	susp.v = 0.0f;

	SimSuspUpdate(&susp);

	EXPECT_FLOAT_EQ(susp.force, 0.0f);
}


TEST(SimSuspUpdateSpringTest, ZeroPreload_AtRest_ZeroForce)
{
	tSuspension susp = makeSuspension(
		-175000.0f, 0.0f, 0.2f, 0.5f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	susp.x = 0.2f;
	susp.v = 0.0f;

	SimSuspUpdate(&susp);

	EXPECT_FLOAT_EQ(susp.force, 0.0f);
}


TEST(SimSuspUpdateSpringTest, LargeCompression_LargeForce)
{
	// K=-175000, x0=0.3, F0=5000, x=0.0 (maximum compression)
	// springForce = -175000 * (0.0 - 0.3) + 5000 = 52500 + 5000 = 57500
	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.3f, 0.5f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	susp.x = 0.0f;
	susp.v = 0.0f;

	SimSuspUpdate(&susp);

	EXPECT_NEAR(susp.force, 57500.0f, 1e-1f);
}


TEST(SimSuspUpdateSpringTest, BellcrankScaling)
{
	// Same spring as CompressedSpring test but bellcrank = 2.0
	// springForce = -175000 * (0.1 - 0.2) + 5000 = 22500
	// force = 22500 * 2.0 = 45000
	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.2f, 0.5f, 2.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	susp.x = 0.1f;
	susp.v = 0.0f;

	SimSuspUpdate(&susp);

	EXPECT_NEAR(susp.force, 45000.0f, 1e-1f);
}


// ---------------------------------------------------------------------------
// Damper tests (with known spring contribution)
// ---------------------------------------------------------------------------

// Helper: create a suspension with a known spring force.
// Uses K=-100000, x0=0.1, x=0.0 => springForce = -100000*(0-0.1) + 0 = 10000
// bellcrank=1.0 so output = (10000 + damperForce) * 1.0
static tSuspension makeDamperTestSusp(
	tdble bumpC1, tdble bumpC2, tdble bumpV1,
	tdble reboundC1, tdble reboundC2, tdble reboundV1,
	tdble velocity
)
{
	tSuspension susp = makeSuspension(
		-100000.0f, 0.0f, 0.1f, 0.5f, 1.0f, 0.0f,
		bumpC1, bumpC2, bumpV1,
		reboundC1, reboundC2, reboundV1
	);
	susp.x = 0.0f;    // springForce = -100000*(0-0.1)+0 = 10000
	susp.v = velocity;
	return susp;
}

static const tdble KNOWN_SPRING_FORCE = 10000.0f;


TEST(SimSuspUpdateDamperTest, ZeroVelocity_ZeroDamperForce)
{
	tSuspension susp = makeDamperTestSusp(
		5000.0f, 3000.0f, 0.5f,
		4000.0f, 2000.0f, 0.5f,
		0.0f  // v = 0
	);

	SimSuspUpdate(&susp);

	EXPECT_NEAR(susp.force, KNOWN_SPRING_FORCE, 1e-1f);
}


TEST(SimSuspUpdateDamperTest, BumpSlow_BelowThreshold)
{
	// v = 0.3 (positive = bump), v1 = 0.5 => slow region
	// damperForce = C1 * |v| * sign(v) = 5000 * 0.3 * 1 = 1500
	// total = (10000 + 1500) * 1.0 = 11500
	tSuspension susp = makeDamperTestSusp(
		5000.0f, 3000.0f, 0.5f,
		4000.0f, 2000.0f, 0.5f,
		0.3f
	);

	SimSuspUpdate(&susp);

	EXPECT_NEAR(susp.force, 11500.0f, 1e-1f);
}


TEST(SimSuspUpdateDamperTest, BumpFast_AboveThreshold)
{
	// v = 0.8 (positive = bump), v1 = 0.5 => fast region
	// b2 = (5000 - 3000) * 0.5 = 1000
	// damperForce = (C2 * |v| + b2) * sign(v) = (3000 * 0.8 + 1000) * 1 = 3400
	// total = (10000 + 3400) * 1.0 = 13400
	tSuspension susp = makeDamperTestSusp(
		5000.0f, 3000.0f, 0.5f,
		4000.0f, 2000.0f, 0.5f,
		0.8f
	);

	SimSuspUpdate(&susp);

	EXPECT_NEAR(susp.force, 13400.0f, 1e-1f);
}


TEST(SimSuspUpdateDamperTest, ReboundSlow_BelowThreshold)
{
	// v = -0.3 (negative = rebound), v1 = 0.5 => slow region
	// damperForce = C1 * |v| * sign(v) = 4000 * 0.3 * (-1) = -1200
	// total = (10000 + (-1200)) * 1.0 = 8800
	tSuspension susp = makeDamperTestSusp(
		5000.0f, 3000.0f, 0.5f,
		4000.0f, 2000.0f, 0.5f,
		-0.3f
	);

	SimSuspUpdate(&susp);

	EXPECT_NEAR(susp.force, 8800.0f, 1e-1f);
}


TEST(SimSuspUpdateDamperTest, ReboundFast_AboveThreshold)
{
	// v = -0.8 (negative = rebound), v1 = 0.5 => fast region
	// b2 = (4000 - 2000) * 0.5 = 1000
	// damperForce = (C2 * |v| + b2) * sign(v) = (2000 * 0.8 + 1000) * (-1) = -2600
	// total = (10000 + (-2600)) * 1.0 = 7400
	tSuspension susp = makeDamperTestSusp(
		5000.0f, 3000.0f, 0.5f,
		4000.0f, 2000.0f, 0.5f,
		-0.8f
	);

	SimSuspUpdate(&susp);

	EXPECT_NEAR(susp.force, 7400.0f, 1e-1f);
}


TEST(SimSuspUpdateDamperTest, VelocityClamped_Above10)
{
	// v = 15.0, clamped to 10.0. Bump slow: v1 = 20 (so always slow region)
	// damperForce = 1000 * 10.0 * 1 = 10000
	// total = (10000 + 10000) * 1.0 = 20000
	tSuspension susp = makeDamperTestSusp(
		1000.0f, 500.0f, 20.0f,
		1000.0f, 500.0f, 20.0f,
		15.0f
	);

	SimSuspUpdate(&susp);

	EXPECT_NEAR(susp.force, 20000.0f, 1e-1f);
}


TEST(SimSuspUpdateDamperTest, VelocityClamped_BelowMinus10)
{
	// v = -15.0, clamped to -10.0. Rebound slow: v1 = 20
	// damperForce = 1000 * 10.0 * (-1) = -10000
	// total = (10000 + (-10000)) = 0 <= 0, clamped to 0
	tSuspension susp = makeDamperTestSusp(
		1000.0f, 500.0f, 20.0f,
		1000.0f, 500.0f, 20.0f,
		-15.0f
	);

	SimSuspUpdate(&susp);

	EXPECT_FLOAT_EQ(susp.force, 0.0f);
}


TEST(SimSuspUpdateDamperTest, ExactlyAtThreshold_UsesFastRegion)
{
	// v = 0.5, v1 = 0.5. Condition is av < v1 (strict), so this is fast region.
	// b2 = (5000 - 3000) * 0.5 = 1000
	// damperForce = (3000 * 0.5 + 1000) * 1 = 2500
	// But also: slow would give 5000 * 0.5 = 2500 (continuous at threshold!)
	// total = (10000 + 2500) * 1.0 = 12500
	tSuspension susp = makeDamperTestSusp(
		5000.0f, 3000.0f, 0.5f,
		4000.0f, 2000.0f, 0.5f,
		0.5f
	);

	SimSuspUpdate(&susp);

	EXPECT_NEAR(susp.force, 12500.0f, 1e-1f);
}


TEST(SimSuspUpdateDamperTest, DamperOverpowersSpring_ClampedToZero)
{
	// Large rebound damping exceeds spring force
	// Spring: 10000, Damper rebound C1 = 50000, v = -0.5
	// damperForce = 50000 * 0.5 * (-1) = -25000
	// total = 10000 + (-25000) = -15000 < 0 => clamped to 0
	tSuspension susp = makeDamperTestSusp(
		5000.0f, 3000.0f, 0.5f,
		50000.0f, 30000.0f, 1.0f,
		-0.5f
	);

	SimSuspUpdate(&susp);

	EXPECT_FLOAT_EQ(susp.force, 0.0f);
}


TEST(SimSuspUpdateDamperTest, InternalForceExactlyZero_ClampedToZero)
{
	// Spring force = 10000, damper force = -10000 => total = 0 <= 0 => clamped
	// Rebound C1 = 20000, v = -0.5, v1 = 1.0 (slow region)
	// damperForce = 20000 * 0.5 * (-1) = -10000
	tSuspension susp = makeDamperTestSusp(
		5000.0f, 3000.0f, 0.5f,
		20000.0f, 10000.0f, 1.0f,
		-0.5f
	);

	SimSuspUpdate(&susp);

	EXPECT_FLOAT_EQ(susp.force, 0.0f);
}


// ---------------------------------------------------------------------------
// Parameterized test: systematic sweep of damper velocities
// ---------------------------------------------------------------------------

struct DamperCase {
	const char* name;
	tdble velocity;
	tdble bumpC1, bumpC2, bumpV1;
	tdble reboundC1, reboundC2, reboundV1;
	tdble expectedForce;
};

class SimSuspUpdateDamperParamTest : public ::testing::TestWithParam<DamperCase> {};

TEST_P(SimSuspUpdateDamperParamTest, DamperForceMatchesExpected)
{
	const DamperCase& c = GetParam();

	tSuspension susp = makeSuspension(
		-100000.0f, 0.0f, 0.1f, 0.5f, 1.0f, 0.0f,
		c.bumpC1, c.bumpC2, c.bumpV1,
		c.reboundC1, c.reboundC2, c.reboundV1
	);
	susp.x = 0.0f;  // springForce = 10000
	susp.v = c.velocity;

	SimSuspUpdate(&susp);

	EXPECT_NEAR(susp.force, c.expectedForce, 1e-1f) << "case: " << c.name;
}

// All cases use springForce = 10000 (K=-100000, x0=0.1, x=0.0, F0=0, bellcrank=1)
INSTANTIATE_TEST_SUITE_P(DamperCases, SimSuspUpdateDamperParamTest, ::testing::Values(
	//                                   bC1     bC2     bv1     rC1     rC2     rv1     expected
	// Bump slow
	DamperCase{"bump_slow_01",   0.1f,   5000.f, 3000.f, 0.5f,  4000.f, 2000.f, 0.5f,  10500.f},
	DamperCase{"bump_slow_04",   0.4f,   5000.f, 3000.f, 0.5f,  4000.f, 2000.f, 0.5f,  12000.f},
	// Bump fast
	DamperCase{"bump_fast_10",   1.0f,   5000.f, 3000.f, 0.5f,  4000.f, 2000.f, 0.5f,  14000.f},
	DamperCase{"bump_fast_50",   5.0f,   5000.f, 3000.f, 0.5f,  4000.f, 2000.f, 0.5f,  26000.f},
	// Rebound slow
	DamperCase{"reb_slow_01",   -0.1f,   5000.f, 3000.f, 0.5f,  4000.f, 2000.f, 0.5f,   9600.f},
	DamperCase{"reb_slow_04",   -0.4f,   5000.f, 3000.f, 0.5f,  4000.f, 2000.f, 0.5f,   8400.f},
	// Rebound fast
	DamperCase{"reb_fast_10",   -1.0f,   5000.f, 3000.f, 0.5f,  4000.f, 2000.f, 0.5f,   7000.f},
	DamperCase{"reb_fast_50",   -5.0f,   5000.f, 3000.f, 0.5f,  4000.f, 2000.f, 0.5f,      0.f},
	// Velocity clamp
	DamperCase{"clamp_pos",     15.0f,   1000.f,  500.f, 20.f,  1000.f,  500.f, 20.f,  20000.f},
	DamperCase{"clamp_neg",    -15.0f,   1000.f,  500.f, 20.f,  1000.f,  500.f, 20.f,      0.f},
	// Symmetric coefficients (C1 == C2, b2 = 0)
	DamperCase{"sym_bump",       0.5f,   5000.f, 5000.f, 0.5f,  5000.f, 5000.f, 0.5f,  12500.f},
	DamperCase{"sym_rebound",   -0.5f,   5000.f, 5000.f, 0.5f,  5000.f, 5000.f, 0.5f,   7500.f},
	// Zero damping
	DamperCase{"zero_damp",      1.0f,      0.f,    0.f, 0.5f,     0.f,    0.f, 0.5f,  10000.f}
));
