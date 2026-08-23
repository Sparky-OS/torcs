/***************************************************************************

    file                 : simbrake_system_update_test.cpp
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
    Unit tests for SimBrakeSystemUpdate().

    SimBrakeSystemUpdate:
    1. Reads brakeCmd and brakeRepartitionCmd from car->ctrl
    2. Clamps brakeRepartitionCmd to [-repCmdMaxClicks, +repCmdMaxClicks]
    3. Computes effective repartition = rep + cmd * clickValue, clamped to [0, 1]
    4. Distributes pressure:
       front = ctrl * repartition
       rear  = ctrl * (1 - repartition)
    where ctrl = brakeCmd * brkSyst.coeff

    Tests are split into: repartition command clamping, effective repartition
    clamping, pressure distribution, and parametrized cases.
*/

#include <gtest/gtest.h>
#include <cmath>
#include "brake_test_helpers.h"


// ===========================================================================
// Repartition command clamping tests
// ===========================================================================

TEST(SimBrakeSystemUpdateCmdTest, BrakeRepartitionCmd_WithinRange_NotClamped)
{
	BrakeSystemUpdateFixture fix;
	fix.setBrakeSystem(0.5f, 1000000.0f, 0.0025f, 20);
	fix.setCtrl(1.0f, 5);

	SimBrakeSystemUpdate(&fix.car);

	// repartition = 0.5 + 5 * 0.0025 = 0.5125
	// front = 1000000 * 0.5125 = 512500
	// rear  = 1000000 * 0.4875 = 487500
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].brake.pressure, 512500.0f, 1e-1f);
	EXPECT_NEAR(fix.car.wheel[REAR_RGT].brake.pressure, 487500.0f, 1e-1f);
}


TEST(SimBrakeSystemUpdateCmdTest, BrakeRepartitionCmd_ExceedsMax_ClampedToMax)
{
	BrakeSystemUpdateFixture fix;
	fix.setBrakeSystem(0.5f, 1000000.0f, 0.0025f, 20);
	fix.setCtrl(1.0f, 30);  // exceeds max of 20

	SimBrakeSystemUpdate(&fix.car);

	// Clamped to 20: repartition = 0.5 + 20 * 0.0025 = 0.55
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].brake.pressure, 550000.0f, 1e-1f);
	EXPECT_NEAR(fix.car.wheel[REAR_RGT].brake.pressure, 450000.0f, 1e-1f);
}


TEST(SimBrakeSystemUpdateCmdTest, BrakeRepartitionCmd_BelowNegMax_ClampedToNegMax)
{
	BrakeSystemUpdateFixture fix;
	fix.setBrakeSystem(0.5f, 1000000.0f, 0.0025f, 20);
	fix.setCtrl(1.0f, -30);  // exceeds -max of -20

	SimBrakeSystemUpdate(&fix.car);

	// Clamped to -20: repartition = 0.5 + (-20) * 0.0025 = 0.45
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].brake.pressure, 450000.0f, 1e-1f);
	EXPECT_NEAR(fix.car.wheel[REAR_RGT].brake.pressure, 550000.0f, 1e-1f);
}


TEST(SimBrakeSystemUpdateCmdTest, BrakeRepartitionCmd_ExactlyAtMax_NotClamped)
{
	BrakeSystemUpdateFixture fix;
	fix.setBrakeSystem(0.5f, 1000000.0f, 0.0025f, 20);
	fix.setCtrl(1.0f, 20);

	SimBrakeSystemUpdate(&fix.car);

	// repartition = 0.5 + 20 * 0.0025 = 0.55
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].brake.pressure, 550000.0f, 1e-1f);
}


TEST(SimBrakeSystemUpdateCmdTest, BrakeRepartitionCmd_ExactlyAtNegMax_NotClamped)
{
	BrakeSystemUpdateFixture fix;
	fix.setBrakeSystem(0.5f, 1000000.0f, 0.0025f, 20);
	fix.setCtrl(1.0f, -20);

	SimBrakeSystemUpdate(&fix.car);

	// repartition = 0.5 + (-20) * 0.0025 = 0.45
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].brake.pressure, 450000.0f, 1e-1f);
}


TEST(SimBrakeSystemUpdateCmdTest, BrakeRepartitionCmd_Zero_NoAdjustment)
{
	BrakeSystemUpdateFixture fix;
	fix.setBrakeSystem(0.5f, 1000000.0f, 0.0025f, 20);
	fix.setCtrl(1.0f, 0);

	SimBrakeSystemUpdate(&fix.car);

	// repartition = 0.5 + 0 = 0.5
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].brake.pressure, 500000.0f, 1e-1f);
	EXPECT_NEAR(fix.car.wheel[REAR_RGT].brake.pressure, 500000.0f, 1e-1f);
}


