/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 2000-2001 The LyX Team.
 * 
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetcite.h"
#include "buffer.h"
#include "BufferView.h"
#include "LaTeXFeatures.h"
#include "LyXView.h"
#include "frontends/Dialogs.h"
#include "support/lstrings.h"

InsetCitation::InsetCitation(InsetCommandParams const & p, bool)
	: InsetCommand(p)
{}

string const InsetCitation::getScreenLabel() const
{
	string keys(getContents());

	// If keys is "too long" then only print out the first few tokens
	string label;
	if (contains(keys, ",")) {
		// Final comma allows while loop to cover all keys
		keys = frontStrip(split(keys, label, ',')) + ",";

		string::size_type const maxSize = 40;
		while (contains( keys, "," )) {
			string key;
			keys = frontStrip(split(keys, key, ','));

			string::size_type size = label.size() + 2 + key.size();
			if (size >= maxSize) {
				label += ", ...";
				break;
			}
			label += ", " + key;
		}
	} else {
		label = keys;
	}

	if (!getOptions().empty())
		label += ", " + getOptions();

	return "[" + label + "]";
}


void InsetCitation::edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getDialogs()->showCitation(this);
}

void InsetCitation::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, 0);
}

int InsetCitation::ascii(Buffer const *, std::ostream & os, int) const
{
        os << "[" << getContents() << "]";
        return 0;
}

// Have to overwrite the default InsetCommand method in order to check that
// the \cite command is valid. Eg, the user has natbib enabled, inputs some
// citations and then changes his mind, turning natbib support off. The output
// should revert to \cite[]{}
int InsetCitation::latex(Buffer const * buffer, std::ostream & os,
			bool /*fragile*/, bool/*fs*/) const
{
	os << "\\";
	if (buffer->params.use_natbib)
		os << getCmdName();
	else
		os << "cite";

	if (!getOptions().empty())
		os << "[" << getOptions() << "]";

	os << "{" << getContents() << "}";

	return 0;
}


void InsetCitation::validate(LaTeXFeatures & features) const
{
	if (getCmdName() != "cite" && features.bufferParams().use_natbib)
		features.natbib = true;
}
