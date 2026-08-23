/***************************************************************************

    file        : portability.h
    created     : Fri Jul 8 15:19:34 CET 2005
    copyright   : (C) 2005 Bernhard Wymann
    email       : berniw@bluewin.ch
    version     : $Id$

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _TORCS_PORTABILITY_H_
#define _TORCS_PORTABILITY_H_

#include <stdlib.h>
#include <cstring>

#ifdef WIN32
#define HAVE_CONFIG_H
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Missing strndup, define it here (for FreeBSD).
// TODO: Move it into library.
// strndup code provided by Thierry Thomas.
#ifndef HAVE_STRNDUP

static char *strndup(const char *str, int len)
{
	char *ret;

	if ((str == NULL || len < 0)) {
		return (NULL);
	}

	ret = (char *) malloc(len + 1);
	if (ret == NULL) {
		return (NULL);
	}

	memcpy(ret, str, len);
	ret[len] = '\0';
	return (ret);
}

#endif


#ifdef WIN32
// Only redefine for MSVC versions older than 1900 (Visual Studio 2015)
// VS 2022 has _MSC_VER = 193x, so snprintf/vsnprintf exist there.
#if _MSC_VER < 1900
#define snprintf  _snprintf
#endif

#if _MSC_VER < 1500 
#define vsnprintf _vsnprintf
#endif

#endif

#ifdef WIN32
#include <math.h>
#if _MSC_VER < 1800 // VS2013 (_MSC_VER=1800) and later have round()
static float round(float x)
{
	return floor(x+0.5f);
}
#endif
#endif

#endif // _TORCS_PORTABILITY_H_

