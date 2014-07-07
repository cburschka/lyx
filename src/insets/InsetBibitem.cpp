/**
 * \file InsetBibitem.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include <algorithm>

#include "InsetBibitem.h"

#include "BiblioInfo.h"
#include "Buffer.h"
#include "Cursor.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Counters.h"
#include "DispatchResult.h"
#include "Encoding.h"
#include "FuncRequest.h"
#include "InsetIterator.h"
#include "InsetList.h"
#include "Language.h"
#include "Lexer.h"
#include "output_xhtml.h"
#include "OutputParams.h"
#include "Paragraph.h"
#include "ParagraphList.h"
#include "ParIterator.h"
#include "TextClass.h"

#include "frontends/alert.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/mutex.h"

using namespace std;
using namespace lyx::support;

namespace lyx {


int InsetBibitem::key_counter = 0;
static Mutex counter_mutex;
docstring const key_prefix = from_ascii("key-");


InsetBibitem::InsetBibitem(Buffer * buf, InsetCommandParams const & p)
	: InsetCommand(buf, p)
{
	buffer().invalidateBibinfoCache();
	if (getParam("key").empty()) {
		Mutex::Locker lock(&counter_mutex);
		setParam("key", key_prefix + convert<docstring>(++key_counter));
	}
}


InsetBibitem::~InsetBibitem()
{
	if (isBufferLoaded())
		buffer().invalidateBibinfoCache();
}


void InsetBibitem::initView()
{
	updateCommand(getParam("key"));
}


void InsetBibitem::updateCommand(docstring const & new_key, bool)
{
	docstring key = new_key;

	vector<docstring> bibkeys = buffer().masterBibInfo().getKeys();

	int i = 1;

	if (find(bibkeys.begin(), bibkeys.end(), key) != bibkeys.end()) {
		// generate unique label
		key = new_key + '-' + convert<docstring>(i);
		while (find(bibkeys.begin(), bibkeys.end(), key) != bibkeys.end()) {
			++i;
			key = new_key + '-' + convert<docstring>(i);
		}
		frontend::Alert::warning(_("Keys must be unique!"),
			bformat(_("The key %1$s already exists,\n"
			"it will be changed to %2$s."), new_key, key));
	}
	setParam("key", key);
	buffer().invalidateBibinfoCache();
}


ParamInfo const & InsetBibitem::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;
	if (param_info_.empty()) {
		param_info_.add("label", ParamInfo::LATEX_OPTIONAL,
				ParamInfo::HANDLING_LATEXIFY);
		param_info_.add("key", ParamInfo::LATEX_REQUIRED,
				ParamInfo::HANDLING_ESCAPE);
	}
	return param_info_;
}


void InsetBibitem::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p(BIBITEM_CODE);
		InsetCommand::string2params(to_utf8(cmd.argument()), p);
		if (p.getCmdName().empty()) {
			cur.noScreenUpdate();
			break;
		}

		cur.recordUndo();

		docstring const & old_key = params()["key"];
		docstring const & old_label = params()["label"];
		docstring label = p["label"];

		// definitions for escaping
		int previous;
		static docstring const backslash = from_ascii("\\");
		static docstring const lbrace = from_ascii("{");
		static docstring const rbrace = from_ascii("}");
		static char_type const chars_escape[6] = {
			'&', '_', '$', '%', '#', '^'};
		static char_type const brackets_escape[2] = {'[', ']'};

		if (!label.empty()) {
			// The characters in chars_name[] need to be changed to a command when
			// they are in the name field.
			for (int k = 0; k < 6; k++)
				for (size_t i = 0, pos;
					(pos = label.find(chars_escape[k], i)) != string::npos;
					i = pos + 2) {
						if (pos == 0)
							previous = 0;
						else
							previous = pos - 1;
						// only if not already escaped
						if (label[previous] != '\\')
							label.replace(pos, 1, backslash + chars_escape[k] + lbrace + rbrace);
				}
			// The characters '[' and ']' need to be put into braces
			for (int k = 0; k < 2; k++)
				for (size_t i = 0, pos;
					(pos = label.find(brackets_escape[k], i)) != string::npos;
					i = pos + 2) {
						if (pos == 0)
							previous = 0;
						else
							previous = pos - 1;
						// only if not already escaped
						if (label[previous] != '{')
							label.replace(pos, 1, lbrace + brackets_escape[k] + rbrace);
				}
		}

		if (old_label != label) {
			p["label"] = label;
			cur.forceBufferUpdate();
			buffer().invalidateBibinfoCache();
		}

		setParam("label", p["label"]);
		if (p["key"] != old_key) {
			updateCommand(p["key"]);
			cur.bv().buffer().changeRefsIfUnique(old_key, params()["key"]);
			cur.forceBufferUpdate();
			buffer().invalidateBibinfoCache();
		}
		break;
	}

	default:
		InsetCommand::doDispatch(cur, cmd);
		break;
	}
}


void InsetBibitem::read(Lexer & lex)
{
	InsetCommand::read(lex);

	if (prefixIs(getParam("key"), key_prefix)) {
		int const key = convert<int>(getParam("key").substr(key_prefix.length()));
		Mutex::Locker lock(&counter_mutex);
		key_counter = max(key_counter, key);
	}
}


docstring InsetBibitem::bibLabel() const
{
	BufferParams const & bp = buffer().masterBuffer()->params();
	if (bp.citeEngineType() == ENGINE_TYPE_NUMERICAL)
		return autolabel_;
	docstring const & label = getParam("label");
	return label.empty() ? autolabel_ : label;
}


docstring InsetBibitem::screenLabel() const
{
	return getParam("key") + " [" + bibLabel() + ']';
}


int InsetBibitem::plaintext(odocstringstream & os,
        OutputParams const &, size_t) const
{
	odocstringstream oss;
	oss << '[' << bibLabel() << "] ";

	docstring const str = oss.str();
	os << str;

	return str.size();
}


// ale070405
docstring bibitemWidest(Buffer const & buffer, OutputParams const & runparams)
{
	BufferParams const & bp = buffer.masterBuffer()->params();
	if (bp.citeEngineType() == ENGINE_TYPE_NUMERICAL)
		return from_ascii("99");

	int w = 0;

	InsetBibitem const * bitem = 0;

	// FIXME: this font is used unitialized for now but should  be set to
	// a proportional font. Here is what Georg Baum has to say about it:
	/*
	bibitemWidest() is supposed to find the bibitem with the widest label in the
	output, because that is needed as an argument of the bibliography
	environment to determine the correct indentation. To be 100% correct we
	would need the metrics of the font that is used in the output, but usually
	we don't have access to these.
	In practice, any proportional font is probably good enough, since we don't
	need to know the final with, we only need to know the which label is the
	widest.
	Unless there is an easy way to get the metrics of the output font I suggest
	to use a hardcoded font like "Times" or so.

	It is very important that the result of this function is the same both with
	and without GUI. After thinking about this it is clear that no Font
	metrics should be used here, since these come from the gui. If we can't
	easily get the LaTeX font metrics we should make our own poor mans font
	metrics replacement, e.g. by hardcoding the metrics of the standard TeX
	font.
	*/

	docstring lbl;

	ParagraphList::const_iterator it = buffer.paragraphs().begin();
	ParagraphList::const_iterator end = buffer.paragraphs().end();

	for (; it != end; ++it) {
		if (it->insetList().empty())
			continue;
		Inset * inset = it->insetList().begin()->inset;
		if (inset->lyxCode() != BIBITEM_CODE)
			continue;

		bitem = static_cast<InsetBibitem const *>(inset);
		docstring const label = bitem->bibLabel();

		// FIXME: we can't be sure using the following that the GUI
		// version and the command-line version will give the same
		// result.
		//
		//int const wx = use_gui?
		//	theFontMetrics(font).width(label): label.size();
		//
		// So for now we just use the label size in order to be sure
		// that GUI and no-GUI gives the same bibitem (even if that is
		// potentially the wrong one.
		int const wx = label.size();

		if (wx > w) {
			w = wx;
			lbl = label;
		}
	}

	if (!lbl.empty()) {
		pair<docstring, docstring> latex_lbl =
			runparams.encoding->latexString(lbl, runparams.dryrun);
		return latex_lbl.first;
	}

	return from_ascii("99");
}


