/**
 * \file InsetCounter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Richard Kimberly Heck
 *
 * Full author contact details are available in file CREDITS.
 */
#include <config.h>

#include "InsetCounter.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "Counters.h"
#include "LaTeXFeatures.h"
#include "OutputParams.h"
#include "output_xhtml.h"
#include "sgml.h"
#include "texstream.h"
#include "TextClass.h"

#include "support/convert.h"
#include "support/counter_reps.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"

#include <map>

/*
#include "Cursor.h"
#include "DispatchResult.h"
#include "Language.h"
#include "LyX.h"
#include "ParIterator.h"
#include "TocBackend.h"

#include "support/debug.h"
#include "support/textutils.h"
*/

using namespace lyx::support;
using namespace std;

namespace lyx {


InsetCounter::InsetCounter(Buffer * buf, InsetCommandParams const & p)
	: InsetCommand(buf, p)
{}


InsetCounter::InsetCounter(InsetCounter const & ir)
	: InsetCommand(ir)
{}


const map<string, string> InsetCounter::counterTable =
{
	{"set", N_("Set Counter To")},
	{"addto", N_("Increase Counter By")},
	{"reset", N_("Reset Counter To 0")},
	{"save", N_("Save Value of Counter")},
	{"restore", N_("Restore Value of Counter")},
};


bool InsetCounter::isCompatibleCommand(string const & s) {
	return counterTable.count(s);
}


ParamInfo const & InsetCounter::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;
	if (param_info_.empty()) {
		param_info_.add("counter", ParamInfo::LYX_INTERNAL);
		param_info_.add("value", ParamInfo::LYX_INTERNAL);
		param_info_.add("lyxonly", ParamInfo::LYX_INTERNAL);
	}
	return param_info_;
}


void InsetCounter::latex(otexstream & os, OutputParams const &) const
{
	bool const lyxonly = lowercase(getParam("lyxonly")) == "true";
	if (lyxonly)
		return;

	string const cmd = getCmdName();
	docstring cntr = getParam("counter");
	Counters & cnts = buffer().params().documentClass().counters();
	if (cmd == "set") {
		docstring const & val = getParam("value");
		os << "\\setcounter{" << cntr << "}{" << val << "}";
	} else if (cmd == "addto") {
		docstring const & val = getParam("value");
		os << "\\addtocounter{" << cntr << "}{" << val << "}";
	} else if (cmd == "reset") {
		os << "\\setcounter{" << cntr << "}{0}";
	} else if (cmd == "save") {
		cnts.saveValue(cntr);
		os << "\\setcounter{" << lyxSaveCounter() 
		   << "}{\\value{" << cntr << "}}";
	} else if (cmd == "restore") {
		cnts.restoreValue(cntr);
		os << "\\setcounter{" << cntr
		   << "{\\value{" << lyxSaveCounter() << "}}";
	}
}


void InsetCounter::toString(odocstream & os) const
{
	os << "[Counter " << from_utf8(getCmdName()) << ": " 
	   <<  getParam("counter") << "]";
}


int InsetCounter::plaintext(odocstringstream & os,
        OutputParams const &, size_t) const
{
	toString(os);
	return 0;
}


#if 0
// save this code until we get it working in InsetInfo
const map<string, string> InsetCounter::valueTable =
{
	{"Roman", N_("Roman Uppercase")},
	{"roman", N_("Roman Lowercase")},
	{"Alpha", N_("Uppercase Letter")},
	{"alpha", N_("Lowercase Letter")},
	{"arabic", N_("Arabic Numeral")}
};


docstring InsetCounter::value() const {
	docstring const & cnt = getParam("counter");
	string const & vtype = getCmdName();
	int const val = buffer().params().documentClass().counters().value(cnt);
	if (vtype   == "Roman")
		return romanCounter(val);
	if (vtype   == "roman")
		return lowerromanCounter(val);
	if (vtype   == "Alpha")
		return docstring(1, alphaCounter(val));
	if (vtype   == "alpha")
		return docstring(1, loweralphaCounter(val));
	if (vtype   == "arabic")
		return convert<docstring>(val);
	LATTEST(false);
	return empty_docstring();
}
#endif

