#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "gettext.h"
#include "insettoc.h"
#include "BufferView.h"
#include "LyXView.h"
#include "frontends/Dialogs.h"
#include "debug.h"

using std::ostream;


string InsetTOC::getScreenLabel() const 
{
	string cmdname( getCmdName() );
	if( cmdname == "tableofcontents" )
		return _("Table of Contents");
	else if( cmdname == "listofalgorithms" )
		return _("List of Algorithms");
	else if( cmdname == "listoffigures" )
		return _("List of Figures");
	else
		return _("List of Tables");
}


Inset::Code InsetTOC::LyxCode() const
{
	string cmdname( getCmdName() );
	if( cmdname == "tableofcontents" )
		return Inset::TOC_CODE;
	else if( cmdname == "listofalgorithms" )
		return Inset::LOA_CODE;
	else if( cmdname == "listoffigures" )
		return Inset::LOF_CODE; 
	else
		return Inset::LOT_CODE;
}


void InsetTOC::Edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getDialogs()->showTOC( this );
}


int InsetTOC::Linuxdoc(Buffer const *, ostream & os) const
{
	if( getCmdName() == "tableofcontents" )
		os << "<toc>";
	return 0;
}


int InsetTOC::DocBook(Buffer const *, ostream & os) const
{
	if( getCmdName() == "tableofcontents" )
		os << "<toc></toc>";
	return 0;
}
