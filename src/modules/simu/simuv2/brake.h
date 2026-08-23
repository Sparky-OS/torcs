/***************************************************************************

    file                 : brake.h
    created              : Sun Mar 19 00:05:34 CET 2000
    copyright            : (C) 2000-2024 by Eric Espie, Bernhard Wymann
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

#ifndef _BRAKE_H_
#define _BRAKE_H_

typedef struct
{
    tdble	pressure;
    tdble	Tq;
    tdble	coeff;
    tdble	I;
    tdble	radius;
    tdble	temp;
} tBrake;

typedef struct
{
    tdble	rep;	/* front/rear repartition, 1.0 means all to front, 0.0 all to rear*/ 
    tdble	coeff;	/* coefficient to convert brake command [0..1] into pressure */
	// Brake repartition adjustment during driving, e.g. 4 clicks with a value of 0.5 would make 2 percent to the front
	// This is used in real world to account change like tire wear, track rubbing in or even on a turn by turn adjustment
	// depending on the track
	int		repCmdMaxClicks;	/* brake repartition maximum clicks, is multiplied with the value below */
	tdble	repCmdClickValue;	/* brake repartition adjustment per click */
} tBrakeSyst;



#endif /* _BRAKE_H_ */ 



