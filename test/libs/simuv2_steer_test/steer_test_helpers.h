/***************************************************************************

    file                 : steer_test_helpers.h
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
    Test helpers for simuv2 steer unit tests.
*/

#ifndef _STEER_TEST_HELPERS_H_
#define _STEER_TEST_HELPERS_H_

#include <tgf.h>
#include <car.h>
#include <raceman.h>
#include "sim.h"


void* createSteerTestParmHandle();


struct SteerFixture {
	tCar car;
	tCarElt carElt;
	tCarCtrl ctrl;
	void* hdle;

	SteerFixture();
	~SteerFixture();

	void attachParmHandle(void* handle);
	void setSteerPitSetup(tdble value, tdble min, tdble max);
};


#endif /* _STEER_TEST_HELPERS_H_ */
