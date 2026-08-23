/***************************************************************************

    file                 : simaxle_update_test.cpp
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
    Unit tests for SimAxleUpdate().

    SimAxleUpdate computes:
    1. Anti-roll bar force: farb = arbSuspSpringK * (stl - str)
       where stl = left wheel susp.x, str = right wheel susp.x
    2. Third element: averages left/right suspension travel and velocity
       into thirdSusp, calls SimSuspUpdate, then extracts force
       (only if thirdSusp.x < xMax AND force > 0)
    3. Distributes forces:
       Right wheel axleFz = farb + fthird
       Left  wheel axleFz = -farb + fthird

    Tests isolate the ARB and third element by zeroing one while testing
    the other, then test combined behavior.
*/

#include <gtest/gtest.h>
#include <cmath>
#include "axle_test_helpers.h"
#include "susp_test_helpers.h"


// ---------------------------------------------------------------------------
// Helper: create a third element suspension that produces a known force.
// Uses spring-only (damper coefficients = 0), bellcrank = 1.0.
// K is stored negative; force = K*(x - x0) + F0, clamped >= 0.
// ---------------------------------------------------------------------------

/// Create a third suspension with known spring: K=-100000, F0=0, x0=0.1, xMax=0.5
/// When thirdSusp.x = 0.0: springForce = -100000*(0 - 0.1) + 0 = 10000
/// force = 10000 * bellcrank(1.0) = 10000
static tSuspension makeThirdSuspWithForce()
{
	return makeSuspension(
		-100000.0f, 0.0f, 0.1f, 0.5f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
}


/// Create a third suspension that produces zero force (extended spring).
/// K=-100000, F0=0, x0=0.0, xMax=0.5. At x=0: force = K*(0-0) + 0 = 0.
static tSuspension makeThirdSuspNoForce()
{
	return makeSuspension(
		-100000.0f, 0.0f, 0.0f, 0.5f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
}


// ---------------------------------------------------------------------------
// Anti-roll bar tests (third element produces no force)
// ---------------------------------------------------------------------------

TEST(SimAxleUpdateArbTest, EqualTravel_ZeroArbForce)
{
	AxleUpdateFixture fix;
	fix.setArbSpringK(FRNT, 50000.0f);
	fix.setThirdSusp(FRNT, makeThirdSuspNoForce());
	fix.setWheelSusp(FRNT_RGT, 0.02f, 0.0f);  // right
	fix.setWheelSusp(FRNT_LFT, 0.02f, 0.0f);  // left

	SimAxleUpdate(&fix.car, FRNT);

	// farb = 50000 * (0.02 - 0.02) = 0
	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_RGT].axleFz, 0.0f);
	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_LFT].axleFz, 0.0f);
}


TEST(SimAxleUpdateArbTest, LeftMoreCompressed_PositiveArbOnRight)
{
	AxleUpdateFixture fix;
	fix.setArbSpringK(FRNT, 50000.0f);
	fix.setThirdSusp(FRNT, makeThirdSuspNoForce());
	fix.setWheelSusp(FRNT_RGT, 0.01f, 0.0f);  // right, str
	fix.setWheelSusp(FRNT_LFT, 0.02f, 0.0f);  // left,  stl

	SimAxleUpdate(&fix.car, FRNT);

	// farb = 50000 * (0.02 - 0.01) = 500
	// right = farb + fthird = 500 + 0 = 500
	// left  = -farb + fthird = -500 + 0 = -500
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].axleFz, 500.0f, 1e-1f);
	EXPECT_NEAR(fix.car.wheel[FRNT_LFT].axleFz, -500.0f, 1e-1f);
}


TEST(SimAxleUpdateArbTest, RightMoreCompressed_NegativeArbOnRight)
{
	AxleUpdateFixture fix;
	fix.setArbSpringK(FRNT, 50000.0f);
	fix.setThirdSusp(FRNT, makeThirdSuspNoForce());
	fix.setWheelSusp(FRNT_RGT, 0.02f, 0.0f);  // right, str
	fix.setWheelSusp(FRNT_LFT, 0.01f, 0.0f);  // left,  stl

	SimAxleUpdate(&fix.car, FRNT);

	// farb = 50000 * (0.01 - 0.02) = -500
	// right = -500, left = 500
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].axleFz, -500.0f, 1e-1f);
	EXPECT_NEAR(fix.car.wheel[FRNT_LFT].axleFz, 500.0f, 1e-1f);
}


