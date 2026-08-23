/***************************************************************************

    file                 : aero_test_stubs.cpp
    created              : Tue Mar 10 00:00:00 CET 2026
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
    Local stubs for simuv2 aero unit tests.
*/

#include <cmath>

#include "sim.h"


tCar* SimCarTable = nullptr;


bool SimAdjustPitCarSetupParam(tCarPitSetupValue* v)
{
	if (fabs(v->max - v->min) >= 0.0001f) {
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