void InsetCounter::trackCounters(string const & cmd) const
{
	Counters & cnts = buffer().params().documentClass().counters();
	docstring cntr = getParam("counter");
	if (cmd == "set") {
		docstring const & val = getParam("value");
		cnts.set(cntr, convert<int>(val));
	} else if (cmd == "addto") {
		docstring const & val = getParam("value");
		cnts.addto(cntr, convert<int>(val));
	} else if (cmd == "reset") {
		cnts.reset(cntr);
	} else if (cmd == "save") {
		cnts.saveValue(cntr);
	} else if (cmd == "restore") {
		cnts.restoreValue(cntr);
	}
}

int InsetCounter::docbook(odocstream &, OutputParams const &) const
{
	// Here, we need to track counter values ourselves,
	// since unlike in the LaTeX case, there is no external
	// mechanism for doing that.
	trackCounters(getCmdName());
	return 0;
}


docstring InsetCounter::xhtml(XHTMLStream &, OutputParams const &) const
{
	// Here, we need to track counter values ourselves,
	// since unlike in the LaTeX case, there is no external
	// mechanism for doing that.
	trackCounters(getCmdName());
	return docstring();
}


void InsetCounter::updateBuffer(ParIterator const &, UpdateType, bool const)
{
	string const cmd = getCmdName();
	docstring cntr = getParam("counter");
	Counters & cnts = buffer().params().documentClass().counters();
	map<string, string>::const_iterator cit = counterTable.find(cmd);
	LASSERT(cit != counterTable.end(), return);
	string const label = cit->second;
	docstring const tlabel = translateIfPossible(from_ascii(label));

	if (cmd == "set") {
		docstring const & val = getParam("value");
		cnts.set(cntr, convert<int>(val));
		screen_label_ = bformat(_("Counter: Set %1$s"), cntr);
		tooltip_ = bformat(_("Set value of counter %1$s to %2$s"), cntr, val);
	} else if (cmd == "addto") {
		docstring const & val = getParam("value");
		cnts.addto(cntr, convert<int>(val));
		screen_label_ = bformat(_("Counter: Add to %1$s"), cntr);
		tooltip_ = bformat(_("Add %1$s to value of counter %2$s"), val, cntr);
	} else if (cmd == "reset") {
		cnts.reset(cntr);		
		screen_label_ = bformat(_("Counter: Reset %1$s"), cntr);
		tooltip_ = bformat(_("Reset value of counter %1$s"), cntr);
	} else if (cmd == "save") {
		cnts.saveValue(cntr);
		screen_label_ = bformat(_("Counter: Save %1$s"), cntr);
		tooltip_ = bformat(_("Save value of counter %1$s"), cntr);
	} else if (cmd == "restore") {
		cnts.restoreValue(cntr);
		screen_label_ = bformat(_("Counter: Restore %1$s"), cntr);
		tooltip_ = bformat(_("Restore value of counter %1$s"), cntr);
	}
}


docstring InsetCounter::lyxSaveCounter() const
{
	docstring cntr = getParam("counter");
	return from_ascii("LyXSave") + cntr;
}


void InsetCounter::validate(LaTeXFeatures & features) const
{
	// create save counter if needed
	string const cmd = getCmdName();
	docstring const lyxonly = getParam("lyxonly");
	if ((cmd == "save" || cmd == "restore")	&& lyxonly != "true") {
		features.addPreambleSnippet(from_ascii("\\newcounter{") + lyxSaveCounter() + "}");
	}
    InsetCommand::validate(features);
}


string InsetCounter::contextMenuName() const 
{ 
	return "context-counter"; 
}
} // namespace lyx