TEST(SimAxleUpdateArbTest, ZeroArbSpring_NoArbForce)
{
	AxleUpdateFixture fix;
	fix.setArbSpringK(FRNT, 0.0f);
	fix.setThirdSusp(FRNT, makeThirdSuspNoForce());
	fix.setWheelSusp(FRNT_RGT, 0.01f, 0.0f);
	fix.setWheelSusp(FRNT_LFT, 0.03f, 0.0f);

	SimAxleUpdate(&fix.car, FRNT);

	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_RGT].axleFz, 0.0f);
	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_LFT].axleFz, 0.0f);
}


TEST(SimAxleUpdateArbTest, LargeAsymmetry_LargeForce)
{
	AxleUpdateFixture fix;
	fix.setArbSpringK(FRNT, 100000.0f);
	fix.setThirdSusp(FRNT, makeThirdSuspNoForce());
	fix.setWheelSusp(FRNT_RGT, 0.0f, 0.0f);   // right
	fix.setWheelSusp(FRNT_LFT, 0.05f, 0.0f);  // left

	SimAxleUpdate(&fix.car, FRNT);

	// farb = 100000 * (0.05 - 0.0) = 5000
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].axleFz, 5000.0f, 1e-1f);
	EXPECT_NEAR(fix.car.wheel[FRNT_LFT].axleFz, -5000.0f, 1e-1f);
}


// ---------------------------------------------------------------------------
// Third element tests (ARB spring = 0 to isolate)
// ---------------------------------------------------------------------------

TEST(SimAxleUpdateThirdTest, ThirdSusp_AveragesTravel)
{
	AxleUpdateFixture fix;
	fix.setArbSpringK(FRNT, 0.0f);

	// Third susp with known spring: K=-100000, x0=0.1, xMax=0.5
	fix.setThirdSusp(FRNT, makeThirdSuspWithForce());
	fix.setWheelSusp(FRNT_RGT, 0.02f, 0.5f);  // right: str=0.02, vr=0.5
	fix.setWheelSusp(FRNT_LFT, 0.04f, 1.0f);  // left:  stl=0.04, vl=1.0

	SimAxleUpdate(&fix.car, FRNT);

	// Verify thirdSusp.x = (stl + str)/2 = (0.04 + 0.02)/2 = 0.03
	EXPECT_NEAR(fix.car.axle[FRNT].thirdSusp.x, 0.03f, 1e-6f);
	// Verify thirdSusp.v = (vl + vr)/2 = (1.0 + 0.5)/2 = 0.75
	EXPECT_NEAR(fix.car.axle[FRNT].thirdSusp.v, 0.75f, 1e-6f);
}


TEST(SimAxleUpdateThirdTest, ThirdSusp_ForcePositive_BelowXMax)
{
	AxleUpdateFixture fix;
	fix.setArbSpringK(FRNT, 0.0f);

	// Third susp: K=-100000, F0=0, x0=0.1, xMax=0.5, bellcrank=1.0
	fix.setThirdSusp(FRNT, makeThirdSuspWithForce());

	// Both wheels at same position => thirdSusp.x = (0.0 + 0.0)/2 = 0.0
	// springForce = -100000*(0.0 - 0.1) + 0 = 10000
	// force = 10000 * 1.0 = 10000
	// x=0.0 < xMax=0.5 and force=10000 > 0 => fthird = 10000/2 = 5000
	fix.setWheelSusp(FRNT_RGT, 0.0f, 0.0f);
	fix.setWheelSusp(FRNT_LFT, 0.0f, 0.0f);

	SimAxleUpdate(&fix.car, FRNT);

	// Both wheels get fthird = 5000 (ARB = 0)
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].axleFz, 5000.0f, 1e-1f);
	EXPECT_NEAR(fix.car.wheel[FRNT_LFT].axleFz, 5000.0f, 1e-1f);
}


TEST(SimAxleUpdateThirdTest, ThirdSusp_AtXMax_NoForce)
{
	AxleUpdateFixture fix;
	fix.setArbSpringK(FRNT, 0.0f);

	// Third susp: K=-100000, F0=5000, x0=0.01, xMax=0.05, bellcrank=1.0
	// Set travel so that thirdSusp.x ends up >= xMax
	tSuspension third = makeSuspension(
		-100000.0f, 5000.0f, 0.01f, 0.05f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	fix.setThirdSusp(FRNT, third);

	// Both wheels at 0.05 => thirdSusp.x = 0.05 >= xMax(0.05)
	// Condition: thirdSusp.x < xMax fails => fthird = 0
	fix.setWheelSusp(FRNT_RGT, 0.05f, 0.0f);
	fix.setWheelSusp(FRNT_LFT, 0.05f, 0.0f);

	SimAxleUpdate(&fix.car, FRNT);

	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_RGT].axleFz, 0.0f);
	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_LFT].axleFz, 0.0f);
}


