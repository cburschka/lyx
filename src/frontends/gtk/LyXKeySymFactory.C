/**
 * \file gtk/LyXKeySymFactory.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GLyXKeySym.h"

#include "frontends/LyXKeySymFactory.h"


namespace LyXKeySymFactory {

LyXKeySym * create()
{
	return new GLyXKeySym;
}

}
