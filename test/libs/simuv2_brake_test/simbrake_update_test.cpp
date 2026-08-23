/***************************************************************************

    file                 : simbrake_update_test.cpp
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
    Unit tests for SimBrakeUpdate().

    SimBrakeUpdate computes:
    1. Brake torque: Tq = coeff * pressure
    2. Temperature model:
       - Cooling: temp -= (|vel.x| * 0.02 + 0.1) * SimDeltaTime
       - Clamp low: temp = max(temp, 0)
       - Heating: temp += (pressure * radius * |spinVel| * 2.5e-8) * SimDeltaTime
       - Clamp high: temp = min(temp, 1.0)

    Tests are split into two groups: torque tests and temperature tests.
    Temperature tests also have a parametrized suite for systematic coverage.
*/

#include <gtest/gtest.h>
#include <cmath>
#include "brake_test_helpers.h"


static void runSimBrakeUpdate(tCar* car, tWheel* wheel, tBrake* brake, tdble dt = 0.002f)
{
	tdble prevDt = SimDeltaTime;
	SimDeltaTime = dt;
	SimBrakeUpdate(car, wheel, brake);
	SimDeltaTime = prevDt;
}


// ===========================================================================
// Torque tests
// ===========================================================================

TEST(SimBrakeUpdateTorqueTest, TorqueIsCoeffTimesPressure)
{
	tCar car = makeCarForBrakeUpdate(0.0f);
	tWheel wheel = makeWheelForBrakeUpdate(0.0f);
	tBrake brake = makeBrake(0.00024f, 0.13f, 0.15f, 5000000.0f);

	runSimBrakeUpdate(&car, &wheel, &brake);

	EXPECT_NEAR(brake.Tq, 0.00024f * 5000000.0f, 1e-1f);
}


TEST(SimBrakeUpdateTorqueTest, ZeroPressure_ZeroTorque)
{
	tCar car = makeCarForBrakeUpdate(30.0f);
	tWheel wheel = makeWheelForBrakeUpdate(50.0f);
	tBrake brake = makeBrake(0.00024f, 0.13f, 0.15f, 0.0f);

	runSimBrakeUpdate(&car, &wheel, &brake);

	EXPECT_FLOAT_EQ(brake.Tq, 0.0f);
}


TEST(SimBrakeUpdateTorqueTest, ZeroCoeff_ZeroTorque)
{
	tCar car = makeCarForBrakeUpdate(30.0f);
	tWheel wheel = makeWheelForBrakeUpdate(50.0f);
	tBrake brake = makeBrake(0.0f, 0.13f, 0.15f, 5000000.0f);

	runSimBrakeUpdate(&car, &wheel, &brake);

	EXPECT_FLOAT_EQ(brake.Tq, 0.0f);
}


TEST(SimBrakeUpdateTorqueTest, HighPressure_HighTorque)
{
	tCar car = makeCarForBrakeUpdate(0.0f);
	tWheel wheel = makeWheelForBrakeUpdate(0.0f);
	tdble coeff = 0.0005f;
	tdble pressure = 10000000.0f;
	tBrake brake = makeBrake(coeff, 0.13f, 0.15f, pressure);

	runSimBrakeUpdate(&car, &wheel, &brake);

	EXPECT_NEAR(brake.Tq, coeff * pressure, 1e-1f);
}


// ===========================================================================
// Temperature tests
// ===========================================================================

TEST(SimBrakeUpdateTempTest, ColdBrake_NoPressure_StaysCold)
{
	// temp=0, no pressure, stationary car
	tCar car = makeCarForBrakeUpdate(0.0f);
	tWheel wheel = makeWheelForBrakeUpdate(0.0f);
	tBrake brake = makeBrake(0.00024f, 0.13f, 0.15f, 0.0f, 0.0f);

	runSimBrakeUpdate(&car, &wheel, &brake);

	// Cooling: temp -= 0 * 0.0001 + 0.0002 = -0.0002, clamped to 0
	EXPECT_FLOAT_EQ(brake.temp, 0.0f);
}


