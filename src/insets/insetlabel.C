/**
 * \file insetlabel.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetlabel.h"

#include "buffer.h"
#include "BufferView.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "InsetList.h"
#include "lyxtext.h"
#include "paragraph.h"
#include "pariterator.h"

#include "frontends/LyXView.h"

#include "support/lstrings.h"
#include "support/lyxalgo.h"
#include "support/std_ostream.h"

using lyx::support::escape;

using std::string;
using std::ostream;
using std::vector;


InsetLabel::InsetLabel(InsetCommandParams const & p)
	: InsetCommand(p, "label")
{}


std::auto_ptr<InsetBase> InsetLabel::clone() const
{
	return std::auto_ptr<InsetBase>(new InsetLabel(params()));
}


void InsetLabel::getLabelList(Buffer const &, std::vector<string> & list) const
{
	list.push_back(getContents());
}


string const InsetLabel::getScreenLabel(Buffer const &) const
{
	return getContents();
}


namespace {

void changeRefsIfUnique(BufferView & bv, string const & from, string const & to)
{
	// Check if the label 'from' appears more than once
	vector<string> labels;
	bv.buffer()->getLabelList(labels);

	if (lyx::count(labels.begin(), labels.end(), from) > 1)
		return;

	InsetBase::Code code = InsetBase::REF_CODE;

	ParIterator it = bv.buffer()->par_iterator_begin();
	ParIterator end = bv.buffer()->par_iterator_end();
	for ( ; it != end; ++it) {
		bool changed_inset = false;
		for (InsetList::iterator it2 = it->insetlist.begin();
		     it2 != it->insetlist.end(); ++it2) {
			if (it2->inset->lyxCode() == code) {
				InsetCommand * inset = static_cast<InsetCommand *>(it2->inset);
				if (inset->getContents() == from) {
					inset->setContents(to);
					//inset->setButtonLabel();
					changed_inset = true;
				}
			}
		}
	}
}

} // namespace anon


void InsetLabel::priv_dispatch(LCursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p;
		InsetCommandMailer::string2params("label", cmd.argument, p);
		if (p.getCmdName().empty()) {
			cur.undispatched();
			break;
		}
		if (p.getContents() != params().getContents())
			changeRefsIfUnique(cur.bv(), params().getContents(),
						       p.getContents());
		setParams(p);
		break;
	}

	default:
		InsetCommand::priv_dispatch(cur, cmd);
		break;
	}
}


int InsetLabel::latex(Buffer const &, ostream & os,
		      OutputParams const &) const
{
	os << escape(getCommand());
	return 0;
}


int InsetLabel::plaintext(Buffer const &, ostream & os,
		      OutputParams const &) const
{
	os << '<' << getContents()  << '>';
	return 0;
}


int InsetLabel::linuxdoc(Buffer const &, ostream & os,
			 OutputParams const &) const
{
	os << "<label id=\"" << getContents() << "\" >";
	return 0;
}


int InsetLabel::docbook(Buffer const &, ostream & os,
			OutputParams const &) const
{
	os << "<!-- anchor id=\"" << getContents() << "\" -->";
	return 0;
}
