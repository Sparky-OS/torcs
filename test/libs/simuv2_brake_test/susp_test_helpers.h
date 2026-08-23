/***************************************************************************

    file                 : susp_test_helpers.h
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
    Test helpers for simuv2 suspension unit tests.
    Provides a local implementation of SimAdjustPitCarSetupParam
    (avoiding linking the full simu.cpp) and utility functions
    for constructing test structs.
*/

#ifndef _SUSP_TEST_HELPERS_H_
#define _SUSP_TEST_HELPERS_H_

#include <tgf.h>
#include <car.h>
#include <raceman.h>
#include "sim.h"


/// Create a zero-initialized tSuspension with configurable parameters.
/// All damper b2 values are computed via initDamper-equivalent logic.
tSuspension makeSuspension(
	tdble springK,
	tdble springF0,
	tdble springX0,
	tdble springXMax,
	tdble bellcrank,
	tdble packers,
	tdble bumpC1,
	tdble bumpC2,
	tdble bumpV1,
	tdble reboundC1,
	tdble reboundC2,
	tdble reboundV1
);

/// Minimal tCar + tCarElt pair for ReConfig tests.
/// Caller owns both objects; carElt is linked into car->carElt.
/// All pit setup values are initialized to non-adjustable (min == max == 0).
struct CarReConfigFixture {
	tCar car;
	tCarElt carElt;

	CarReConfigFixture();

	/// Set a pit setup value as adjustable (min != max) for a wheel suspension field.
	void setSuspSetupValue(tCarPitSetupValue* field, tdble value, tdble min, tdble max);
};


#endif /* _SUSP_TEST_HELPERS_H_ */