TEST(SimBrakeUpdateTempTest, HotBrake_NoPressure_Cools)
{
	tCar car = makeCarForBrakeUpdate(50.0f);
	tWheel wheel = makeWheelForBrakeUpdate(100.0f);
	tBrake brake = makeBrake(0.00024f, 0.13f, 0.15f, 0.0f, 0.5f);

	runSimBrakeUpdate(&car, &wheel, &brake);

	// Cooling: temp -= (|50| * 0.00004 + 0.0002) = 0.0012
	// Heating: temp += 0 (no pressure)
	// Expected: 0.5 - 0.0012 = 0.4978
	EXPECT_NEAR(brake.temp, 0.4978f, 1e-5f);
}


TEST(SimBrakeUpdateTempTest, CoolingClampedToZero)
{
	// Very small temp, car moving fast -> cooling exceeds temp
	tCar car = makeCarForBrakeUpdate(100.0f);
	tWheel wheel = makeWheelForBrakeUpdate(0.0f);
	tBrake brake = makeBrake(0.00024f, 0.13f, 0.15f, 0.0f, 0.001f);

	runSimBrakeUpdate(&car, &wheel, &brake);

	// Cooling: 0.001 - (100 * 0.00002 + 0.0002) = 0.001 - 0.0022 = -0.0012 -> clamped to 0
	// Heating: 0 (no pressure)
	EXPECT_FLOAT_EQ(brake.temp, 0.0f);
}


TEST(SimBrakeUpdateTempTest, HeatingWithPressure)
{
	tCar car = makeCarForBrakeUpdate(0.0f);
	tWheel wheel = makeWheelForBrakeUpdate(200.0f);
	tdble pressure = 5000000.0f;
	tdble radius = 0.15f;
	tBrake brake = makeBrake(0.00024f, 0.13f, radius, pressure, 0.0f);

	runSimBrakeUpdate(&car, &wheel, &brake);

	// Cooling: 0 - (0 * 0.0001 + 0.0002) = -0.0002 -> clamped to 0
	// Heating: 0 + 5000000 * 0.15 * |200| * 5e-11 = 0.0075
	EXPECT_NEAR(brake.temp, 0.0075f, 1e-6f);
}


TEST(SimBrakeUpdateTempTest, TemperatureClampedToOne)
{
	tCar car = makeCarForBrakeUpdate(0.0f);
	tWheel wheel = makeWheelForBrakeUpdate(500.0f);
	tdble pressure = 10000000.0f;
	tdble radius = 0.20f;
	tBrake brake = makeBrake(0.00024f, 0.13f, radius, pressure, 0.99f);

	runSimBrakeUpdate(&car, &wheel, &brake);

	// Cooling: 0.99 - 0.0002 = 0.9898
	// Heating: 0.9898 + 10000000 * 0.20 * 500 * 5e-11 = 0.9898 + 0.5 = 1.4898
	// Clamped to 1.0
	EXPECT_FLOAT_EQ(brake.temp, 1.0f);
}


TEST(SimBrakeUpdateTempTest, StationaryCar_CoolsSlower)
{
	tCar car = makeCarForBrakeUpdate(0.0f);
	tWheel wheel = makeWheelForBrakeUpdate(0.0f);
	tBrake brake = makeBrake(0.00024f, 0.13f, 0.15f, 0.0f, 0.5f);

	runSimBrakeUpdate(&car, &wheel, &brake);

	// Cooling: 0.5 - (0 * 0.0001 + 0.0002) = 0.5 - 0.0002 = 0.4998
	EXPECT_NEAR(brake.temp, 0.4998f, 1e-6f);
}


TEST(SimBrakeUpdateTempTest, FastCar_CoolsFaster)
{
	tCar car = makeCarForBrakeUpdate(80.0f);
	tWheel wheel = makeWheelForBrakeUpdate(0.0f);
	tBrake brake = makeBrake(0.00024f, 0.13f, 0.15f, 0.0f, 0.5f);

	runSimBrakeUpdate(&car, &wheel, &brake);

	// Cooling: 0.5 - (80 * 0.00004 + 0.0002) = 0.5 - 0.0034 = 0.4966
	EXPECT_NEAR(brake.temp, 0.4966f, 1e-5f);
}


