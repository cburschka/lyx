// -*- C++ -*-
/**
 * \file ExternalTemplate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef EXTERNALTEMPLATE_H
#define EXTERNALTEMPLATE_H


#include <iosfwd>
#include <map>
#include "support/std_string.h"
#include <boost/utility.hpp>

class LyXLex;

///
struct ExternalTemplate {
	/// What is the name of this template in the LyX format?
	string lyxName;
	/// What will the button in the GUI say?
	string guiName;
	/// A short help text
	string helpText;
	/** The format of the input file. Can be "*", in which case we try and
	    ascertain the format from the contents of the file.
	*/
	string inputFormat;
	/// A file extension regular expression for the file browser
	string fileRegExp;
	/// What command should be executed to edit the file?
	string editCommand;
	/// Should we do automatic production of the output?
	bool automaticProduction;
	/// This is the information needed to support a specific output format
	struct FormatTemplate {
		/// The text that should be inserted into the exported file
		string product;
		/// The filename of the resulting file
		string updateResult;
		/// The format of this file.
		string updateFormat;
		/// What features does this external inset require?
		string requirement;
		/// Identify the preamble snippet using \c preambleName.
		string preambleName;
		///
		void readFormat(LyXLex &);
		/// This constructor has to default a command for safety reasons!
		FormatTemplate();
	};
	///
	void readTemplate(LyXLex &);
	///
	typedef std::map<string, FormatTemplate> Formats;
	///
	Formats formats;
	///
	void dumpFormats(std::ostream &) const;

	/// We have to have default commands for safety reasons!
	ExternalTemplate();
};


/**
   A singleton class that manages the external inset templates
*/
class ExternalTemplateManager : boost::noncopyable {
public:
	/// Map from the LyX name of the template to the template structure
	typedef std::map<string, ExternalTemplate> Templates;
	/** Map from the LyX name of the preamble definition to the preamble
	 *  definition itself.
	 */
	typedef std::map<string, string> PreambleDefs;

	static ExternalTemplateManager & get();
	Templates & getTemplates();
	Templates const & getTemplates() const;
	/// return the template by LyX name
	ExternalTemplate const & getTemplateByName(string const & name);
	string const getPreambleDefByName(string const & name) const;
private:
	ExternalTemplateManager();
	void readTemplates(string const & path);
	void dumpTemplates(std::ostream &) const;
	void dumpPreambleDefs(std::ostream &) const;

	Templates templates;
	PreambleDefs preambledefs;
};

#endif
