/***************************************************************************

    file                 : axle_test_helpers.cpp
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
    Implementation of test helpers for simuv2 axle unit tests.
    Contains factory functions for constructing test structs.
*/

#include "axle_test_helpers.h"
#include <cstdio>
#include <cstring>


void* createAxleTestParmHandle()
{
	const char* tmpFile = "simuv2_test_axle_config.xml";
	void* hdle = GfParmReadFile(tmpFile, GFPARM_RMODE_CREAT);
	return hdle;
}


AxleConfigFixture::AxleConfigFixture()
{
	memset(&car, 0, sizeof(car));
	hdle = createAxleTestParmHandle();
	car.params = hdle;
}


AxleConfigFixture::~AxleConfigFixture()
{
	if (hdle) {
		GfParmReleaseHandle(hdle);
		hdle = nullptr;
	}
	std::remove("simuv2_test_axle_config.xml");
}


AxleUpdateFixture::AxleUpdateFixture()
{
	memset(&car, 0, sizeof(car));
}


void AxleUpdateFixture::setWheelSusp(int wheelIdx, tdble x, tdble v)
{
	car.wheel[wheelIdx].susp.x = x;
	car.wheel[wheelIdx].susp.v = v;
}


void AxleUpdateFixture::setArbSpringK(int axleIdx, tdble k)
{
	car.axle[axleIdx].arbSuspSpringK = k;
}


void AxleUpdateFixture::setThirdSusp(int axleIdx, const tSuspension& susp)
{
	car.axle[axleIdx].thirdSusp = susp;
}


AxleReConfigFixture::AxleReConfigFixture()
{
	memset(&car, 0, sizeof(car));
	memset(&carElt, 0, sizeof(carElt));
	car.carElt = &carElt;
}


void AxleReConfigFixture::setSetupValue(tCarPitSetupValue* field, tdble value, tdble min, tdble max)
{
	field->value = value;
	field->min = min;
	field->max = max;
}


void AxleReConfigFixture::setAxle(int axleIdx, tdble arbSpringK, const tSuspension& thirdSusp)
{
	car.axle[axleIdx].arbSuspSpringK = arbSpringK;
	car.axle[axleIdx].thirdSusp = thirdSusp;
}