// ===========================================================================
// Effective repartition clamping tests
// ===========================================================================

TEST(SimBrakeSystemUpdateRepTest, EffectiveRepartition_ClampedToOne)
{
	BrakeSystemUpdateFixture fix;
	// rep=0.9, 20 clicks * 0.01 = 0.2 -> 0.9 + 0.2 = 1.1, clamped to 1.0
	fix.setBrakeSystem(0.9f, 1000000.0f, 0.01f, 20);
	fix.setCtrl(1.0f, 20);

	SimBrakeSystemUpdate(&fix.car);

	// Repartition clamped to 1.0: all to front, none to rear
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].brake.pressure, 1000000.0f, 1e-1f);
	EXPECT_FLOAT_EQ(fix.car.wheel[REAR_RGT].brake.pressure, 0.0f);
}


TEST(SimBrakeSystemUpdateRepTest, EffectiveRepartition_ClampedToZero)
{
	BrakeSystemUpdateFixture fix;
	// rep=0.1, -20 clicks * 0.01 = -0.2 -> 0.1 - 0.2 = -0.1, clamped to 0.0
	fix.setBrakeSystem(0.1f, 1000000.0f, 0.01f, 20);
	fix.setCtrl(1.0f, -20);

	SimBrakeSystemUpdate(&fix.car);

	// Repartition clamped to 0.0: all to rear, none to front
	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_RGT].brake.pressure, 0.0f);
	EXPECT_NEAR(fix.car.wheel[REAR_RGT].brake.pressure, 1000000.0f, 1e-1f);
}


TEST(SimBrakeSystemUpdateRepTest, EffectiveRepartition_ExactlyOne)
{
	BrakeSystemUpdateFixture fix;
	// rep=0.5, 20 clicks * 0.025 = 0.5 -> 0.5 + 0.5 = 1.0 (exactly at boundary)
	fix.setBrakeSystem(0.5f, 1000000.0f, 0.025f, 20);
	fix.setCtrl(1.0f, 20);

	SimBrakeSystemUpdate(&fix.car);

	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].brake.pressure, 1000000.0f, 1e-1f);
	EXPECT_FLOAT_EQ(fix.car.wheel[REAR_RGT].brake.pressure, 0.0f);
}


TEST(SimBrakeSystemUpdateRepTest, EffectiveRepartition_ExactlyZero)
{
	BrakeSystemUpdateFixture fix;
	// rep=0.5, -20 clicks * 0.025 = -0.5 -> 0.5 - 0.5 = 0.0 (exactly at boundary)
	fix.setBrakeSystem(0.5f, 1000000.0f, 0.025f, 20);
	fix.setCtrl(1.0f, -20);

	SimBrakeSystemUpdate(&fix.car);

	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_RGT].brake.pressure, 0.0f);
	EXPECT_NEAR(fix.car.wheel[REAR_RGT].brake.pressure, 1000000.0f, 1e-1f);
}


// ===========================================================================
// Pressure distribution tests
// ===========================================================================

TEST(SimBrakeSystemUpdateDistTest, EqualRepartition_EqualPressure)
{
	BrakeSystemUpdateFixture fix;
	fix.setBrakeSystem(0.5f, 1000000.0f, 0.0025f, 20);
	fix.setCtrl(1.0f, 0);

	SimBrakeSystemUpdate(&fix.car);

	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].brake.pressure, 500000.0f, 1e-1f);
	EXPECT_NEAR(fix.car.wheel[FRNT_LFT].brake.pressure, 500000.0f, 1e-1f);
	EXPECT_NEAR(fix.car.wheel[REAR_RGT].brake.pressure, 500000.0f, 1e-1f);
	EXPECT_NEAR(fix.car.wheel[REAR_LFT].brake.pressure, 500000.0f, 1e-1f);
}


TEST(SimBrakeSystemUpdateDistTest, AllFront_NoPressureOnRear)
{
	BrakeSystemUpdateFixture fix;
	fix.setBrakeSystem(1.0f, 1000000.0f, 0.0025f, 20);
	fix.setCtrl(1.0f, 0);

	SimBrakeSystemUpdate(&fix.car);

	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].brake.pressure, 1000000.0f, 1e-1f);
	EXPECT_NEAR(fix.car.wheel[FRNT_LFT].brake.pressure, 1000000.0f, 1e-1f);
	EXPECT_FLOAT_EQ(fix.car.wheel[REAR_RGT].brake.pressure, 0.0f);
	EXPECT_FLOAT_EQ(fix.car.wheel[REAR_LFT].brake.pressure, 0.0f);
}


