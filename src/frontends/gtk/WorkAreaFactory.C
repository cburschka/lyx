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

#include "GWorkArea.h"

#include "frontends/WorkAreaFactory.h"

#include <gtkmm.h>


namespace WorkAreaFactory {


WorkArea * create(LyXView & owner, int w, int h)
{
	return new lyx::frontend::GWorkArea(owner, w, h);
}


}