TEST(SimBrakeUpdateTempTest, ZeroSpinVel_NoHeating)
{
	tCar car = makeCarForBrakeUpdate(0.0f);
	tWheel wheel = makeWheelForBrakeUpdate(0.0f);
	tBrake brake = makeBrake(0.00024f, 0.13f, 0.15f, 5000000.0f, 0.5f);

	runSimBrakeUpdate(&car, &wheel, &brake);

	// Cooling: 0.5 - 0.0002 = 0.4998
	// Heating: 5000000 * 0.15 * 0 * 5e-11 = 0
	EXPECT_NEAR(brake.temp, 0.4998f, 1e-6f);
}


TEST(SimBrakeUpdateTempTest, NegativeSpinVel_AbsUsed)
{
	tCar car = makeCarForBrakeUpdate(0.0f);
	tWheel wheelPos = makeWheelForBrakeUpdate(200.0f);
	tWheel wheelNeg = makeWheelForBrakeUpdate(-200.0f);
	tBrake brakePos = makeBrake(0.00024f, 0.13f, 0.15f, 5000000.0f, 0.0f);
	tBrake brakeNeg = makeBrake(0.00024f, 0.13f, 0.15f, 5000000.0f, 0.0f);

	runSimBrakeUpdate(&car, &wheelPos, &brakePos);
	runSimBrakeUpdate(&car, &wheelNeg, &brakeNeg);

	EXPECT_FLOAT_EQ(brakePos.temp, brakeNeg.temp);
}


TEST(SimBrakeUpdateTempTest, NegativeVelX_AbsUsed)
{
	tCar carPos = makeCarForBrakeUpdate(50.0f);
	tCar carNeg = makeCarForBrakeUpdate(-50.0f);
	tWheel wheel = makeWheelForBrakeUpdate(0.0f);
	tBrake brakePos = makeBrake(0.00024f, 0.13f, 0.15f, 0.0f, 0.5f);
	tBrake brakeNeg = makeBrake(0.00024f, 0.13f, 0.15f, 0.0f, 0.5f);

	runSimBrakeUpdate(&carPos, &wheel, &brakePos);
	runSimBrakeUpdate(&carNeg, &wheel, &brakeNeg);

	EXPECT_FLOAT_EQ(brakePos.temp, brakeNeg.temp);
}


TEST(SimBrakeUpdateTempTest, CoolingThenHeating_NetEffect)
{
	// Demonstrate that cooling happens first, then heating.
	// Start at temp=0.0001, cooling brings it to 0 (clamped),
	// then heating adds a small amount.
	tCar car = makeCarForBrakeUpdate(10.0f);
	tWheel wheel = makeWheelForBrakeUpdate(200.0f);
	tdble pressure = 5000000.0f;
	tdble radius = 0.15f;
	tBrake brake = makeBrake(0.00024f, 0.13f, radius, pressure, 0.0001f);

	runSimBrakeUpdate(&car, &wheel, &brake);

	// Cooling: 0.0001 - (10 * 0.00002 + 0.0002) = 0.0001 - 0.0004 = -0.0003 -> clamped to 0
	// Heating: 0 + 5000000 * 0.15 * 200 * 5e-11 = 0.0075
	EXPECT_NEAR(brake.temp, 0.0075f, 1e-6f);
}


TEST(SimBrakeUpdateTempTest, TemperatureStepScalesWithSimDeltaTime)
{
	tCar car = makeCarForBrakeUpdate(80.0f);
	tWheel wheel = makeWheelForBrakeUpdate(120.0f);
	tBrake brakeDt1 = makeBrake(0.00024f, 0.13f, 0.15f, 3000000.0f, 0.5f);
	tBrake brakeDt2 = brakeDt1;

	runSimBrakeUpdate(&car, &wheel, &brakeDt1, 0.001f);
	runSimBrakeUpdate(&car, &wheel, &brakeDt2, 0.002f);

	const tdble delta1 = brakeDt1.temp - 0.5f;
	const tdble delta2 = brakeDt2.temp - 0.5f;
	EXPECT_NEAR(delta2, 2.0f * delta1, 1e-6f);
}


// ---------------------------------------------------------------------------
// Parametrized temperature tests
// ---------------------------------------------------------------------------

struct BrakeTempCase {
	const char* name;
	tdble initialTemp, pressure, radius, velX, spinVel;
	tdble expectedTemp;
};

