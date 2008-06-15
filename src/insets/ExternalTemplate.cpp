/**
 * \file ExternalTemplate.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ExternalTemplate.h"

#include "Lexer.h"

#include "support/debug.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/Package.h"
#include "support/Path.h"

#include <ostream>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace external {


typedef Translator<TransformID, string> TransformIDTranslator;

static TransformIDTranslator const initIDTranslator()
{
	TransformIDTranslator translator(TransformID(-1), "");
	translator.addPair(Rotate, "Rotate");
	translator.addPair(Resize, "Resize");
	translator.addPair(Clip,   "Clip");
	translator.addPair(Extra,  "Extra");
	return translator;
}

static TransformIDTranslator const & transformIDTranslator()
{
	static TransformIDTranslator const translator = initIDTranslator();
	return translator;
}

// We have to have dummy default commands for security reasons!
Template::Template()
	: inputFormat("*")
{}


Template::Format::Format()
{}


TemplateManager::TemplateManager()
{
	readTemplates(package().user_support());
	if (lyxerr.debugging(Debug::EXTERNAL)) {
		dumpPreambleDefs(lyxerr);
		lyxerr << '\n';
		dumpTemplates(lyxerr);
	}
}


class DumpPreambleDef {
public:
	typedef TemplateManager::PreambleDefs::value_type value_type;

	DumpPreambleDef(ostream & os) : os_(os) {}

	void operator()(value_type const & vt) {
		os_ << "PreambleDef " << vt.first << '\n'
		    << vt.second
		    << "PreambleDefEnd" << endl;
	}

private:
	ostream & os_;
};


class DumpTemplate {
public:
	typedef TemplateManager::Templates::value_type value_type;

	DumpTemplate(ostream & os) : os_(os) {}

	void operator()(value_type const & vt) {
		Template const & et = vt.second;

		os_ << "Template " << et.lyxName << '\n'
		    << "\tGuiName " << et.guiName << '\n'
		    << "\tHelpText\n"
		    << et.helpText
		    << "\tHelpTextEnd\n"
		    << "\tInputFormat " << et.inputFormat << '\n'
		    << "\tFileFilter " << et.fileRegExp << '\n'
		    << "\tAutomaticProduction " << et.automaticProduction << '\n'
		    << "\tPreview ";
		switch (et.preview_mode) {
			case PREVIEW_OFF:
				os_ << "Off\n";
				break;
			case PREVIEW_GRAPHICS:
				os_ << "Graphics\n";
				break;
			case PREVIEW_INSTANT:
				os_ << "InstantPreview\n";
				break;
		}
		typedef vector<TransformID> IDs;
		IDs::const_iterator it  = et.transformIds.begin();
		IDs::const_iterator end = et.transformIds.end();
		for (; it != end; ++it) {
			os_ << "\tTransform "
			    << transformIDTranslator().find(*it) << '\n';
		}

		et.dumpFormats(os_);
		os_ << "TemplateEnd" << endl;

	}

private:
	ostream & os_;
};

class DumpFormat {
public:
	typedef Template::Formats::value_type value_type;

	DumpFormat(ostream & o) : os_(o) {}

	void operator()(value_type const & vt) const {
		Template::Format const & ft = vt.second;
		os_ << "\tFormat " << vt.first << '\n'
		    << "\t\tProduct " << ft.product << '\n'
		    << "\t\tUpdateFormat " << ft.updateFormat << '\n'
		    << "\t\tUpdateResult " << ft.updateResult << '\n';

		vector<string>::const_iterator qit = ft.requirements.begin();
		vector<string>::const_iterator qend = ft.requirements.end();
		for (; qit != qend; ++qit) {
			lyxerr << "req:" << *qit << endl;
			os_ << "\t\tRequirement " << *qit << '\n';
		}

		typedef vector<Template::Option> Options;
		Options::const_iterator oit  = ft.options.begin();
		Options::const_iterator oend = ft.options.end();
		for (; oit != oend; ++oit) {
			os_ << "\t\tOption "
			    << oit->name
			    << ": "
			    << oit->option
			    << '\n';
		}

		vector<string>::const_iterator pit  = ft.preambleNames.begin();
		vector<string>::const_iterator pend = ft.preambleNames.end();
		for (; pit != pend; ++pit) {
			os_ << "\t\tPreamble " << *pit << '\n';
		}

		typedef Template::Format::FileMap FileMap;
		FileMap::const_iterator rit  = ft.referencedFiles.begin();
		FileMap::const_iterator rend = ft.referencedFiles.end();
		for (; rit != rend; ++rit) {
			vector<string>::const_iterator fit  = rit->second.begin();
			vector<string>::const_iterator fend = rit->second.end();
			for (; fit != fend; ++fit) {
				os_ << "\t\tReferencedFile " << rit->first
				    << " \"" << *fit << "\"\n";
			}
		}

		os_ << "\tFormatEnd\n";
	}
private:
	ostream & os_;
};


void Template::dumpFormats(ostream & os) const
{
	for_each(formats.begin(), formats.end(), DumpFormat(os));
}


void TemplateManager::dumpPreambleDefs(ostream & os) const
{
	for_each(preambledefs.begin(), preambledefs.end(), DumpPreambleDef(os));
}


void TemplateManager::dumpTemplates(ostream & os) const
{
	for_each(templates.begin(), templates.end(), DumpTemplate(os));
}


TemplateManager & TemplateManager::get()
{
	static TemplateManager externalTemplateManager;
	return externalTemplateManager;
}


TemplateManager::Templates const & TemplateManager::getTemplates() const
{
	return templates;
}


Template const *
TemplateManager::getTemplateByName(string const & name) const
{
	Templates::const_iterator it = templates.find(name);
	return (it == templates.end()) ? 0 : &it->second;
}


string const
TemplateManager::getPreambleDefByName(string const & name) const
{
	string const trimmed_name = trim(name);
	if (trimmed_name.empty())
		return string();

	PreambleDefs::const_iterator it = preambledefs.find(trimmed_name);
	if (it == preambledefs.end())
		return string();

	return it->second;
}


void TemplateManager::readTemplates(FileName const & path)
{
	PathChanger p(path);

	enum {
		TM_PREAMBLEDEF = 1,
		TM_PREAMBLEDEF_END,
		TM_TEMPLATE,
		TM_TEMPLATE_END
	};

	LexerKeyword templatetags[] = {
		{ "preambledef", TM_PREAMBLEDEF },
		{ "preambledefend", TM_PREAMBLEDEF_END },
		{ "template", TM_TEMPLATE },
		{ "templateend", TM_TEMPLATE_END }
	};

	Lexer lex(templatetags);

	FileName const filename = libFileSearch("", "external_templates");
	if (filename.empty() || !lex.setFile(filename)) {
		lex.printError("external::TemplateManager::readTemplates: "
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
			Template & tmp = templates[name];
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


void Template::readTemplate(Lexer & lex)
{
	enum {
		TO_GUINAME = 1,
		TO_HELPTEXT,
		TO_INPUTFORMAT,
		TO_FILTER,
		TO_AUTOMATIC,
		TO_PREVIEW,
		TO_TRANSFORM,
		TO_FORMAT,
		TO_END
	};

	LexerKeyword templateoptiontags[] = {
		{ "automaticproduction", TO_AUTOMATIC },
		{ "filefilter", TO_FILTER },
		{ "format", TO_FORMAT },
		{ "guiname", TO_GUINAME },
		{ "helptext", TO_HELPTEXT },
		{ "inputformat", TO_INPUTFORMAT },
		{ "preview", TO_PREVIEW },
		{ "templateend", TO_END },
		{ "transform", TO_TRANSFORM }
	};

	PushPopHelper pph(lex, templateoptiontags);
	lex.setContext("Template::readTemplate");

	string token;
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

		case TO_AUTOMATIC:
			lex.next();
			automaticProduction = lex.getBool();
			break;

		case TO_PREVIEW:
			lex >> token;
			if (token == "InstantPreview")
				preview_mode = PREVIEW_INSTANT;
			else if (token == "Graphics")
				preview_mode = PREVIEW_GRAPHICS;
			else
				preview_mode = PREVIEW_OFF;
			break;

		case TO_TRANSFORM: {
			lex >> token;
			TransformID id = transformIDTranslator().find(token);
			if (int(id) == -1)
				LYXERR0("Transform " << token << " is not recognized");
			else
				transformIds.push_back(id);
			break;
		}

		case TO_FORMAT:
			lex.next(true);
			formats[lex.getString()].readFormat(lex);
			break;

		case TO_END:
			return;

		default:
			lex.printError("external::Template::readTemplate: "
				       "Wrong tag: $$Token");
			LASSERT(false, /**/);
			break;
		}
	}
}


