/** A small helper function
    \author Jean-Marc Lasgouttes (2003)
 */

#include <iostream>

#include "context.h"

using std::ostream;
using std::endl;

Context::Context(bool need_layout_,
		 LyXTextClass const & textclass_,
		 LyXLayout_ptr layout_, LyXLayout_ptr parent_layout_)
	: need_layout(need_layout_),
	  need_end_layout(false), need_end_deeper(false),
	  textclass(textclass_),
	  layout(layout_), parent_layout(parent_layout_)
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
		
		os << "\n\\begin_layout " << layout->name() << "\n\n";
		need_layout=false;
		need_end_layout = true;
		if (!extra_stuff.empty()) {
			os << extra_stuff;
			extra_stuff.erase();
		}
	}
}


void Context::check_end_layout(ostream & os) 
{
	if (need_end_layout) {
		os << "\n\\end_layout\n";
		need_end_layout = false;
	}
}


void Context::check_deeper(ostream & os)
{
	if (parent_layout->isEnvironment()) {
		if (need_end_deeper) {
				// no need to have \end_deeper \begin_deeper
// FIXME: This does not work because \par already calls check_end_layout
			need_end_deeper = false;
		} else {
			os << "\n\\begin_deeper \n";
			need_end_deeper = true;
		}
	} else
		check_end_deeper(os);
}


void Context::check_end_deeper(ostream & os) 
{
	if (need_end_deeper) {
		os << "\n\\end_deeper \n";
		need_end_deeper = false;
	}
}


void Context::dump(ostream & os, string const & desc) const
{
	os << "\n" << desc <<" [";
	if (need_layout)
		os << "need_layout ";
	if (need_end_layout)
		os << "need_end_layout ";
	if (!extra_stuff.empty())
		os << "extrastuff=[" << extra_stuff << "] ";
	os << "layout=" << layout->name();
	os << " parent_layout=" << parent_layout->name() << "]" << endl;
}
