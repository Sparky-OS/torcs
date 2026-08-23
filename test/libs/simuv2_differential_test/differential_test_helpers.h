/***************************************************************************

    file                 : differential_test_helpers.h
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
    Test helpers for simuv2 differential unit tests.
*/

#ifndef _DIFFERENTIAL_TEST_HELPERS_H_
#define _DIFFERENTIAL_TEST_HELPERS_H_

#include <tgf.h>
#include <car.h>
#include <raceman.h>
#include "sim.h"


void* createDifferentialTestParmHandle();


struct DifferentialConfigFixture {
	tDifferential differential;
	tDynAxis inAxis[2];
	tDynAxis outAxis[2];
	void* hdle;

	DifferentialConfigFixture();
	~DifferentialConfigFixture();
};


struct DifferentialReConfigFixture {
	tCar car;
	tCarElt carElt;
	tDynAxis inAxis[3][2];
	tDynAxis outAxis[3][2];

	DifferentialReConfigFixture();

	void setSetupValue(tCarPitSetupValue* field, tdble value, tdble min, tdble max);
	void setDifferentialAxes(int index, tdble inI0, tdble inI1, tdble outI0, tdble outI1);
};


struct DifferentialUpdateFixture {
	tCar car;
	tDifferential differential;
	tDynAxis inAxis[2];
	tDynAxis outAxis[2];

	DifferentialUpdateFixture();
};


void DifferentialTestEnableEngineStub(tdble returnValue);
void DifferentialTestDisableEngineStub();
int DifferentialTestGetEngineStubCallCount();
tdble DifferentialTestGetEngineStubLastAxleRpm();


#endif /* _DIFFERENTIAL_TEST_HELPERS_H_ */
