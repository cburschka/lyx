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

#include <cstdio>
#include <utility>

#include "insetexternal.h"
#include "ExternalTemplate.h"
#include "BufferView.h"
#include "buffer.h"
#include "LyXView.h"
#include "frontends/Dialogs.h"
#include "lyx_main.h"
#include "LaTeXFeatures.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/path.h"
#include "support/syscall.h"
#include "gettext.h"
#include "debug.h"

using std::endl;

InsetExternal::InsetExternal() 
	: view_(0)
{
	tempname_ = lyx::tempName(string(), "lyxext");
	//ExternalTemplateManager::Templates::const_iterator i1;
	params_.templ = ExternalTemplateManager::get().getTemplates().begin()->second;
}


InsetExternal::~InsetExternal()
{
	lyx::unlink(tempname_);
	hideDialog();
}


InsetExternal::Params InsetExternal::params() const
{
	return params_;
}
 
 
void InsetExternal::setFromParams(Params const & p)
{
	params_.filename = p.filename;
	params_.parameters = p.parameters;
	params_.templ = p.templ;
}      


string const InsetExternal::editMessage() const
{
	return doSubstitution(0, params_.templ.guiName);
}


void InsetExternal::edit(BufferView * bv,
			 int /*x*/, int /*y*/, unsigned int /*button*/)
{
	view_ = bv;
	view_->owner()->getDialogs()->showExternal(this);
}


void InsetExternal::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, 0);
}


void InsetExternal::write(Buffer const *, std::ostream & os) const
{
	os << "External " << params_.templ.lyxName << ",\"" << params_.filename 
	   << "\",\"" << params_.parameters << "\"\n";
}


void InsetExternal::read(Buffer const *, LyXLex & lex)
{
	string format;
	string token;

	// Read inset data from lex and store in format
	if (lex.eatLine()) {
		format = lex.getString();
	} else {
		lex.printError("InsetExternal: Parse error: `$$Token'");
	}
	
	while (lex.isOK()) {
		lex.nextToken();
		token = lex.getString();
		if (token == "\\end_inset")
			break;
	}
	if (token != "\\end_inset") {
		lex.printError("Missing \\end_inset at this point. "
			       "Read: `$$Token'");
	}

	// Parse string format...
	string::size_type const pos1 = format.find(",");
	params_.templ = ExternalTemplateManager::get().getTemplateByName(format.substr(0, pos1));
	string::size_type const pos2 = format.find("\",\"", pos1);
	params_.filename = format.substr(pos1 + 2, pos2 - (pos1 + 2));
	params_.parameters = format.substr(pos2 + 3, format.length() - (pos2 + 4));

	lyxerr[Debug::INFO] << "InsetExternal::Read: " << params_.templ.lyxName
			    << " " << params_.filename
			    << " " << params_.parameters << endl;
}


int InsetExternal::write(string const & format,
			 Buffer const * buf, std::ostream & os) const
{
	ExternalTemplate const & et = params_.templ;
	ExternalTemplate::Formats::const_iterator cit =
		et.formats.find(format);
	if (cit == et.formats.end()) {
		lyxerr << "External template format '" << format
		       << "' not specified in template " << params_.templ.lyxName
		       << endl;
		return 0;
	}
	
	if (et.automaticProduction) {
		executeCommand(doSubstitution(buf,
					      cit->second.updateCommand),
			       buf);
	}
	
	os << doSubstitution(buf, cit->second.product);
	return 0; // CHECK  (FIXME check what ? - jbl)
}


int InsetExternal::latex(Buffer const * buf,
			 std::ostream & os, bool, bool) const
{
	return write("LaTeX", buf, os);
}


int InsetExternal::ascii(Buffer const * buf, std::ostream & os, int) const
{
	return write("Ascii", buf, os);
}


int InsetExternal::linuxdoc(Buffer const * buf, std::ostream & os) const
{
	return write("LinuxDoc", buf, os);
}


