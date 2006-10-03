/**
 * \file frontends/lyx_gui.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lyx_gui.h"

#include "LyXView.h"
#include "Application.h"

#include "funcrequest.h"

using std::string;

lyx::frontend::Application * theApp;


namespace lyx_gui {

bool use_gui = true;

void parse_lyxrc()
{}


LyXView * create_view(unsigned int width, unsigned int height, int posx, int posy,
          bool maximize)
{
	return &theApp->createView(width, height, posx, posy, maximize);
}


int start(LyXView * view, string const & batch)
{
	return theApp->start(batch);
}


void exit(int status)
{
	theApp->exit(status);
}


FuncStatus getStatus(FuncRequest const & ev)
{
	FuncStatus flag;
	switch (ev.action) {
	case LFUN_TOOLTIPS_TOGGLE:
		flag.unknown(true);
		break;
	default:
		break;
	}

	return flag;
}


string const roman_font_name()
{
	if (!use_gui)
		return "serif";

	return theApp->romanFontName();
}


string const sans_font_name()
{
	if (!use_gui)
		return "sans";

	return theApp->sansFontName();
}


string const typewriter_font_name()
{
	if (!use_gui)
		return "monospace";

	return theApp->typewriterFontName();
}

}; // namespace lyx_gui
