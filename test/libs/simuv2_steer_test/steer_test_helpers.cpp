/***************************************************************************

    file                 : steer_test_helpers.cpp
    created              : Thu Mar 12 00:00:00 CET 2026
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
    Implementation of test helpers for simuv2 steer unit tests.
*/

#include "steer_test_helpers.h"

#include <cmath>
#include <cstdio>
#include <cstring>


tdble SimDeltaTime = 0.0f;


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


void* createSteerTestParmHandle()
{
	const char* tmpFile = "simuv2_test_steer_config.xml";
	return GfParmReadFile(tmpFile, GFPARM_RMODE_CREAT);
}


SteerFixture::SteerFixture()
{
	memset(&car, 0, sizeof(car));
	memset(&carElt, 0, sizeof(carElt));
	memset(&ctrl, 0, sizeof(ctrl));
	car.carElt = &carElt;
	car.ctrl = &ctrl;
	hdle = nullptr;
}


SteerFixture::~SteerFixture()
{
	if (hdle) {
		GfParmReleaseHandle(hdle);
		hdle = nullptr;
	}
	std::remove("simuv2_test_steer_config.xml");
}


void SteerFixture::attachParmHandle(void* handle)
{
	hdle = handle;
	car.params = hdle;
}


void SteerFixture::setSteerPitSetup(tdble value, tdble min, tdble max)
{
	tCarPitSetupValue* v = &car.carElt->pitcmd.setup.steerLock;
	v->value = value;
	v->min = min;
	v->max = max;
}
