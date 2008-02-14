/**
 * \file InsetBibtex.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Richard Heck (BibTeX parser improvements)
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetBibtex.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "DispatchResult.h"
#include "support/debug.h"
#include "EmbeddedFiles.h"
#include "Encoding.h"
#include "FuncRequest.h"
#include "support/gettext.h"
#include "LaTeXFeatures.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "TextClass.h"

#include "frontends/alert.h"

#include "support/ExceptionMessage.h"
#include "support/docstream.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/Path.h"
#include "support/textutils.h"

#include <boost/tokenizer.hpp>
#include <limits>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace Alert = frontend::Alert;
namespace os = support::os;


InsetBibtex::InsetBibtex(InsetCommandParams const & p)
	: InsetCommand(p, "bibtex")
{}


CommandInfo const * InsetBibtex::findInfo(string const & /* cmdName */)
{
	static const char * const paramnames[] = 
		{"options", "btprint", "bibfiles", "embed", ""};
	static const bool isoptional[] = {true, true, false, false};
	static const CommandInfo info = {4, paramnames, isoptional};
	return &info;
}


Inset * InsetBibtex::clone() const
{
	return new InsetBibtex(*this);
}


void InsetBibtex::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p(BIBTEX_CODE);
		try {
			if (!InsetCommandMailer::string2params("bibtex", 
					to_utf8(cmd.argument()), p)) {
				cur.noUpdate();
				break;
			}
		} catch (ExceptionMessage const & message) {
			if (message.type_ == WarningException) {
				Alert::warning(message.title_, message.details_);
				cur.noUpdate();
			} else 
				throw message;
			break;
		}
		//
		InsetCommandParams orig = params();
		// returned "embed" is composed of "true" or "false", which needs to be adjusted
		string tmp;
		string emb;
		
		string newBibfiles;
		string newEmbedStatus;
		
		string bibfiles = to_utf8(p["bibfiles"]);
		string embedStatus = to_utf8(p["embed"]);
		
		bibfiles = split(bibfiles, tmp, ',');
		embedStatus = split(embedStatus, emb, ',');
		while (!tmp.empty()) {
			EmbeddedFile file(changeExtension(tmp, "bib"), cur.buffer().filePath());
			if (!newBibfiles.empty())
				newBibfiles += ",";
			newBibfiles += tmp;
			if (!newEmbedStatus.empty())
				newEmbedStatus += ",";
			if (emb == "true")
				newEmbedStatus += file.inzipName();
			// Get next file name
			bibfiles = split(bibfiles, tmp, ',');
			embedStatus = split(embedStatus, emb, ',');
		}
		LYXERR(Debug::FILES, "Update parameters from " << p["bibfiles"]
			<< " " << p["embed"] << " to " << newBibfiles << " "
			<< newEmbedStatus);
		p["bibfiles"] = from_utf8(newBibfiles);
		p["embed"] = from_utf8(newEmbedStatus);
		
		setParams(p);
		try {
			// test parameter and copy files
			getFiles(cur.buffer());
		} catch (ExceptionMessage const & message) {
			Alert::error(message.title_, message.details_);
			// do not set parameter if an error happens
			setParams(orig);
			break;
		}
		cur.buffer().updateBibfilesCache();
		break;
	}

	default:
		InsetCommand::doDispatch(cur, cmd);
		break;
	}
}


docstring const InsetBibtex::getScreenLabel(Buffer const &) const
{
	return _("BibTeX Generated Bibliography");
}


namespace {

string normalizeName(Buffer const & buffer, OutputParams const & runparams,
		      string const & name, string const & ext)
{
	string const fname = makeAbsPath(name, buffer.filePath()).absFilename();
	if (FileName(name).isAbsolute() || !FileName(fname + ext).isReadableFile())
		return name;
	if (!runparams.nice)
		return fname;

	// FIXME UNICODE
	return to_utf8(makeRelPath(from_utf8(fname),
					 from_utf8(buffer.masterBuffer()->filePath())));
}

}