class SimBrakeUpdateTempParamTest : public ::testing::TestWithParam<BrakeTempCase> {};


TEST_P(SimBrakeUpdateTempParamTest, TemperatureProducesExpectedResult)
{
	const BrakeTempCase& c = GetParam();
	tCar car = makeCarForBrakeUpdate(c.velX);
	tWheel wheel = makeWheelForBrakeUpdate(c.spinVel);
	tBrake brake = makeBrake(0.00024f, 0.13f, c.radius, c.pressure, c.initialTemp);

	runSimBrakeUpdate(&car, &wheel, &brake);

	EXPECT_NEAR(brake.temp, c.expectedTemp, 1e-5f) << "case: " << c.name;
}


/// Helper to compute expected temperature after one SimBrakeUpdate step.
/// This mirrors the logic in brake.cpp exactly.
static tdble computeExpectedTemp(tdble temp, tdble velX, tdble pressure, tdble radius, tdble spinVel, tdble deltaTime = 0.002f)
{
	const tdble timeScale = deltaTime / 0.002f;
	temp -= (fabs(velX) * 0.00004f + 0.0002f) * timeScale;
	if (temp < 0.0f) temp = 0.0f;
	temp += (pressure * radius * fabs(spinVel) * 0.00000000005f) * timeScale;
	if (temp > 1.0f) temp = 1.0f;
	return temp;
}


INSTANTIATE_TEST_SUITE_P(BrakeTempCases, SimBrakeUpdateTempParamTest, ::testing::Values(
	// Cold + no braking + stationary
	BrakeTempCase{"cold_stationary", 0.0f, 0.0f, 0.15f, 0.0f, 0.0f,
		computeExpectedTemp(0.0f, 0.0f, 0.0f, 0.15f, 0.0f)},
	// Cold + heavy braking
	BrakeTempCase{"cold_heavy_brake", 0.0f, 8000000.0f, 0.15f, 30.0f, 150.0f,
		computeExpectedTemp(0.0f, 30.0f, 8000000.0f, 0.15f, 150.0f)},
	// Hot + no braking (cooling)
	BrakeTempCase{"hot_no_brake", 0.8f, 0.0f, 0.15f, 60.0f, 100.0f,
		computeExpectedTemp(0.8f, 60.0f, 0.0f, 0.15f, 100.0f)},
	// Hot + heavy braking (stays hot, possibly clamped)
	BrakeTempCase{"hot_heavy_brake", 0.95f, 10000000.0f, 0.20f, 10.0f, 300.0f,
		computeExpectedTemp(0.95f, 10.0f, 10000000.0f, 0.20f, 300.0f)},
	// Moderate temp, moderate brake
	BrakeTempCase{"moderate", 0.4f, 3000000.0f, 0.15f, 40.0f, 100.0f,
		computeExpectedTemp(0.4f, 40.0f, 3000000.0f, 0.15f, 100.0f)},
	// Clamp at zero: high speed cooling on low temp
	BrakeTempCase{"clamp_zero", 0.002f, 0.0f, 0.15f, 80.0f, 0.0f,
		computeExpectedTemp(0.002f, 80.0f, 0.0f, 0.15f, 0.0f)},
	// Clamp at one: extreme heating
	BrakeTempCase{"clamp_one", 0.5f, 20000000.0f, 0.25f, 0.0f, 500.0f,
		computeExpectedTemp(0.5f, 0.0f, 20000000.0f, 0.25f, 500.0f)},
	// Negative velocity (abs used)
	BrakeTempCase{"negative_vel", 0.5f, 0.0f, 0.15f, -60.0f, -100.0f,
		computeExpectedTemp(0.5f, -60.0f, 0.0f, 0.15f, -100.0f)},
	// Very small radius, large pressure
	BrakeTempCase{"small_radius", 0.3f, 10000000.0f, 0.05f, 20.0f, 200.0f,
		computeExpectedTemp(0.3f, 20.0f, 10000000.0f, 0.05f, 200.0f)},
	// Large radius, small pressure
	BrakeTempCase{"large_radius", 0.3f, 500000.0f, 0.25f, 20.0f, 100.0f,
		computeExpectedTemp(0.3f, 20.0f, 500000.0f, 0.25f, 100.0f)}
));
