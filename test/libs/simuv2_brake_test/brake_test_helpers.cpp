/***************************************************************************

    file                 : brake_test_helpers.cpp
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
    Implementation of test helpers for simuv2 brake unit tests.
    Contains factory functions for constructing test structs.
*/

#include "brake_test_helpers.h"
#include <cstring>


tBrake makeBrake(
	tdble coeff,
	tdble I,
	tdble radius,
	tdble pressure,
	tdble temp,
	tdble Tq
)
{
	tBrake brake{};
	brake.coeff = coeff;
	brake.I = I;
	brake.radius = radius;
	brake.pressure = pressure;
	brake.temp = temp;
	brake.Tq = Tq;
	return brake;
}


tCar makeCarForBrakeUpdate(tdble velX)
{
	tCar car;
	memset(&car, 0, sizeof(car));
	car.DynGC.vel.x = velX;
	return car;
}


tWheel makeWheelForBrakeUpdate(tdble spinVel)
{
	tWheel wheel{};
	wheel.spinVel = spinVel;
	return wheel;
}


BrakeSystemUpdateFixture::BrakeSystemUpdateFixture()
{
	memset(&car, 0, sizeof(car));
	memset(&ctrl, 0, sizeof(ctrl));
	car.ctrl = &ctrl;
}


void BrakeSystemUpdateFixture::setBrakeSystem(tdble rep, tdble coeff, tdble clickValue, int maxClicks)
{
	car.brkSyst.rep = rep;
	car.brkSyst.coeff = coeff;
	car.brkSyst.repCmdClickValue = clickValue;
	car.brkSyst.repCmdMaxClicks = maxClicks;
}


void BrakeSystemUpdateFixture::setCtrl(tdble brakeCmd, int brakeRepartitionCmd)
{
	ctrl.brakeCmd = brakeCmd;
	ctrl.brakeRepartitionCmd = brakeRepartitionCmd;
}
