/**
 * \file LyXKeySymFactory.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger & Juergen
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>
 
#ifdef __GNUG__
#pragma implementation
#endif

#include "frontends/LyXKeySymFactory.h"

#include "QLyXKeySym.h"

namespace LyXKeySymFactory {

LyXKeySym * create()
{
	return new QLyXKeySym();
}
 
}
