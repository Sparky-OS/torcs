/***************************************************************************

    file                 : susp_test_helpers.cpp
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
    Implementation of test helpers for simuv2 suspension unit tests.
    Contains the local stub for SimAdjustPitCarSetupParam and
    factory functions for test struct construction.
*/

#include "susp_test_helpers.h"
#include <cstring>
#include <cmath>


/*
 * Local implementation of SimAdjustPitCarSetupParam.
 * This is a faithful copy of the logic from simu.cpp:491-505,
 * provided here to avoid pulling in all of simu.cpp and its
 * heavy dependencies.
 */
bool SimAdjustPitCarSetupParam(tCarPitSetupValue* v)
{
	// If min == max there is nothing to adjust
	if (fabs(v->max - v->min) >= 0.0001f) {
		// Ensure that value is in intended borders
		if (v->value > v->max) {
			v->value = v->max;
		} else if (v->value < v->min) {
			v->value = v->min;
		}
		return true;
	}

	v->value = v->max;
	return false;
}


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
)
{
	tSuspension susp{};

	susp.spring.K = springK;
	susp.spring.F0 = springF0;
	susp.spring.x0 = springX0;
	susp.spring.xMax = springXMax;
	susp.spring.bellcrank = bellcrank;
	susp.spring.packers = packers;

	susp.damper.bump.C1 = bumpC1;
	susp.damper.bump.C2 = bumpC2;
	susp.damper.bump.v1 = bumpV1;

	susp.damper.rebound.C1 = reboundC1;
	susp.damper.rebound.C2 = reboundC2;
	susp.damper.rebound.v1 = reboundV1;

	// Compute b2 values (same as initDamper in susp.cpp)
	susp.damper.bump.b2 = (bumpC1 - bumpC2) * bumpV1;
	susp.damper.rebound.b2 = (reboundC1 - reboundC2) * reboundV1;

	return susp;
}


CarReConfigFixture::CarReConfigFixture()
{
	memset(&car, 0, sizeof(car));
	memset(&carElt, 0, sizeof(carElt));
	car.carElt = &carElt;
}


void CarReConfigFixture::setSuspSetupValue(tCarPitSetupValue* field, tdble value, tdble min, tdble max)
{
	field->value = value;
	field->min = min;
	field->max = max;
}
