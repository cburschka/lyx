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

// Too hard to make concept checks work with this file
#ifdef _GLIBCXX_CONCEPT_CHECKS
#undef _GLIBCXX_CONCEPT_CHECKS
#endif
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "GWorkArea.h"

#include "frontends/WorkAreaFactory.h"

#include <gtkmm.h>


namespace WorkAreaFactory {


WorkArea * create(LyXView & owner, int w, int h)
{
	return new lyx::frontend::GWorkArea(owner, w, h);
}


}
