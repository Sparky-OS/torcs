/***************************************************************************

    file                 : simtransmission_config_test.cpp
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
    Unit tests for SimTransmissionConfig().
*/

#include <gtest/gtest.h>

#include "transmission_test_helpers.h"

static const float kEps = 1e-5f;


class SimTransmissionConfigTest : public ::testing::Test {
protected:
	TransmissionFixture fix;

	void SetUp() override
	{
		TransmissionTestResetStubState();
		fix.attachParmHandle(createTransmissionTestParmHandle());
		ASSERT_NE(fix.hdle, nullptr) << "Failed to create parameter handle";
		fix.car.engine.I = 1.5f;
	}
};


TEST_F(SimTransmissionConfigTest, RwdReadsGearDataAndInitializesState)
{
	fix.setTransmissionType(VAL_TRANS_RWD);
	fix.setShiftTime(0.25f);
	fix.setDifferentialRatio(SECT_REARDIFFERENTIAL, 3.5f);

	fix.setGearRatioByIndex(0, -3.0f);
	fix.setGearRatioByIndex(2, 3.0f);
	fix.setGearRatioByIndex(3, 2.0f);
	fix.setGearRatioByIndex(4, 1.5f);

	fix.setGearEfficiencyByIndex(2, 1.2f);
	fix.setGearEfficiencyByIndex(3, -0.4f);
	fix.setGearEfficiencyByIndex(4, 0.8f);

	fix.setGearInertiaByIndex(2, 0.1f);
	fix.setGearInertiaByIndex(3, 0.2f);

	fix.setWheelFeedBackInertia(2, 1.3f);
	fix.setWheelFeedBackInertia(3, 1.7f);

	SimTransmissionConfig(&fix.car);

	EXPECT_EQ(TransmissionTestGetDifferentialConfigCallCount(), 1);
	EXPECT_STREQ(TransmissionTestGetDifferentialConfigSection(0), SECT_REARDIFFERENTIAL);

	EXPECT_EQ(fix.car.transmission.type, TRANS_RWD);
	EXPECT_EQ(fix.car.transmission.gearbox.gearMax, 3);
	EXPECT_EQ(fix.car.transmission.gearbox.gearMin, -1);
	EXPECT_EQ(fix.car.carElt->priv.gearOffset, 1);
	EXPECT_EQ(fix.car.carElt->priv.gearNb, 4);

	EXPECT_NEAR(fix.car.transmission.overallRatio[2], 10.5f, kEps);
	EXPECT_NEAR(fix.car.carElt->priv.gearRatio[2], 10.5f, kEps);
	EXPECT_NEAR(fix.car.transmission.gearEff[2], 1.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.gearEff[3], 0.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.gearEff[4], 0.8f, kEps);

	EXPECT_NEAR(fix.car.transmission.driveI[2], (1.5f + 0.1f) * (3.0f * 3.0f * 3.5f * 3.5f), kEps);
	EXPECT_NEAR(fix.car.transmission.freeI[2], 0.1f * (3.0f * 3.0f * 3.5f * 3.5f), kEps);

	EXPECT_EQ(fix.car.transmission.clutch.state, CLUTCH_RELEASING);
	EXPECT_NEAR(fix.car.transmission.clutch.releaseTime, 0.25f, kEps);
	EXPECT_NEAR(fix.car.transmission.clutch.timeToRelease, 0.0f, kEps);
	EXPECT_EQ(fix.car.transmission.gearbox.gear, 0);
	EXPECT_NEAR(fix.car.transmission.curI, fix.car.transmission.freeI[1], kEps);

	EXPECT_NEAR(fix.car.transmission.differential[TRANS_REAR_DIFF].outAxis[0]->I, 1.3f, kEps);
	EXPECT_NEAR(fix.car.transmission.differential[TRANS_REAR_DIFF].outAxis[1]->I, 1.7f, kEps);
	EXPECT_NEAR(fix.car.transmission.differential[TRANS_REAR_DIFF].outAxis[0]->Tq, 0.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.differential[TRANS_REAR_DIFF].outAxis[1]->Tq, 0.0f, kEps);
}


TEST_F(SimTransmissionConfigTest, NoReverseSetsGearMinAndOffsetToZero)
{
	fix.setTransmissionType(VAL_TRANS_FWD);
	fix.setDifferentialRatio(SECT_FRNTDIFFERENTIAL, 2.0f);
	fix.setGearRatioByIndex(2, 2.5f);

	SimTransmissionConfig(&fix.car);

	EXPECT_EQ(fix.car.transmission.gearbox.gearMin, 0);
	EXPECT_EQ(fix.car.carElt->priv.gearOffset, 0);
}


struct TransmissionConfigTypeCase {
	const char* name;
	const char* type;
	int expectedTransType;
	tdble expectedFinalRatio;
	int expectedDiffConfigCalls;
};

class SimTransmissionConfigTypeTest : public ::testing::TestWithParam<TransmissionConfigTypeCase> {};


TEST_P(SimTransmissionConfigTypeTest, UsesExpectedDifferentialRatio)
{
	const TransmissionConfigTypeCase& c = GetParam();
	TransmissionFixture fix;
	TransmissionTestResetStubState();

	fix.attachParmHandle(createTransmissionTestParmHandle());
	ASSERT_NE(fix.hdle, nullptr);

	fix.car.engine.I = 1.0f;
	fix.setTransmissionType(c.type);
	fix.setDifferentialRatio(SECT_FRNTDIFFERENTIAL, 2.0f);
	fix.setDifferentialRatio(SECT_REARDIFFERENTIAL, 3.0f);
	fix.setDifferentialRatio(SECT_CENTRALDIFFERENTIAL, 4.0f);
	fix.setGearRatioByIndex(2, 2.5f);

	SimTransmissionConfig(&fix.car);

	EXPECT_EQ(fix.car.transmission.type, c.expectedTransType) << c.name;
	EXPECT_EQ(TransmissionTestGetDifferentialConfigCallCount(), c.expectedDiffConfigCalls) << c.name;
	EXPECT_NEAR(fix.car.transmission.overallRatio[2], c.expectedFinalRatio * 2.5f, kEps) << c.name;
}


INSTANTIATE_TEST_SUITE_P(DriveTypes, SimTransmissionConfigTypeTest, ::testing::Values(
	TransmissionConfigTypeCase{"Rwd", VAL_TRANS_RWD, TRANS_RWD, 3.0f, 1},
	TransmissionConfigTypeCase{"Fwd", VAL_TRANS_FWD, TRANS_FWD, 2.0f, 1},
	TransmissionConfigTypeCase{"4wd", VAL_TRANS_4WD, TRANS_4WD, 4.0f, 3}
));
