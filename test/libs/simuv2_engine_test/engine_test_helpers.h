/***************************************************************************

    file                 : engine_test_helpers.h
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
    Test helpers for simuv2 engine unit tests.
*/

#ifndef _ENGINE_TEST_HELPERS_H_
#define _ENGINE_TEST_HELPERS_H_

#include <tgf.h>
#include <car.h>
#include <raceman.h>
#include "sim.h"


void* createEngineTestParmHandle();


struct EngineFixture {
	tCar car;
	tCarElt carElt;
	tCarCtrl ctrl;
	void* hdle;

	EngineFixture();
	~EngineFixture();

	void attachParmHandle(void* handle);
	void allocateCurve(int nbPts);
	void setCurveElem(int index, tdble rads, tdble a, tdble b);
	void setCurvePoint(int oneBasedIndex, tdble rpm, tdble tq);
};


#endif /* _ENGINE_TEST_HELPERS_H_ */