TEST(SimAxleUpdateThirdTest, ThirdSusp_ForceNegativeOrZero_NoContribution)
{
	AxleUpdateFixture fix;
	fix.setArbSpringK(FRNT, 0.0f);

	// Third susp: K=-100000, F0=0, x0=0.0, xMax=0.5
	// At x=0.0: springForce = -100000*(0 - 0) + 0 = 0
	// force <= 0 => fthird = 0
	fix.setThirdSusp(FRNT, makeThirdSuspNoForce());
	fix.setWheelSusp(FRNT_RGT, 0.0f, 0.0f);
	fix.setWheelSusp(FRNT_LFT, 0.0f, 0.0f);

	SimAxleUpdate(&fix.car, FRNT);

	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_RGT].axleFz, 0.0f);
	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_LFT].axleFz, 0.0f);
}


TEST(SimAxleUpdateThirdTest, ThirdSusp_ExtendedSpring_ZeroForce)
{
	AxleUpdateFixture fix;
	fix.setArbSpringK(FRNT, 0.0f);

	// K=-100000, F0=0, x0=0.1, xMax=0.5
	// At x = 0.2 (extended beyond x0):
	// springForce = -100000*(0.2 - 0.1) + 0 = -10000 < 0 => clamped to 0
	// force = 0 * 1.0 = 0 => fthird = 0
	fix.setThirdSusp(FRNT, makeThirdSuspWithForce());
	fix.setWheelSusp(FRNT_RGT, 0.2f, 0.0f);
	fix.setWheelSusp(FRNT_LFT, 0.2f, 0.0f);

	SimAxleUpdate(&fix.car, FRNT);

	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_RGT].axleFz, 0.0f);
	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_LFT].axleFz, 0.0f);
}


// ---------------------------------------------------------------------------
// Combined ARB + third element tests
// ---------------------------------------------------------------------------

TEST(SimAxleUpdateCombinedTest, CombinedForces_AddCorrectly)
{
	AxleUpdateFixture fix;
	fix.setArbSpringK(FRNT, 40000.0f);

	// Third susp: K=-100000, F0=0, x0=0.1, xMax=0.5
	fix.setThirdSusp(FRNT, makeThirdSuspWithForce());

	// right (str) = 0.01, left (stl) = 0.03
	// farb = 40000 * (0.03 - 0.01) = 800
	fix.setWheelSusp(FRNT_RGT, 0.01f, 0.0f);
	fix.setWheelSusp(FRNT_LFT, 0.03f, 0.0f);

	// thirdSusp.x = (0.03 + 0.01)/2 = 0.02
	// springForce = -100000*(0.02 - 0.1) + 0 = 8000
	// force = 8000 * 1.0 = 8000
	// x=0.02 < xMax=0.5 and force > 0 => fthird = 8000/2 = 4000

	SimAxleUpdate(&fix.car, FRNT);

	// right = farb + fthird = 800 + 4000 = 4800
	// left  = -farb + fthird = -800 + 4000 = 3200
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].axleFz, 4800.0f, 1e-1f);
	EXPECT_NEAR(fix.car.wheel[FRNT_LFT].axleFz, 3200.0f, 1e-1f);
}


TEST(SimAxleUpdateCombinedTest, SymmetricTravel_OnlyThirdContributes)
{
	AxleUpdateFixture fix;
	fix.setArbSpringK(FRNT, 60000.0f);
	fix.setThirdSusp(FRNT, makeThirdSuspWithForce());

	// Symmetric travel => farb = 0
	fix.setWheelSusp(FRNT_RGT, 0.02f, 0.0f);
	fix.setWheelSusp(FRNT_LFT, 0.02f, 0.0f);

	// thirdSusp.x = 0.02, springForce = -100000*(0.02 - 0.1) = 8000
	// fthird = 8000/2 = 4000

	SimAxleUpdate(&fix.car, FRNT);

	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].axleFz, 4000.0f, 1e-1f);
	EXPECT_NEAR(fix.car.wheel[FRNT_LFT].axleFz, 4000.0f, 1e-1f);
}


// ---------------------------------------------------------------------------
// Index tests: front vs rear axle
// ---------------------------------------------------------------------------