int InsetBibtex::latex(Buffer const & buffer, odocstream & os,
		       OutputParams const & runparams) const
{
	// the sequence of the commands:
	// 1. \bibliographystyle{style}
	// 2. \addcontentsline{...} - if option bibtotoc set
	// 3. \bibliography{database}
	// and with bibtopic:
	// 1. \bibliographystyle{style}
	// 2. \begin{btSect}{database}
	// 3. \btPrint{Cited|NotCited|All}
	// 4. \end{btSect}

	// Database(s)
	// If we are processing the LaTeX file in a temp directory then
	// copy the .bib databases to this temp directory, mangling their
	// names in the process. Store this mangled name in the list of
	// all databases.
	// (We need to do all this because BibTeX *really*, *really*
	// can't handle "files with spaces" and Windows users tend to
	// use such filenames.)
	// Otherwise, store the (maybe absolute) path to the original,
	// unmangled database name.
	EmbeddedFileList const bibs = getFiles(buffer);
	EmbeddedFileList::const_iterator it = bibs.begin();
	EmbeddedFileList::const_iterator it_end = bibs.end();
	odocstringstream dbs;
	for (; it != it_end; ++it) {
		string utf8input = removeExtension(it->availableFile().absFilename());
		string database =
			normalizeName(buffer, runparams, utf8input, ".bib");
		FileName const try_in_file(makeAbsPath(database + ".bib", buffer.filePath()));
		bool const not_from_texmf = try_in_file.isReadableFile();

		if (!runparams.inComment && !runparams.dryrun && !runparams.nice &&
		    not_from_texmf) {

			// mangledFilename() needs the extension
			DocFileName const in_file = DocFileName(try_in_file);
			database = removeExtension(in_file.mangledFilename());
			FileName const out_file = makeAbsPath(database + ".bib",
					buffer.masterBuffer()->temppath());

			bool const success = in_file.copyTo(out_file);
			if (!success) {
				lyxerr << "Failed to copy '" << in_file
				       << "' to '" << out_file << "'"
				       << endl;
			}
		} else if (!runparams.inComment && runparams.nice && not_from_texmf &&
			   !isValidLaTeXFilename(database)) {
				frontend::Alert::warning(_("Invalid filename"),
						         _("The following filename is likely to cause trouble "
							   "when running the exported file through LaTeX: ") +
							    from_utf8(database));
		}

		if (it != bibs.begin())
			dbs << ',';
		// FIXME UNICODE
		dbs << from_utf8(latex_path(database));
	}
	docstring const db_out = dbs.str();

	// Post this warning only once.
	static bool warned_about_spaces = false;
	if (!warned_about_spaces &&
	    runparams.nice && db_out.find(' ') != docstring::npos) {
		warned_about_spaces = true;

		Alert::warning(_("Export Warning!"),
			       _("There are spaces in the paths to your BibTeX databases.\n"
					      "BibTeX will be unable to find them."));
	}

	// Style-Options
	string style = to_utf8(getParam("options")); // maybe empty! and with bibtotoc
	string bibtotoc;
	if (prefixIs(style, "bibtotoc")) {
		bibtotoc = "bibtotoc";
		if (contains(style, ','))
			style = split(style, bibtotoc, ',');
	}

	// line count
	int nlines = 0;

	if (!style.empty()) {
		string base = normalizeName(buffer, runparams, style, ".bst");
		FileName const try_in_file(makeAbsPath(base + ".bst", buffer.filePath()));
		bool const not_from_texmf = try_in_file.isReadableFile();
		// If this style does not come from texmf and we are not
		// exporting to .tex copy it to the tmp directory.
		// This prevents problems with spaces and 8bit charcaters
		// in the file name.
		if (!runparams.inComment && !runparams.dryrun && !runparams.nice &&
		    not_from_texmf) {
			// use new style name
			DocFileName const in_file = DocFileName(try_in_file);
			base = removeExtension(in_file.mangledFilename());
			FileName const out_file(makeAbsPath(base + ".bst",
					buffer.masterBuffer()->temppath()));
			bool const success = in_file.copyTo(out_file);
			if (!success) {
				lyxerr << "Failed to copy '" << in_file
				       << "' to '" << out_file << "'"
				       << endl;
			}
		}
		// FIXME UNICODE
		os << "\\bibliographystyle{"
		   << from_utf8(latex_path(normalizeName(buffer, runparams, base, ".bst")))
		   << "}\n";
		nlines += 1;
	}

	// Post this warning only once.
	static bool warned_about_bst_spaces = false;
	if (!warned_about_bst_spaces && runparams.nice && contains(style, ' ')) {
		warned_about_bst_spaces = true;
		Alert::warning(_("Export Warning!"),
			       _("There are spaces in the path to your BibTeX style file.\n"
					      "BibTeX will be unable to find it."));
	}

	if (!db_out.empty() && buffer.params().use_bibtopic){
		os << "\\begin{btSect}{" << db_out << "}\n";
		docstring btprint = getParam("btprint");
		if (btprint.empty())
			// default
			btprint = from_ascii("btPrintCited");
		os << "\\" << btprint << "\n"
		   << "\\end{btSect}\n";
		nlines += 3;
	}

	// bibtotoc-Option
	if (!bibtotoc.empty() && !buffer.params().use_bibtopic) {
		// maybe a problem when a textclass has no "art" as
		// part of its name, because it's than book.
		// For the "official" lyx-layouts it's no problem to support
		// all well
		if (!contains(buffer.params().getTextClass().name(),
			      "art")) {
			if (buffer.params().sides == OneSide) {
				// oneside
				os << "\\clearpage";
			} else {
				// twoside
				os << "\\cleardoublepage";
			}

			// bookclass
			os << "\\addcontentsline{toc}{chapter}{\\bibname}";

		} else {
			// article class
			os << "\\addcontentsline{toc}{section}{\\refname}";
		}
	}

	if (!db_out.empty() && !buffer.params().use_bibtopic){
		docstring btprint = getParam("btprint");
		if (btprint == "btPrintAll") {
			os << "\\nocite{*}\n";
			nlines += 1;
		}
		os << "\\bibliography{" << db_out << "}\n";
		nlines += 1;
	}

	return nlines;
}