int InsetExternal::docBook(Buffer const * buf, std::ostream & os) const
{
	return write("DocBook", buf, os);
}


void InsetExternal::validate(LaTeXFeatures & features) const
{
	ExternalTemplate const & et = params_.templ;
	ExternalTemplate::Formats::const_iterator cit =
		et.formats.find("LaTeX");

	if (cit == et.formats.end())
		return;
	
	if (!cit->second.requirement.empty()) {
		features.require(cit->second.requirement);
	}
	if (!cit->second.preamble.empty()) {
		features.externalPreambles += cit->second.preamble + "\n";
	}
}


Inset * InsetExternal::clone(Buffer const &, bool same_id) const
{
	InsetExternal * inset = new InsetExternal();
	inset->params_ = params_;
	inset->view_ = view_;
	if (same_id)
		inset->id_ = id_;
	return inset;
}


string const InsetExternal::getScreenLabel(Buffer const *) const
{
	ExternalTemplate const & et = params_.templ;
	if (et.guiName.empty())
		return _("External");
	else
		return doSubstitution(0, et.guiName);
}


void InsetExternal::executeCommand(string const & s,
				   Buffer const * buffer) const
{
	string buf = MakeAbsPath(buffer->fileName());
	string path = OnlyPath(buf);
	Path p(path);
	Systemcalls one;
	if (lyxerr.debugging()) {
		lyxerr << "Executing '" << s << "' in '"
		       << path << "'" << endl;
	}
	one.startscript(Systemcalls::Wait, s);
}


string const InsetExternal::doSubstitution(Buffer const * buffer,
				     string const & s) const
{
	string result;
	string const basename = ChangeExtension(params_.filename, string());
	result = subst(s, "$$FName", params_.filename);
	result = subst(result, "$$Basename", basename);
	result = subst(result, "$$Parameters", params_.parameters);
	result = ReplaceEnvironmentPath(result);
	result = subst(result, "$$Tempname", tempname_);
	result = subst(result, "$$Sysdir", system_lyxdir);
	
	// Handle the $$Contents(filename) syntax
	if (contains(result, "$$Contents(\"")) {

		string::size_type const pos = result.find("$$Contents(\"");
		string::size_type const end = result.find("\")", pos);
		string const file = result.substr(pos + 12, end - (pos + 12));
		string contents;
		if (buffer) {
			// Make sure we are in the directory of the buffer
			string const buf = MakeAbsPath(buffer->fileName());
			string const path = OnlyPath(buf);
			Path p(path);
			contents = GetFileContents(file);
		} else {
			contents = GetFileContents(file);
		}
		result = subst(result,
			       ("$$Contents(\"" + file + "\")").c_str(),
			       contents);
	}

	return result;
}


void InsetExternal::updateExternal() const
{
	ExternalTemplate const & et = params_.templ;
	ExternalTemplate::Formats::const_iterator cit =
		et.formats.find("LaTeX");
	if (cit == et.formats.end())
		return;
	
	executeCommand(doSubstitution(view_->buffer(),
				      cit->second.updateCommand),
		       view_->buffer());
}


void InsetExternal::viewExternal() const
{
	ExternalTemplate const & et = params_.templ;
	if (et.automaticProduction)
		updateExternal();

	executeCommand(doSubstitution(view_->buffer(),
				      et.viewCommand),
		       view_->buffer());
}


void InsetExternal::editExternal() const
{
	ExternalTemplate const & et = params_.templ;
	if (et.automaticProduction)
		updateExternal();

	executeCommand(doSubstitution(view_->buffer(),
				      et.editCommand),
		       view_->buffer());
}


bool operator==(InsetExternal::Params const & left,
		InsetExternal::Params const & right)
{
	return ((left.filename   == right.filename) &&
		(left.parameters == right.parameters) &&
		(left.templ.lyxName == right.templ.lyxName));
}


bool operator!=(InsetExternal::Params const & left,
		InsetExternal::Params const & right)
{
	return 	!(left == right);
}

