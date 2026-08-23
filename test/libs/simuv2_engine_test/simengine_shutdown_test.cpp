/***************************************************************************

    file                 : simengine_shutdown_test.cpp
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
    Unit tests for SimEngineShutdown().
*/

#include <gtest/gtest.h>

#include <cstdlib>
#include <cstring>

#include "sim.h"


TEST(SimEngineShutdownTest, FreesCurveData_Smoke)
{
	tCar car;
	memset(&car, 0, sizeof(car));

	car.engine.curve.nbPts = 2;
	car.engine.curve.data = (tEngineCurveElem*)malloc(2 * sizeof(tEngineCurveElem));
	ASSERT_NE(car.engine.curve.data, nullptr);

	SimEngineShutdown(&car);
	SUCCEED();
}
