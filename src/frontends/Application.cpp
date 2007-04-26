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
#include "frontends/WorkArea.h"

#include "funcrequest.h"
#include "FuncStatus.h"
#include "lyx_main.h"  // for lyx::use_gui
#include "lyxfont.h"
#include "lyxfunc.h"
#include "lyxrc.h"

#include "support/lstrings.h"
#include "support/os.h"

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>


namespace lyx {
namespace frontend {


Application::Application(int &, char **)
	: current_view_(0)
{
}


LyXView & Application::createView(unsigned int width,
				  unsigned int height,
				  int posx, int posy,
				  int maximized,
				  unsigned int iconSizeXY,
				  const std::string & geometryArg)
{
	LyXView & view = gui().createRegisteredView();
	int view_id = view.id();
	
	theLyXFunc().setLyXView(&view);

	/*int workArea_id_ =*/ gui().newWorkArea(width, height, view_id);

	view.init();
	view.setGeometry(width, height, posx, posy, maximized, iconSizeXY, geometryArg);
	view.setFocus();

	setCurrentView(view);

	return view;
}


LyXView const * Application::currentView() const
{
	return current_view_;
}


LyXView * Application::currentView()
{
	return current_view_;
}


void Application::setCurrentView(LyXView & current_view)
{
	current_view_ = &current_view;
}

} // namespace frontend



frontend::FontLoader & theFontLoader()
{
	static frontend::NoGuiFontLoader no_gui_font_loader;

	if (!use_gui)
		return no_gui_font_loader;

	BOOST_ASSERT(theApp());
	return theApp()->fontLoader();
}


frontend::FontMetrics const & theFontMetrics(LyXFont const & f)
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
