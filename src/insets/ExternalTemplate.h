// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef EXTERNALTEMPLATE_H
#define EXTERNALTEMPLATE_H

#ifdef __GNUG__
#pragma interface
#endif

#include <iosfwd>
#include <map>
#include "LString.h"
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
	/// A file extension regular expression for the file browser
	string fileRegExp;
	/// What command should be executed to view the file?
	string viewCommand;
	/// What command should be executed to edit the file?
	string editCommand;
	/// Should we do automatic production of the output?
	bool automaticProduction;
	/// This is the information needed to support a specific output format
	struct FormatTemplate {
		/// The text that should be inserted into the exported file
		string product;
		/// The shell command to produce a resulting file
		string updateCommand;
		/// What features does this external inset require?
		string requirement;
		/// What should be inserted into the preamble
		string preamble;
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
class ExternalTemplateManager : public noncopyable {
public:
	/// Map from the LyX name of the template to the template structure
	typedef std::map<string, ExternalTemplate> Templates;
	
	static ExternalTemplateManager & get();
	Templates & getTemplates();
	Templates const & getTemplates() const;
private:
	ExternalTemplateManager();
	void readTemplates(string const & path);
	void dumpTemplates() const;
	Templates templates;
};

#endif

