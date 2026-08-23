/***************************************************************************

    file                 : simdifferential_update_brake_integration_test.cpp
    created              : Wed Mar 11 00:00:00 CET 2026
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
    Parameterized tests that focus specifically on the repeated brake
    integration logic inside SimDifferentialUpdate() for simuv2.

    Why this file exists:
    ---------------------
    The differential update currently applies very similar brake integration
    code in three locations:
      1) Spool path (single combined axle speed)
      2) Split path side 0
      3) Split path side 1

    Important test strategy choices:
    --------------------------------
    - We deliberately isolate the brake math from engine and drive-torque
      effects whenever possible.
    - We use DIFF_NONE for split-path tests so DrTq0/DrTq1 are zero by design;
      this keeps pre-brake wheel speed unchanged and makes expectations easy
      to verify from first principles.
    - We disable the engine stub and use first=0 to avoid additional scaling.

    Coverage status versus the canonical matrix (A-F) per call site:
    ----------------------------------------------------------------
    Canonical matrix:
      A: spinVel > 0, no overshoot
      B: spinVel > 0, overshoot clamp to zero
      C: spinVel < 0, no overshoot
      D: spinVel < 0, overshoot clamp to zero
      E: spinVel == 0, brkTq > 0 (zero-spin guard path)
      F: spinVel == 0, brkTq == 0 (baseline no-op)

    Call sites in simuv2 differential update:
      1) Spool path brake integration
      2) Split path axis 0 brake integration         
      3) Split path axis 1 brake integration

    Notes:
      - Axis 0 and axis 1 include mirrored single-axis coverage for symmetry.
      - mixed_axis_behaviour verifies side independence under asymmetric inputs.
*/

#include <gtest/gtest.h>
#include "differential_test_helpers.h"


