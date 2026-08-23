/***************************************************************************

    file                 : rttrack_sidenormalg_test.cpp
    created              : Mon Nov 11 20:00:00 CET 2025
    copyright            : (C) 2025-2025 by Bernhard Wymann
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


#include <gtest/gtest.h>
#include <cmath>

#include <track.h>
#include <robottools.h>


// Small helpers
static tdble length(const t3Dd& v)
{
    return static_cast<tdble>(std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z));
}


static void ExpectUnitVector2D(const t3Dd& v, tdble eps = 1e-6)
{
    tdble len = static_cast<tdble>(std::sqrt(v.x * v.x + v.y * v.y));
    EXPECT_NEAR(1.0, len, eps);
}


TEST(RtTrackSideNormalGTest, StraightSegment_RightSideUsesRgtSideNormal)
{
    tTrackSeg seg{};
    seg.type = TR_STR;

    // Right-side normal
    seg.rgtSideNormal.x = 0.0;
    seg.rgtSideNormal.y = 1.0;

    t3Dd norm{};
    // X,Y don't matter for TR_STR case
    RtTrackSideNormalG(&seg, 0.0, 0.0, TR_RGT, &norm);

    EXPECT_NEAR(0.0, norm.x, 1e-9);
    EXPECT_NEAR(1.0, norm.y, 1e-9);
    ExpectUnitVector2D(norm);
}


TEST(RtTrackSideNormalGTest, StraightSegment_LeftSideIsNegatedRightSide)
{
    tTrackSeg seg{};
    seg.type = TR_STR;

    // Right-side normal
    seg.rgtSideNormal.x = 0.0;
    seg.rgtSideNormal.y = 1.0;

    t3Dd norm{};
    // X,Y don't matter for TR_STR case
    RtTrackSideNormalG(&seg, 0.0, 0.0, TR_LFT, &norm);

    EXPECT_NEAR(0.0, norm.x, 1e-9);
    EXPECT_NEAR(-1.0, norm.y, 1e-9);
    ExpectUnitVector2D(norm);
}


struct CurveCase {
    int segType;     // TR_RGT or TR_LFT
    int side;        // TR_RGT or TR_LFT
    double expX;     // Expected normalized x
    double expY;     // Expected normalized y
    const char* name; // Test case name
};


class RtTrackSideNormalGCurveTest : public ::testing::TestWithParam<CurveCase> {
protected:
    static tdble length2D(const t3Dd& v) {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }
};


TEST_P(RtTrackSideNormalGCurveTest, ProducesCorrectNormal)
{
    CurveCase c = GetParam();

    tTrackSeg seg{};
    seg.type = c.segType;

    // arbitrary center / point combination that yields vector (3,4)
    seg.center.x = 10.0;
    seg.center.y = 20.0;
    tdble X = 13.0;
    tdble Y = 24.0;

    t3Dd norm{};
    RtTrackSideNormalG(&seg, X, Y, c.side, &norm);

    // Check direction
    EXPECT_NEAR(c.expX, norm.x, 1e-6) << "case: " << c.name;
    EXPECT_NEAR(c.expY, norm.y, 1e-6) << "case: " << c.name;

    // Check normalization
    EXPECT_NEAR(1.0, length2D(norm), 1e-6) << "case: " << c.name;
}


INSTANTIATE_TEST_SUITE_P(
    CurveCases,
    RtTrackSideNormalGCurveTest,
    ::testing::Values(
        // "outward" = from curve center to point (pos - center)
        // "inward"  = from point to curve center (center - pos)
        CurveCase{ TR_RGT, TR_RGT,  0.6,  0.8, "RightCurve_RightSide_Outward" },
        CurveCase{ TR_RGT, TR_LFT, -0.6, -0.8, "RightCurve_LeftSide_Inward" },
        CurveCase{ TR_LFT, TR_LFT,  0.6,  0.8, "LeftCurve_LeftSide_Outward" },
        CurveCase{ TR_LFT, TR_RGT, -0.6, -0.8, "LeftCurve_RightSide_Inward" }
    )
);
