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

extern BufferView * current_view;


InsetRef::InsetRef(string const & cmd, Buffer * bf)
	: master(bf)
{
	scanCommand(cmd);
	if (getCmdName() == "ref")
		flag = InsetRef::REF;
	else
		flag = InsetRef::PAGE_REF;
}


InsetRef::InsetRef(InsetCommand const & inscmd, Buffer * bf)
	: master(bf)
{
	setCmdName(inscmd.getCmdName());
	setContents(inscmd.getContents());
	setOptions(inscmd.getOptions());
	if (getCmdName() == "ref")
		flag = InsetRef::REF;
	else
		flag = InsetRef::PAGE_REF;
}


void InsetRef::Edit(int, int)
{
        current_view->owner()->getLyXFunc()
		->Dispatch(LFUN_REFGOTO, getContents().c_str());
}


string InsetRef::getScreenLabel() const
{
	string temp;
	if (flag == InsetRef::PAGE_REF)
		temp += _("Page: ");
	else 
		temp += _("Ref: ");
	temp += getContents();
	if(!current_view->buffer()->isLatex()
	   && !getOptions().empty()) {
		temp += "||";
		temp += getOptions();
	}
	return temp;
}


int InsetRef::Latex(ostream & os, signed char /*fragile*/)
{
	if(getOptions().empty())
		os << escape(getCommand());
	else {
		string ns;
		InsetCommand clone = InsetCommand(getCmdName(),
						  getContents(), ns);
		os << escape(clone.getCommand());
	}
	return 0;
}


int InsetRef::Latex(string & file, signed char /*fragile*/)
{
	if(getOptions().empty())
		file += escape(getCommand());
	else {
		string ns;
		InsetCommand clone = InsetCommand(getCmdName(),
						  getContents(), ns);
		file += escape(clone.getCommand());
	}
	return 0;
}


int InsetRef::Linuxdoc(string & file)
{
	file += "<ref id=\"" + getContents()
		+ "\" name=\""+ getOptions() +"\" >" ;

	return 0;
}


int InsetRef::DocBook(string & file)
{
	file += "<link linkend=\"" + getContents()
		+ "\">"+ getOptions() +"</link>" ;

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
