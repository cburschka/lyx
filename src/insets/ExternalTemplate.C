/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <algorithm>

#include "ExternalTemplate.h"

#include "lyxlex.h"
#include "debug.h"
#include "support/path.h"
#include "support/LAssert.h"
#include "support/filetools.h"

using std::endl;
using std::ostream;
using std::for_each;

extern string user_lyxdir;


// We have to have dummy default commands for security reasons!

ExternalTemplate::ExternalTemplate()
{}


ExternalTemplate::FormatTemplate::FormatTemplate()
{}


ExternalTemplateManager::ExternalTemplateManager()
{
	// gimp gnuchess gnuplot ical netscape tetris xpaint
	readTemplates(user_lyxdir);
	if (lyxerr.debugging())
		dumpTemplates();
}


class dumpTemplate {
public:
	dumpTemplate(ostream & o)
		: ost(o) {}
	void operator()(ExternalTemplateManager::Templates::value_type const & vt) {
		ExternalTemplate const & et = vt.second;

		ost << "Template " << et.lyxName << "\n"
		    << "\tGuiName " << et.guiName << "\n"
		    << "\tHelpText\n"
		    << et.helpText
		    << "\tHelpTextEnd\n"
		    << "\tFileFilter " << et.fileRegExp << "\n"
		    << "\tViewCommand " << et.viewCommand << "\n"
		    << "\tEditCommand " << et.editCommand << "\n"
		    << "\tAutomaticProduction " << et.automaticProduction << "\n";
		et.dumpFormats(ost);
		ost << "TemplateEnd" << endl;

	}

private:
	ostream & ost;
};

class dumpFormat {
public:
	dumpFormat(ostream & o)
		: ost(o) {}
	void operator()(ExternalTemplate::Formats::value_type const & vt) const{
		ExternalTemplate::FormatTemplate const & ft = vt.second;
		ost << "\tFormat " << vt.first << "\n"
		    << "\t\tProduct " << ft.product << "\n"
		    << "\t\tUpdateCommand " << ft.updateCommand << "\n"
		    << "\t\tUpdateResult " << ft.updateResult << "\n"
		    << "\t\tRequirement " << ft.requirement << "\n"
		    << "\t\tPreamble\n"
		    << ft.preamble
		    << "\t\tPreambleEnd\n"
		    << "\tFormatEnd\n";
	}
private:
	ostream & ost;
};


void ExternalTemplate::dumpFormats(ostream & os) const
{
	for_each(formats.begin(), formats.end(), dumpFormat(os));
}


void ExternalTemplateManager::dumpTemplates() const
{
	for_each(templates.begin(), templates.end(), dumpTemplate(lyxerr));
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


void ExternalTemplateManager::readTemplates(string const & path)
{
	Path p(path);

	enum TemplateTags {
		TM_TEMPLATE = 1,
		TM_END
	};

	keyword_item templatetags[] = {
		{ "template", TM_TEMPLATE },
		{ "templateend", TM_END }
	};

	string filename = LibFileSearch("", "external_templates");
	if (filename.empty()) {
		lyxerr << "ExternalTemplateManager::readTemplates: "
			"No template file" << endl;
		return;
	}

	LyXLex lex(templatetags, TM_END);
	if (!lex.setFile(filename)) {
		lyxerr << "ExternalTemplateManager::readTemplates: "
			"No template file" << endl;
		return;
	}

	while (lex.isOK()) {
		switch (lex.lex()) {
		case TM_TEMPLATE: {
			lex.next();
			string const temp = lex.getString();
			ExternalTemplate & tmp = templates[temp];
			tmp.lyxName = temp;
			tmp.readTemplate(lex);
		}
		break;

		case TM_END:
			lex.printError("Warning: End outside Template.");
		break;
		}
	}
}


void ExternalTemplate::readTemplate(LyXLex & lex)
{
	enum TemplateOptionTags {
		TO_GUINAME = 1,
		TO_HELPTEXT,
		TO_FILTER,
		TO_VIEWCMD,
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
		{ "templateend", TO_END },
		{ "viewcommand", TO_VIEWCMD }
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

		case TO_FILTER:
			lex.next(true);
			fileRegExp = lex.getString();
			break;

		case TO_VIEWCMD:
			lex.next(true);
			viewCommand = lex.getString();
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
			lyx::Assert(false);
			break;
		}
	}
}


void ExternalTemplate::FormatTemplate::readFormat(LyXLex & lex)
{
	enum FormatTags {
		FO_PRODUCT = 1,
		FO_UPDATECMD,
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
		{ "updatecommand", FO_UPDATECMD },
		{ "updateresult", FO_UPDATERESULT }
	};

	pushpophelper pph(lex, formattags, FO_END);

	while (lex.isOK()) {
		switch (lex.lex()) {
		case FO_PRODUCT:
			lex.next(true);
			product = lex.getString();
			break;

		case FO_UPDATECMD:
			lex.next(true);
			updateCommand = lex.getString();
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
			preamble = lex.getLongString("preambleend");
			break;

		case FO_END:
			if (lyxerr.debugging())
				lex.printError("FormatEnd");
			return;
		}
	}
}