TEST(SimAxleUpdateIndexTest, FrontAxle_UsesWheels0and1)
{
	AxleUpdateFixture fix;
	fix.setArbSpringK(FRNT, 50000.0f);
	fix.setThirdSusp(FRNT, makeThirdSuspNoForce());
	fix.setWheelSusp(FRNT_RGT, 0.01f, 0.0f);  // right front
	fix.setWheelSusp(FRNT_LFT, 0.03f, 0.0f);  // left front

	// Also set rear wheels to different values to verify they're not read
	fix.setWheelSusp(REAR_RGT, 0.10f, 0.0f);
	fix.setWheelSusp(REAR_LFT, 0.20f, 0.0f);

	SimAxleUpdate(&fix.car, FRNT);

	// farb = 50000 * (0.03 - 0.01) = 1000
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].axleFz, 1000.0f, 1e-1f);
	EXPECT_NEAR(fix.car.wheel[FRNT_LFT].axleFz, -1000.0f, 1e-1f);

	// Rear wheels should be untouched (still 0 from init)
	EXPECT_FLOAT_EQ(fix.car.wheel[REAR_RGT].axleFz, 0.0f);
	EXPECT_FLOAT_EQ(fix.car.wheel[REAR_LFT].axleFz, 0.0f);
}


TEST(SimAxleUpdateIndexTest, RearAxle_UsesWheels2and3)
{
	AxleUpdateFixture fix;
	fix.setArbSpringK(REAR, 50000.0f);
	fix.setThirdSusp(REAR, makeThirdSuspNoForce());
	fix.setWheelSusp(REAR_RGT, 0.01f, 0.0f);  // right rear
	fix.setWheelSusp(REAR_LFT, 0.03f, 0.0f);  // left rear

	// Also set front wheels to different values
	fix.setWheelSusp(FRNT_RGT, 0.10f, 0.0f);
	fix.setWheelSusp(FRNT_LFT, 0.20f, 0.0f);

	SimAxleUpdate(&fix.car, REAR);

	// farb = 50000 * (0.03 - 0.01) = 1000
	EXPECT_NEAR(fix.car.wheel[REAR_RGT].axleFz, 1000.0f, 1e-1f);
	EXPECT_NEAR(fix.car.wheel[REAR_LFT].axleFz, -1000.0f, 1e-1f);

	// Front wheels should be untouched
	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_RGT].axleFz, 0.0f);
	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_LFT].axleFz, 0.0f);
}


// ---------------------------------------------------------------------------
// Parameterized test: systematic sweep of axle update scenarios
// ---------------------------------------------------------------------------

struct AxleUpdateCase {
	const char* name;
	int index;		// Axle, FRNT or REAR
	tdble arbK;		// anti rollbar spring constant
	tdble str;      // right wheel susp.x
	tdble stl;      // left wheel susp.x
	tdble vr;       // right wheel susp.v
	tdble vl;       // left wheel susp.v
	// Third element: K=-100000, F0=0, x0=0.1, xMax (configurable)
	tdble thirdXMax;
	tdble expectedRightFz;
	tdble expectedLeftFz;
};

/// Compute expected axle forces mirroring SimAxleUpdate logic.
static void computeExpectedAxleForces(
	tdble arbK, tdble str, tdble stl, tdble vr, tdble vl,
	tdble thirdK, tdble thirdF0, tdble thirdX0, tdble thirdXMax,
	tdble thirdBellcrank,
	tdble& outRight, tdble& outLeft
)
{
	// Anti-roll bar
	tdble farb = arbK * (stl - str);

	// Third element
	tdble tx = (stl + str) / 2.0f;
	tdble tv = (vl + vr) / 2.0f;

	// Spring force: K * (x - x0) + F0, clamped >= 0
	tdble sf = thirdK * (tx - thirdX0) + thirdF0;
	if (sf < 0.0f) sf = 0.0f;

	// Damper force: 0 (all damper coefficients are 0 in our test cases)
	tdble df = 0.0f;

	tdble internalForce = sf + df;
	tdble thirdForce;
	if (internalForce <= 0.0f) {
		thirdForce = 0.0f;
	} else {
		thirdForce = internalForce * thirdBellcrank;
	}

	tdble fthird = 0.0f;
	if (tx < thirdXMax && thirdForce > 0.0f) {
		fthird = thirdForce / 2.0f;
	}

	outRight = farb + fthird;
	outLeft = -farb + fthird;
}

class SimAxleUpdateParamTest : public ::testing::TestWithParam<AxleUpdateCase> {};

