/**
 * \file gtk/lyx_gui.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjnes
 * \author John Levon
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "lyx_gui.h"

#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"

#include "LColor.h"
#include "LyXAction.h"
#include "lyx_main.h"
#include "lyxrc.h"
#include "lyxfont.h"
#include "graphics/LoaderQueue.h"

#include "io_callback.h"

// FIXME: move this stuff out again
#include "bufferlist.h"
#include "buffer_funcs.h"
#include "lyxfunc.h"
#include "lyxserver.h"
#include "lyxsocket.h"
#include "BufferView.h"

#include "GView.h"
#include "GtkmmX.h"

#include "xftFontLoader.h"
#include "GWorkArea.h"

//just for xforms
#include "lyx_forms.h"
#include "xformsImage.h"
#include "xforms_helpers.h"

#include "support/lyxlib.h"
#include "support/os.h"
#include "support/filetools.h"
#include "support/path_defines.h"

#include <gtkmm.h>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <fcntl.h>

#include <sstream>
#include <iomanip>

namespace os = lyx::support::os;

using std::ostringstream;
using std::string;

using lyx::frontend::colorCache;
using lyx::frontend::GView;
using lyx::frontend::XformsColor;


extern BufferList bufferlist;

// FIXME: wrong place !
LyXServer * lyxserver;
LyXServerSocket * lyxsocket;

bool lyx_gui::use_gui = true;

namespace {

/// quit lyx
bool finished = false;


/// estimate DPI from X server
int getDPI()
{
	Screen * scr = ScreenOfDisplay(getDisplay(), getScreen());
	return int(((HeightOfScreen(scr) * 25.4 / HeightMMOfScreen(scr)) +
		(WidthOfScreen(scr) * 25.4 / WidthMMOfScreen(scr))) / 2);
}


/// set default GUI configuration
void setDefaults()
{
	FL_IOPT cntl;
	cntl.buttonFontSize = FL_NORMAL_SIZE;
	cntl.browserFontSize = FL_NORMAL_SIZE;
	cntl.labelFontSize = FL_NORMAL_SIZE;
	cntl.choiceFontSize = FL_NORMAL_SIZE;
	cntl.inputFontSize = FL_NORMAL_SIZE;
	cntl.menuFontSize  = FL_NORMAL_SIZE;
	cntl.borderWidth = -1;
	cntl.vclass = FL_DefaultVisual;
	fl_set_defaults(FL_PDVisual
			| FL_PDButtonFontSize
			| FL_PDBrowserFontSize
			| FL_PDLabelFontSize
			| FL_PDChoiceFontSize
			| FL_PDInputFontSize
			| FL_PDMenuFontSize
			| FL_PDBorderWidth, &cntl);
}


extern "C" {


int LyX_XErrHandler(Display * display, XErrorEvent * xeev) {
	// We don't abort on BadWindow
	if (xeev->error_code == BadWindow) {
		lyxerr << "BadWindow received !" << std::endl;
		lyxerr << "If you're using xforms 1.0 or greater, "
		       << " please report this to lyx-devel@lists.lyx.org"
		       << std::endl;
		return 0;
	}

	// emergency cleanup
	LyX::cref().emergencyCleanup();

	// Get the reason for the crash.
	char etxt[513];
	XGetErrorText(display, xeev->error_code, etxt, 512);
	lyxerr << etxt << " id: " << xeev->resourceid << std::endl;
	// By doing an abort we get a nice backtrace. (hopefully)
	lyx::support::abort();
	return 0;
}

}

/// read in geometry specification
char geometry[40];

} // namespace anon


void parse_init_xforms(int & argc, char * argv[])
{
	setDefaults();

	FL_CMD_OPT cmdopt[] = {
		{"-geometry", "*.geometry", XrmoptionSepArg, "690x510"}
	};

	FL_resource res[] = {
		{"geometry", "geometryClass", FL_STRING, geometry, "", 40}
	};

	const int num_res = sizeof(res)/sizeof(FL_resource);
	fl_initialize(&argc, argv, "LyX", cmdopt, num_res);

	// It appears that, in xforms >=0.89.5, fl_initialize()
	// calls setlocale() and ruins our LC_NUMERIC setting.

	fl_get_app_resources(res, num_res);

	Display * display = fl_get_display();

	if (!display) {
		lyxerr << "LyX: unable to access X display, exiting"
		       << std::endl;
		lyx::support::os::warn("Unable to access X display, exiting");
		::exit(1);
	}

	fcntl(ConnectionNumber(display), F_SETFD, FD_CLOEXEC);

	XSetErrorHandler(LyX_XErrHandler);

	using namespace lyx::graphics;

	// connect the image loader based on the xforms library
	Image::newImage = boost::bind(&xformsImage::newImage);
	Image::loadableFormats = boost::bind(&xformsImage::loadableFormats);
}


void lyx_gui::parse_init(int & argc, char * argv[])
{
	new Gtk::Main(argc, argv);

	parse_init_xforms(argc, argv);

	locale_init();

	// must do this /before/ lyxrc gets read
	lyxrc.dpi = getDPI();
}


void parse_lyxrc_xforms()
{
	XformsColor::read(lyx::support::AddName(
				  lyx::support::user_lyxdir(), "preferences.xform"));

	if (lyxrc.popup_font_encoding.empty())
		lyxrc.popup_font_encoding = lyxrc.font_norm;
	// Set the font name for popups and menus
	string boldfontname = lyxrc.popup_bold_font
			       + "-*-*-*-?-*-*-*-*-"
			       + lyxrc.popup_font_encoding;
		// "?" means "scale that font"
	string fontname = lyxrc.popup_normal_font
			       + "-*-*-*-?-*-*-*-*-"
			       + lyxrc.popup_font_encoding;

	int bold = fl_set_font_name(FL_BOLD_STYLE, boldfontname.c_str());
	int normal = fl_set_font_name(FL_NORMAL_STYLE, fontname.c_str());
	if (bold < 0)
		lyxerr << "Could not set menu font to "
		       << boldfontname << std::endl;

	if (normal < 0)
		lyxerr << "Could not set popup font to "
		       << fontname << std::endl;

	if (bold < 0 && normal < 0) {
		lyxerr << "Using 'helvetica' font for menus" << std::endl;
		boldfontname = "-*-helvetica-bold-r-*-*-*-?-*-*-*-*-iso8859-1";
		fontname = "-*-helvetica-medium-r-*-*-*-?-*-*-*-*-iso8859-1";
		bold = fl_set_font_name(FL_BOLD_STYLE, boldfontname.c_str());
		normal = fl_set_font_name(FL_NORMAL_STYLE, fontname.c_str());

		if (bold < 0 && normal < 0) {
			lyxerr << "Could not find helvetica font. Using 'fixed'."
			       << std::endl;
			fl_set_font_name(FL_NORMAL_STYLE, "fixed");
			normal = bold = 0;
		}
	}
	if (bold < 0)
		fl_set_font_name(FL_BOLD_STYLE, fontname.c_str());
	else if (normal < 0)
		fl_set_font_name(FL_NORMAL_STYLE, boldfontname.c_str());

	fl_setpup_fontstyle(FL_NORMAL_STYLE);
	fl_setpup_fontsize(FL_NORMAL_SIZE);
	fl_setpup_color(FL_MCOL, FL_BLACK);
	fl_set_goodies_font(FL_NORMAL_STYLE, FL_NORMAL_SIZE);
	fl_set_tooltip_font(FL_NORMAL_STYLE, FL_NORMAL_SIZE);
}


void lyx_gui::parse_lyxrc()
{
	parse_lyxrc_xforms();
}


void start_xforms()
{
	// initial geometry
	int xpos = -1;
	int ypos = -1;
	unsigned int width = 690;
	unsigned int height = 510;

	int const geometryBitmask =
		XParseGeometry(geometry,
			       &xpos, &ypos, &width, &height);

	// if width is not set by geometry, check it against monitor width
	if (!(geometryBitmask & WidthValue)) {
		Screen * scr = ScreenOfDisplay(fl_get_display(), fl_screen);
		if (WidthOfScreen(scr) - 8 < int(width))
			width = WidthOfScreen(scr) - 8;
	}

	// if height is not set by geometry, check it against monitor height
	if (!(geometryBitmask & HeightValue)) {
		Screen * scr = ScreenOfDisplay(fl_get_display(), fl_screen);
		if (HeightOfScreen(scr) - 24 < int(height))
			height = HeightOfScreen(scr) - 24;
	}

	Screen * s = ScreenOfDisplay(fl_get_display(), fl_screen);

	// recalculate xpos if it's not set
	if (xpos == -1)
		xpos = (WidthOfScreen(s) - width) / 2;

	// recalculate ypos if it's not set
	if (ypos == -1)
		ypos = (HeightOfScreen(s) - height) / 2;

	lyxerr[Debug::GUI] << "Creating view: " << width << 'x' << height
			   << '+' << xpos << '+' << ypos << std::endl;

//	XFormsView view(width, height);
//	view.show(xpos, ypos, "LyX");
//	view.init();
}


static void events_xforms()
{
	if (fl_check_forms() == FL_EVENT) {
		XEvent ev;
		fl_XNextEvent(&ev);
		lyxerr[Debug::GUI]
			<< "Received unhandled X11 event" << std::endl
			<< "Type: " << ev.xany.type
			<< " Target: 0x" << std::hex << ev.xany.window
			<< std::dec << std::endl;
	}
}


void lyx_gui::start(string const & batch, std::vector<string> const & files)
{
	start_xforms();
	// just for debug
	XSynchronize(getDisplay(), true);

	boost::shared_ptr<GView> view_ptr(new GView);
	LyX::ref().addLyXView(view_ptr);

	GView & view = *view_ptr.get();
	view.show();
	view.init();

	// FIXME: server code below needs moving

	lyxserver = new LyXServer(&view.getLyXFunc(), lyxrc.lyxpipes);
	lyxsocket = new LyXServerSocket(&view.getLyXFunc(),
			  os::internal_path(os::getTmpDir() + "/lyxsocket"));

	for_each(files.begin(), files.end(),
		 bind(&BufferView::loadLyXFile, view.view(), _1, true));

	// handle the batch commands the user asked for
	if (!batch.empty()) {
		view.getLyXFunc().dispatch(lyxaction.lookupFunc(batch));
	}

	// enter the event loop
	while (!finished) {
		while (Gtk::Main::events_pending())
			Gtk::Main::iteration(false);
		events_xforms();
	}

	// FIXME: breaks emergencyCleanup
	delete lyxsocket;
	delete lyxserver;
}


void lyx_gui::exit()
{
	finished = true;
}


FuncStatus lyx_gui::getStatus(FuncRequest const & ev)
{
	FuncStatus flag;
	switch (ev.action) {
	// Add this back if the gtk doc prefs dialog includes preamble - jcs
	/*case LFUN_DIALOG_SHOW:
		if (ev.argument == "preamble")
			flag.unknown(true);
		break;*/
	case LFUN_TOOLTIPS_TOGGLE:
		flag.unknown(true);
		break;
	default:
		break;
	}

	return flag;
}