namespace {

void transform_not_found(ostream & os, string const & transform)
{
	os << "external::Format::readFormat. Transformation \""
	   << transform << "\" is unrecognized." << endl;
}


void transform_class_not_found(ostream & os, string const & tclass)
{
	os << "external::Format::readFormat. Transformation class \""
	   << tclass << "\" is unrecognized." << endl;
}


void setCommandFactory(Template::Format & format, string const & transform,
		       string const & transformer_class)
{
	bool class_found = false;
	if (transform == "Resize" && transformer_class == "ResizeLatexCommand") {
		class_found = true;
		ResizeCommandFactory factory = ResizeLatexCommand::factory;
		format.command_transformers[Resize] =
			TransformStore(Resize, factory);

	} else if (transform == "Rotate" &&
		   transformer_class == "RotationLatexCommand") {
		class_found = true;
		RotationCommandFactory factory = RotationLatexCommand::factory;
		format.command_transformers[Rotate] =
			TransformStore(Rotate, factory);

	} else
		transform_not_found(lyxerr, transform);

	if (!class_found)
		transform_class_not_found(lyxerr, transformer_class);
}


void setOptionFactory(Template::Format & format, string const & transform,
		string const & transformer_class)
{
	bool class_found = false;
	if (transform == "Clip" && transformer_class == "ClipLatexOption") {
		class_found = true;
		ClipOptionFactory factory = ClipLatexOption::factory;
		format.option_transformers[Clip] =
				TransformStore(Clip, factory);

	} else if (transform == "Extra" && transformer_class == "ExtraOption") {
		class_found = true;
		ExtraOptionFactory factory = ExtraOption::factory;
		format.option_transformers[Extra] =
			TransformStore(Extra, factory);

	} else if (transform == "Resize" &&
		   transformer_class == "ResizeLatexOption") {
		class_found = true;
		ResizeOptionFactory factory = ResizeLatexOption::factory;
		format.option_transformers[Resize] =
			TransformStore(Resize, factory);

	} else if (transform == "Rotate" &&
		   transformer_class == "RotationLatexOption") {
		class_found = true;
		RotationOptionFactory factory = RotationLatexOption::factory;
		format.option_transformers[Rotate] =
			TransformStore(Rotate, factory);

	} else
		transform_not_found(lyxerr, transform);

	if (!class_found)
		transform_class_not_found(lyxerr, transformer_class);
}

} // namespace anon


