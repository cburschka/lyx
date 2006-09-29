/**
 * \file qt4/GuiApplication.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
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

#include "GuiApplication.h"

#include "GView.h"
#include "GuiWorkArea.h"
#include "GtkmmX.h"

#include "BufferView.h"

// FIXME: this is needed for now because LyXFunc is still constructed
// there.
#include "frontends/Application_pimpl.h"

#include "graphics/LoaderQueue.h"

#include "support/lstrings.h"
#include "support/os.h"
#include "support/package.h"

#include "lyx_main.h"
#include "lyxrc.h"
#include "debug.h"

#include <gtkmm.h>

#include "LyXGdkImage.h"


using lyx::support::subst;

using std::string;
using std::endl;


namespace {

/// estimate DPI from X server
int getDPI()
{
	//TODO use GDK instead
	Screen * scr = ScreenOfDisplay(getDisplay(), getScreen());
	return int(((HeightOfScreen(scr) * 25.4 / HeightMMOfScreen(scr)) +
		(WidthOfScreen(scr) * 25.4 / WidthMMOfScreen(scr))) / 2);
}

} // namespace anon


namespace lyx {
namespace frontend {

GuiApplication::GuiApplication(int & argc, char ** argv)
	: Gtk::Main(argc, argv), Application(argc, argv)
{
	using namespace lyx::graphics;
	Image::newImage = boost::bind(&LyXGdkImage::newImage);
	Image::loadableFormats = boost::bind(&LyXGdkImage::loadableFormats);

	// needs to be done before reading lyxrc
	lyxrc.dpi = getDPI();

	LoaderQueue::setPriority(10,100);
}


Clipboard& GuiApplication::clipboard()
{
	return clipboard_;
}


Selection& GuiApplication::selection()
{
	return selection_;
}


int const GuiApplication::exec()
{
	run();
	return EXIT_SUCCESS;
}


void GuiApplication::exit(int /*status*/)
{
	// FIXME: Don't ignore status
	guiApp->quit();
}


} // namespace frontend
} // namespace lyx
