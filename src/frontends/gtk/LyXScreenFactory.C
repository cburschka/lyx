/**
 * \file gtk/LyXScreenFactory.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include <gtkmm.h>
#include "frontends/LyXScreenFactory.h"
#include "GWorkArea.h"
#include "GScreen.h"




namespace LyXScreenFactory {


LyXScreen * create(WorkArea & owner)
{
	using lyx::frontend::GScreen;
	using lyx::frontend::GWorkArea;

	return new GScreen(static_cast<GWorkArea &>(owner));
}


}
