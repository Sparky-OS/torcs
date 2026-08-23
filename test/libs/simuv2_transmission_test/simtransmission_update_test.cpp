/***************************************************************************

    file                 : simtransmission_update_test.cpp
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
    Unit tests for SimTransmissionUpdate().
*/

#include <gtest/gtest.h>

#include "transmission_test_helpers.h"

static const float kEps = 1e-5f;


TEST(SimTransmissionUpdateTest, RwdCallsDifferentialAndUpdatesFreeWheels)
{
	TransmissionFixture fix;
	TransmissionTestResetStubState();

	fix.car.transmission.type = TRANS_RWD;
	fix.car.transmission.clutch.transferValue = 0.4f;
	fix.car.transmission.curOverallRatio = 3.2f;
	fix.car.engine.Tq = 100.0f;

	SimTransmissionUpdate(&fix.car);

	EXPECT_NEAR(fix.car.transmission.differential[TRANS_REAR_DIFF].in.Tq, 320.0f, kEps);
	EXPECT_EQ(TransmissionTestGetDifferentialUpdateCallCount(), 1);
	EXPECT_EQ(TransmissionTestGetDifferentialUpdateFirst(0), 1);
	EXPECT_EQ(TransmissionTestGetDifferentialUpdatePtr(0), &fix.car.transmission.differential[TRANS_REAR_DIFF]);
	EXPECT_EQ(TransmissionTestGetUpdateFreeWheelsCallCount(), 1);
	EXPECT_EQ(TransmissionTestGetUpdateFreeWheelsLastAxle(), 0);
}


TEST(SimTransmissionUpdateTest, FwdCallsDifferentialAndUpdatesFreeWheels)
{
	TransmissionFixture fix;
	TransmissionTestResetStubState();

	fix.car.transmission.type = TRANS_FWD;
	fix.car.transmission.clutch.transferValue = 1.0f;
	fix.car.transmission.curOverallRatio = 2.0f;
	fix.car.engine.Tq = 30.0f;

	SimTransmissionUpdate(&fix.car);

	EXPECT_NEAR(fix.car.transmission.differential[TRANS_FRONT_DIFF].in.Tq, 60.0f, kEps);
	EXPECT_EQ(TransmissionTestGetDifferentialUpdateCallCount(), 1);
	EXPECT_EQ(TransmissionTestGetDifferentialUpdateFirst(0), 1);
	EXPECT_EQ(TransmissionTestGetDifferentialUpdatePtr(0), &fix.car.transmission.differential[TRANS_FRONT_DIFF]);
	EXPECT_EQ(TransmissionTestGetUpdateFreeWheelsCallCount(), 1);
	EXPECT_EQ(TransmissionTestGetUpdateFreeWheelsLastAxle(), 1);
}


TEST(SimTransmissionUpdateTest, FourWheelDriveAggregatesAndCallsThreeDifferentials)
{
	TransmissionFixture fix;
	TransmissionTestResetStubState();

	fix.car.transmission.type = TRANS_4WD;
	fix.car.transmission.clutch.transferValue = 0.2f;
	fix.car.transmission.curOverallRatio = 4.0f;
	fix.car.engine.Tq = 50.0f;

	fix.car.transmission.differential[TRANS_CENTRAL_DIFF].ratio = 2.0f;
	fix.car.transmission.differential[TRANS_FRONT_DIFF].inAxis[0]->spinVel = 10.0f;
	fix.car.transmission.differential[TRANS_FRONT_DIFF].inAxis[1]->spinVel = 14.0f;
	fix.car.transmission.differential[TRANS_REAR_DIFF].inAxis[0]->spinVel = 20.0f;
	fix.car.transmission.differential[TRANS_REAR_DIFF].inAxis[1]->spinVel = 30.0f;

	fix.car.transmission.differential[TRANS_FRONT_DIFF].inAxis[0]->Tq = 40.0f;
	fix.car.transmission.differential[TRANS_FRONT_DIFF].inAxis[1]->Tq = 20.0f;
	fix.car.transmission.differential[TRANS_REAR_DIFF].inAxis[0]->Tq = 70.0f;
	fix.car.transmission.differential[TRANS_REAR_DIFF].inAxis[1]->Tq = 10.0f;

	fix.car.transmission.differential[TRANS_FRONT_DIFF].inAxis[0]->brkTq = 8.0f;
	fix.car.transmission.differential[TRANS_FRONT_DIFF].inAxis[1]->brkTq = 12.0f;
	fix.car.transmission.differential[TRANS_REAR_DIFF].inAxis[0]->brkTq = 6.0f;
	fix.car.transmission.differential[TRANS_REAR_DIFF].inAxis[1]->brkTq = 2.0f;

	SimTransmissionUpdate(&fix.car);

	EXPECT_NEAR(fix.car.transmission.differential[TRANS_CENTRAL_DIFF].in.Tq, 120.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.differential[TRANS_CENTRAL_DIFF].inAxis[0]->spinVel, 12.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.differential[TRANS_CENTRAL_DIFF].inAxis[1]->spinVel, 25.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.differential[TRANS_CENTRAL_DIFF].inAxis[0]->Tq, 30.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.differential[TRANS_CENTRAL_DIFF].inAxis[1]->Tq, 40.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.differential[TRANS_CENTRAL_DIFF].inAxis[0]->brkTq, 10.0f, kEps);
	EXPECT_NEAR(fix.car.transmission.differential[TRANS_CENTRAL_DIFF].inAxis[1]->brkTq, 4.0f, kEps);

	EXPECT_EQ(TransmissionTestGetDifferentialUpdateCallCount(), 3);
	EXPECT_EQ(TransmissionTestGetDifferentialUpdatePtr(0), &fix.car.transmission.differential[TRANS_CENTRAL_DIFF]);
	EXPECT_EQ(TransmissionTestGetDifferentialUpdateFirst(0), 1);
	EXPECT_EQ(TransmissionTestGetDifferentialUpdatePtr(1), &fix.car.transmission.differential[TRANS_FRONT_DIFF]);
	EXPECT_EQ(TransmissionTestGetDifferentialUpdateFirst(1), 0);
	EXPECT_EQ(TransmissionTestGetDifferentialUpdatePtr(2), &fix.car.transmission.differential[TRANS_REAR_DIFF]);
	EXPECT_EQ(TransmissionTestGetDifferentialUpdateFirst(2), 0);
	EXPECT_EQ(TransmissionTestGetUpdateFreeWheelsCallCount(), 0);
}
