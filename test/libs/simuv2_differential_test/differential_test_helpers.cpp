/***************************************************************************

    file                 : differential_test_helpers.cpp
    created              : Sun Mar  8 14:00:00 CET 2026
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
    Implementation of test helpers for simuv2 differential unit tests.
*/

#include "differential_test_helpers.h"
#include <cstdio>
#include <cstring>


static bool gEngineStubEnabled = false;
static tdble gEngineStubReturnValue = 0.0f;
static int gEngineStubCallCount = 0;
static tdble gEngineStubLastAxleRpm = 0.0f;


tdble SimDeltaTime = 0.0f;


void* createDifferentialTestParmHandle()
{
	const char* tmpFile = "simuv2_test_differential_config.xml";
	return GfParmReadFile(tmpFile, GFPARM_RMODE_CREAT);
}


DifferentialConfigFixture::DifferentialConfigFixture()
{
	memset(&differential, 0, sizeof(differential));
	memset(inAxis, 0, sizeof(inAxis));
	memset(outAxis, 0, sizeof(outAxis));
	differential.inAxis[0] = &inAxis[0];
	differential.inAxis[1] = &inAxis[1];
	differential.outAxis[0] = &outAxis[0];
	differential.outAxis[1] = &outAxis[1];
	hdle = createDifferentialTestParmHandle();
}


DifferentialConfigFixture::~DifferentialConfigFixture()
{
	if (hdle) {
		GfParmReleaseHandle(hdle);
		hdle = nullptr;
	}
	std::remove("simuv2_test_differential_config.xml");
}


DifferentialReConfigFixture::DifferentialReConfigFixture()
{
	memset(&car, 0, sizeof(car));
	memset(&carElt, 0, sizeof(carElt));
	memset(inAxis, 0, sizeof(inAxis));
	memset(outAxis, 0, sizeof(outAxis));
	car.carElt = &carElt;

	for (int i = 0; i < 3; i++) {
		car.transmission.differential[i].inAxis[0] = &inAxis[i][0];
		car.transmission.differential[i].inAxis[1] = &inAxis[i][1];
		car.transmission.differential[i].outAxis[0] = &outAxis[i][0];
		car.transmission.differential[i].outAxis[1] = &outAxis[i][1];
	}
}


void DifferentialReConfigFixture::setSetupValue(tCarPitSetupValue* field, tdble value, tdble min, tdble max)
{
	field->value = value;
	field->min = min;
	field->max = max;
}


void DifferentialReConfigFixture::setDifferentialAxes(int index, tdble inI0, tdble inI1, tdble outI0, tdble outI1)
{
	car.transmission.differential[index].inAxis[0]->I = inI0;
	car.transmission.differential[index].inAxis[1]->I = inI1;
	car.transmission.differential[index].outAxis[0]->I = outI0;
	car.transmission.differential[index].outAxis[1]->I = outI1;
}


DifferentialUpdateFixture::DifferentialUpdateFixture()
{
	memset(&car, 0, sizeof(car));
	memset(&differential, 0, sizeof(differential));
	memset(inAxis, 0, sizeof(inAxis));
	memset(outAxis, 0, sizeof(outAxis));

	differential.inAxis[0] = &inAxis[0];
	differential.inAxis[1] = &inAxis[1];
	differential.outAxis[0] = &outAxis[0];
	differential.outAxis[1] = &outAxis[1];
}


void DifferentialTestEnableEngineStub(tdble returnValue)
{
	gEngineStubEnabled = true;
	gEngineStubReturnValue = returnValue;
	gEngineStubCallCount = 0;
	gEngineStubLastAxleRpm = 0.0f;
}


void DifferentialTestDisableEngineStub()
{
	gEngineStubEnabled = false;
	gEngineStubReturnValue = 0.0f;
	gEngineStubCallCount = 0;
	gEngineStubLastAxleRpm = 0.0f;
}


int DifferentialTestGetEngineStubCallCount()
{
	return gEngineStubCallCount;
}


tdble DifferentialTestGetEngineStubLastAxleRpm()
{
	return gEngineStubLastAxleRpm;
}


tdble SimEngineUpdateRpm(tCar*, tdble axleRpm)
{
	gEngineStubCallCount++;
	gEngineStubLastAxleRpm = axleRpm;

	if (gEngineStubEnabled) {
		return gEngineStubReturnValue;
	}

	return 0.0f;
}
