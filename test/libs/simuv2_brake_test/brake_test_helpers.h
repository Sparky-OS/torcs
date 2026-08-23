/***************************************************************************

    file                 : brake_test_helpers.h
    created              : Tue Mar  4 12:00:00 CET 2026
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
    Test helpers for simuv2 brake unit tests.
    Provides factory functions for constructing tBrake, tCar, tWheel,
    and a fixture struct for SimBrakeSystemUpdate tests.
*/

#ifndef _BRAKE_TEST_HELPERS_H_
#define _BRAKE_TEST_HELPERS_H_

#include <tgf.h>
#include <car.h>
#include <raceman.h>
#include "sim.h"


/// Create a zero-initialized tBrake with configurable parameters.
tBrake makeBrake(
	tdble coeff,
	tdble I,
	tdble radius,
	tdble pressure = 0.0f,
	tdble temp = 0.0f,
	tdble Tq = 0.0f
);


/// Create a zero-initialized tCar with DynGC.vel.x set.
/// Used by SimBrakeUpdate for the temperature cooling term.
tCar makeCarForBrakeUpdate(tdble velX);


/// Create a zero-initialized tWheel with spinVel set.
/// Used by SimBrakeUpdate for the temperature heating term.
tWheel makeWheelForBrakeUpdate(tdble spinVel);


/// Fixture for SimBrakeSystemUpdate tests.
/// Owns tCar and tCarCtrl, with car.ctrl linked to ctrl.
/// All fields zero-initialized; provides setters for convenience.
struct BrakeSystemUpdateFixture {
	tCar car;
	tCarCtrl ctrl;

	BrakeSystemUpdateFixture();

	/// Set brake system parameters on car.brkSyst.
	void setBrakeSystem(tdble rep, tdble coeff, tdble clickValue, int maxClicks);

	/// Set driver control inputs.
	void setCtrl(tdble brakeCmd, int brakeRepartitionCmd);
};


#endif /* _BRAKE_TEST_HELPERS_H_ */
