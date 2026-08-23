/***************************************************************************

    file                 : simdifferential_config_test.cpp
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
    Unit tests for SimDifferentialConfig().
*/

#include <gtest/gtest.h>
#include "differential_test_helpers.h"


TEST(SimDifferentialConfigTest, DefaultValues_EmptySection)
{
	DifferentialConfigFixture fix;
	fix.inAxis[0].I = 1.2f;
	fix.inAxis[1].I = 0.8f;

	SimDifferentialConfig(fix.hdle, SECT_REARDIFFERENTIAL, &fix.differential);

	EXPECT_FLOAT_EQ(fix.differential.I, 0.1f);
	EXPECT_FLOAT_EQ(fix.differential.efficiency, 1.0f);
	EXPECT_FLOAT_EQ(fix.differential.ratio, 1.0f);
	EXPECT_FLOAT_EQ(fix.differential.dTqMin, 0.05f);
	EXPECT_FLOAT_EQ(fix.differential.dTqMax, 0.75f);
	EXPECT_FLOAT_EQ(fix.differential.dSlipMax, 0.03f);
	EXPECT_FLOAT_EQ(fix.differential.lockInputTq, 3000.0f);
	EXPECT_NEAR(fix.differential.lockBrakeInputTq, 990.0f, 1e-4f);
	EXPECT_FLOAT_EQ(fix.differential.viscosity, 1.0f);
	EXPECT_EQ(fix.differential.type, DIFF_NONE);

	tdble expectedFeedbackI = 0.1f * 1.0f * 1.0f + (1.2f + 0.8f) / 1.0f;
	EXPECT_NEAR(fix.differential.feedBack.I, expectedFeedbackI, 1e-6f);
}


TEST(SimDifferentialConfigTest, TypeMapping_AllKnownStrings)
{
	struct TypeCase {
		const char* typeStr;
		int expectedType;
	};

	TypeCase cases[] = {
		{VAL_DIFF_LIMITED_SLIP, DIFF_LIMITED_SLIP},
		{VAL_DIFF_VISCOUS_COUPLER, DIFF_VISCOUS_COUPLER},
		{VAL_DIFF_SPOOL, DIFF_SPOOL},
		{VAL_DIFF_FREE, DIFF_FREE},
		{VAL_DIFF_NONE, DIFF_NONE},
	};

	for (const auto& c : cases) {
		DifferentialConfigFixture fix;
		GfParmSetStr(fix.hdle, SECT_REARDIFFERENTIAL, PRM_TYPE, c.typeStr);

		SimDifferentialConfig(fix.hdle, SECT_REARDIFFERENTIAL, &fix.differential);

		EXPECT_EQ(fix.differential.type, c.expectedType) << "type: " << c.typeStr;
	}
}


TEST(SimDifferentialConfigTest, UnknownType_FallsBackToNone)
{
	DifferentialConfigFixture fix;
	GfParmSetStr(fix.hdle, SECT_REARDIFFERENTIAL, PRM_TYPE, "UNKNOWN_TYPE");

	SimDifferentialConfig(fix.hdle, SECT_REARDIFFERENTIAL, &fix.differential);

	EXPECT_EQ(fix.differential.type, DIFF_NONE);
}


TEST(SimDifferentialConfigTest, MaxTorqueBiasBelowMin_ClampedToZero)
{
	DifferentialConfigFixture fix;
	GfParmSetNum(fix.hdle, SECT_REARDIFFERENTIAL, PRM_MIN_TQ_BIAS, (char*)NULL, 0.70f);
	GfParmSetNum(fix.hdle, SECT_REARDIFFERENTIAL, PRM_MAX_TQ_BIAS, (char*)NULL, 0.50f);

	SimDifferentialConfig(fix.hdle, SECT_REARDIFFERENTIAL, &fix.differential);

	EXPECT_FLOAT_EQ(fix.differential.dTqMin, 0.70f);
	EXPECT_FLOAT_EQ(fix.differential.dTqMax, 0.0f);
}


TEST(SimDifferentialConfigTest, FeedbackInertia_UsesEfficiencyAndRatio)
{
	DifferentialConfigFixture fix;
	fix.inAxis[0].I = 1.0f;
	fix.inAxis[1].I = 2.0f;

	GfParmSetNum(fix.hdle, SECT_REARDIFFERENTIAL, PRM_INERTIA, (char*)NULL, 0.2f);
	GfParmSetNum(fix.hdle, SECT_REARDIFFERENTIAL, PRM_EFFICIENCY, (char*)NULL, 0.5f);
	GfParmSetNum(fix.hdle, SECT_REARDIFFERENTIAL, PRM_RATIO, (char*)NULL, 3.0f);

	SimDifferentialConfig(fix.hdle, SECT_REARDIFFERENTIAL, &fix.differential);

	// feedBack.I = I * ratio^2 + (in0.I + in1.I) / efficiency
	tdble expectedFeedbackI = 0.2f * 9.0f + 3.0f / 0.5f;
	EXPECT_NEAR(fix.differential.feedBack.I, expectedFeedbackI, 1e-6f);
}