void InsetBibitem::collectBibKeys(InsetIterator const & it) const
{
	docstring const key = getParam("key");
	docstring const label = getParam("label");
	BibTeXInfo keyvalmap(false);
	keyvalmap.key(key);
	keyvalmap.label(label);

	BufferParams const & bp = buffer().masterBuffer()->params();
	Counters & counters = bp.documentClass().counters();
	docstring const bibitem = from_ascii("bibitem");
	if (bp.citeEngineType() == ENGINE_TYPE_NUMERICAL || getParam("label").empty()) {
		if (counters.hasCounter(bibitem))
			counters.step(bibitem, InternalUpdate);
		string const & lang = it.paragraph().getParLanguage(bp)->code();
		keyvalmap.setCiteNumber(counters.theCounter(bibitem, lang));
	}

	DocIterator doc_it(it);
	doc_it.forwardPos();
	keyvalmap[from_ascii("ref")] = doc_it.paragraph().asString(
		AS_STR_INSETS | AS_STR_SKIPDELETE);
	buffer().addBibTeXInfo(key, keyvalmap);
}


// Update the counters of this inset and of its contents
void InsetBibitem::updateBuffer(ParIterator const & it, UpdateType utype)
{
	BufferParams const & bp = buffer().masterBuffer()->params();
	Counters & counters = bp.documentClass().counters();
	docstring const bibitem = from_ascii("bibitem");
	if (bp.citeEngineType() == ENGINE_TYPE_NUMERICAL || getParam("label").empty()) {
		if (counters.hasCounter(bibitem))
			counters.step(bibitem, utype);
		string const & lang = it.paragraph().getParLanguage(bp)->code();
		autolabel_ = counters.theCounter(bibitem, lang);
	} else {
		autolabel_ = from_ascii("??");
	}
}


docstring InsetBibitem::xhtml(XHTMLStream & xs, OutputParams const &) const
{
	// FIXME XHTML
	// XHTML 1.1 doesn't have the "name" attribute for <a>, so we have to use
	// the "id" atttribute to get the document to validate. Probably, we will
	// need to use "name" anyway, eventually, because some browsers do not
	// handle jumping to ids. If we don't do that, though, we can just put the
	// id into the span tag.
	string const attrs =
		"id='LyXCite-" + to_utf8(html::cleanAttr(getParam("key"))) + "'";
	xs << html::CompTag("a", attrs);
	xs << html::StartTag("span", "class='bibitemlabel'");
	xs << bibLabel();
	xs << html::EndTag("span");
	return docstring();
}


} // namespace lyx
