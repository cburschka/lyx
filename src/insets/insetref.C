#include <config.h>

#include <cstdlib>

#ifdef __GNUG__
#pragma implementation
#endif

#include FORMS_H_LOCATION 
#include "insetref.h"
#include "buffer.h"
#include "debug.h"
#include "lyx_gui_misc.h" // CancelCloseBoxCB
#include "LyXView.h"
#include "lyxfunc.h"
#include "commandtags.h"
#include "gettext.h"
#include "LaTeXFeatures.h"

using std::ostream;
using std::endl;

extern BufferView * current_view;


InsetRef::InsetRef(InsetCommandParams const & p, Buffer * bf)
	: InsetCommand(p), master(bf)
{
	GenerateFlag();
}

void InsetRef::GenerateFlag()
{
	if (getCmdName() == "ref")
		flag = REF;
	else if (getCmdName() == "pageref")
		flag = PAGE_REF;
	else if (getCmdName() == "vref")
		flag = VREF;
	else if (getCmdName() == "vpageref")
		flag = VPAGE_REF;
	else if (getCmdName() == "prettyref")
		flag = PRETTY_REF;
	else {
		lyxerr << "ERROR (InsetRef::GenerateFlag): Unknown command name "
		       << getCmdName() << endl;
		flag = REF;
	}
}


void InsetRef::Toggle() {
	static string const cmd_names[REF_LAST+1] 
		= {"ref", "pageref", "vref", "vpageref", "prettyref"};
	
	if (flag == REF_LAST)
		flag = REF_FIRST;
	else
		flag = static_cast<Ref_Flags>(flag + 1);
	setCmdName(cmd_names[flag]);
}


void InsetRef::Edit(BufferView * bv, int, int, unsigned int)
{
        bv->owner()->getLyXFunc()->
		Dispatch(LFUN_REFGOTO, getContents().c_str());
}


string InsetRef::getScreenLabel() const
{
	static char const * labels[REF_LAST+1]
		= { N_("Ref: "), N_("Page: "), N_("TextRef: "), N_("TextPage: "),
		    N_("PrettyRef: ")};
	string temp = _(labels[flag]) + getContents();
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
string InsetRef::escape(string const & lab) const
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
	switch (flag) {
	case VREF:
	case VPAGE_REF:	
		features.varioref = true;
		break;
	case PRETTY_REF:
		features.prettyref = true;
		break;
	case REF:
	case PAGE_REF:
		break;
	}
}


void InsetRef::gotoLabel()
{
    if (master) {
	master->getUser()->gotoLabel(getContents());
    }
}