void Template::Format::readFormat(Lexer & lex)
{
	enum {
		FO_PRODUCT = 1,
		FO_UPDATEFORMAT,
		FO_UPDATERESULT,
		FO_REQUIREMENT,
		FO_OPTION,
		FO_PREAMBLE,
		FO_TRANSFORMCOMMAND,
		FO_TRANSFORMOPTION,
		FO_REFERENCEDFILE,
		FO_END
	};

	LexerKeyword formattags[] = {
		{ "formatend", FO_END },
		{ "option", FO_OPTION },
		{ "preamble", FO_PREAMBLE },
		{ "product", FO_PRODUCT },
		{ "referencedfile", FO_REFERENCEDFILE },
		{ "requirement", FO_REQUIREMENT },
		{ "transformcommand", FO_TRANSFORMCOMMAND },
		{ "transformoption", FO_TRANSFORMOPTION },
		{ "updateformat", FO_UPDATEFORMAT },
		{ "updateresult", FO_UPDATERESULT }
	};

	PushPopHelper pph(lex, formattags);

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
			requirements.push_back(lex.getString());
			break;

		case FO_PREAMBLE:
			lex.next(true);
			preambleNames.push_back(lex.getString());
			break;

		case FO_TRANSFORMCOMMAND: {
			lex.next(true);
			string const name = lex.getString();
			lex.next(true);
			setCommandFactory(*this, name, lex.getString());
			break;
		}

		case FO_TRANSFORMOPTION: {
			lex.next(true);
			string const name = lex.getString();
			lex.next(true);
			setOptionFactory(*this, name, lex.getString());
			break;
		}

		case FO_OPTION: {
			lex.next(true);
			string const name = lex.getString();
			lex.next(true);
			string const opt = lex.getString();
			options.push_back(Option(name, opt));
			break;
		}

		case FO_REFERENCEDFILE: {
			lex.next(true);
			string const format = lex.getString();
			lex.next(true);
			string const file = lex.getString();
			referencedFiles[format].push_back(file);
			break;
		}

		case FO_END:
			return;
		}
	}
}

} // namespace external
} // namespace lyx
