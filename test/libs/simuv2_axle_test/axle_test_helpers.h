/***************************************************************************

    file                 : axle_test_helpers.h
    created              : Sat Mar  8 12:00:00 CET 2026
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
    Test helpers for simuv2 axle unit tests.
    Provides factory functions for constructing tCar with the specific
    fields that axle.cpp reads and writes, and fixture structs for
    SimAxleConfig and SimAxleUpdate tests.
*/

#ifndef _AXLE_TEST_HELPERS_H_
#define _AXLE_TEST_HELPERS_H_

#include <tgf.h>
#include <car.h>
#include <raceman.h>
#include "sim.h"


/// Create a parameter handle pointing to a temp file.
/// Uses GfParmReadFile with GFPARM_RMODE_CREAT.
/// Caller must release with GfParmReleaseHandle.
void* createAxleTestParmHandle();


/// Fixture for SimAxleConfig tests.
/// Owns a tCar with a parameter handle set in car.params.
/// All fields zero-initialized except the parameter handle.
struct AxleConfigFixture {
	tCar car;
	void* hdle;

	AxleConfigFixture();
	~AxleConfigFixture();
};


/// Fixture for SimAxleUpdate tests.
/// Owns a zero-initialized tCar. Provides convenience setters
/// for the fields that SimAxleUpdate reads.
struct AxleUpdateFixture {
	tCar car;

	AxleUpdateFixture();

	/// Set wheel suspension travel and velocity for a wheel.
	void setWheelSusp(int wheelIdx, tdble x, tdble v);

	/// Set the anti-roll bar spring constant for an axle.
	void setArbSpringK(int axleIdx, tdble k);

	/// Set the third element suspension for an axle.
	void setThirdSusp(int axleIdx, const tSuspension& susp);
};


/// Fixture for SimAxleReConfig tests.
/// Owns tCar + tCarElt, with car.carElt linked.
/// All fields zero-initialized. Provides convenience setters
/// for pit setup values and pre-populating axle state.
struct AxleReConfigFixture {
	tCar car;
	tCarElt carElt;

	AxleReConfigFixture();

	/// Set a pit setup value as adjustable (min != max).
	void setSetupValue(tCarPitSetupValue* field, tdble value, tdble min, tdble max);

	/// Pre-populate an axle with known values before reconfig.
	void setAxle(int axleIdx, tdble arbSpringK, const tSuspension& thirdSusp);
};


#endif /* _AXLE_TEST_HELPERS_H_ */