EmbeddedFileList const InsetBibtex::getFiles(Buffer const & buffer) const
{
	FileName path(buffer.filePath());
	PathChanger p(path);

	EmbeddedFileList vec;

	string tmp;
	string emb;
	// FIXME UNICODE
	string bibfiles = to_utf8(getParam("bibfiles"));
	string embedStatus = to_utf8(getParam("embed"));
	bibfiles = split(bibfiles, tmp, ',');
	embedStatus = split(embedStatus, emb, ',');
	while (!tmp.empty()) {
		if (!emb.empty()) {
			EmbeddedFile file(changeExtension(tmp, "bib"), buffer.filePath());
			// If the file structure is correct, this should not fail.
			file.setEmbed(true);
			file.enable(buffer.embedded(), &buffer);
			vec.push_back(file);
		} else {
			// this includes the cases when the embed parameter is empty
			FileName const file = findtexfile(changeExtension(tmp, "bib"), "bib");

			// If we didn't find a matching file name just fail silently
			if (!file.empty()) {
				EmbeddedFile efile = EmbeddedFile(file.absFilename(), buffer.filePath());
				efile.setEmbed(false);
				efile.enable(buffer.embedded(), &buffer);
				vec.push_back(efile);
			}
		}

		// Get next file name
		bibfiles = split(bibfiles, tmp, ',');
		embedStatus = split(embedStatus, emb, ',');
	}

	return vec;
}

namespace {

	// methods for parsing bibtex files

	typedef map<docstring, docstring> VarMap;

	/// remove whitespace characters, optionally a single comma,
	/// and further whitespace characters from the stream.
	/// @return true if a comma was found, false otherwise
	///
	bool removeWSAndComma(idocfstream & ifs) {
		char_type ch;

		if (!ifs)
			return false;

		// skip whitespace
		do {
			ifs.get(ch);
		} while (ifs && isSpace(ch));

		if (!ifs)
			return false;

		if (ch != ',') {
			ifs.putback(ch);
			return false;
		}

		// skip whitespace
		do {
			ifs.get(ch);
		} while (ifs && isSpace(ch));

		if (ifs) {
			ifs.putback(ch);
		}

		return true;
	}


	enum charCase {
		makeLowerCase,
		keepCase
	};

	/// remove whitespace characters, read characer sequence
	/// not containing whitespace characters or characters in
	/// delimChars, and remove further whitespace characters.
	///
	/// @return true if a string of length > 0 could be read.
	///
	bool readTypeOrKey(docstring & val, idocfstream & ifs,
		docstring const & delimChars, docstring const &illegalChars, 
		charCase chCase) {

		char_type ch;

		val.clear();

		if (!ifs)
			return false;

		// skip whitespace
		do {
			ifs.get(ch);
		} while (ifs && isSpace(ch));

		if (!ifs)
			return false;

		// read value
		bool legalChar = true;
		while (ifs && !isSpace(ch) && 
						 delimChars.find(ch) == docstring::npos &&
						 (legalChar = (illegalChars.find(ch) == docstring::npos))
					) 
		{
			if (chCase == makeLowerCase)
				val += lowercase(ch);
			else
				val += ch;
			ifs.get(ch);
		}
		
		if (!legalChar) {
			ifs.putback(ch);
			return false;
		}

		// skip whitespace
		while (ifs && isSpace(ch)) {
			ifs.get(ch);
		}

		if (ifs) {
			ifs.putback(ch);
		}

		return val.length() > 0;
	}

