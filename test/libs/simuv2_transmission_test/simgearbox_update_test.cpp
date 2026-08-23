/***************************************************************************

    file                 : simgearbox_update_test.cpp
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
    Unit tests for SimGearboxUpdate().
*/

#include <gtest/gtest.h>

#include "transmission_test_helpers.h"

static const float kEps = 1e-5f;


class SimGearboxUpdateTest : public ::testing::Test {
protected:
	TransmissionFixture fix;
	tdble prevDt;

	void SetUp() override
	{
		prevDt = SimDeltaTime;
		SimDeltaTime = 0.1f;

		fix.car.transmission.type = TRANS_RWD;
		fix.car.transmission.gearbox.gear = 1;
		fix.car.transmission.gearbox.gearMin = -1;
		fix.car.transmission.gearbox.gearMax = 5;
		fix.car.transmission.clutch.releaseTime = 0.4f;

		fix.car.transmission.freeI[2] = 2.0f;
		fix.car.transmission.driveI[2] = 10.0f;
		fix.car.transmission.freeI[3] = 4.0f;
		fix.car.transmission.gearEff[2] = 1.0f;
		fix.car.transmission.gearEff[3] = 0.5f;
		fix.car.transmission.overallRatio[3] = 7.0f;

		fix.setDiffFeedBackInertia(TRANS_REAR_DIFF, 6.0f);
		fix.car.transmission.differential[TRANS_REAR_DIFF].inAxis[0]->I = 1.0f;
		fix.car.transmission.differential[TRANS_REAR_DIFF].inAxis[1]->I = 3.0f;

		fix.car.ctrl->gear = 1;
		fix.car.ctrl->accelCmd = 0.8f;
	}

	void TearDown() override
	{
		SimDeltaTime = prevDt;
	}
};


TEST_F(SimGearboxUpdateTest, ReleasingAutoClutchForcesTransferAndCapsAccel)
{
	fix.car.transmission.clutch.state = CLUTCH_RELEASING;
	fix.car.transmission.clutch.timeToRelease = 0.5f;
	fix.car.transmission.clutch.transferValue = 1.0f;

	SimGearboxUpdate(&fix.car);

	EXPECT_EQ(fix.car.transmission.clutch.state, CLUTCH_RELEASING);
	EXPECT_NEAR(fix.car.transmission.clutch.timeToRelease, 0.4f, kEps);
	EXPECT_NEAR(fix.car.transmission.clutch.transferValue, 0.0f, kEps);
	EXPECT_NEAR(fix.car.ctrl->accelCmd, 0.1f, kEps);
	EXPECT_NEAR(fix.car.transmission.curI, fix.car.transmission.freeI[2], kEps);
}


TEST_F(SimGearboxUpdateTest, ReleasingCompletesWhenTimeElapsed)
{
	fix.car.transmission.clutch.state = CLUTCH_RELEASING;
	fix.car.transmission.clutch.timeToRelease = 0.05f;
	fix.car.transmission.clutch.transferValue = 0.5f;

	SimGearboxUpdate(&fix.car);

	EXPECT_EQ(fix.car.transmission.clutch.state, CLUTCH_RELEASED);
}


TEST_F(SimGearboxUpdateTest, UpshiftUpdatesRatioAndDifferentialInertia)
{
	fix.car.transmission.clutch.state = CLUTCH_RELEASED;
	fix.car.transmission.clutch.transferValue = 0.4f;
	fix.car.ctrl->gear = 2;

	SimGearboxUpdate(&fix.car);

	EXPECT_EQ(fix.car.transmission.gearbox.gear, 2);
	EXPECT_EQ(fix.car.transmission.clutch.state, CLUTCH_RELEASING);
	EXPECT_NEAR(fix.car.transmission.clutch.timeToRelease, 0.4f, kEps);
	EXPECT_NEAR(fix.car.transmission.curOverallRatio, 7.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.curI, 4.0f, kEps);

	EXPECT_NEAR(fix.car.transmission.differential[TRANS_REAR_DIFF].in.I, 16.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.differential[TRANS_REAR_DIFF].outAxis[0]->I, 4.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.differential[TRANS_REAR_DIFF].outAxis[1]->I, 8.0f, kEps);
}


TEST_F(SimGearboxUpdateTest, ShiftOutOfRangeIsIgnored)
{
	fix.car.transmission.clutch.state = CLUTCH_RELEASED;
	fix.car.ctrl->gear = 6;

	SimGearboxUpdate(&fix.car);

	EXPECT_EQ(fix.car.transmission.gearbox.gear, 1);
}


TEST_F(SimGearboxUpdateTest, FourWheelDriveSplitsInertiaAcrossFrontAndRear)
{
	fix.car.transmission.type = TRANS_4WD;
	fix.car.transmission.clutch.state = CLUTCH_RELEASED;
	fix.car.ctrl->gear = 2;
	fix.car.transmission.differential[TRANS_CENTRAL_DIFF].inAxis[0]->I = 2.0f;
	fix.car.transmission.differential[TRANS_CENTRAL_DIFF].inAxis[1]->I = 6.0f;
	fix.setDiffFeedBackInertia(TRANS_CENTRAL_DIFF, 8.0f);
	fix.car.transmission.differential[TRANS_FRONT_DIFF].inAxis[0]->I = 1.0f;
	fix.car.transmission.differential[TRANS_FRONT_DIFF].inAxis[1]->I = 1.5f;
	fix.car.transmission.differential[TRANS_REAR_DIFF].inAxis[0]->I = 2.5f;
	fix.car.transmission.differential[TRANS_REAR_DIFF].inAxis[1]->I = 3.0f;

	SimGearboxUpdate(&fix.car);

	EXPECT_NEAR(fix.car.transmission.differential[TRANS_CENTRAL_DIFF].outAxis[0]->I, 6.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.differential[TRANS_CENTRAL_DIFF].outAxis[1]->I, 14.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.differential[TRANS_FRONT_DIFF].outAxis[0]->I, 3.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.differential[TRANS_FRONT_DIFF].outAxis[1]->I, 4.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.differential[TRANS_REAR_DIFF].outAxis[0]->I, 6.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.differential[TRANS_REAR_DIFF].outAxis[1]->I, 7.0f, kEps);
}
