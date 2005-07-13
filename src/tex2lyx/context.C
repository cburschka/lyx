/**
 * \file context.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include <iostream>

#include "support/lstrings.h"
#include "context.h"

using std::ostream;
using std::endl;
using std::string;


namespace {

void begin_layout(ostream & os, LyXLayout_ptr layout, Font const & font,
                  Font const & normalfont)
{
	os << "\n\\begin_layout " << layout->name() << "\n";
	// FIXME: This is not enough for things like
	// \\Huge par1 \\par par2
	output_font_change(os, normalfont, font);
}


void end_layout(ostream & os)
{
	os << "\n\\end_layout\n";
}


void begin_deeper(ostream & os)
{
	os << "\n\\begin_deeper";
}


void end_deeper(ostream & os)
{
	os << "\n\\end_deeper";
}

}

void output_font_change(ostream & os, Font const & oldfont,
                        Font const & newfont)
{
	if (oldfont.family != newfont.family)
		os << "\n\\family " << newfont.family << '\n';
	if (oldfont.series != newfont.series)
		os << "\n\\series " << newfont.series << '\n';
	if (oldfont.shape != newfont.shape)
		os << "\n\\shape " << newfont.shape << '\n';
	if (oldfont.size != newfont.size)
		os << "\n\\size " << newfont.size << '\n';
}


Font Context::normalfont;


Context::Context(bool need_layout_,
		 LyXTextClass const & textclass_,
		 LyXLayout_ptr layout_, LyXLayout_ptr parent_layout_,
		 Font font_)
	: need_layout(need_layout_),
	  need_end_layout(false), need_end_deeper(false),
	  has_item(false), deeper_paragraph(false),
	  textclass(textclass_),
	  layout(layout_), parent_layout(parent_layout_),
	  font(font_)
{
	if (!layout.get())
		layout = textclass.defaultLayout();
	if (!parent_layout.get())
		parent_layout = textclass.defaultLayout();
}


void Context::check_layout(ostream & os)
{
	if (need_layout) {
		check_end_layout(os);

		// are we in a list-like environment?
		if (layout->isEnvironment()
		    && layout->latextype != LATEX_ENVIRONMENT) {
			// A list-like environment
			if (has_item) {
				// a new item. If we had a standard
				// paragraph before, we have to end it.
				if (deeper_paragraph) {
					end_deeper(os);
					deeper_paragraph = false;
				}
				begin_layout(os, layout, font, normalfont);
				has_item = false;
				need_layout=false;
				need_end_layout = true;
			} else {
				// a standard paragraph in an
				// enumeration. We have to recognize
				// that this may require a begin_deeper.
				if (!deeper_paragraph)
					begin_deeper(os);
				begin_layout(os, textclass.defaultLayout(),
				             font, normalfont);
				need_layout=false;
				need_end_layout = true;
				deeper_paragraph = true;
			}
		} else {
			// No list-like environment
			begin_layout(os, layout, font, normalfont);
			need_layout=false;
			need_end_layout = true;
		}
		if (!extra_stuff.empty()) {
			os << extra_stuff;
			extra_stuff.erase();
		}
		os << "\n";
	}
}


void Context::check_end_layout(ostream & os)
{
	if (need_end_layout) {
		end_layout(os);
		need_end_layout = false;
	}
}


void Context::check_deeper(ostream & os)
{
	if (parent_layout->isEnvironment()) {
		// We start a nested environment.
		// We need to increase the depth.
		if (need_end_deeper) {
			// no need to have \end_deeper \begin_deeper
			need_end_deeper = false;
		} else {
			begin_deeper(os);
			need_end_deeper = true;
		}
	} else
		check_end_deeper(os);
}


void Context::check_end_deeper(ostream & os)
{
	if (need_end_deeper) {
		end_deeper(os);
		need_end_deeper = false;
	}
	if (deeper_paragraph) {
		end_deeper(os);
		deeper_paragraph = false;
	}
}


void Context::set_item()
{
	need_layout = true;
	has_item = true;
}


void Context::new_paragraph(ostream & os)
{
	check_end_layout(os);
	need_layout = true;
}


void Context::add_extra_stuff(std::string const & stuff)
{
	if (!lyx::support::contains(extra_stuff, stuff))
		extra_stuff += stuff;
}


void Context::dump(ostream & os, string const & desc) const
{
	os << "\n" << desc <<" [";
	if (need_layout)
		os << "need_layout ";
	if (need_end_layout)
		os << "need_end_layout ";
	if (need_end_deeper)
		os << "need_end_deeper ";
	if (has_item)
		os << "has_item ";
	if (deeper_paragraph)
		os << "deeper_paragraph ";
	if (!extra_stuff.empty())
		os << "extrastuff=[" << extra_stuff << "] ";
	os << "layout=" << layout->name();
	os << " parent_layout=" << parent_layout->name() << "]" << endl;
}
