/**
 * \file LyXKeySymFactory.C
 * Read the file COPYING
 *
 * \author Asger & Juergen
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>
 
#include "frontends/LyXKeySymFactory.h"

#include "XLyXKeySym.h"

namespace LyXKeySymFactory {

LyXKeySym * create()
{
	return new XLyXKeySym();
} 
 
}
