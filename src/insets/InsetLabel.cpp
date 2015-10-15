/**
 * \file InsetLabel.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetLabel.h"

#include "InsetRef.h"

#include "buffer_funcs.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "CutAndPaste.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "InsetIterator.h"
#include "Language.h"
#include "LyX.h"
#include "output_xhtml.h"
#include "ParIterator.h"
#include "sgml.h"
#include "Text.h"
#include "TextClass.h"
#include "TocBackend.h"

#include "mathed/InsetMathHull.h"
#include "mathed/InsetMathRef.h"

#include "frontends/alert.h"

#include "support/convert.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h"

using namespace std;
using namespace lyx::support;

namespace lyx {


InsetLabel::InsetLabel(Buffer * buf, InsetCommandParams const & p)
	: InsetCommand(buf, p)
{}


void InsetLabel::initView()
{
	// This seems to be used only for inset creation.
	// Therefore we do not update refs here, since this would
	// erroneously change refs from existing duplicate labels
	// (#8141).
	updateLabel(getParam("name"));
}


void InsetLabel::uniqueLabel(docstring & label) const
{
	docstring const new_label = label;
	int i = 1;
	while (buffer().insetLabel(label)) {
		label = new_label + '-' + convert<docstring>(i);
		++i;
	}
	if (label != new_label) {
		// Warn the user that the label has been changed to something else.
		frontend::Alert::warning(_("Label names must be unique!"),
			bformat(_("The label %1$s already exists,\n"
			"it will be changed to %2$s."), new_label, label));
	}
}


void InsetLabel::updateLabel(docstring const & new_label)
{
	docstring label = new_label;
	uniqueLabel(label);
	setParam("name", label);
}


void InsetLabel::updateLabelAndRefs(docstring const & new_label,
		Cursor * cursor)
{
	docstring const old_label = getParam("name");
	docstring label = new_label;
	uniqueLabel(label);
	if (label == old_label)
		return;

	buffer().undo().beginUndoGroup();
	if (cursor)
		cursor->recordUndo();
	setParam("name", label);
	updateReferences(old_label, label);
	buffer().undo().endUndoGroup();
}


void InsetLabel::updateReferences(docstring const & old_label,
		docstring const & new_label)
{
	Buffer::References const & refs = buffer().references(old_label);
	Buffer::References::const_iterator it = refs.begin();
	Buffer::References::const_iterator end = refs.end();
	for (; it != end; ++it) {
		buffer().undo().recordUndo(CursorData(it->second));
		if (it->first->lyxCode() == MATH_REF_CODE) {
			InsetMathRef * mi = it->first->asInsetMath()->asRefInset();
			mi->changeTarget(new_label);
		} else {
			InsetCommand * ref = it->first->asInsetCommand();
			ref->setParam("reference", new_label);
		}
	}
}


ParamInfo const & InsetLabel::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;
	if (param_info_.empty())
		param_info_.add("name", ParamInfo::LATEX_REQUIRED,
				ParamInfo::HANDLING_ESCAPE);
	return param_info_;
}


docstring InsetLabel::screenLabel() const
{
	return screen_label_;
}


void InsetLabel::updateBuffer(ParIterator const & par, UpdateType utype)
{
	docstring const & label = getParam("name");
	if (buffer().insetLabel(label)) {
		// Problem: We already have an InsetLabel with the same name!
		screen_label_ = _("DUPLICATE: ") + label;
		return;
	}
	buffer().setInsetLabel(label, this);
	screen_label_ = label;

	if (utype == OutputUpdate) {
		// save info on the active counter
		Counters const & cnts = 
			buffer().masterBuffer()->params().documentClass().counters();
		active_counter_ = cnts.currentCounter();
		Language const * lang = par->getParLanguage(buffer().params());
		if (lang && !active_counter_.empty()) {
			counter_value_ = cnts.theCounter(active_counter_, lang->code());
			pretty_counter_ = cnts.prettyCounter(active_counter_, lang->code());
		} else {
			counter_value_ = from_ascii("#");
			pretty_counter_ = from_ascii("#");
		}
	}
}


void InsetLabel::addToToc(DocIterator const & cpit, bool output_active,
						  UpdateType) const
{
	docstring const & label = getParam("name");
	shared_ptr<Toc> toc = buffer().tocBackend().toc("label");
	if (buffer().insetLabel(label) != this) {
		toc->push_back(TocItem(cpit, 0, screen_label_, output_active));
	} else {
		toc->push_back(TocItem(cpit, 0, screen_label_, output_active));
		Buffer::References const & refs = buffer().references(label);
		Buffer::References::const_iterator it = refs.begin();
		Buffer::References::const_iterator end = refs.end();
		for (; it != end; ++it) {
			DocIterator const ref_pit(it->second);
			if (it->first->lyxCode() == MATH_REF_CODE)
				toc->push_back(TocItem(ref_pit, 1,
						it->first->asInsetMath()->asRefInset()->screenLabel(),
						output_active));
			else
				toc->push_back(TocItem(ref_pit, 1,
						static_cast<InsetRef *>(it->first)->getTOCString(),
						output_active));
		}
	}
}


bool InsetLabel::getStatus(Cursor & cur, FuncRequest const & cmd,
			   FuncStatus & status) const
{
	bool enabled;
	switch (cmd.action()) {
	case LFUN_LABEL_INSERT_AS_REFERENCE:
	case LFUN_LABEL_COPY_AS_REFERENCE:
		enabled = true;
		break;
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "changetype") {
			// this is handled by InsetCommand,
			// but not by InsetLabel.
			enabled = false;
			break;
		}
	default:
		return InsetCommand::getStatus(cur, cmd, status);
	}

	status.setEnabled(enabled);
	return true;
}


void InsetLabel::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY: {
		// the only other option here is "changetype", and we
		// do not have different types.
		if (cmd.getArg(0) != "label") {
			cur.undispatched();
			return;
		}
		InsetCommandParams p(LABEL_CODE);
		// FIXME UNICODE
		InsetCommand::string2params(to_utf8(cmd.argument()), p);
		if (p.getCmdName().empty()) {
			cur.noScreenUpdate();
			break;
		}
		if (p["name"] != params()["name"]) {
			// undo is handled in updateLabelAndRefs
			updateLabelAndRefs(p["name"], &cur);
		}
		cur.forceBufferUpdate();
		break;
	}

	case LFUN_LABEL_COPY_AS_REFERENCE: {
		InsetCommandParams p(REF_CODE, "ref");
		p["reference"] = getParam("name");
		cap::clearSelection();
		cap::copyInset(cur, new InsetRef(buffer_, p), getParam("name"));
		break;
	}

	case LFUN_LABEL_INSERT_AS_REFERENCE: {
		InsetCommandParams p(REF_CODE, "ref");
		p["reference"] = getParam("name");
		string const data = InsetCommand::params2string(p);
		lyx::dispatch(FuncRequest(LFUN_INSET_INSERT, data));
		break;
	}

	default:
		InsetCommand::doDispatch(cur, cmd);
		break;
	}
}


int InsetLabel::plaintext(odocstringstream & os,
        OutputParams const &, size_t) const
{
	docstring const str = getParam("name");
	os << '<' << str << '>';
	return 2 + str.size();
}


int InsetLabel::docbook(odocstream & os, OutputParams const & runparams) const
{
	os << "<!-- anchor id=\""
	   << sgml::cleanID(buffer(), runparams, getParam("name"))
	   << "\" -->";
	return 0;
}


docstring InsetLabel::xhtml(XHTMLStream & xs, OutputParams const &) const
{
	// FIXME XHTML
	// Unfortunately, the name attribute has been deprecated, so we have to use
	// id here to get the document to validate as XHTML 1.1. This will cause a 
	// problem with some browsers, though, I'm sure. (Guess which!) So we will
	// have to figure out what to do about this later. 
	docstring const attr = "id=\"" + html::cleanAttr(getParam("name")) + '"';
	xs << html::CompTag("a", to_utf8(attr));
	return docstring();
}


} // namespace lyx