TEST_P(SimAxleUpdateParamTest, ForcesMatchExpected)
{
	const AxleUpdateCase& c = GetParam();

	AxleUpdateFixture fix;
	fix.setArbSpringK(c.index, c.arbK);

	// Third susp: K=-100000, F0=0, x0=0.1, xMax=c.thirdXMax, bellcrank=1.0
	tSuspension third = makeSuspension(
		-100000.0f, 0.0f, 0.1f, c.thirdXMax, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	fix.setThirdSusp(c.index, third);

	int rWheel = c.index * 2;
	int lWheel = c.index * 2 + 1;
	fix.setWheelSusp(rWheel, c.str, c.vr);
	fix.setWheelSusp(lWheel, c.stl, c.vl);

	SimAxleUpdate(&fix.car, c.index);

	EXPECT_NEAR(fix.car.wheel[rWheel].axleFz, c.expectedRightFz, 1e-1f) << "case: " << c.name << " (right)";
	EXPECT_NEAR(fix.car.wheel[lWheel].axleFz, c.expectedLeftFz, 1e-1f) << "case: " << c.name << " (left)";
}


INSTANTIATE_TEST_SUITE_P(AxleUpdateCases, SimAxleUpdateParamTest, ::testing::Values(
	//                        idx   arbK      str     stl     vr    vl    xMax    expR     expL
	
	// ARB only (third produces no force: avg travel >= x0=0.1 so springForce <= 0)
	AxleUpdateCase{ "equal_no_third", FRNT, 50000.f, 0.10f, 0.10f, 0.f, 0.f, 0.5f, 0.f, 0.f },
	
	// avg=(0.12+0.14)/2=0.13 >= x0=0.1 => springForce=0 => fthird=0
	// farb=50000*(0.14-0.12)=1000
	AxleUpdateCase{ "left_comp", FRNT, 50000.f, 0.12f, 0.14f, 0.f, 0.f, 0.5f, 1000.f, -1000.f },
	
	// farb=50000*(0.12-0.14)=-1000
	AxleUpdateCase{"right_comp", FRNT, 50000.f, 0.14f, 0.12f, 0.f, 0.f, 0.5f, -1000.f, 1000.f},

	// avg=(0.20+0.30)/2=0.25 >= x0=0.1 => fthird=0
	// farb=100000*(0.30-0.20)=10000
	AxleUpdateCase{"large_asym", REAR, 100000.f, 0.20f, 0.30f, 0.f, 0.f, 0.5f, 10000.f, -10000.f},

	// Third only (arbK = 0, travel compressed so spring > 0)
	// avg = 0.0, spring = -100000*(0.0 - 0.1) + 0 = 10000, fthird = 10000/2 = 5000
	AxleUpdateCase{"third_only", FRNT, 0.f, 0.0f, 0.0f, 0.f, 0.f, 0.5f, 5000.f, 5000.f},

	// avg = 0.04, spring = -100000*(0.04-0.1) = 6000, fthird = 3000
	AxleUpdateCase{"third_asym", FRNT, 0.f, 0.02f, 0.06f, 0.f, 0.f, 0.5f, 3000.f, 3000.f},

	// Third at xMax (no contribution)
	// avg = 0.50 >= xMax=0.50 => fthird = 0
	AxleUpdateCase{"third_at_xmax", FRNT, 0.f, 0.50f, 0.50f, 0.f, 0.f, 0.5f, 0.f, 0.f},

	// Combined ARB + third
	// arbK=40000, str=0.01, stl=0.03 => farb = 40000*(0.03-0.01) = 800
	// avg = 0.02, spring = -100000*(0.02-0.1) = 8000, fthird = 4000
	// right = 800 + 4000 = 4800, left = -800 + 4000 = 3200
	AxleUpdateCase{"combined", FRNT, 40000.f, 0.01f, 0.03f, 0.f, 0.f, 0.5f, 4800.f, 3200.f},

	// Rear axle index (ARB only, avg travel >= x0 => fthird=0)
	// avg=(0.12+0.14)/2=0.13 >= x0=0.1 => fthird=0
	// farb=50000*(0.14-0.12)=1000
	AxleUpdateCase{"rear_arb", REAR, 50000.f, 0.12f, 0.14f, 0.f, 0.f, 0.5f, 1000.f, -1000.f},

	// Zero everything
	AxleUpdateCase{"all_zero", FRNT, 0.f, 0.0f, 0.0f, 0.f, 0.f, 0.0f, 0.f, 0.f}
));
