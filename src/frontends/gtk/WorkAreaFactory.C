/**
 * \file gtk/WorkAreaFactory.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include <gtkmm.h>

#include "frontends/WorkAreaFactory.h"

#include "GWorkArea.h"


namespace WorkAreaFactory {


WorkArea * create(int /*x*/, int /*y*/, int w, int h)
{
	return new GWorkArea(w, h);
}


}