TEST(SimBrakeSystemUpdateDistTest, AllRear_NoPressureOnFront)
{
	BrakeSystemUpdateFixture fix;
	fix.setBrakeSystem(0.0f, 1000000.0f, 0.0025f, 20);
	fix.setCtrl(1.0f, 0);

	SimBrakeSystemUpdate(&fix.car);

	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_RGT].brake.pressure, 0.0f);
	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_LFT].brake.pressure, 0.0f);
	EXPECT_NEAR(fix.car.wheel[REAR_RGT].brake.pressure, 1000000.0f, 1e-1f);
	EXPECT_NEAR(fix.car.wheel[REAR_LFT].brake.pressure, 1000000.0f, 1e-1f);
}


TEST(SimBrakeSystemUpdateDistTest, ZeroBrakeCmd_ZeroPressure)
{
	BrakeSystemUpdateFixture fix;
	fix.setBrakeSystem(0.5f, 1000000.0f, 0.0025f, 20);
	fix.setCtrl(0.0f, 0);

	SimBrakeSystemUpdate(&fix.car);

	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_RGT].brake.pressure, 0.0f);
	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_LFT].brake.pressure, 0.0f);
	EXPECT_FLOAT_EQ(fix.car.wheel[REAR_RGT].brake.pressure, 0.0f);
	EXPECT_FLOAT_EQ(fix.car.wheel[REAR_LFT].brake.pressure, 0.0f);
}


TEST(SimBrakeSystemUpdateDistTest, FullBrakeCmd_MaxPressure)
{
	BrakeSystemUpdateFixture fix;
	fix.setBrakeSystem(0.6f, 2000000.0f, 0.0025f, 20);
	fix.setCtrl(1.0f, 0);

	SimBrakeSystemUpdate(&fix.car);

	// ctrl = 1.0 * 2000000 = 2000000
	// front = 2000000 * 0.6 = 1200000, rear = 2000000 * 0.4 = 800000
	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].brake.pressure, 1200000.0f, 1e-1f);
	EXPECT_NEAR(fix.car.wheel[REAR_RGT].brake.pressure, 800000.0f, 1e-1f);
}


TEST(SimBrakeSystemUpdateDistTest, LeftRightSymmetry)
{
	BrakeSystemUpdateFixture fix;
	fix.setBrakeSystem(0.65f, 1500000.0f, 0.0025f, 20);
	fix.setCtrl(0.8f, 5);

	SimBrakeSystemUpdate(&fix.car);

	// Left and right on same axle must be equal
	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_RGT].brake.pressure,
	                fix.car.wheel[FRNT_LFT].brake.pressure);
	EXPECT_FLOAT_EQ(fix.car.wheel[REAR_RGT].brake.pressure,
	                fix.car.wheel[REAR_LFT].brake.pressure);
}


TEST(SimBrakeSystemUpdateDistTest, FrontRearSumEqualsTotal)
{
	BrakeSystemUpdateFixture fix;
	fix.setBrakeSystem(0.55f, 1800000.0f, 0.0025f, 20);
	fix.setCtrl(0.75f, 3);

	SimBrakeSystemUpdate(&fix.car);

	tdble totalCtrl = 0.75f * 1800000.0f;
	tdble frontP = fix.car.wheel[FRNT_RGT].brake.pressure;
	tdble rearP = fix.car.wheel[REAR_RGT].brake.pressure;

	// front + rear should equal total control pressure
	EXPECT_NEAR(frontP + rearP, totalCtrl, 1e-1f);
}


// ===========================================================================
// Parametrized tests
// ===========================================================================

struct BrakeSystemUpdateCase {
	const char* name;
	tdble brakeCmd;
	int brakeRepartitionCmd;
	tdble rep, coeff, clickValue;
	int maxClicks;
	tdble expectedFrontPressure, expectedRearPressure;
};

class SimBrakeSystemUpdateParamTest : public ::testing::TestWithParam<BrakeSystemUpdateCase> {};


