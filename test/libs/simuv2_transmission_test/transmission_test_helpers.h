/***************************************************************************

    file                 : transmission_test_helpers.h
    created              : Mon Mar  9 22:00:00 CET 2026
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
    Test helpers for simuv2 transmission unit tests.
*/

#ifndef _TRANSMISSION_TEST_HELPERS_H_
#define _TRANSMISSION_TEST_HELPERS_H_

#include <tgf.h>
#include <car.h>
#include <raceman.h>
#include "sim.h"


void* createTransmissionTestParmHandle();

void TransmissionTestResetStubState();
int TransmissionTestGetDifferentialConfigCallCount();
const char* TransmissionTestGetDifferentialConfigSection(int index);
int TransmissionTestGetDifferentialReConfigCallCount();
int TransmissionTestGetDifferentialReConfigIndex(int index);
int TransmissionTestGetDifferentialUpdateCallCount();
int TransmissionTestGetDifferentialUpdateFirst(int index);
const tDifferential* TransmissionTestGetDifferentialUpdatePtr(int index);
int TransmissionTestGetUpdateFreeWheelsCallCount();
int TransmissionTestGetUpdateFreeWheelsLastAxle();


struct TransmissionFixture {
	tCar car;
	tCarElt carElt;
	tCarCtrl ctrl;
	void* hdle;

	TransmissionFixture();
	~TransmissionFixture();

	void attachParmHandle(void* handle);
	void setTransmissionType(const char* type);
	void setShiftTime(tdble value);
	void setDifferentialRatio(const char* section, tdble ratio);
	void setGearRatioByIndex(int index, tdble ratio);
	void setGearEfficiencyByIndex(int index, tdble efficiency);
	void setGearInertiaByIndex(int index, tdble inertia);
	void setPitGearRatioByGear(int gear, tdble value, tdble min, tdble max);
	void setPitDiffRatio(int index, tdble value, tdble min, tdble max);
	void setWheelFeedBackInertia(int wheelIndex, tdble inertia);
	void setDiffFeedBackInertia(int index, tdble inertia);
};


#endif /* _TRANSMISSION_TEST_HELPERS_H_ */
