/***************************************************************************

    file                 : simdifferential_reconfig_test.cpp
    created              : Sun Mar  8 14:00:00 CET 2026
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
    Unit tests for SimDifferentialReConfig().
*/

#include <gtest/gtest.h>
#include "differential_test_helpers.h"
#include "transmission.h"


TEST(SimDifferentialReConfigTest, AllAdjustable_AllFieldsUpdated)
{
	DifferentialReConfigFixture fix;
	const int idx = TRANS_REAR_DIFF;
	tDifferential& d = fix.car.transmission.differential[idx];
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	fix.setDifferentialAxes(idx, 1.2f, 0.8f, 2.0f, 2.0f);

	d.I = 0.2f;
	d.efficiency = 0.8f;
	d.ratio = 2.0f;
	d.dTqMin = 0.10f;
	d.dTqMax = 0.30f;
	d.dSlipMax = 0.03f;
	d.lockInputTq = 1000.0f;
	d.lockBrakeInputTq = 300.0f;
	d.feedBack.I = -1.0f;

	fix.setSetupValue(&setup.diffratio[idx], 3.0f, 1.0f, 5.0f);
	fix.setSetupValue(&setup.diffmintqbias[idx], 0.20f, 0.0f, 1.0f);
	fix.setSetupValue(&setup.diffmaxtqbias[idx], 0.70f, 0.0f, 1.0f);
	fix.setSetupValue(&setup.diffslipbias[idx], 0.05f, 0.0f, 1.0f);
	fix.setSetupValue(&setup.difflockinginputtq[idx], 1500.0f, 500.0f, 3000.0f);
	fix.setSetupValue(&setup.difflockinginputbraketq[idx], 450.0f, 100.0f, 1000.0f);

	SimDifferentialReConfig(&fix.car, idx);

	EXPECT_FLOAT_EQ(d.ratio, 3.0f);
	EXPECT_FLOAT_EQ(d.dTqMin, 0.20f);
	EXPECT_FLOAT_EQ(d.dTqMax, 0.50f); // 0.70f - 0.20f
	EXPECT_FLOAT_EQ(d.dSlipMax, 0.05f);
	EXPECT_FLOAT_EQ(d.lockInputTq, 1500.0f);
	EXPECT_FLOAT_EQ(d.lockBrakeInputTq, 450.0f);

	tdble expectedFeedbackI = d.I * d.ratio * d.ratio + (1.2f + 0.8f) / d.efficiency;
	EXPECT_NEAR(d.feedBack.I, expectedFeedbackI, 1e-6f);
}


TEST(SimDifferentialReConfigTest, NoneAdjustable_NoFieldsChanged)
{
	DifferentialReConfigFixture fix;
	const int idx = TRANS_FRONT_DIFF;
	tDifferential& d = fix.car.transmission.differential[idx];

	fix.setDifferentialAxes(idx, 1.0f, 1.0f, 2.0f, 2.0f);

	d.I = 0.2f;
	d.efficiency = 1.0f;
	d.ratio = 2.5f;
	d.dTqMin = 0.15f;
	d.dTqMax = 0.25f;
	d.dSlipMax = 0.04f;
	d.lockInputTq = 1200.0f;
	d.lockBrakeInputTq = 300.0f;
	d.feedBack.I = 4.2f;

	SimDifferentialReConfig(&fix.car, idx);

	EXPECT_FLOAT_EQ(d.ratio, 2.5f);
	EXPECT_FLOAT_EQ(d.dTqMin, 0.15f);
	EXPECT_FLOAT_EQ(d.dTqMax, 0.25f);
	EXPECT_FLOAT_EQ(d.dSlipMax, 0.04f);
	EXPECT_FLOAT_EQ(d.lockInputTq, 1200.0f);
	EXPECT_FLOAT_EQ(d.lockBrakeInputTq, 300.0f);
	EXPECT_FLOAT_EQ(d.feedBack.I, 4.2f);
}


TEST(SimDifferentialReConfigTest, MaxBiasBelowMin_ClampsAndRewritesPitValue)
{
	DifferentialReConfigFixture fix;
	const int idx = TRANS_CENTRAL_DIFF;
	tDifferential& d = fix.car.transmission.differential[idx];
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	d.dTqMin = 0.10f;
	d.dTqMax = 0.20f;

	fix.setSetupValue(&setup.diffmintqbias[idx], 0.40f, 0.0f, 1.0f);
	fix.setSetupValue(&setup.diffmaxtqbias[idx], 0.20f, 0.0f, 1.0f);

	SimDifferentialReConfig(&fix.car, idx);

	EXPECT_FLOAT_EQ(d.dTqMin, 0.40f);
	EXPECT_FLOAT_EQ(d.dTqMax, 0.0f);
	EXPECT_FLOAT_EQ(setup.diffmaxtqbias[idx].value, 0.40f);
}


TEST(SimDifferentialReConfigTest, OutOfRangePitValue_ClampedBySetupAdjust)
{
	DifferentialReConfigFixture fix;
	const int idx = TRANS_REAR_DIFF;
	tDifferential& d = fix.car.transmission.differential[idx];
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;

	fix.setDifferentialAxes(idx, 1.0f, 1.0f, 2.0f, 2.0f);
	d.I = 0.2f;
	d.efficiency = 1.0f;
	d.ratio = 2.0f;

	fix.setSetupValue(&setup.diffratio[idx], 10.0f, 1.0f, 4.0f);

	SimDifferentialReConfig(&fix.car, idx);

	EXPECT_FLOAT_EQ(setup.diffratio[idx].value, 4.0f);
	EXPECT_FLOAT_EQ(d.ratio, 4.0f);
}


struct DifferentialReConfigIndexCase {
	const char* name;
	int index;
};

class SimDifferentialReConfigIndexTest : public ::testing::TestWithParam<DifferentialReConfigIndexCase> {};


TEST_P(SimDifferentialReConfigIndexTest, ReadsCorrectArrayElement)
{
	const DifferentialReConfigIndexCase& c = GetParam();
	DifferentialReConfigFixture fix;
	tCarPitSetup& setup = fix.carElt.pitcmd.setup;
	tDifferential& d = fix.car.transmission.differential[c.index];

	d.ratio = 1.0f;
	fix.setDifferentialAxes(c.index, 1.0f, 1.0f, 2.0f, 2.0f);
	d.I = 0.1f;
	d.efficiency = 1.0f;

	tdble uniqueRatio = 1.5f + c.index;
	fix.setSetupValue(&setup.diffratio[c.index], uniqueRatio, 1.0f, 5.0f);

	SimDifferentialReConfig(&fix.car, c.index);

	EXPECT_FLOAT_EQ(d.ratio, uniqueRatio) << "case: " << c.name;
}


INSTANTIATE_TEST_SUITE_P(DiffIndices, SimDifferentialReConfigIndexTest, ::testing::Values(
	DifferentialReConfigIndexCase{"Front", TRANS_FRONT_DIFF},
	DifferentialReConfigIndexCase{"Rear", TRANS_REAR_DIFF},
	DifferentialReConfigIndexCase{"Central", TRANS_CENTRAL_DIFF}
));
