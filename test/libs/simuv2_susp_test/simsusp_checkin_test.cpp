/***************************************************************************

    file                 : simsusp_checkin_test.cpp
    created              : Tue Mar  3 12:00:00 CET 2026
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
    Unit tests for SimSuspCheckIn().

    SimSuspCheckIn resets state, clamps x against packers (minimum travel),
    scales x by bellcrank, and clamps against xMax (maximum travel).
    It sets SIM_SUSP_COMP when packers are hit and SIM_SUSP_EXT when
    fully extended.
*/

#include <gtest/gtest.h>
#include <cmath>
#include "susp_test_helpers.h"


// ---------------------------------------------------------------------------
// Plain TEST cases for SimSuspCheckIn
// ---------------------------------------------------------------------------

TEST(SimSuspCheckInTest, NormalTravel_NoClamp)
{
	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.1f, 0.5f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	susp.x = 0.2f;

	SimSuspCheckIn(&susp);

	EXPECT_FLOAT_EQ(susp.x, 0.2f);  // 0.2 * bellcrank(1.0) = 0.2, within [0, 0.5]
	EXPECT_EQ(susp.state, 0);
}


TEST(SimSuspCheckInTest, PackerContact_ClampsAndSetsCompFlag)
{
	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.1f, 0.5f, 1.0f, 0.05f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	susp.x = 0.02f;  // Below packers (0.05)

	SimSuspCheckIn(&susp);

	EXPECT_FLOAT_EQ(susp.x, 0.05f);  // Clamped to packers, then * bellcrank(1.0)
	EXPECT_EQ(susp.state, SIM_SUSP_COMP);
}


TEST(SimSuspCheckInTest, FullExtension_ClampsAndSetsExtFlag)
{
	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.1f, 0.3f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	susp.x = 0.4f;  // After bellcrank(1.0) scaling: 0.4 > xMax(0.3)

	SimSuspCheckIn(&susp);

	EXPECT_FLOAT_EQ(susp.x, 0.3f);  // Clamped to xMax
	EXPECT_EQ(susp.state, SIM_SUSP_EXT);
}


TEST(SimSuspCheckInTest, ExactlyAtPackers_NoCompFlag)
{
	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.1f, 0.5f, 1.0f, 0.05f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	susp.x = 0.05f;  // Exactly at packers, condition is strict <

	SimSuspCheckIn(&susp);

	EXPECT_FLOAT_EQ(susp.x, 0.05f);
	EXPECT_EQ(susp.state, 0);  // No COMP flag because x is not < packers
}


TEST(SimSuspCheckInTest, ExactlyAtXMax_NoExtFlag)
{
	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.1f, 0.3f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	susp.x = 0.3f;  // After bellcrank(1.0): 0.3 == xMax(0.3), not >

	SimSuspCheckIn(&susp);

	EXPECT_FLOAT_EQ(susp.x, 0.3f);
	EXPECT_EQ(susp.state, 0);  // No EXT flag because x is not > xMax
}


TEST(SimSuspCheckInTest, BellcrankScaling_Half)
{
	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.1f, 0.5f, 0.5f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	susp.x = 0.4f;

	SimSuspCheckIn(&susp);

	EXPECT_FLOAT_EQ(susp.x, 0.2f);  // 0.4 * 0.5 = 0.2
	EXPECT_EQ(susp.state, 0);
}


TEST(SimSuspCheckInTest, BellcrankScaling_Double)
{
	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.1f, 0.5f, 2.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	susp.x = 0.2f;

	SimSuspCheckIn(&susp);

	EXPECT_FLOAT_EQ(susp.x, 0.4f);  // 0.2 * 2.0 = 0.4
	EXPECT_EQ(susp.state, 0);
}


TEST(SimSuspCheckInTest, BellcrankCausesExtension)
{
	// x is within range before bellcrank, but exceeds xMax after scaling
	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.1f, 0.5f, 2.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	susp.x = 0.3f;  // 0.3 * 2.0 = 0.6 > xMax(0.5)

	SimSuspCheckIn(&susp);

	EXPECT_FLOAT_EQ(susp.x, 0.5f);  // Clamped to xMax
	EXPECT_EQ(susp.state, SIM_SUSP_EXT);
}