string const lyx_gui::hexname(LColor_color col)
{
	Gdk::Color gdkColor;
	Gdk::Color * gclr = colorCache.getColor(col);
	if (!gclr) {
		gclr = &gdkColor;
		gclr->parse(lcolor.getX11Name(col));
	}

	std::ostringstream os;

	// Note that X stores the RGB values in the range 0 - 65535
	// whilst we require them in the range 0 - 255.
	os << std::setbase(16) << std::setfill('0')
	   << std::setw(2) << (gclr->get_red() / 256)
	   << std::setw(2) << (gclr->get_green() / 256)
	   << std::setw(2) << (gclr->get_blue() / 256);

	return os.str();
}


void lyx_gui::update_color(LColor_color /*col*/)
{
	colorCache.clear();
}


void lyx_gui::update_fonts()
{
	fontLoader.update();
}


bool lyx_gui::font_available(LyXFont const & font)
{
	return fontLoader.available(font);
}


namespace {

std::map<int, boost::shared_ptr<io_callback> > callbacks;

} // NS anon


void lyx_gui::register_socket_callback(int fd,
				       boost::function<void()> func)
{
	callbacks[fd] = boost::shared_ptr<io_callback>(new io_callback(fd, func));
}


void lyx_gui::unregister_socket_callback(int fd)
{
	callbacks.erase(fd);
}


string const lyx_gui::roman_font_name()
{
	return "times";
}


string const lyx_gui::sans_font_name()
{
	return "helvetica";
}


string const lyx_gui::typewriter_font_name()
{
	return "courier";
}


void lyx_gui::sync_events()
{
	// FIXME
}
