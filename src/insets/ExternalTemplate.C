/**
 * \file ExternalTemplate.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ExternalTemplate.h"

#include "debug.h"
#include "lyxlex.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/path.h"
#include "support/path_defines.h"

#include <boost/assert.hpp>

#include <algorithm>

namespace support = lyx::support;

using std::endl;
using std::for_each;

using std::ostream;


// We have to have dummy default commands for security reasons!

ExternalTemplate::ExternalTemplate()
	: inputFormat("*")
{}


ExternalTemplate::FormatTemplate::FormatTemplate()
{}


ExternalTemplateManager::ExternalTemplateManager()
{
	// gimp gnuchess gnuplot ical netscape tetris xpaint
	readTemplates(support::user_lyxdir());
	if (lyxerr.debugging(Debug::EXTERNAL)) {
		dumpPreambleDefs(lyxerr);
		lyxerr << '\n';
		dumpTemplates(lyxerr);
	}
}


class dumpPreambleDef {
public:
	typedef ExternalTemplateManager::PreambleDefs::value_type value_type;

	dumpPreambleDef(ostream & o) : ost(o) {}

	void operator()(value_type const & vt) {
		ost << "PreambleDef " << vt.first << '\n'
		    << vt.second
		    << "PreambleDefEnd" << endl;
	}

private:
	ostream & ost;
};


class dumpTemplate {
public:
	typedef ExternalTemplateManager::Templates::value_type value_type;

	dumpTemplate(ostream & o) : ost(o) {}

	void operator()(value_type const & vt) {
		ExternalTemplate const & et = vt.second;

		ost << "Template " << et.lyxName << '\n'
		    << "\tGuiName " << et.guiName << '\n'
		    << "\tHelpText\n"
		    << et.helpText
		    << "\tHelpTextEnd\n"
		    << "\tInputFormat " << et.inputFormat << '\n'
		    << "\tFileFilter " << et.fileRegExp << '\n'
		    << "\tEditCommand " << et.editCommand << '\n'
		    << "\tAutomaticProduction " << et.automaticProduction << '\n';
		et.dumpFormats(ost);
		ost << "TemplateEnd" << endl;

	}

private:
	ostream & ost;
};

class dumpFormat {
public:
	typedef ExternalTemplate::Formats::value_type value_type;

	dumpFormat(ostream & o) : ost(o) {}

	void operator()(value_type const & vt) const{
		ExternalTemplate::FormatTemplate const & ft = vt.second;
		ost << "\tFormat " << vt.first << '\n'
		    << "\t\tProduct " << ft.product << '\n'
		    << "\t\tUpdateFormat " << ft.updateFormat << '\n'
		    << "\t\tUpdateResult " << ft.updateResult << '\n'
		    << "\t\tRequirement " << ft.requirement << '\n'
		    << "\t\tPreamble " << ft.preambleName << '\n'
		    << "\tFormatEnd\n";
	}
private:
	ostream & ost;
};


void ExternalTemplate::dumpFormats(ostream & os) const
{
	for_each(formats.begin(), formats.end(), dumpFormat(os));
}


void ExternalTemplateManager::dumpPreambleDefs(ostream & os) const
{
	for_each(preambledefs.begin(), preambledefs.end(), dumpPreambleDef(os));
}


void ExternalTemplateManager::dumpTemplates(ostream & os) const
{
	for_each(templates.begin(), templates.end(), dumpTemplate(os));
}


ExternalTemplateManager & ExternalTemplateManager::get()
{
	static ExternalTemplateManager externalTemplateManager;
	return externalTemplateManager;
}


ExternalTemplateManager::Templates &
ExternalTemplateManager::getTemplates()
{
	return templates;
}


ExternalTemplateManager::Templates const &
ExternalTemplateManager::getTemplates() const
{
	return templates;
}


ExternalTemplate const & ExternalTemplateManager::getTemplateByName(string const & name)
{
	return templates[name];
}


string const
ExternalTemplateManager::getPreambleDefByName(string const & name) const
{
	string const trimmed_name = support::trim(name);
	if (trimmed_name.empty())
		return string();

	PreambleDefs::const_iterator it = preambledefs.find(trimmed_name);
	if (it == preambledefs.end())
		return string();

	return it->second;
}


void ExternalTemplateManager::readTemplates(string const & path)
{
	support::Path p(path);

	enum TemplateTags {
		TM_PREAMBLEDEF = 1,
		TM_PREAMBLEDEF_END,
		TM_TEMPLATE,
		TM_TEMPLATE_END
	};

	keyword_item templatetags[] = {
		{ "preambledef", TM_PREAMBLEDEF },
		{ "preambledefend", TM_PREAMBLEDEF_END },
		{ "template", TM_TEMPLATE },
		{ "templateend", TM_TEMPLATE_END }
	};

	LyXLex lex(templatetags, TM_TEMPLATE_END);

	string filename = support::LibFileSearch("", "external_templates");
	if (filename.empty() || !lex.setFile(filename)) {
		lex.printError("ExternalTemplateManager::readTemplates: "
			       "No template file");
		return;
	}

	char const * const preamble_end_tag =
		templatetags[TM_PREAMBLEDEF_END-1].tag;

	while (lex.isOK()) {
		switch (lex.lex()) {
		case TM_PREAMBLEDEF: {
			lex.next();
			string const name = lex.getString();
			preambledefs[name] = lex.getLongString(preamble_end_tag);
		}
		break;

		case TM_TEMPLATE: {
			lex.next();
			string const name = lex.getString();
			ExternalTemplate & tmp = templates[name];
			tmp.lyxName = name;
			tmp.readTemplate(lex);
		}
		break;

		case TM_TEMPLATE_END:
			lex.printError("Warning: End outside Template.");
		break;

		case TM_PREAMBLEDEF_END:
			lex.printError("Warning: End outside PreambleDef.");
		break;
		}
	}
}


void ExternalTemplate::readTemplate(LyXLex & lex)
{
	enum TemplateOptionTags {
		TO_GUINAME = 1,
		TO_HELPTEXT,
		TO_INPUTFORMAT,
		TO_FILTER,
		TO_EDITCMD,
		TO_AUTOMATIC,
		TO_FORMAT,
		TO_END
	};

	keyword_item templateoptiontags[] = {
		{ "automaticproduction", TO_AUTOMATIC },
		{ "editcommand", TO_EDITCMD },
		{ "filefilter", TO_FILTER },
		{ "format", TO_FORMAT },
		{ "guiname", TO_GUINAME },
		{ "helptext", TO_HELPTEXT },
		{ "inputformat", TO_INPUTFORMAT },
		{ "templateend", TO_END }
	};

	pushpophelper pph(lex, templateoptiontags, TO_END);

	while (lex.isOK()) {
		switch (lex.lex()) {
		case TO_GUINAME:
			lex.next(true);
			guiName = lex.getString();
			break;

		case TO_HELPTEXT:
			helpText = lex.getLongString("HelpTextEnd");
			break;

		case TO_INPUTFORMAT:
			lex.next(true);
			inputFormat = lex.getString();
			break;

		case TO_FILTER:
			lex.next(true);
			fileRegExp = lex.getString();
			break;

		case TO_EDITCMD:
			lex.next(true);
			editCommand = lex.getString();
			break;

		case TO_AUTOMATIC:
			lex.next();
			automaticProduction = lex.getBool();
			break;

		case TO_FORMAT:
			lex.next(true);
			formats[lex.getString()].readFormat(lex);
			break;

		case TO_END:
			return;

		default:
			lex.printError("ExternalTemplate::readTemplate: "
				       "Wrong tag: $$Token");
			BOOST_ASSERT(false);
			break;
		}
	}
}


void ExternalTemplate::FormatTemplate::readFormat(LyXLex & lex)
{
	enum FormatTags {
		FO_PRODUCT = 1,
		FO_UPDATEFORMAT,
		FO_UPDATERESULT,
		FO_REQUIREMENT,
		FO_PREAMBLE,
		FO_END
	};

	keyword_item formattags[] = {
		{ "formatend", FO_END },
		{ "preamble", FO_PREAMBLE },
		{ "product", FO_PRODUCT },
		{ "requirement", FO_REQUIREMENT },
		{ "updateformat", FO_UPDATEFORMAT },
		{ "updateresult", FO_UPDATERESULT }
	};

	pushpophelper pph(lex, formattags, FO_END);

	while (lex.isOK()) {
		switch (lex.lex()) {
		case FO_PRODUCT:
			lex.next(true);
			product = lex.getString();
			break;

		case FO_UPDATEFORMAT:
			lex.next(true);
			updateFormat = lex.getString();
			break;

		case FO_UPDATERESULT:
			lex.next(true);
			updateResult = lex.getString();
			break;

		case FO_REQUIREMENT:
			lex.next(true);
			requirement = lex.getString();
			break;

		case FO_PREAMBLE:
			lex.next(true);
			preambleName = lex.getString();
			break;

		case FO_END:
			return;
		}
	}
}
