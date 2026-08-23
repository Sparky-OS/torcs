/***************************************************************************

    file                 : grmultitexstate.h
    created              : Fri Mar 22 23:16:44 CET 2002
    copyright            : (C) 2001 by Christophe Guionneau
    version              : $Id$

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __GRMULTI
#define __GRMULTI

#include "grtexture.h"

class grMultiTexState : public grManagedState
{
	public:
		~grMultiTexState() {}
		virtual void apply (int unit) ;
};

#endif // __GRMULTI
