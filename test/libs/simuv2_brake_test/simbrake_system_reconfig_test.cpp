/***************************************************************************

    file                 : simbrake_system_reconfig_test.cpp
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
    Unit tests for SimBrakeSystemReConfig().

    SimBrakeSystemReConfig reads pit setup values for brakeRepartition
    and brakePressure from tCarElt, and conditionally updates
    car->brkSyst.rep and car->brkSyst.coeff via SimAdjustPitCarSetupParam.

    Uses CarReConfigFixture from susp_test_helpers.h (reused, not duplicated).
*/

#include <gtest/gtest.h>
#include <cmath>
#include "susp_test_helpers.h"
#include "brake_test_helpers.h"


TEST(SimBrakeSystemReConfigTest, BothAdjustable_BothUpdated)
{
	CarReConfigFixture fix;
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	// Set both as adjustable (min != max)
	fix.setSuspSetupValue(&setup.brakeRepartition, 0.6f, 0.3f, 0.8f);
	fix.setSuspSetupValue(&setup.brakePressure, 2000000.0f, 500000.0f, 3000000.0f);

	// Pre-existing values
	fix.car.brkSyst.rep = 0.5f;
	fix.car.brkSyst.coeff = 1000000.0f;

	SimBrakeSystemReConfig(&fix.car);

	EXPECT_FLOAT_EQ(fix.car.brkSyst.rep, 0.6f);
	EXPECT_FLOAT_EQ(fix.car.brkSyst.coeff, 2000000.0f);
}


TEST(SimBrakeSystemReConfigTest, NoneAdjustable_NoChange)
{
	CarReConfigFixture fix;
	// Both min == max == 0 -> not adjustable (default from memset)

	fix.car.brkSyst.rep = 0.5f;
	fix.car.brkSyst.coeff = 1000000.0f;

	SimBrakeSystemReConfig(&fix.car);

	// Values should not change
	EXPECT_FLOAT_EQ(fix.car.brkSyst.rep, 0.5f);
	EXPECT_FLOAT_EQ(fix.car.brkSyst.coeff, 1000000.0f);
}


TEST(SimBrakeSystemReConfigTest, OnlyRepartitionAdjustable)
{
	CarReConfigFixture fix;
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	fix.setSuspSetupValue(&setup.brakeRepartition, 0.7f, 0.3f, 0.8f);
	// brakePressure stays at default (min == max == 0)

	fix.car.brkSyst.rep = 0.5f;
	fix.car.brkSyst.coeff = 1000000.0f;

	SimBrakeSystemReConfig(&fix.car);

	EXPECT_FLOAT_EQ(fix.car.brkSyst.rep, 0.7f);
	EXPECT_FLOAT_EQ(fix.car.brkSyst.coeff, 1000000.0f);  // unchanged
}


TEST(SimBrakeSystemReConfigTest, OnlyPressureAdjustable)
{
	CarReConfigFixture fix;
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	// brakeRepartition stays at default (min == max == 0)
	fix.setSuspSetupValue(&setup.brakePressure, 1500000.0f, 500000.0f, 2000000.0f);

	fix.car.brkSyst.rep = 0.5f;
	fix.car.brkSyst.coeff = 1000000.0f;

	SimBrakeSystemReConfig(&fix.car);

	EXPECT_FLOAT_EQ(fix.car.brkSyst.rep, 0.5f);  // unchanged
	EXPECT_FLOAT_EQ(fix.car.brkSyst.coeff, 1500000.0f);
}


TEST(SimBrakeSystemReConfigTest, ValueClampedToMax)
{
	CarReConfigFixture fix;
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	// Value exceeds max: SimAdjustPitCarSetupParam should clamp it
	fix.setSuspSetupValue(&setup.brakeRepartition, 0.95f, 0.3f, 0.8f);

	fix.car.brkSyst.rep = 0.5f;

	SimBrakeSystemReConfig(&fix.car);

	EXPECT_FLOAT_EQ(fix.car.brkSyst.rep, 0.8f);  // clamped to max
}


TEST(SimBrakeSystemReConfigTest, ValueClampedToMin)
{
	CarReConfigFixture fix;
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	// Value below min: SimAdjustPitCarSetupParam should clamp it
	fix.setSuspSetupValue(&setup.brakePressure, 100000.0f, 500000.0f, 3000000.0f);

	fix.car.brkSyst.coeff = 1000000.0f;

	SimBrakeSystemReConfig(&fix.car);

	EXPECT_FLOAT_EQ(fix.car.brkSyst.coeff, 500000.0f);  // clamped to min
}