namespace {

struct SpoolBrakeCase {
	const char* name;
	tdble spinVel;
	tdble brkTq0;
	tdble brkTq1;
	tdble outI0;
	tdble outI1;
	tdble expectedSpinVel;
};


class SimDifferentialUpdateSpoolBrakeParamTest
	: public ::testing::TestWithParam<SpoolBrakeCase> {
};


TEST_P(SimDifferentialUpdateSpoolBrakeParamTest, BrakeIntegrationMatchesExpectedBehavior)
{
	/*
		This test targets the spool branch only.

		Setup details:
		- We configure DIFF_SPOOL so updateSpool() is used.
		- Drive torque contribution is neutralized by setting:
		    differential.in.Tq = 0
		    inAxis[0].Tq = 0
		    inAxis[1].Tq = 0
		  Therefore, the pre-brake spin stays exactly at the initial spin value.
		- We assign identical wheel input spin values because spool models one
		  combined speed and writes it back to both output sides.

		Expected behavior under test:
		- Brake torque sign follows SIGN(spinVel)
		- ndot = dt * BrTq / (I0 + I1)
		- If braking would cross zero in one step, speed clamps exactly to zero
		- If speed is exactly zero and computed ndot is negative, ndot is forced
		  to zero (no artificial negative motion from standstill)
	*/
	const SpoolBrakeCase& c = GetParam();

	DifferentialUpdateFixture fix;
	DifferentialTestDisableEngineStub();
	SimDeltaTime = 0.01f;

	fix.differential.type = DIFF_SPOOL;
	fix.differential.in.Tq = 0.0f;

	fix.inAxis[0].spinVel = c.spinVel;
	fix.inAxis[1].spinVel = c.spinVel;
	fix.inAxis[0].Tq = 0.0f;
	fix.inAxis[1].Tq = 0.0f;
	fix.inAxis[0].brkTq = c.brkTq0;
	fix.inAxis[1].brkTq = c.brkTq1;
	fix.outAxis[0].I = c.outI0;
	fix.outAxis[1].I = c.outI1;

	SimDifferentialUpdate(&fix.car, &fix.differential, 0);

	EXPECT_NEAR(fix.outAxis[0].spinVel, c.expectedSpinVel, 1e-6f);
	EXPECT_NEAR(fix.outAxis[1].spinVel, c.expectedSpinVel, 1e-6f);
}


INSTANTIATE_TEST_SUITE_P(
	SpoolBrakeCases,
	SimDifferentialUpdateSpoolBrakeParamTest,
	::testing::Values(
		/*
			Case 1: Positive spin, moderate brake, no overshoot.
			BrTq = -20, ndot = 0.01 * (-20) / (2+2) = -0.05
			Expected: 5.00 -> 4.95
		*/
		SpoolBrakeCase{"pos_no_overshoot", 5.0f, 10.0f, 10.0f, 2.0f, 2.0f, 4.95f},
		/*
			Case 2: Negative spin, moderate brake, no overshoot.
			SIGN(-) flips BrTq positive.
			BrTq = +20, ndot = +0.05
			Expected: -5.00 -> -4.95
		*/
		SpoolBrakeCase{"neg_no_overshoot", -5.0f, 10.0f, 10.0f, 2.0f, 2.0f, -4.95f},
		/*
			Case 3: Positive spin, very strong brake, overshoot clamp.
			Raw ndot would cross zero in one step, so logic clamps exactly to zero.
		*/
		SpoolBrakeCase{"pos_overshoot_clamp_zero", 0.2f, 200.0f, 200.0f, 2.0f, 2.0f, 0.0f},
		/*
			Case 4: Negative spin, very strong brake, overshoot clamp.
			This is the mirror image of case 3 and protects the reverse-rotation
			branch from regressions.
		*/
		SpoolBrakeCase{"neg_overshoot_clamp_zero", -0.2f, 200.0f, 200.0f, 2.0f, 2.0f, 0.0f},
		/*
			Case 5: Exactly zero spin with positive brake torque.
			SIGN(0) is +1 in TORCS, giving negative BrTq and negative ndot.
			The dedicated zero-velocity guard forces ndot back to 0.
		*/
		SpoolBrakeCase{"zero_spin_guard_positive_brake", 0.0f, 50.0f, 50.0f, 2.0f, 2.0f, 0.0f},
		/*
			Case 6: Exactly zero spin and zero brake torque.
			Pure baseline sanity case: ndot is naturally zero and the result
			must remain exactly zero without any guard intervention.
		*/
		SpoolBrakeCase{"zero_spin_zero_brake", 0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 0.0f},
		/*
			Case 7: Exact boundary |ndot| == |spinVel|.
			This hardens the strict-inequality branch boundary used by the
			overshoot check. The update lands exactly at zero in one step.
		*/
		SpoolBrakeCase{"exact_boundary_equal_magnitude", 0.2f, 20.0f, 20.0f, 1.0f, 1.0f, 0.0f}
	),
	[](const ::testing::TestParamInfo<SpoolBrakeCase>& info) {
		return info.param.name;
	}
);


struct SplitBrakeCase {
	const char* name;
	tdble spinVel0;
	tdble spinVel1;
	tdble brkTq0;
	tdble brkTq1;
	tdble outI0;
	tdble outI1;
	tdble expectedSpinVel0;
	tdble expectedSpinVel1;
};


class SimDifferentialUpdateSplitBrakeParamTest
	: public ::testing::TestWithParam<SplitBrakeCase> {
};


TEST_P(SimDifferentialUpdateSplitBrakeParamTest, BrakeIntegrationPerSideMatchesExpectedBehavior)
{
	/*
		This test targets the two duplicated non-spool brake blocks:
		- side 0 brake integration
		- side 1 brake integration

		Setup details:
		- We force non-spool code path via DIFF_NONE.
		- For DIFF_NONE, code sets DrTq0 = DrTq1 = 0 regardless of input torque.
		- We set inAxis torque feedback to zero as well.
		  Therefore, the pre-brake integration step contributes exactly 0 and
		  each side enters the brake block with the initial side-specific spin.

		Expected behavior under test (per side independently):
		- BrTq = -SIGN(spinVelSide) * brkTqSide
		- ndot = dt * BrTq / outISide
		- overshoot clamp prevents sign flip through zero
		- zero-spin guard prevents negative motion from standstill

		By using asymmetric side values, we verify that side 0 and side 1 are
		computed independently and not accidentally coupled.
	*/
	const SplitBrakeCase& c = GetParam();

	DifferentialUpdateFixture fix;
	DifferentialTestDisableEngineStub();
	SimDeltaTime = 0.01f;

	fix.differential.type = DIFF_NONE;
	fix.differential.in.Tq = 0.0f;

	fix.inAxis[0].spinVel = c.spinVel0;
	fix.inAxis[1].spinVel = c.spinVel1;
	fix.inAxis[0].Tq = 0.0f;
	fix.inAxis[1].Tq = 0.0f;
	fix.inAxis[0].brkTq = c.brkTq0;
	fix.inAxis[1].brkTq = c.brkTq1;
	fix.outAxis[0].I = c.outI0;
	fix.outAxis[1].I = c.outI1;

	SimDifferentialUpdate(&fix.car, &fix.differential, 0);

	EXPECT_NEAR(fix.outAxis[0].spinVel, c.expectedSpinVel0, 1e-6f);
	EXPECT_NEAR(fix.outAxis[1].spinVel, c.expectedSpinVel1, 1e-6f);
}


INSTANTIATE_TEST_SUITE_P(
	SplitBrakeCases,
	SimDifferentialUpdateSplitBrakeParamTest,
	::testing::Values(
		/*
			Case 1: Axis 0 only, positive spin, no overshoot.
			Axis 1 has zero brake torque and should remain unchanged.
		*/
		SplitBrakeCase{"axis0_pos_no_overshoot_axis1_zero_brake", 4.0f, 2.0f, 20.0f, 0.0f, 2.0f, 1.0f, 3.9f, 2.0f},
		/*
			Case 2: Axis 0 only, negative spin, no overshoot.
			Confirms SIGN handling for reverse rotation on axis 0.
		*/
		SplitBrakeCase{"axis0_neg_no_overshoot_axis1_zero_brake", -4.0f, 2.0f, 20.0f, 0.0f, 2.0f, 1.0f, -3.9f, 2.0f},
		/*
			Case 3: Axis 0 only, positive overshoot clamp to zero.
		*/
		SplitBrakeCase{"axis0_pos_overshoot_axis1_zero_brake", 0.1f, 2.0f, 50.0f, 0.0f, 1.0f, 1.0f, 0.0f, 2.0f},
		/*
			Case 4: Axis 0 only, negative overshoot clamp to zero.
			Mirror of case 3 for reverse rotation.
		*/
		SplitBrakeCase{"axis0_neg_overshoot_axis1_zero_brake", -0.1f, 2.0f, 50.0f, 0.0f, 1.0f, 1.0f, 0.0f, 2.0f},
		/*
			Case 5: Axis 0 zero-spin guard, axis 1 untouched.
		*/
		SplitBrakeCase{"axis0_zero_spin_guard_axis1_zero_brake", 0.0f, 2.0f, 40.0f, 0.0f, 2.0f, 1.0f, 0.0f, 2.0f},
		/*
			Case 6: Axis 0 zero-spin baseline with zero brake torque.
			This is the axis-0 no-op mirror of the spool baseline case and
			ensures no accidental drift is introduced when both spin and brake
			input are zero.
		*/
		SplitBrakeCase{"axis0_zero_spin_zero_brake_axis1_zero_brake", 0.0f, 2.0f, 0.0f, 0.0f, 2.0f, 1.0f, 0.0f, 2.0f},
		/*
			Case 7: Axis 1 only, positive spin, no overshoot.
			Axis 0 has zero brake torque and should remain unchanged.
		*/
		SplitBrakeCase{"axis1_pos_no_overshoot_axis0_zero_brake", 2.0f, 4.0f, 0.0f, 20.0f, 1.0f, 2.0f, 2.0f, 3.9f},
		/*
			Case 8: Axis 1 only, positive overshoot clamp to zero.
			Mirror of axis-0 positive overshoot coverage.
		*/
		SplitBrakeCase{"axis1_pos_overshoot_axis0_zero_brake", 2.0f, 0.1f, 0.0f, 50.0f, 1.0f, 1.0f, 2.0f, 0.0f},
		/*
			Case 9: Axis 1 only, negative spin, no overshoot.
			Mirror of axis-0 negative no-overshoot coverage.
		*/
		SplitBrakeCase{"axis1_neg_no_overshoot_axis0_zero_brake", 2.0f, -4.0f, 0.0f, 20.0f, 1.0f, 2.0f, 2.0f, -3.9f},
		/*
			Case 10: Axis 1 only, negative overshoot clamp to zero.
			Covers reverse-rotation overshoot branch on side 1.
		*/
		SplitBrakeCase{"axis1_neg_overshoot_axis0_zero_brake", 2.0f, -0.1f, 0.0f, 50.0f, 1.0f, 1.0f, 2.0f, 0.0f},
		/*
			Case 11: Axis 1 zero-spin guard, axis 0 untouched.
			Mirror of axis-0 zero-guard coverage.
		*/
		SplitBrakeCase{"axis1_zero_spin_guard_axis0_zero_brake", 2.0f, 0.0f, 0.0f, 40.0f, 1.0f, 2.0f, 2.0f, 0.0f},
		/*
			Case 12: Axis 1 zero-spin baseline with zero brake torque.
			Mirror of axis-0 zero baseline coverage.
		*/
		SplitBrakeCase{"axis1_zero_spin_zero_brake_axis0_zero_brake", 2.0f, 0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 2.0f, 0.0f},
		/*
			Case 13: Mixed behavior in one update call.
			Axis 0 overshoots to zero while axis 1 decelerates normally.
			This verifies side computations remain independent under asymmetric
			brake torques and inertias.
		*/
		SplitBrakeCase{"mixed_axis_behaviour", 0.3f, 2.0f, 60.0f, 10.0f, 2.0f, 2.0f, 0.0f, 1.95f},
		/*
			Case 14: Exact boundary on axis 1 with axis 0 untouched.
			Axis 1 uses |ndot| == |spinVel| and should land exactly at zero.
			This is the split-path mirror of the spool boundary hardening case.
		*/
		SplitBrakeCase{"axis1_exact_boundary_equal_magnitude", 2.0f, 0.2f, 0.0f, 20.0f, 1.0f, 1.0f, 2.0f, 0.0f}
	),
	[](const ::testing::TestParamInfo<SplitBrakeCase>& info) {
		return info.param.name;
	}
);

} // namespace
