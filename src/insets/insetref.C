#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetref.h"
#include "buffer.h"
#include "commandtags.h"
#include "debug.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "lyxfunc.h"
#include "LyXView.h"
#include "frontends/Dialogs.h"

using std::ostream;

extern BufferView * current_view;


InsetRef::InsetRef(InsetCommandParams const & p)
	: InsetCommand(p)
{}

void InsetRef::Edit(BufferView * bv, int, int, unsigned int button)
{
	// Eventually trigger dialog with button 3 not 1
	if( button == 3 )
	  	bv->owner()->getLyXFunc()->
			Dispatch(LFUN_REF_GOTO, getContents().c_str());
	else if( button == 1 )
		bv->owner()->getDialogs()->showRef( this );
}


string const InsetRef::getScreenLabel() const
{
	string temp;
	if (getCmdName() == "ref")
		temp = _( "Ref: " );
	else if (getCmdName() == "pageref")
		temp = _( "Page: " );
	else if (getCmdName() == "vref")
		temp = _( "TextRef: " );
	else if (getCmdName() == "vpageref")
		temp = _( "TextPage: " );
	else
		temp = _( "PrettyRef: " );

	temp += getContents();

	if(!current_view->buffer()->isLatex()
	   && !getOptions().empty()) {
		temp += "||";
		temp += getOptions();
	}
	return temp;
}


int InsetRef::Latex(Buffer const *, ostream & os,
		    bool /*fragile*/, bool /*fs*/) const
{
	if(getOptions().empty())
		os << escape(getCommand());
	else {
		InsetCommandParams p( getCmdName(), getContents(), "" );
		os << escape(p.getCommand());
	}
	return 0;
}


int InsetRef::Ascii(Buffer const *, ostream & os) const
{
	os << "[" << getContents() << "]";
	return 0;
}


int InsetRef::Linuxdoc(Buffer const *, ostream & os) const
{
	os << "<ref id=\"" << getContents()
	   << "\" name=\"" << getOptions() << "\" >";
	return 0;
}


int InsetRef::DocBook(Buffer const *, ostream & os) const
{
	os << "<link linkend=\"" << getContents()
	   << "\">" << getOptions() << "</link>";
	return 0;
}


// This function escapes 8-bit characters and other problematic characters
// It's exactly the same code as in insetlabel.C.
string const InsetRef::escape(string const & lab) const
{
	char hexdigit[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
			      '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	string enc;
	for (string::size_type i = 0; i < lab.length(); ++i) {
		unsigned char c= lab[i];
		if (c >= 128 || c == '=' || c == '%') {
			enc += '=';
			enc += hexdigit[c>>4];
			enc += hexdigit[c & 15];
		} else {
			enc += c;
		}
	}
	return enc;
}


void InsetRef::Validate(LaTeXFeatures & features) const
{
	if (getCmdName() == "vref" || getCmdName() == "vpageref")
		features.varioref = true;
	else if(getCmdName() == "prettyref")
		features.prettyref = true;
}
