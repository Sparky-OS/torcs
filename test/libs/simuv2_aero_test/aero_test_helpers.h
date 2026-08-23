/***************************************************************************

    file                 : aero_test_helpers.h
    created              : Tue Mar 10 00:00:00 CET 2026
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
    Test helpers for simuv2 aero unit tests.
*/

#ifndef _AERO_TEST_HELPERS_H_
#define _AERO_TEST_HELPERS_H_

#include <tgf.h>
#include <car.h>
#include <raceman.h>
#include "sim.h"


void* createAeroTestParmHandle();


struct AeroFixture {
	tCar car;
	tCarElt carElt;
	tCarCtrl ctrl;
	void* hdle;

	AeroFixture();
	~AeroFixture();

	void attachParmHandle(void* handle);
	void setAeroParam(const char* name, tdble value);
	void setWingParam(int index, const char* name, tdble value);
	void setRideHeights(tdble frntRgt, tdble frntLft, tdble rearRgt, tdble rearLft);
	void setPosition(tdble x, tdble y, tdble yaw);
	void setVelocities(tdble vxLocal, tdble vzLocal, tdble vxGlobal, tdble vyGlobal);
};


#endif /* _AERO_TEST_HELPERS_H_ */
