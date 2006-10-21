/**
 * \file frontend/Application.C
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
#include "lyx_main.h"
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
{
	// FIXME: please confirm: with unicode, I think initEncoding()
	// is not needed anymore!
	
	// this can't be done before because it needs the Languages object
	//initEncodings();
}


void Application::setBufferView(BufferView * buffer_view)
{
	buffer_view_ = buffer_view;
}


LyXView & Application::createView(unsigned int width,
								  unsigned int height,
								  int posx, int posy,
								  bool maximize)
{
	int view_id = gui().newView();
	LyXView & view = gui().view(view_id);

	theLyXFunc().setLyXView(&view);

	/*int workArea_id_ =*/ gui().newWorkArea(width, height, view_id);

	view.init();
	view.setGeometry(width, height, posx, posy, maximize);

	return view;
}


int Application::start(std::string const & /*batch*/)
{
	return exec();
}


} // namespace frontend



frontend::FontLoader & theFontLoader()
{
	static frontend::NoGuiFontLoader no_gui_font_loader;

	if (!use_gui)
		return no_gui_font_loader;

	BOOST_ASSERT(theApp);
	return theApp->fontLoader();
}


frontend::FontMetrics const & theFontMetrics(LyXFont const & f)
{
	static frontend::NoGuiFontMetrics no_gui_font_metrics;

	if (!use_gui)
		return no_gui_font_metrics;

	BOOST_ASSERT(theApp);
	return theApp->fontLoader().metrics(f);
}


frontend::Clipboard & theClipboard()
{
	BOOST_ASSERT(theApp);
	return theApp->clipboard();
}


frontend::Selection & theSelection()
{
	BOOST_ASSERT(theApp);
	return theApp->selection();
}


} // namespace lyx
