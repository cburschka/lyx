/**
 * \file frontend/Application.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "frontends/Application.h"

#include "frontends/NoGuiFontLoader.h"
#include "frontends/NoGuiFontMetrics.h"
#include "frontends/FontLoader.h"
#include "frontends/FontMetrics.h"
#include "frontends/Gui.h"
#include "frontends/LyXView.h"

#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LyX.h"  // for lyx::use_gui
#include "Font.h"
#include "LyXFunc.h"
#include "LyXRC.h"

#include "support/lstrings.h"
#include "support/os.h"


namespace lyx {


frontend::FontLoader & theFontLoader()
{
	static frontend::NoGuiFontLoader no_gui_font_loader;

	if (!use_gui)
		return no_gui_font_loader;

	BOOST_ASSERT(theApp());
	return theApp()->fontLoader();
}


frontend::FontMetrics const & theFontMetrics(Font const & f)
{
	return theFontMetrics(f.fontInfo());
}


frontend::FontMetrics const & theFontMetrics(FontInfo const & f)
{
	static frontend::NoGuiFontMetrics no_gui_font_metrics;

	if (!use_gui)
		return no_gui_font_metrics;

	BOOST_ASSERT(theApp());
	return theApp()->fontLoader().metrics(f);
}


frontend::Clipboard & theClipboard()
{
	BOOST_ASSERT(theApp());
	return theApp()->clipboard();
}


frontend::Selection & theSelection()
{
	BOOST_ASSERT(theApp());
	return theApp()->selection();
}


} // namespace lyx
