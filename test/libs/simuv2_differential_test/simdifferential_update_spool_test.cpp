/***************************************************************************

    file                 : simdifferential_update_spool_test.cpp
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
    Unit tests for SimDifferentialUpdate() spool behavior.
*/

#include <gtest/gtest.h>
#include "differential_test_helpers.h"


TEST(SimDifferentialUpdateSpoolTest, Spool_NoBrake_NoEngine_BasicIntegration)
{
	DifferentialUpdateFixture fix;
	DifferentialTestDisableEngineStub();
	SimDeltaTime = 0.01f;

	fix.differential.type = DIFF_SPOOL;
	fix.differential.in.Tq = 40.0f;

	fix.inAxis[0].spinVel = 5.0f;
	fix.inAxis[1].spinVel = 5.0f;
	fix.inAxis[0].Tq = 10.0f;
	fix.inAxis[1].Tq = 6.0f;
	fix.inAxis[0].brkTq = 0.0f;
	fix.inAxis[1].brkTq = 0.0f;
	fix.outAxis[0].I = 2.0f;
	fix.outAxis[1].I = 2.0f;

	SimDifferentialUpdate(&fix.car, &fix.differential, 0);

	EXPECT_NEAR(fix.outAxis[0].spinVel, 5.06f, 1e-6f);
	EXPECT_NEAR(fix.outAxis[1].spinVel, 5.06f, 1e-6f);
	EXPECT_NEAR(fix.outAxis[0].Tq, 12.0f, 2e-5f);
	EXPECT_NEAR(fix.outAxis[1].Tq, 12.0f, 2e-5f);
}


TEST(SimDifferentialUpdateSpoolTest, Spool_BrakeOvershoot_ClampedToZero)
{
	DifferentialUpdateFixture fix;
	DifferentialTestDisableEngineStub();
	SimDeltaTime = 0.01f;

	fix.differential.type = DIFF_SPOOL;
	fix.differential.in.Tq = 0.0f;

	fix.inAxis[0].spinVel = 0.5f;
	fix.inAxis[1].spinVel = 0.5f;
	fix.inAxis[0].Tq = 0.0f;
	fix.inAxis[1].Tq = 0.0f;
	fix.inAxis[0].brkTq = 200.0f;
	fix.inAxis[1].brkTq = 200.0f;
	fix.outAxis[0].I = 2.0f;
	fix.outAxis[1].I = 2.0f;

	SimDifferentialUpdate(&fix.car, &fix.differential, 0);

	EXPECT_FLOAT_EQ(fix.outAxis[0].spinVel, 0.0f);
	EXPECT_FLOAT_EQ(fix.outAxis[1].spinVel, 0.0f);
}


TEST(SimDifferentialUpdateSpoolTest, Spool_FirstCallsEngineStub_ReplacesSpin)
{
	DifferentialUpdateFixture fix;
	DifferentialTestEnableEngineStub(13.0f);
	SimDeltaTime = 0.01f;

	fix.differential.type = DIFF_SPOOL;
	fix.differential.in.Tq = 0.0f;

	fix.inAxis[0].spinVel = 7.0f;
	fix.inAxis[1].spinVel = 7.0f;
	fix.outAxis[0].I = 1.0f;
	fix.outAxis[1].I = 1.0f;

	SimDifferentialUpdate(&fix.car, &fix.differential, 1);

	EXPECT_EQ(DifferentialTestGetEngineStubCallCount(), 1);
	EXPECT_NEAR(DifferentialTestGetEngineStubLastAxleRpm(), 7.0f, 1e-6f);
	EXPECT_NEAR(fix.outAxis[0].spinVel, 13.0f, 1e-6f);
	EXPECT_NEAR(fix.outAxis[1].spinVel, 13.0f, 1e-6f);

	DifferentialTestDisableEngineStub();
}


TEST(SimDifferentialUpdateSpoolTest, LimitedSlip_AboveLockInput_UsesSpoolPath)
{
	DifferentialUpdateFixture fix;
	DifferentialTestDisableEngineStub();
	SimDeltaTime = 0.01f;

	fix.differential.type = DIFF_LIMITED_SLIP;
	fix.differential.lockInputTq = 100.0f;
	fix.differential.lockBrakeInputTq = 50.0f;
	fix.differential.in.Tq = 101.0f;

	fix.inAxis[0].spinVel = 4.0f;
	fix.inAxis[1].spinVel = 4.0f;
	fix.inAxis[0].Tq = 5.0f;
	fix.inAxis[1].Tq = 7.0f;
	fix.outAxis[0].I = 2.0f;
	fix.outAxis[1].I = 2.0f;

	SimDifferentialUpdate(&fix.car, &fix.differential, 0);

	// Spool expected: spin += dt * (101 - (5+7)) / (2+2) = 0.2225
	EXPECT_NEAR(fix.outAxis[0].spinVel, 4.2225f, 1e-6f);
	EXPECT_NEAR(fix.outAxis[1].spinVel, 4.2225f, 1e-6f);
}


TEST(SimDifferentialUpdateSpoolTest, LimitedSlip_BelowBrakeLock_UsesSpoolPath)
{
	DifferentialUpdateFixture fix;
	DifferentialTestDisableEngineStub();
	SimDeltaTime = 0.01f;

	fix.differential.type = DIFF_LIMITED_SLIP;
	fix.differential.lockInputTq = 100.0f;
	fix.differential.lockBrakeInputTq = 50.0f;
	fix.differential.in.Tq = -51.0f;

	fix.inAxis[0].spinVel = 4.0f;
	fix.inAxis[1].spinVel = 4.0f;
	fix.inAxis[0].Tq = 5.0f;
	fix.inAxis[1].Tq = 7.0f;
	fix.outAxis[0].I = 2.0f;
	fix.outAxis[1].I = 2.0f;

	SimDifferentialUpdate(&fix.car, &fix.differential, 0);

	// Spool expected: spin += dt * (-51 - (5+7)) / 4 = -0.1575
	EXPECT_NEAR(fix.outAxis[0].spinVel, 3.8425f, 1e-6f);
	EXPECT_NEAR(fix.outAxis[1].spinVel, 3.8425f, 1e-6f);
}
