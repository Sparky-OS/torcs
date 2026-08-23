/***************************************************************************

    file                 : aero_test_helpers.cpp
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
    Implementation of test helpers for simuv2 aero unit tests.
*/

#include "aero_test_helpers.h"

#include <cstdio>
#include <cstring>


void* createAeroTestParmHandle()
{
	const char* tmpFile = "simuv2_test_aero_config.xml";
	return GfParmReadFile(tmpFile, GFPARM_RMODE_CREAT);
}


AeroFixture::AeroFixture()
{
	memset(&car, 0, sizeof(car));
	memset(&carElt, 0, sizeof(carElt));
	memset(&ctrl, 0, sizeof(ctrl));
	car.carElt = &carElt;
	car.ctrl = &ctrl;
	hdle = nullptr;
}


AeroFixture::~AeroFixture()
{
	if (hdle) {
		GfParmReleaseHandle(hdle);
		hdle = nullptr;
	}
	std::remove("simuv2_test_aero_config.xml");
}


void AeroFixture::attachParmHandle(void* handle)
{
	hdle = handle;
	car.params = hdle;
}


void AeroFixture::setAeroParam(const char* name, tdble value)
{
	GfParmSetNum(hdle, SECT_AERODYNAMICS, name, (char*)NULL, value);
}


void AeroFixture::setWingParam(int index, const char* name, tdble value)
{
	const char* section = (index == 0) ? SECT_FRNTWING : SECT_REARWING;
	GfParmSetNum(hdle, section, name, (char*)NULL, value);
}


void AeroFixture::setRideHeights(tdble frntRgt, tdble frntLft, tdble rearRgt, tdble rearLft)
{
	car.wheel[FRNT_RGT].rideHeight = frntRgt;
	car.wheel[FRNT_LFT].rideHeight = frntLft;
	car.wheel[REAR_RGT].rideHeight = rearRgt;
	car.wheel[REAR_LFT].rideHeight = rearLft;
}


void AeroFixture::setPosition(tdble x, tdble y, tdble yaw)
{
	car.DynGCg.pos.x = x;
	car.DynGCg.pos.y = y;
	car.DynGCg.pos.az = yaw;
}


void AeroFixture::setVelocities(tdble vxLocal, tdble vzLocal, tdble vxGlobal, tdble vyGlobal)
{
	car.DynGC.vel.x = vxLocal;
	car.DynGC.vel.z = vzLocal;
	car.DynGCg.vel.x = vxGlobal;
	car.DynGCg.vel.y = vyGlobal;
}
