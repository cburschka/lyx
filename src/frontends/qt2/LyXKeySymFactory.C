/**
 * \file LyXKeySymFactory.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Asger & Juergen
 */

#include <config.h>
 
#include "frontends/LyXKeySymFactory.h"

#include "QLyXKeySym.h"

namespace LyXKeySymFactory {

LyXKeySym * create()
{
	return new QLyXKeySym();
}
 
}
