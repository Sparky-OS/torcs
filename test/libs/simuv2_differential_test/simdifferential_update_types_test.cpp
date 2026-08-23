/***************************************************************************

    file                 : simdifferential_update_types_test.cpp
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
    Unit tests for SimDifferentialUpdate() non-spool types.
*/

#include <gtest/gtest.h>
#include "differential_test_helpers.h"


TEST(SimDifferentialUpdateTypesTest, CommonSpinZero_SplitsDriveTorqueInHalf)
{
	DifferentialUpdateFixture fix;
	DifferentialTestDisableEngineStub();
	SimDeltaTime = 0.01f;

	fix.differential.type = DIFF_FREE;
	fix.differential.in.Tq = 40.0f;
	fix.inAxis[0].spinVel = 0.0f;
	fix.inAxis[1].spinVel = 0.0f;
	fix.inAxis[0].Tq = 10.0f; // resists more
	fix.inAxis[1].Tq = 6.0f; //resists less
	fix.outAxis[0].I = 2.0f;
	fix.outAxis[1].I = 2.0f;

	SimDifferentialUpdate(&fix.car, &fix.differential, 0);

	EXPECT_NEAR(fix.outAxis[0].spinVel, 0.05f, 1e-6f); // higher resistance accelerates slower
	EXPECT_NEAR(fix.outAxis[1].spinVel, 0.07f, 1e-6f); // lower resistance accelerates faster
}


TEST(SimDifferentialUpdateTypesTest, FreeDifferential_SpiderTorqueRedistribution)
{
	DifferentialUpdateFixture fix;
	DifferentialTestDisableEngineStub();
	SimDeltaTime = 0.01f;

	fix.differential.type = DIFF_FREE;
	fix.differential.in.Tq = 100.0f;
	fix.inAxis[0].spinVel = 10.0f;
	fix.inAxis[1].spinVel = 10.0f;
	fix.inAxis[0].Tq = 10.0f; // resists less
	fix.inAxis[1].Tq = 30.0f; // resists more
	fix.outAxis[0].I = 2.0f;
	fix.outAxis[1].I = 2.0f;

	SimDifferentialUpdate(&fix.car, &fix.differential, 0);

	EXPECT_NEAR(fix.outAxis[0].spinVel, 10.25f, 1e-6f); // lower resistance accelerates faster
	EXPECT_NEAR(fix.outAxis[1].spinVel, 10.05f, 1e-6f); // higher resistance accelerates slower
}


TEST(SimDifferentialUpdateTypesTest, LimitedSlip_NoBiasWhenSlipBelowLimit)
{
	DifferentialUpdateFixture fix;
	DifferentialTestDisableEngineStub();
	SimDeltaTime = 0.01f;

	fix.differential.type = DIFF_LIMITED_SLIP;
	fix.differential.in.Tq = 80.0f;
	fix.differential.lockInputTq = 200.0f;
	fix.differential.lockBrakeInputTq = 100.0f;
	fix.differential.dSlipMax = 0.1f;

	fix.inAxis[0].spinVel = 10.0f;
	fix.inAxis[1].spinVel = 10.0f;
	fix.inAxis[0].Tq = 5.0f;
	fix.inAxis[1].Tq = 9.0f;
	fix.outAxis[0].I = 2.0f;
	fix.outAxis[1].I = 2.0f;

	SimDifferentialUpdate(&fix.car, &fix.differential, 0);

	EXPECT_NEAR(fix.outAxis[0].spinVel, 10.185f, 1e-6f);
	EXPECT_NEAR(fix.outAxis[1].spinVel, 10.145f, 1e-6f);
}


TEST(SimDifferentialUpdateTypesTest, LimitedSlip_BiasApplied_PositiveDrive)
{
	DifferentialUpdateFixture fix;
	DifferentialTestDisableEngineStub();
	SimDeltaTime = 0.01f;

	fix.differential.type = DIFF_LIMITED_SLIP;
	fix.differential.in.Tq = 100.0f;
	fix.differential.lockInputTq = 200.0f;
	fix.differential.lockBrakeInputTq = 100.0f;
	fix.differential.dSlipMax = 0.1f;

	fix.inAxis[0].spinVel = 12.0f;
	fix.inAxis[1].spinVel = 8.0f;
	fix.inAxis[0].Tq = 0.0f;
	fix.inAxis[1].Tq = 0.0f;
	fix.outAxis[0].I = 2.0f;
	fix.outAxis[1].I = 2.0f;

	SimDifferentialUpdate(&fix.car, &fix.differential, 0);

	EXPECT_NEAR(fix.outAxis[0].spinVel, 10.7125f, 1e-5f);
	EXPECT_NEAR(fix.outAxis[1].spinVel, 9.7875f, 1e-5f);
}


