/***************************************************************************

    file                 : transmission_test_stubs.cpp
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
    Local stubs for simuv2 transmission unit tests.
*/

#include "transmission_test_helpers.h"

#include <cmath>
#include <cstring>


tdble SimDeltaTime = 0.0f;


static const int MAX_STUB_CALLS = 32;

static int gDiffConfigCallCount = 0;
static char gDiffConfigSection[MAX_STUB_CALLS][64];

static int gDiffReConfigCallCount = 0;
static int gDiffReConfigIndex[MAX_STUB_CALLS];

static int gDiffUpdateCallCount = 0;
static int gDiffUpdateFirst[MAX_STUB_CALLS];
static const tDifferential* gDiffUpdatePtr[MAX_STUB_CALLS];

static int gFreeWheelsCallCount = 0;
static int gFreeWheelsLastAxle = -1;


void TransmissionTestResetStubState()
{
	gDiffConfigCallCount = 0;
	gDiffReConfigCallCount = 0;
	gDiffUpdateCallCount = 0;
	gFreeWheelsCallCount = 0;
	gFreeWheelsLastAxle = -1;
	memset(gDiffConfigSection, 0, sizeof(gDiffConfigSection));
	memset(gDiffReConfigIndex, 0, sizeof(gDiffReConfigIndex));
	memset(gDiffUpdateFirst, 0, sizeof(gDiffUpdateFirst));
	memset(gDiffUpdatePtr, 0, sizeof(gDiffUpdatePtr));
}


int TransmissionTestGetDifferentialConfigCallCount()
{
	return gDiffConfigCallCount;
}


const char* TransmissionTestGetDifferentialConfigSection(int index)
{
	if ((index < 0) || (index >= gDiffConfigCallCount)) {
		return "";
	}
	return gDiffConfigSection[index];
}


int TransmissionTestGetDifferentialReConfigCallCount()
{
	return gDiffReConfigCallCount;
}


int TransmissionTestGetDifferentialReConfigIndex(int index)
{
	if ((index < 0) || (index >= gDiffReConfigCallCount)) {
		return -1;
	}
	return gDiffReConfigIndex[index];
}


int TransmissionTestGetDifferentialUpdateCallCount()
{
	return gDiffUpdateCallCount;
}


int TransmissionTestGetDifferentialUpdateFirst(int index)
{
	if ((index < 0) || (index >= gDiffUpdateCallCount)) {
		return -1;
	}
	return gDiffUpdateFirst[index];
}


const tDifferential* TransmissionTestGetDifferentialUpdatePtr(int index)
{
	if ((index < 0) || (index >= gDiffUpdateCallCount)) {
		return nullptr;
	}
	return gDiffUpdatePtr[index];
}


int TransmissionTestGetUpdateFreeWheelsCallCount()
{
	return gFreeWheelsCallCount;
}


int TransmissionTestGetUpdateFreeWheelsLastAxle()
{
	return gFreeWheelsLastAxle;
}


void SimDifferentialConfig(void* hdle, const char* section, tDifferential *differential)
{
	if (gDiffConfigCallCount < MAX_STUB_CALLS) {
		snprintf(gDiffConfigSection[gDiffConfigCallCount], sizeof(gDiffConfigSection[gDiffConfigCallCount]), "%s", section);
	}
	gDiffConfigCallCount++;

	differential->ratio = GfParmGetNum(hdle, section, PRM_RATIO, (char*)NULL, 1.0f);
	differential->efficiency = 1.0f;
}


void SimDifferentialReConfig(tCar* car, int index)
{
	if (gDiffReConfigCallCount < MAX_STUB_CALLS) {
		gDiffReConfigIndex[gDiffReConfigCallCount] = index;
	}
	gDiffReConfigCallCount++;

	tCarPitSetupValue* v = &car->carElt->pitcmd.setup.diffratio[index];
	if (SimAdjustPitCarSetupParam(v)) {
		car->transmission.differential[index].ratio = v->value;
	}
}


void SimDifferentialUpdate(tCar*, tDifferential *differential, int first)
{
	if (gDiffUpdateCallCount < MAX_STUB_CALLS) {
		gDiffUpdateFirst[gDiffUpdateCallCount] = first;
		gDiffUpdatePtr[gDiffUpdateCallCount] = differential;
	}
	gDiffUpdateCallCount++;
}


void SimUpdateFreeWheels(tCar*, int axlenb)
{
	gFreeWheelsCallCount++;
	gFreeWheelsLastAxle = axlenb;
}


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
