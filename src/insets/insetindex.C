#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetindex.h"
#include "BufferView.h"
#include "frontends/LyXView.h"
#include "frontends/Dialogs.h"
#include "LaTeXFeatures.h"
#include "gettext.h"
#include "support/LOstream.h"

using std::ostream;

InsetIndex::InsetIndex(InsetCommandParams const & p, bool)
	: InsetCommand(p)
{}


string const InsetIndex::getScreenLabel(Buffer const *) const
{
	return _("Idx");
}


void InsetIndex::edit(BufferView * bv, int, int, mouse_button::state)
{
	bv->owner()->getDialogs()->showIndex(this);
}


void InsetIndex::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, mouse_button::none);
}


int InsetIndex::docbook(Buffer const *, ostream & os) const
{
	os << "<indexterm><primary>" << getContents()
	   << "</primary></indexterm>";
	return 0;
}


Inset::Code InsetIndex::lyxCode() const
{
	return Inset::INDEX_CODE;
}


InsetPrintIndex::InsetPrintIndex(InsetCommandParams const & p, bool)
	: InsetCommand(p)
{}


string const InsetPrintIndex::getScreenLabel(Buffer const *) const
{
	return _("Index");
}


void InsetPrintIndex::validate(LaTeXFeatures & features) const
{
	features.require("makeidx");
}


Inset::Code InsetPrintIndex::lyxCode() const
{
	return Inset::INDEX_PRINT_CODE;
}
