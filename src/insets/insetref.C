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
#include "LyXView.h"
#include "frontends/Dialogs.h"
#include "lyxfunc.h"

using std::ostream;

InsetRef::InsetRef(InsetCommandParams const & p, Buffer const & buf)
	: InsetCommand(p), isLatex(buf.isLatex())
{}

void InsetRef::Edit(BufferView * bv, int, int, unsigned int button)
{
	// Eventually trigger dialog with button 3 not 1
	if (button == 3 )
	  	bv->owner()->getLyXFunc()->
			Dispatch(LFUN_REF_GOTO, getContents());
	else if (button == 1 )
		bv->owner()->getDialogs()->showRef( this );
}


string const InsetRef::getScreenLabel() const
{
	string temp;
	for (int i = 0; !types[i].latex_name.empty(); ++ i)
		if (getCmdName() == types[i].latex_name) {
			temp = _(types[i].short_gui_name);
			break;
		}
	temp += getContents();

	if (!isLatex
	   && !getOptions().empty()) {
		temp += "||";
		temp += getOptions();
	}
	return temp;
}


int InsetRef::Latex(Buffer const *, ostream & os,
		    bool /*fragile*/, bool /*fs*/) const
{
	if (getOptions().empty())
		os << escape(getCommand());
	else {
		InsetCommandParams p( getCmdName(), getContents(), "" );
		os << escape(p.getCommand());
	}
	return 0;
}


int InsetRef::Ascii(Buffer const *, ostream & os, int) const
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
	else if (getCmdName() == "prettyref")
		features.prettyref = true;
}

InsetRef::type_info InsetRef::types[] = {
	{ "ref",	N_("Standard"),			N_("Ref: ")},
	{ "pageref",	N_("Page Number"),		N_("Page: ")},
	{ "vpageref",	N_("Textual Page Number"),	N_("TextPage: ")},
	{ "vref",	N_("Standard+Textual Page"),	N_("Ref+Text: ")},
	{ "prettyref",	N_("PrettyRef"),		N_("PrettyRef: ")},
	{ "", "", "" }
};


int InsetRef::getType(string const & name)
{
	for (int i = 0; !types[i].latex_name.empty(); ++i)
		if (name == types[i].latex_name)
			return i;
	return 0;
}


string const & InsetRef::getName(int type)
{
	return types[type].latex_name;
}
