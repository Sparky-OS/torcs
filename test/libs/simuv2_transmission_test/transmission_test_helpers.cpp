/***************************************************************************

    file                 : transmission_test_helpers.cpp
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
    Implementation of test helpers for simuv2 transmission unit tests.
*/

#include "transmission_test_helpers.h"

#include <cstdio>
#include <cstring>


static const char *gearname[MAX_GEARS] = {"r", "n", "1", "2", "3", "4", "5", "6", "7", "8"};


void* createTransmissionTestParmHandle()
{
	const char* tmpFile = "simuv2_test_transmission_config.xml";
	return GfParmReadFile(tmpFile, GFPARM_RMODE_CREAT);
}


TransmissionFixture::TransmissionFixture()
{
	memset(&car, 0, sizeof(car));
	memset(&carElt, 0, sizeof(carElt));
	memset(&ctrl, 0, sizeof(ctrl));
	car.carElt = &carElt;
	car.ctrl = &ctrl;
	hdle = nullptr;

	for (int i = 0; i < MAX_GEARS; i++) {
		car.transmission.gearEff[i] = 1.0f;
	}

	for (int j = 0; j < 2; j++) {
		car.transmission.differential[TRANS_FRONT_DIFF].inAxis[j] = &(car.wheel[j].feedBack);
		car.transmission.differential[TRANS_FRONT_DIFF].outAxis[j] = &(car.wheel[j].in);
		car.transmission.differential[TRANS_REAR_DIFF].inAxis[j] = &(car.wheel[2 + j].feedBack);
		car.transmission.differential[TRANS_REAR_DIFF].outAxis[j] = &(car.wheel[2 + j].in);
	}

	car.transmission.differential[TRANS_CENTRAL_DIFF].inAxis[0] = &(car.transmission.differential[TRANS_FRONT_DIFF].feedBack);
	car.transmission.differential[TRANS_CENTRAL_DIFF].outAxis[0] = &(car.transmission.differential[TRANS_FRONT_DIFF].in);
	car.transmission.differential[TRANS_CENTRAL_DIFF].inAxis[1] = &(car.transmission.differential[TRANS_REAR_DIFF].feedBack);
	car.transmission.differential[TRANS_CENTRAL_DIFF].outAxis[1] = &(car.transmission.differential[TRANS_REAR_DIFF].in);
}


TransmissionFixture::~TransmissionFixture()
{
	if (hdle) {
		GfParmReleaseHandle(hdle);
		hdle = nullptr;
	}
	std::remove("simuv2_test_transmission_config.xml");
}


void TransmissionFixture::attachParmHandle(void* handle)
{
	hdle = handle;
	car.params = hdle;
}


void TransmissionFixture::setTransmissionType(const char* type)
{
	GfParmSetStr(hdle, SECT_DRIVETRAIN, PRM_TYPE, type);
}


void TransmissionFixture::setShiftTime(tdble value)
{
	GfParmSetNum(hdle, SECT_GEARBOX, PRM_SHIFTTIME, (char*)NULL, value);
}


void TransmissionFixture::setDifferentialRatio(const char* section, tdble ratio)
{
	GfParmSetNum(hdle, section, PRM_RATIO, (char*)NULL, ratio);
}


void TransmissionFixture::setGearRatioByIndex(int index, tdble ratio)
{
	const int BUFSIZE = 256;
	char path[BUFSIZE];
	snprintf(path, BUFSIZE, "%s/%s/%s", SECT_GEARBOX, ARR_GEARS, gearname[index]);
	GfParmSetNum(hdle, path, PRM_RATIO, (char*)NULL, ratio);
}


void TransmissionFixture::setGearEfficiencyByIndex(int index, tdble efficiency)
{
	const int BUFSIZE = 256;
	char path[BUFSIZE];
	snprintf(path, BUFSIZE, "%s/%s/%s", SECT_GEARBOX, ARR_GEARS, gearname[index]);
	GfParmSetNum(hdle, path, PRM_EFFICIENCY, (char*)NULL, efficiency);
}


void TransmissionFixture::setGearInertiaByIndex(int index, tdble inertia)
{
	const int BUFSIZE = 256;
	char path[BUFSIZE];
	snprintf(path, BUFSIZE, "%s/%s/%s", SECT_GEARBOX, ARR_GEARS, gearname[index]);
	GfParmSetNum(hdle, path, PRM_INERTIA, (char*)NULL, inertia);
}


void TransmissionFixture::setPitGearRatioByGear(int gear, tdble value, tdble min, tdble max)
{
	tCarPitSetupValue* v = &carElt.pitcmd.setup.gearsratio[gear - 1];
	v->value = value;
	v->min = min;
	v->max = max;
}


void TransmissionFixture::setPitDiffRatio(int index, tdble value, tdble min, tdble max)
{
	tCarPitSetupValue* v = &carElt.pitcmd.setup.diffratio[index];
	v->value = value;
	v->min = min;
	v->max = max;
}


void TransmissionFixture::setWheelFeedBackInertia(int wheelIndex, tdble inertia)
{
	car.wheel[wheelIndex].feedBack.I = inertia;
}


void TransmissionFixture::setDiffFeedBackInertia(int index, tdble inertia)
{
	car.transmission.differential[index].feedBack.I = inertia;
}