	/// read subsequent bibtex values that are delimited with a #-character.
	/// Concatenate all parts and replace names with the associated string in
	/// the variable strings.
	/// @return true if reading was successfull (all single parts were delimited
	/// correctly)
	bool readValue(docstring & val, idocfstream & ifs, const VarMap & strings) {

		char_type ch;

		val.clear();

		if (!ifs)
			return false;

		do {
			// skip whitespace
			do {
				ifs.get(ch);
			} while (ifs && isSpace(ch));

			if (!ifs)
				return false;

			// check for field type
			if (isDigit(ch)) {

				// read integer value
				do {
					val += ch;
					ifs.get(ch);
				} while (ifs && isDigit(ch));

				if (!ifs)
					return false;

			} else if (ch == '"' || ch == '{') {
				// set end delimiter
				char_type delim = ch == '"' ? '"': '}';

				//Skip whitespace
				do {
					ifs.get(ch);
				} while (ifs && isSpace(ch));
				
				if (!ifs)
					return false;
				
				//We now have the first non-whitespace character
				//We'll collapse adjacent whitespace.
				bool lastWasWhiteSpace = false;
				
 				// inside this delimited text braces must match.
 				// Thus we can have a closing delimiter only
 				// when nestLevel == 0
				int nestLevel = 0;
 
				while (ifs && (nestLevel > 0 || ch != delim)) {
					if (isSpace(ch)) {
						lastWasWhiteSpace = true;
						ifs.get(ch);
						continue;
					}
					//We output the space only after we stop getting 
					//whitespace so as not to output any whitespace
					//at the end of the value.
					if (lastWasWhiteSpace) {
						lastWasWhiteSpace = false;
						val += ' ';
					}
					
					val += ch;

					// update nesting level
					switch (ch) {
						case '{':
							++nestLevel;
							break;
						case '}':
							--nestLevel;
							if (nestLevel < 0) return false;
							break;
					}

					ifs.get(ch);
				}

				if (!ifs)
					return false;

				ifs.get(ch);

				if (!ifs)
					return false;

			} else {

				// reading a string name
				docstring strName;

				while (ifs && !isSpace(ch) && ch != '#' && ch != ',' && ch != '}' && ch != ')') {
					strName += lowercase(ch);
					ifs.get(ch);
				}

				if (!ifs)
					return false;

				// replace the string with its assigned value or
				// discard it if it's not assigned
				if (strName.length()) {
					VarMap::const_iterator pos = strings.find(strName);
					if (pos != strings.end()) {
						val += pos->second;
					}
				}
			}

			// skip WS
			while (ifs && isSpace(ch)) {
				ifs.get(ch);
			}

			if (!ifs)
				return false;

			// continue reading next value on concatenate with '#'
		} while (ch == '#');

		ifs.putback(ch);

		return true;
	}
}