TEST(SimDifferentialUpdateTypesTest, LimitedSlip_BiasApplied_NegativeDriveUsesBrakeLock)
{
	DifferentialUpdateFixture fix;
	DifferentialTestDisableEngineStub();
	SimDeltaTime = 0.01f;

	fix.differential.type = DIFF_LIMITED_SLIP;
	fix.differential.in.Tq = -50.0f;
	fix.differential.lockInputTq = 200.0f;
	fix.differential.lockBrakeInputTq = 100.0f;
	fix.differential.dSlipMax = 0.1f;

	fix.inAxis[0].spinVel = 8.0f;
	fix.inAxis[1].spinVel = 12.0f;
	fix.inAxis[0].Tq = 0.0f;
	fix.inAxis[1].Tq = 0.0f;
	fix.outAxis[0].I = 2.0f;
	fix.outAxis[1].I = 2.0f;

	SimDifferentialUpdate(&fix.car, &fix.differential, 0);

	EXPECT_NEAR(fix.outAxis[0].spinVel, 9.39375f, 1e-5f);
	EXPECT_NEAR(fix.outAxis[1].spinVel, 10.35625f, 1e-5f);
}


TEST(SimDifferentialUpdateTypesTest, ViscousCoupler_Spin0GeSpin1_UsesMinSplit)
{
	DifferentialUpdateFixture fix;
	DifferentialTestDisableEngineStub();
	SimDeltaTime = 0.01f;

	fix.differential.type = DIFF_VISCOUS_COUPLER;
	fix.differential.in.Tq = 100.0f;
	fix.differential.dTqMin = 0.2f;

	fix.inAxis[0].spinVel = 10.0f;
	fix.inAxis[1].spinVel = 8.0f;
	fix.outAxis[0].I = 2.0f;
	fix.outAxis[1].I = 2.0f;

	SimDifferentialUpdate(&fix.car, &fix.differential, 0);

	EXPECT_NEAR(fix.outAxis[0].spinVel, 10.1f, 1e-6f);
	EXPECT_NEAR(fix.outAxis[1].spinVel, 8.4f, 1e-6f);
}


TEST(SimDifferentialUpdateTypesTest, ViscousCoupler_Spin0LtSpin1_UsesExponentialSplit)
{
	DifferentialUpdateFixture fix;
	DifferentialTestDisableEngineStub();
	SimDeltaTime = 0.01f;

	fix.differential.type = DIFF_VISCOUS_COUPLER;
	fix.differential.in.Tq = 100.0f;
	fix.differential.dTqMin = 0.2f;
	fix.differential.dTqMax = 0.5f;
	fix.differential.viscosity = 2.0f;

	fix.inAxis[0].spinVel = 8.0f;
	fix.inAxis[1].spinVel = 10.0f;
	fix.outAxis[0].I = 2.0f;
	fix.outAxis[1].I = 2.0f;

	SimDifferentialUpdate(&fix.car, &fix.differential, 0);

	EXPECT_NEAR(fix.outAxis[0].spinVel, 8.345421f, 1e-5f);
	EXPECT_NEAR(fix.outAxis[1].spinVel, 10.154579f, 1e-5f);
}


TEST(SimDifferentialUpdateTypesTest, NoneType_NoDriveTorqueApplied)
{
	DifferentialUpdateFixture fix;
	DifferentialTestDisableEngineStub();
	SimDeltaTime = 0.01f;

	fix.differential.type = DIFF_NONE;
	fix.differential.in.Tq = 100.0f;

	fix.inAxis[0].spinVel = 5.0f;
	fix.inAxis[1].spinVel = 6.0f;
	fix.inAxis[0].Tq = 2.0f;
	fix.inAxis[1].Tq = 4.0f;
	fix.outAxis[0].I = 2.0f;
	fix.outAxis[1].I = 2.0f;

	SimDifferentialUpdate(&fix.car, &fix.differential, 0);

	EXPECT_NEAR(fix.outAxis[0].spinVel, 4.99f, 1e-6f);
	EXPECT_NEAR(fix.outAxis[1].spinVel, 5.98f, 1e-6f);
}


TEST(SimDifferentialUpdateTypesTest, FirstEngineScaling_ScalesBothSides)
{
	DifferentialUpdateFixture fix;
	DifferentialTestEnableEngineStub(12.0f);
	SimDeltaTime = 0.01f;

	fix.differential.type = DIFF_FREE;
	fix.differential.in.Tq = 0.0f;

	fix.inAxis[0].spinVel = 10.0f;
	fix.inAxis[1].spinVel = 10.0f;
	fix.inAxis[0].Tq = 0.0f;
	fix.inAxis[1].Tq = 0.0f;
	fix.outAxis[0].I = 1.0f;
	fix.outAxis[1].I = 1.0f;

	SimDifferentialUpdate(&fix.car, &fix.differential, 1);

	EXPECT_EQ(DifferentialTestGetEngineStubCallCount(), 1);
	EXPECT_NEAR(DifferentialTestGetEngineStubLastAxleRpm(), 10.0f, 1e-6f);
	EXPECT_NEAR(fix.outAxis[0].spinVel, 12.0f, 1e-6f);
	EXPECT_NEAR(fix.outAxis[1].spinVel, 12.0f, 1e-6f);

	DifferentialTestDisableEngineStub();
}