TEST_P(SimBrakeSystemUpdateParamTest, UpdateProducesExpectedPressures)
{
	const BrakeSystemUpdateCase& c = GetParam();
	BrakeSystemUpdateFixture fix;
	fix.setBrakeSystem(c.rep, c.coeff, c.clickValue, c.maxClicks);
	fix.setCtrl(c.brakeCmd, c.brakeRepartitionCmd);

	SimBrakeSystemUpdate(&fix.car);

	EXPECT_NEAR(fix.car.wheel[FRNT_RGT].brake.pressure, c.expectedFrontPressure, 1e-1f)
		<< "case: " << c.name << " (front)";
	EXPECT_NEAR(fix.car.wheel[REAR_RGT].brake.pressure, c.expectedRearPressure, 1e-1f)
		<< "case: " << c.name << " (rear)";
	// Also verify symmetry
	EXPECT_FLOAT_EQ(fix.car.wheel[FRNT_RGT].brake.pressure,
	                fix.car.wheel[FRNT_LFT].brake.pressure)
		<< "case: " << c.name << " (front L/R symmetry)";
	EXPECT_FLOAT_EQ(fix.car.wheel[REAR_RGT].brake.pressure,
	                fix.car.wheel[REAR_LFT].brake.pressure)
		<< "case: " << c.name << " (rear L/R symmetry)";
}


/// Helper to compute expected pressures.
static void computeExpectedPressures(
	tdble brakeCmd, int brakeRepartitionCmd,
	tdble rep, tdble coeff, tdble clickValue, int maxClicks,
	tdble& outFront, tdble& outRear
)
{
	// Clamp command
	if (brakeRepartitionCmd > maxClicks) {
		brakeRepartitionCmd = maxClicks;
	} else if (brakeRepartitionCmd < -maxClicks) {
		brakeRepartitionCmd = -maxClicks;
	}

	// Compute effective repartition
	tdble repartition = rep + brakeRepartitionCmd * clickValue;
	if (repartition > 1.0f) repartition = 1.0f;
	else if (repartition < 0.0f) repartition = 0.0f;

	tdble ctrl = brakeCmd * coeff;
	outFront = ctrl * repartition;
	outRear = ctrl * (1.0f - repartition);
}


// Macro to define a parametrized case with auto-computed expected values
#define BRAKE_CASE(name_, cmd_, repCmd_, rep_, coeff_, click_, maxC_) \
	[]() -> BrakeSystemUpdateCase { \
		tdble f, r; \
		computeExpectedPressures(cmd_, repCmd_, rep_, coeff_, click_, maxC_, f, r); \
		return BrakeSystemUpdateCase{name_, cmd_, repCmd_, rep_, coeff_, click_, maxC_, f, r}; \
	}()


INSTANTIATE_TEST_SUITE_P(BrakeSystemUpdateCases, SimBrakeSystemUpdateParamTest, ::testing::Values(
	// Zero brake
	BRAKE_CASE("zero_brake", 0.0f, 0, 0.5f, 1000000.0f, 0.0025f, 20),
	// Full brake, neutral repartition
	BRAKE_CASE("full_neutral", 1.0f, 0, 0.5f, 1000000.0f, 0.0025f, 20),
	// Full brake, 30% front
	BRAKE_CASE("full_30pct", 1.0f, 0, 0.3f, 1000000.0f, 0.0025f, 20),
	// Full brake, 70% front
	BRAKE_CASE("full_70pct", 1.0f, 0, 0.7f, 1000000.0f, 0.0025f, 20),
	// Half brake, positive cmd
	BRAKE_CASE("half_pos_cmd", 0.5f, 10, 0.5f, 1000000.0f, 0.0025f, 20),
	// Half brake, negative cmd
	BRAKE_CASE("half_neg_cmd", 0.5f, -10, 0.5f, 1000000.0f, 0.0025f, 20),
	// Cmd exceeds max (clamped)
	BRAKE_CASE("cmd_over_max", 1.0f, 50, 0.5f, 1000000.0f, 0.0025f, 20),
	// Cmd below -max (clamped)
	BRAKE_CASE("cmd_under_neg_max", 1.0f, -50, 0.5f, 1000000.0f, 0.0025f, 20),
	// Repartition clamped to 1.0
	BRAKE_CASE("rep_clamp_one", 1.0f, 20, 0.9f, 1000000.0f, 0.01f, 20),
	// Repartition clamped to 0.0
	BRAKE_CASE("rep_clamp_zero", 1.0f, -20, 0.1f, 1000000.0f, 0.01f, 20),
	// Large coeff
	BRAKE_CASE("large_coeff", 0.8f, 0, 0.55f, 5000000.0f, 0.0025f, 20),
	// Small coeff
	BRAKE_CASE("small_coeff", 1.0f, 0, 0.5f, 100000.0f, 0.0025f, 20),
	// Large click value, small max clicks
	BRAKE_CASE("large_click", 1.0f, 5, 0.5f, 1000000.0f, 0.02f, 5),
	// Zero max clicks (cmd always clamped to 0)
	BRAKE_CASE("zero_max_clicks", 1.0f, 10, 0.5f, 1000000.0f, 0.0025f, 0)
));
