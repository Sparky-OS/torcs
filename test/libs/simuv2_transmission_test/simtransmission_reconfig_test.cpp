/***************************************************************************

    file                 : simtransmission_reconfig_test.cpp
    created              : Mon Mar  9 22:00:00 CET 2026
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
    Unit tests for SimTransmissionReConfig().
*/

#include <gtest/gtest.h>

#include "transmission_test_helpers.h"

static const float kEps = 1e-5f;


class SimTransmissionReConfigTest : public ::testing::Test {
protected:
	TransmissionFixture fix;

	void SetUp() override
	{
		TransmissionTestResetStubState();
		fix.attachParmHandle(createTransmissionTestParmHandle());
		ASSERT_NE(fix.hdle, nullptr) << "Failed to create parameter handle";
		fix.car.engine.I = 2.0f;
	}
};


TEST_F(SimTransmissionReConfigTest, RwdUpdatesForwardAndReverseRatios)
{
	fix.car.transmission.type = TRANS_RWD;
	fix.car.transmission.overallRatio[0] = -1.0f;
	fix.car.transmission.overallRatio[2] = 1.0f;
	fix.car.transmission.overallRatio[3] = 1.0f;

	fix.setPitDiffRatio(TRANS_REAR_DIFF, 4.0f, 1.0f, 5.0f);
	fix.setPitGearRatioByGear(1, 3.0f, 1.0f, 6.0f);
	fix.setPitGearRatioByGear(2, 2.0f, 1.0f, 6.0f);

	fix.setGearInertiaByIndex(0, 0.15f);
	fix.setGearInertiaByIndex(2, 0.10f);
	fix.setGearInertiaByIndex(3, 0.20f);
	fix.setGearRatioByIndex(0, -2.0f);

	SimTransmissionReConfig(&fix.car);

	EXPECT_EQ(TransmissionTestGetDifferentialReConfigCallCount(), 1);
	EXPECT_EQ(TransmissionTestGetDifferentialReConfigIndex(0), TRANS_REAR_DIFF);

	EXPECT_NEAR(fix.car.transmission.differential[TRANS_REAR_DIFF].ratio, 4.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.overallRatio[2], 12.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.overallRatio[3], 8.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.overallRatio[0], -8.0f, kEps);

	EXPECT_NEAR(fix.car.transmission.driveI[2], (2.0f + 0.10f) * (3.0f * 3.0f * 4.0f * 4.0f), kEps);
	EXPECT_NEAR(fix.car.transmission.freeI[2], 0.10f * (3.0f * 3.0f * 4.0f * 4.0f), kEps);
	EXPECT_NEAR(fix.car.carElt->priv.gearRatio[2], 12.0f, kEps);
	EXPECT_EQ(fix.car.transmission.gearbox.gear, 0);
}


TEST_F(SimTransmissionReConfigTest, ClampsPitRatioBeforeApplying)
{
	fix.car.transmission.type = TRANS_FWD;
	fix.car.transmission.overallRatio[2] = 1.0f;

	fix.setPitDiffRatio(TRANS_FRONT_DIFF, 3.0f, 1.0f, 5.0f);
	fix.setPitGearRatioByGear(1, 10.0f, 1.0f, 5.0f);
	fix.setGearInertiaByIndex(2, 0.1f);

	SimTransmissionReConfig(&fix.car);

	EXPECT_NEAR(fix.car.carElt->pitcmd.setup.gearsratio[0].value, 5.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.overallRatio[2], 15.0f, kEps);
}


struct ReConfigTypeCase {
	const char* name;
	int transType;
	int expectedCalls;
};

class SimTransmissionReConfigTypeTest : public ::testing::TestWithParam<ReConfigTypeCase> {};


TEST_P(SimTransmissionReConfigTypeTest, CallsExpectedDifferentialReconfigCount)
{
	const ReConfigTypeCase& c = GetParam();
	TransmissionFixture fix;
	TransmissionTestResetStubState();

	fix.attachParmHandle(createTransmissionTestParmHandle());
	ASSERT_NE(fix.hdle, nullptr);

	fix.car.transmission.type = c.transType;
	fix.car.transmission.overallRatio[2] = 1.0f;

	fix.setPitGearRatioByGear(1, 2.0f, 1.0f, 3.0f);
	fix.setPitDiffRatio(TRANS_FRONT_DIFF, 2.0f, 1.0f, 3.0f);
	fix.setPitDiffRatio(TRANS_REAR_DIFF, 3.0f, 1.0f, 4.0f);
	fix.setPitDiffRatio(TRANS_CENTRAL_DIFF, 4.0f, 1.0f, 5.0f);

	SimTransmissionReConfig(&fix.car);

	EXPECT_EQ(TransmissionTestGetDifferentialReConfigCallCount(), c.expectedCalls) << c.name;
}


INSTANTIATE_TEST_SUITE_P(DriveTypes, SimTransmissionReConfigTypeTest, ::testing::Values(
	ReConfigTypeCase{"Rwd", TRANS_RWD, 1},
	ReConfigTypeCase{"Fwd", TRANS_FWD, 1},
	ReConfigTypeCase{"4wd", TRANS_4WD, 3}
));