// This method returns a comma separated list of Bibtex entries
void InsetBibtex::fillWithBibKeys(Buffer const & buffer,
		BiblioInfo & keylist, InsetIterator const & /*di*/) const
{
	EmbeddedFileList const files = getFiles(buffer);
	for (vector<EmbeddedFile>::const_iterator it = files.begin();
	     it != files.end(); ++ it) {
		// This bibtex parser is a first step to parse bibtex files
		// more precisely.
		//
		// - it reads the whole bibtex entry and does a syntax check
		//   (matching delimiters, missing commas,...
		// - it recovers from errors starting with the next @-character
		// - it reads @string definitions and replaces them in the
		//   field values.
		// - it accepts more characters in keys or value names than
		//   bibtex does.
		//
		// Officially bibtex does only support ASCII, but in practice
		// you can use the encoding of the main document as long as
		// some elements like keys and names are pure ASCII. Therefore
		// we convert the file from the buffer encoding.
		// We don't restrict keys to ASCII in LyX, since our own
		// InsetBibitem can generate non-ASCII keys, and nonstandard
		// 8bit clean bibtex forks exist.
		
		idocfstream ifs(it->availableFile().toFilesystemEncoding().c_str(),
			ios_base::in,
			buffer.params().encoding().iconvName());

		char_type ch;
		VarMap strings;

		while (ifs) {

			ifs.get(ch);
			if (!ifs)
				break;

			if (ch != '@')
				continue;

			docstring entryType;

			if (!readTypeOrKey(entryType, ifs, from_ascii("{("), 
			                   docstring(), makeLowerCase) || !ifs)
				continue;

			if (entryType == from_ascii("comment")) {

				ifs.ignore(numeric_limits<int>::max(), '\n');
				continue;
			}

			ifs.get(ch);
			if (!ifs)
				break;

			if ((ch != '(') && (ch != '{')) {
				// invalid entry delimiter
				ifs.putback(ch);
				continue;
			}

			// process the entry
			if (entryType == from_ascii("string")) {

				// read string and add it to the strings map
				// (or replace it's old value)
				docstring name;
				docstring value;

				if (!readTypeOrKey(name, ifs, from_ascii("="), 
				                   from_ascii("#{}(),"), makeLowerCase) || !ifs)
					continue;

				// next char must be an equal sign
				ifs.get(ch);
				if (!ifs || ch != '=')
					continue;

				if (!readValue(value, ifs, strings))
					continue;

				strings[name] = value;

			} else if (entryType == from_ascii("preamble")) {

				// preamble definitions are discarded.
				// can they be of any use in lyx?
				docstring value;

				if (!readValue(value, ifs, strings))
					continue;

			} else {

				// Citation entry. Try to read the key.
				docstring key;

				if (!readTypeOrKey(key, ifs, from_ascii(","), 
				                   from_ascii("}"), keepCase) || !ifs)
					continue;

				/////////////////////////////////////////////
				// now we have a key, so we will add an entry 
 				// (even if it's empty, as bibtex does)
				//
				// we now read the field = value pairs.
 				// all items must be separated by a comma. If
 				// it is missing the scanning of this entry is
 				// stopped and the next is searched.
				docstring fields;
				docstring name;
				docstring value;
				docstring commaNewline;
				docstring data;
				BibTeXInfo keyvalmap(key, entryType);
				
				bool readNext = removeWSAndComma(ifs);
 
				while (ifs && readNext) {

					// read field name
					if (!readTypeOrKey(name, ifs, from_ascii("="), 
					                   from_ascii("{}(),"), makeLowerCase) || !ifs)
						break;

					// next char must be an equal sign
					ifs.get(ch);
					if (!ifs)
						break;
					if (ch != '=') {
						ifs.putback(ch);
						break;
					}

					// read field value
					if (!readValue(value, ifs, strings))
						break;

					keyvalmap[name] = value;
					data += "\n\n" + value;
					keylist.addFieldName(name);
					readNext = removeWSAndComma(ifs);
				}

				// add the new entry
				keylist.addEntryType(entryType);
				keyvalmap.allData(data);
				keylist[key] = keyvalmap;
			}
		} //< searching '@'
	} //< for loop over files
}



bool InsetBibtex::addDatabase(string const & db)
{
	// FIXME UNICODE
	string bibfiles(to_utf8(getParam("bibfiles")));
	if (tokenPos(bibfiles, ',', db) == -1) {
		if (!bibfiles.empty())
			bibfiles += ',';
		setParam("bibfiles", from_utf8(bibfiles + db));
		return true;
	}
	return false;
}


bool InsetBibtex::delDatabase(string const & db)
{
	// FIXME UNICODE
	string bibfiles(to_utf8(getParam("bibfiles")));
	if (contains(bibfiles, db)) {
		int const n = tokenPos(bibfiles, ',', db);
		string bd = db;
		if (n > 0) {
			// this is not the first database
			string tmp = ',' + bd;
			setParam("bibfiles", from_utf8(subst(bibfiles, tmp, string())));
		} else if (n == 0)
			// this is the first (or only) database
			setParam("bibfiles", from_utf8(split(bibfiles, bd, ',')));
		else
			return false;
	}
	return true;
}


void InsetBibtex::validate(LaTeXFeatures & features) const
{
	if (features.bufferParams().use_bibtopic)
		features.require("bibtopic");
}


void InsetBibtex::registerEmbeddedFiles(Buffer const & buffer, EmbeddedFileList & files) const
{
	EmbeddedFileList const dbs = getFiles(buffer);
	for (vector<EmbeddedFile>::const_iterator it = dbs.begin();
		it != dbs.end(); ++ it)
		files.registerFile(*it, this, buffer);		
}


void InsetBibtex::updateEmbeddedFile(Buffer const & buf, EmbeddedFile const & file)
{
	// look for the item and update status
	docstring bibfiles;
	docstring embed;

	bool first = true;
	EmbeddedFileList dbs = getFiles(buf);
	for (EmbeddedFileList::iterator it = dbs.begin();
		it != dbs.end(); ++ it) {
		// update from file
		if (it->absFilename() == file.absFilename())
			it->setEmbed(file.embedded());
		// write parameter string
		if (!first) {
			bibfiles += ',';
			embed += ',';
		} else
			first = false;
		bibfiles += from_utf8(it->outputFilename(buf.filePath()));
		if (it->embedded())
			embed += from_utf8(it->inzipName());
	}
	setParam("bibfiles", bibfiles);
	setParam("embed", embed);
}


} // namespace lyx
