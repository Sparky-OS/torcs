/***************************************************************************

    file                 : engine_test_helpers.cpp
    created              : Sun Mar  8 22:00:00 CET 2026
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
    Implementation of test helpers for simuv2 engine unit tests.
*/

#include "engine_test_helpers.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>


tdble rulesFuelFactor = 1.0f;
tdble SimDeltaTime = 0.0f;


void* createEngineTestParmHandle()
{
	const char* tmpFile = "simuv2_test_engine_config.xml";
	return GfParmReadFile(tmpFile, GFPARM_RMODE_CREAT);
}


EngineFixture::EngineFixture()
{
	memset(&car, 0, sizeof(car));
	memset(&carElt, 0, sizeof(carElt));
	memset(&ctrl, 0, sizeof(ctrl));
	car.carElt = &carElt;
	car.ctrl = &ctrl;
	hdle = nullptr;
}


EngineFixture::~EngineFixture()
{
	if (car.engine.curve.data) {
		free(car.engine.curve.data);
		car.engine.curve.data = nullptr;
	}
	if (hdle) {
		GfParmReleaseHandle(hdle);
		hdle = nullptr;
	}
	std::remove("simuv2_test_engine_config.xml");
}


void EngineFixture::attachParmHandle(void* handle)
{
	hdle = handle;
	car.params = hdle;
}


void EngineFixture::allocateCurve(int nbPts)
{
	if (car.engine.curve.data) {
		free(car.engine.curve.data);
	}
	car.engine.curve.nbPts = nbPts;
	car.engine.curve.data = (tEngineCurveElem*)calloc(nbPts, sizeof(tEngineCurveElem));
}


void EngineFixture::setCurveElem(int index, tdble rads, tdble a, tdble b)
{
	car.engine.curve.data[index].rads = rads;
	car.engine.curve.data[index].a = a;
	car.engine.curve.data[index].b = b;
}


void EngineFixture::setCurvePoint(int oneBasedIndex, tdble rpm, tdble tq)
{
	char idx[128];
	snprintf(idx, sizeof(idx), "%s/%s/%d", SECT_ENGINE, ARR_DATAPTS, oneBasedIndex);
	GfParmSetNum(hdle, idx, PRM_RPM, (char*)NULL, rpm);
	GfParmSetNum(hdle, idx, PRM_TQ, (char*)NULL, tq);
}
