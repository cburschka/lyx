#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetindex.h"
#include "BufferView.h"
#include "LyXView.h"
#include "frontends/Dialogs.h"
#include "LaTeXFeatures.h"
#include "gettext.h"

InsetIndex::InsetIndex(InsetCommandParams const & p, bool)
	: InsetCommand(p)
{}


string const InsetIndex::getScreenLabel() const
{
	return _("Idx");
}


void InsetIndex::edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getDialogs()->showIndex( this );
}


InsetPrintIndex::InsetPrintIndex(InsetCommandParams const & p, bool)
	: InsetCommand(p)
{}

string const InsetPrintIndex::getScreenLabel() const
{
	return _("Index");
}


void InsetPrintIndex::validate(LaTeXFeatures & features) const
{
	features.makeidx = true;
}


Inset::Code InsetPrintIndex::lyxCode() const
{
 	return Inset::INDEX_CODE;
}