TEST(SimSuspCheckInTest, PackersThenBellcrankThenExtension)
{
	// Packers clamp first, then bellcrank scaling causes extension
	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.1f, 0.1f, 2.0f, 0.08f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	susp.x = 0.02f;  // < packers(0.08), so clamped to 0.08, then * 2.0 = 0.16 > xMax(0.1)

	SimSuspCheckIn(&susp);

	EXPECT_FLOAT_EQ(susp.x, 0.1f);  // Clamped to xMax after bellcrank scaling
	// Both conditions triggered: COMP first, then EXT overwrites
	EXPECT_EQ(susp.state, SIM_SUSP_EXT);
}


TEST(SimSuspCheckInTest, StateResetEachCall)
{
	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.1f, 0.5f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	// First call: force extension
	susp.x = 0.6f;
	SimSuspCheckIn(&susp);
	EXPECT_EQ(susp.state, SIM_SUSP_EXT);

	// Second call: normal range, state should be reset to 0
	susp.x = 0.2f;
	SimSuspCheckIn(&susp);
	EXPECT_EQ(susp.state, 0);
}


TEST(SimSuspCheckInTest, NegativeX_HitsPackers)
{
	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.1f, 0.5f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	susp.x = -0.1f;  // Negative, below packers (0.0)

	SimSuspCheckIn(&susp);

	EXPECT_FLOAT_EQ(susp.x, 0.0f);  // Clamped to packers (0.0), * 1.0
	EXPECT_EQ(susp.state, SIM_SUSP_COMP);
}


// ---------------------------------------------------------------------------
// Parameterized test for systematic coverage of CheckIn scenarios
// ---------------------------------------------------------------------------

struct CheckInCase {
	const char* name;
	tdble inputX;
	tdble packers;
	tdble bellcrank;
	tdble xMax;
	tdble expectedX;
	int expectedState;
};

class SimSuspCheckInParamTest : public ::testing::TestWithParam<CheckInCase> {};

TEST_P(SimSuspCheckInParamTest, CheckInProducesExpectedResult)
{
	const CheckInCase& c = GetParam();

	tSuspension susp = makeSuspension(
		-175000.0f, 5000.0f, 0.1f, c.xMax, c.bellcrank, c.packers,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f
	);
	susp.x = c.inputX;

	SimSuspCheckIn(&susp);

	EXPECT_NEAR(susp.x, c.expectedX, 1e-6f) << "case: " << c.name;
	EXPECT_EQ(susp.state, c.expectedState) << "case: " << c.name;
}

INSTANTIATE_TEST_SUITE_P(CheckInCases, SimSuspCheckInParamTest, ::testing::Values(
	// Normal range, no clamping
	CheckInCase{"normal_bc1",   0.2f,  0.0f,  1.0f, 0.5f,  0.2f,  0},
	CheckInCase{"normal_bc05",  0.4f,  0.0f,  0.5f, 0.5f,  0.2f,  0},
	CheckInCase{"normal_bc20",  0.1f,  0.0f,  2.0f, 0.5f,  0.2f,  0},
	// Packer contact
	CheckInCase{"packer_hit",   0.01f, 0.05f, 1.0f, 0.5f,  0.05f, SIM_SUSP_COMP},
	CheckInCase{"packer_neg",  -0.05f, 0.0f,  1.0f, 0.5f,  0.0f,  SIM_SUSP_COMP},
	// Extension clamp
	CheckInCase{"ext_bc1",      0.6f,  0.0f,  1.0f, 0.5f,  0.5f,  SIM_SUSP_EXT},
	CheckInCase{"ext_bc15",     0.4f,  0.0f,  1.5f, 0.5f,  0.5f,  SIM_SUSP_EXT},
	// Boundary: exactly at limit
	CheckInCase{"at_packer",    0.05f, 0.05f, 1.0f, 0.5f,  0.05f, 0},
	CheckInCase{"at_xmax",      0.5f,  0.0f,  1.0f, 0.5f,  0.5f,  0},
	// Packer then extension
	CheckInCase{"packer_ext",   0.01f, 0.08f, 3.0f, 0.2f,  0.2f,  SIM_SUSP_EXT},
	// Zero bellcrank
	CheckInCase{"bc_zero",      0.3f,  0.0f,  0.0f, 0.5f,  0.0f,  0}
));
