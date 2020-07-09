/**
 * \file InsetBibtex.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Richard Heck (BibTeX parser improvements)
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetBibtex.h"

#include "BiblioInfo.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "CiteEnginesList.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "Encoding.h"
#include "Exporter.h"
#include "Format.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LaTeXFeatures.h"
#include "output_latex.h"
#include "xml.h"
#include "OutputParams.h"
#include "PDFOptions.h"
#include "texstream.h"
#include "TextClass.h"
#include "TocBackend.h"

#include "frontends/alert.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/docstring_list.h"
#include "support/ExceptionMessage.h"
#include "support/FileNameList.h"
#include "support/filetools.h"
#include "support/regex.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/PathChanger.h"
#include "support/textutils.h"

#include <limits>
#include <map>
#include <utility>

#include <iostream>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace Alert = frontend::Alert;
namespace os = support::os;


InsetBibtex::InsetBibtex(Buffer * buf, InsetCommandParams const & p)
	: InsetCommand(buf, p)
{}


ParamInfo const & InsetBibtex::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;
	if (param_info_.empty()) {
		param_info_.add("btprint", ParamInfo::LATEX_OPTIONAL);
		param_info_.add("bibfiles", ParamInfo::LATEX_REQUIRED);
		param_info_.add("options", ParamInfo::LYX_INTERNAL);
		param_info_.add("encoding", ParamInfo::LYX_INTERNAL);
		param_info_.add("file_encodings", ParamInfo::LYX_INTERNAL);
		param_info_.add("biblatexopts", ParamInfo::LATEX_OPTIONAL);
	}
	return param_info_;
}


void InsetBibtex::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {

	case LFUN_INSET_EDIT:
		editDatabases(cmd.argument());
		break;

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p(BIBTEX_CODE);
		try {
			if (!InsetCommand::string2params(to_utf8(cmd.argument()), p)) {
				cur.noScreenUpdate();
				break;
			}
		} catch (ExceptionMessage const & message) {
			if (message.type_ == WarningException) {
				Alert::warning(message.title_, message.details_);
				cur.noScreenUpdate();
			} else
				throw;
			break;
		}

		cur.recordUndo();
		setParams(p);
		cur.buffer()->clearBibFileCache();
		cur.forceBufferUpdate();
		break;
	}

	default:
		InsetCommand::doDispatch(cur, cmd);
		break;
	}
}


bool InsetBibtex::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action()) {
	case LFUN_INSET_EDIT:
		flag.setEnabled(true);
		return true;

	default:
		return InsetCommand::getStatus(cur, cmd, flag);
	}
}


void InsetBibtex::editDatabases(docstring const & db) const
{
	vector<docstring> bibfilelist = getVectorFromString(getParam("bibfiles"));

	if (bibfilelist.empty())
		return;

	size_t nr_databases = bibfilelist.size();
	if (nr_databases > 1 && db.empty()) {
			docstring const engine = usingBiblatex() ? _("Biblatex") : _("BibTeX");
			docstring message = bformat(_("The %1$s[[BibTeX/Biblatex]] inset includes %2$s databases.\n"
						       "If you proceed, all of them will be opened."),
							engine, convert<docstring>(nr_databases));
			int const ret = Alert::prompt(_("Open Databases?"),
				message, 0, 1, _("&Cancel"), _("&Proceed"));

			if (ret == 0)
				return;
	}

	vector<docstring>::const_iterator it = bibfilelist.begin();
	vector<docstring>::const_iterator en = bibfilelist.end();
	for (; it != en; ++it) {
		if (!db.empty() && db != *it)
			continue;
		FileName const bibfile = buffer().getBibfilePath(*it);
		theFormats().edit(buffer(), bibfile,
		     theFormats().getFormatFromFile(bibfile));
	}
}


bool InsetBibtex::usingBiblatex() const
{
	return buffer().masterParams().useBiblatex();
}


docstring InsetBibtex::screenLabel() const
{
	return usingBiblatex() ? _("Biblatex Generated Bibliography")
			       : _("BibTeX Generated Bibliography");
}


docstring InsetBibtex::toolTip(BufferView const & /*bv*/, int /*x*/, int /*y*/) const
{
	docstring tip = _("Databases:");
	vector<docstring> bibfilelist = getVectorFromString(getParam("bibfiles"));

	tip += "<ul>";
	if (bibfilelist.empty())
		tip += "<li>" + _("none") + "</li>";
	else
		for (docstring const & bibfile : bibfilelist)
			tip += "<li>" + bibfile + "</li>";
	tip += "</ul>";

	// Style-Options
	bool toc = false;
	docstring style = getParam("options"); // maybe empty! and with bibtotoc
	docstring bibtotoc = from_ascii("bibtotoc");
	if (prefixIs(style, bibtotoc)) {
		toc = true;
		if (contains(style, char_type(',')))
			style = split(style, bibtotoc, char_type(','));
	}

	docstring const btprint = getParam("btprint");
	if (!usingBiblatex()) {
		tip += _("Style File:");
		tip += "<ul><li>" + (style.empty() ? _("none") : style) + "</li></ul>";

		tip += _("Lists:") + " ";
		if (btprint == "btPrintAll")
			tip += _("all references");
		else if (btprint == "btPrintNotCited")
			tip += _("all uncited references");
		else
			tip += _("all cited references");
		if (toc) {
			tip += ", ";
			tip += _("included in TOC");
		}
		if (!buffer().parent()
		    && buffer().params().multibib == "child") {
			tip += "<br />";
			tip += _("Note: This bibliography is not output, since bibliographies in the master file "
				 "are not allowed with the setting 'Multiple bibliographies per child document'");
		}
	} else {
		tip += _("Lists:") + " ";
		if (btprint == "bibbysection")
			tip += _("all reference units");
		else if (btprint == "btPrintAll")
			tip += _("all references");
		else
			tip += _("all cited references");
		if (toc) {
			tip += ", ";
			tip += _("included in TOC");
		}
		if (!getParam("biblatexopts").empty()) {
			tip += "<br />";
			tip += _("Options: ") + getParam("biblatexopts");
		}
	}

	return tip;
}


void InsetBibtex::latex(otexstream & os, OutputParams const & runparams) const
{
	// The sequence of the commands:
	// With normal BibTeX:
	// 1. \bibliographystyle{style}
	// 2. \addcontentsline{...} - if option bibtotoc set
	// 3. \bibliography{database}
	// With bibtopic:
	// 1. \bibliographystyle{style}
	// 2. \begin{btSect}{database}
	// 3. \btPrint{Cited|NotCited|All}
	// 4. \end{btSect}
	// With Biblatex:
	// \printbibliography[biblatexopts]
	// or
	// \bibbysection[biblatexopts] - if btprint is "bibbysection"

	// chapterbib does not allow bibliographies in the master
	if (!usingBiblatex() && !runparams.is_child
	    && buffer().params().multibib == "child")
		return;

	if (runparams.inDeletedInset) {
		// We cannot strike-out bibligraphies,
		// so we just output a note.
		os << "\\textbf{"
		   << buffer().B_("[BIBLIOGRAPHY DELETED!]")
		   << "}";
		return;
	}

	string style = to_utf8(getParam("options")); // maybe empty! and with bibtotoc
	string bibtotoc;
	if (prefixIs(style, "bibtotoc")) {
		bibtotoc = "bibtotoc";
		if (contains(style, ','))
			style = split(style, bibtotoc, ',');
	}

	if (usingBiblatex()) {
		// Options
		string opts = to_utf8(getParam("biblatexopts"));
		// bibtotoc-Option
		if (!bibtotoc.empty())
			opts = opts.empty() ? "heading=bibintoc" : "heading=bibintoc," + opts;
		// The bibliography command
		docstring btprint = getParam("btprint");
		if (btprint == "btPrintAll")
			os << "\\nocite{*}\n";
		if (btprint == "bibbysection" && !buffer().masterParams().multibib.empty())
			os << "\\bibbysection";
		else
			os << "\\printbibliography";
		if (!opts.empty())
			os << "[" << opts << "]";
		os << "\n";
	} else {// using BibTeX
		// Database(s)
		vector<pair<docstring, string>> const dbs =
			buffer().prepareBibFilePaths(runparams, getBibFiles(), false);
		vector<docstring> db_out;
		for (pair<docstring, string> const & db : dbs)
			db_out.push_back(db.first);
		// Style options
		if (style == "default")
			style = buffer().masterParams().defaultBiblioStyle();
		if (!style.empty() && !buffer().masterParams().useBibtopic()) {
			string base = buffer().masterBuffer()->prepareFileNameForLaTeX(style, ".bst", runparams.nice);
			FileName const try_in_file =
				makeAbsPath(base + ".bst", buffer().filePath());
			bool const not_from_texmf = try_in_file.isReadableFile();
			// If this style does not come from texmf and we are not
			// exporting to .tex copy it to the tmp directory.
			// This prevents problems with spaces and 8bit characters
			// in the file name.
			if (!runparams.inComment && !runparams.dryrun && !runparams.nice &&
			    not_from_texmf) {
				// use new style name
				DocFileName const in_file = DocFileName(try_in_file);
				base = removeExtension(in_file.mangledFileName());
				FileName const out_file = makeAbsPath(base + ".bst",
						buffer().masterBuffer()->temppath());
				bool const success = in_file.copyTo(out_file);
				if (!success) {
					LYXERR0("Failed to copy '" << in_file
					       << "' to '" << out_file << "'");
				}
			}
			// FIXME UNICODE
			os << "\\bibliographystyle{"
			   << from_utf8(latex_path(buffer().prepareFileNameForLaTeX(base, ".bst", runparams.nice)))
			   << "}\n";
		}
		// Warn about spaces in bst path. Warn only once.
		static bool warned_about_bst_spaces = false;
		if (!warned_about_bst_spaces && runparams.nice && contains(style, ' ')) {
			warned_about_bst_spaces = true;
			Alert::warning(_("Export Warning!"),
				       _("There are spaces in the path to your BibTeX style file.\n"
						      "BibTeX will be unable to find it."));
		}
		// Encoding
		bool encoding_switched = false;
		Encoding const * const save_enc = runparams.encoding;
		docstring const encoding = getParam("encoding");
		if (!encoding.empty() && encoding != from_ascii("default")) {
			Encoding const * const enc = encodings.fromLyXName(to_ascii(encoding));
			if (enc != runparams.encoding) {
				os << "\\bgroup";
				switchEncoding(os.os(), buffer().params(), runparams, *enc, true);
				runparams.encoding = enc;
				encoding_switched = true;
			}
		}
		// Handle the bibtopic case
		if (!db_out.empty() && buffer().masterParams().useBibtopic()) {
			os << "\\begin{btSect}";
			if (!style.empty())
				os << "[" << style << "]";
			os << "{" << getStringFromVector(db_out) << "}\n";
			docstring btprint = getParam("btprint");
			if (btprint.empty())
				// default
				btprint = from_ascii("btPrintCited");
			os << "\\" << btprint << "\n"
			   << "\\end{btSect}\n";
		}
		// bibtotoc option
		if (!bibtotoc.empty() && !buffer().masterParams().useBibtopic()
		    && !buffer().masterParams().documentClass().bibInToc()) {
			// set label for hyperref, see http://www.lyx.org/trac/ticket/6470
			if (buffer().masterParams().pdfoptions().use_hyperref)
					os << "\\phantomsection";
			if (buffer().masterParams().documentClass().hasLaTeXLayout("chapter"))
				os << "\\addcontentsline{toc}{chapter}{\\bibname}";
			else if (buffer().masterParams().documentClass().hasLaTeXLayout("section"))
				os << "\\addcontentsline{toc}{section}{\\refname}";
		}
		// The bibliography command
		if (!db_out.empty() && !buffer().masterParams().useBibtopic()) {
			docstring btprint = getParam("btprint");
			if (btprint == "btPrintAll") {
				os << "\\nocite{*}\n";
			}
			os << "\\bibliography{" << getStringFromVector(db_out) << "}\n";
		}
		if (encoding_switched){
			// Switch back
			switchEncoding(os.os(), buffer().params(),
				       runparams, *save_enc, true, true);
			os << "\\egroup" << breakln;
			runparams.encoding = save_enc;
		}
	}
}


docstring_list InsetBibtex::getBibFiles() const
{
	return getVectorFromString(getParam("bibfiles"));
}

namespace {

	// methods for parsing bibtex files

	typedef map<docstring, docstring> VarMap;

	/// remove whitespace characters, optionally a single comma,
	/// and further whitespace characters from the stream.
	/// @return true if a comma was found, false otherwise
	///
	bool removeWSAndComma(ifdocstream & ifs) {
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

	/// remove whitespace characters, read character sequence
	/// not containing whitespace characters or characters in
	/// delimChars, and remove further whitespace characters.
	///
	/// @return true if a string of length > 0 could be read.
	///
	bool readTypeOrKey(docstring & val, ifdocstream & ifs,
		docstring const & delimChars, docstring const & illegalChars,
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
		while (ifs && !isSpace(ch) &&
		       delimChars.find(ch) == docstring::npos &&
		       illegalChars.find(ch) == docstring::npos)
		{
			if (chCase == makeLowerCase)
				val += lowercase(ch);
			else
				val += ch;
			ifs.get(ch);
		}

		if (illegalChars.find(ch) != docstring::npos) {
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
	/// @return true if reading was successful (all single parts were delimited
	/// correctly)
	bool readValue(docstring & val, ifdocstream & ifs, const VarMap & strings) {

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
			if (isDigitASCII(ch)) {

				// read integer value
				do {
					val += ch;
					ifs.get(ch);
				} while (ifs && isDigitASCII(ch));

				if (!ifs)
					return false;

			} else if (ch == '"' || ch == '{') {
				// set end delimiter
				char_type delim = ch == '"' ? '"': '}';

				// Skip whitespace
				do {
					ifs.get(ch);
				} while (ifs && isSpace(ch));

				if (!ifs)
					return false;

				// We now have the first non-whitespace character
				// We'll collapse adjacent whitespace.
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
					// We output the space only after we stop getting
					// whitespace so as not to output any whitespace
					// at the end of the value.
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
							if (nestLevel < 0)
								return false;
							break;
					}

					if (ifs)
						ifs.get(ch);
				}

				if (!ifs)
					return false;

				// FIXME Why is this here?
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
} // namespace


void InsetBibtex::collectBibKeys(InsetIterator const & /*di*/, FileNameList & checkedFiles) const
{
	parseBibTeXFiles(checkedFiles);
}


void InsetBibtex::parseBibTeXFiles(FileNameList & checkedFiles) const
{
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
	// you can use any encoding as long as some elements like keys
	// and names are pure ASCII. We support specifying an encoding,
	// and we convert the file from that (default is buffer encoding).
	// We don't restrict keys to ASCII in LyX, since our own
	// InsetBibitem can generate non-ASCII keys, and nonstandard
	// 8bit clean bibtex forks exist.

	BiblioInfo keylist;

	docstring_list const files = getBibFiles();
	for (auto const & bf : files) {
		FileName const bibfile = buffer().getBibfilePath(bf);
		if (bibfile.empty()) {
			LYXERR0("Unable to find path for " << bf << "!");
			continue;
		}
		if (find(checkedFiles.begin(), checkedFiles.end(), bibfile) != checkedFiles.end())
			// already checked this one. Skip.
			continue;
		else
			// record that we check this.
			checkedFiles.push_back(bibfile);
		string encoding = buffer().masterParams().encoding().iconvName();
		string ienc = buffer().masterParams().bibFileEncoding(to_utf8(bf));
		if (ienc.empty() || ienc == "general")
			ienc = to_ascii(params()["encoding"]);

		if (!ienc.empty() && ienc != "auto-legacy-plain" && ienc != "auto-legacy" && encodings.fromLyXName(ienc))
			encoding = encodings.fromLyXName(ienc)->iconvName();
		ifdocstream ifs(bibfile.toFilesystemEncoding().c_str(),
			ios_base::in, encoding);

		char_type ch;
		VarMap strings;

		while (ifs) {
			ifs.get(ch);
			if (!ifs)
				break;

			if (ch != '@')
				continue;

			docstring entryType;

			if (!readTypeOrKey(entryType, ifs, from_ascii("{("), docstring(), makeLowerCase)) {
				lyxerr << "BibTeX Parser: Error reading entry type." << std::endl;
				continue;
			}

			if (!ifs) {
				lyxerr << "BibTeX Parser: Unexpected end of file." << std::endl;
				continue;
			}

			if (entryType == from_ascii("comment")) {
				ifs.ignore(numeric_limits<int>::max(), '\n');
				continue;
			}

			ifs.get(ch);
			if (!ifs) {
				lyxerr << "BibTeX Parser: Unexpected end of file." << std::endl;
				break;
			}

			if ((ch != '(') && (ch != '{')) {
				lyxerr << "BibTeX Parser: Invalid entry delimiter." << std::endl;
				ifs.putback(ch);
				continue;
			}

			// process the entry
			if (entryType == from_ascii("string")) {

				// read string and add it to the strings map
				// (or replace it's old value)
				docstring name;
				docstring value;

				if (!readTypeOrKey(name, ifs, from_ascii("="), from_ascii("#{}(),"), makeLowerCase)) {
					lyxerr << "BibTeX Parser: Error reading string name." << std::endl;
					continue;
				}

				if (!ifs) {
					lyxerr << "BibTeX Parser: Unexpected end of file." << std::endl;
					continue;
				}

				// next char must be an equal sign
				ifs.get(ch);
				if (!ifs || ch != '=') {
					lyxerr << "BibTeX Parser: No `=' after string name: " <<
							name << "." << std::endl;
					continue;
				}

				if (!readValue(value, ifs, strings)) {
					lyxerr << "BibTeX Parser: Unable to read value for string: " <<
							name << "." << std::endl;
					continue;
				}

				strings[name] = value;

			} else if (entryType == from_ascii("preamble")) {

				// preamble definitions are discarded.
				// can they be of any use in lyx?
				docstring value;

				if (!readValue(value, ifs, strings)) {
					lyxerr << "BibTeX Parser: Unable to read preamble value." << std::endl;
					continue;
				}

			} else {

				// Citation entry. Try to read the key.
				docstring key;

				if (!readTypeOrKey(key, ifs, from_ascii(","), from_ascii("}"), keepCase)) {
					lyxerr << "BibTeX Parser: Unable to read key for entry type:" <<
							entryType << "." << std::endl;
					continue;
				}

				if (!ifs) {
					lyxerr << "BibTeX Parser: Unexpected end of file." << std::endl;
					continue;
				}

				/////////////////////////////////////////////
				// now we have a key, so we will add an entry
 				// (even if it's empty, as bibtex does)
				//
				// we now read the field = value pairs.
 				// all items must be separated by a comma. If
 				// it is missing the scanning of this entry is
 				// stopped and the next is searched.
				docstring name;
				docstring value;
				docstring data;
				BibTeXInfo keyvalmap(key, entryType);

				bool readNext = removeWSAndComma(ifs);

				while (ifs && readNext) {

					// read field name
					if (!readTypeOrKey(name, ifs, from_ascii("="),
					                   from_ascii("{}(),"), makeLowerCase) || !ifs)
						break;

					// next char must be an equal sign
					// FIXME Whitespace??
					ifs.get(ch);
					if (!ifs) {
						lyxerr << "BibTeX Parser: Unexpected end of file." << std::endl;
						break;
					}
					if (ch != '=') {
						lyxerr << "BibTeX Parser: Missing `=' after field name: " <<
								name << ", for key: " << key << "." << std::endl;
						ifs.putback(ch);
						break;
					}

					// read field value
					if (!readValue(value, ifs, strings)) {
						lyxerr << "BibTeX Parser: Unable to read value for field: " <<
								name << ", for key: " << key << "." << std::endl;
						break;
					}

					keyvalmap[name] = value;
					data += "\n\n" + value;
					keylist.addFieldName(name);
					readNext = removeWSAndComma(ifs);
				}

				// add the new entry
				keylist.addEntryType(entryType);
				keyvalmap.setAllData(data);
				keylist[key] = keyvalmap;
			} //< else (citation entry)
		} //< searching '@'
	} //< for loop over files

	buffer().addBiblioInfo(keylist);
}


bool InsetBibtex::addDatabase(docstring const & db)
{
	docstring bibfiles = getParam("bibfiles");
	if (tokenPos(bibfiles, ',', db) != -1)
		return false;
	if (!bibfiles.empty())
		bibfiles += ',';
	setParam("bibfiles", bibfiles + db);
	return true;
}


bool InsetBibtex::delDatabase(docstring const & db)
{
	docstring bibfiles = getParam("bibfiles");
	if (contains(bibfiles, db)) {
		int const n = tokenPos(bibfiles, ',', db);
		docstring bd = db;
		if (n > 0) {
			// this is not the first database
			docstring tmp = ',' + bd;
			setParam("bibfiles", subst(bibfiles, tmp, docstring()));
		} else if (n == 0)
			// this is the first (or only) database
			setParam("bibfiles", split(bibfiles, bd, ','));
		else
			return false;
	}
	return true;
}


void InsetBibtex::validate(LaTeXFeatures & features) const
{
	BufferParams const & mparams = features.buffer().masterParams();
	if (mparams.useBibtopic())
		features.require("bibtopic");
	else if (!mparams.useBiblatex() && mparams.multibib == "child")
		features.require("chapterbib");
	// FIXME XHTML
	// It'd be better to be able to get this from an InsetLayout, but at present
	// InsetLayouts do not seem really to work for things that aren't InsetTexts.
	if (features.runparams().flavor == OutputParams::HTML)
		features.addCSSSnippet("div.bibtexentry { margin-left: 2em; text-indent: -2em; }\n"
			"span.bibtexlabel:before{ content: \"[\"; }\n"
			"span.bibtexlabel:after{ content: \"] \"; }");
}


void InsetBibtex::updateBuffer(ParIterator const &, UpdateType, bool const /*deleted*/)
{
	buffer().registerBibfiles(getBibFiles());
	// record encoding of bib files for biblatex
	string const enc = (params()["encoding"] == from_ascii("default")) ?
				string() : to_ascii(params()["encoding"]);
	bool invalidate = false;
	if (buffer().params().bibEncoding() != enc) {
		buffer().params().setBibEncoding(enc);
		invalidate = true;
	}
	map<string, string> encs = getFileEncodings();
	map<string, string>::const_iterator it = encs.begin();
	for (; it != encs.end(); ++it) {
		if (buffer().params().bibFileEncoding(it->first) != it->second) {
			buffer().params().setBibFileEncoding(it->first, it->second);
			invalidate = true;
		}
	}
	if (invalidate)
		buffer().invalidateBibinfoCache();
}


map<string, string> InsetBibtex::getFileEncodings() const
{
	vector<string> ps =
		getVectorFromString(to_utf8(getParam("file_encodings")), "\t");
	std::map<string, string> res;
	for (string const & s: ps) {
		string key;
		string val = split(s, key, ' ');
		res[key] = val;
	}
	return res;
}


docstring InsetBibtex::getRefLabel() const
{
	if (buffer().masterParams().documentClass().hasLaTeXLayout("chapter"))
		return buffer().B_("Bibliography");
	return buffer().B_("References");
}


void InsetBibtex::addToToc(DocIterator const & cpit, bool output_active,
			   UpdateType, TocBackend & backend) const
{
	if (!prefixIs(to_utf8(getParam("options")), "bibtotoc"))
		return;

	docstring const str = getRefLabel();
	shared_ptr<Toc> toc = backend.toc("tableofcontents");
	// Assign to appropriate level
	int const item_depth =
		(buffer().masterParams().documentClass().hasLaTeXLayout("chapter")) 
			? 1 : 2;
	toc->push_back(TocItem(cpit, item_depth, str, output_active));
}


int InsetBibtex::plaintext(odocstringstream & os,
       OutputParams const & op, size_t max_length) const
{
	docstring const reflabel = getRefLabel();

	// We could output more information here, e.g., what databases are included
	// and information about options. But I don't necessarily see any reason to
	// do this right now.
	if (op.for_tooltip || op.for_toc || op.for_search) {
		os << '[' << reflabel << ']' << '\n';
		return PLAINTEXT_NEWLINE;
	}

	BiblioInfo bibinfo = buffer().masterBibInfo();
	bibinfo.makeCitationLabels(buffer());
	vector<docstring> const & cites = bibinfo.citedEntries();

	size_t start_size = os.str().size();
	docstring refoutput;
	refoutput += reflabel + "\n\n";

	// Tell BiblioInfo our purpose
	CiteItem ci;
	ci.context = CiteItem::Export;

	// Now we loop over the entries
	vector<docstring>::const_iterator vit = cites.begin();
	vector<docstring>::const_iterator const ven = cites.end();
	for (; vit != ven; ++vit) {
		if (start_size + refoutput.size() >= max_length)
			break;
		BiblioInfo::const_iterator const biit = bibinfo.find(*vit);
		if (biit == bibinfo.end())
			continue;
		BibTeXInfo const & entry = biit->second;
		refoutput += "[" + entry.label() + "] ";
		// FIXME Right now, we are calling BibInfo::getInfo on the key,
		// which will give us all the cross-referenced info. But for every
		// entry, so there's a lot of repetition. This should be fixed.
		refoutput += bibinfo.getInfo(entry.key(), buffer(), ci) + "\n\n";
	}
	os << refoutput;
	return int(refoutput.size());
}


// FIXME
// docstring InsetBibtex::entriesAsXHTML(vector<docstring> const & entries)
// And then here just: entriesAsXHTML(buffer().masterBibInfo().citedEntries())
docstring InsetBibtex::xhtml(XMLStream & xs, OutputParams const &) const
{
	BiblioInfo const & bibinfo = buffer().masterBibInfo();
	bool const all_entries = getParam("btprint") == "btPrintAll";
	vector<docstring> const & cites =
	    all_entries ? bibinfo.getKeys() : bibinfo.citedEntries();

	docstring const reflabel = buffer().B_("References");

	// tell BiblioInfo our purpose
	CiteItem ci;
	ci.context = CiteItem::Export;
	ci.richtext = true;
	ci.max_key_size = UINT_MAX;

	xs << xml::StartTag("h2", "class='bibtex'")
		<< reflabel
		<< xml::EndTag("h2")
		<< xml::StartTag("div", "class='bibtex'");

	// Now we loop over the entries
	vector<docstring>::const_iterator vit = cites.begin();
	vector<docstring>::const_iterator const ven = cites.end();
	for (; vit != ven; ++vit) {
		BiblioInfo::const_iterator const biit = bibinfo.find(*vit);
		if (biit == bibinfo.end())
			continue;

		BibTeXInfo const & entry = biit->second;
		string const attr = "class='bibtexentry' id='LyXCite-"
		    + to_utf8(xml::cleanAttr(entry.key())) + "'";
		xs << xml::StartTag("div", attr);

		// don't print labels if we're outputting all entries
		if (!all_entries) {
			xs << xml::StartTag("span", "class='bibtexlabel'")
				<< entry.label()
				<< xml::EndTag("span");
		}

		// FIXME Right now, we are calling BibInfo::getInfo on the key,
		// which will give us all the cross-referenced info. But for every
		// entry, so there's a lot of repetition. This should be fixed.
		xs << xml::StartTag("span", "class='bibtexinfo'")
		   << XMLStream::ESCAPE_AND
		   << bibinfo.getInfo(entry.key(), buffer(), ci)
		   << xml::EndTag("span")
		   << xml::EndTag("div")
		   << xml::CR();
	}
	xs << xml::EndTag("div");
	return docstring();
}


void InsetBibtex::docbook(XMLStream & xs, OutputParams const &) const
{
	BiblioInfo const & bibinfo = buffer().masterBibInfo();
	bool const all_entries = getParam("btprint") == "btPrintAll";
	vector<docstring> const & cites =
			all_entries ? bibinfo.getKeys() : bibinfo.citedEntries();

	docstring const reflabel = buffer().B_("References");

	// Tell BiblioInfo our purpose (i.e. generate HTML rich text).
	CiteItem ci;
	ci.context = CiteItem::Export;
	ci.richtext = true;
	ci.max_key_size = UINT_MAX;

	// Header for bibliography (title required).
	xs << xml::StartTag("bibliography");
	xs << xml::CR();
	xs << xml::StartTag("title");
	xs << reflabel;
	xs << xml::EndTag("title") << xml::CR();

	// Translation between keys in each entry and DocBook tags.
	// IDs for publications; list: http://tdg.docbook.org/tdg/5.2/biblioid.html.
	vector<pair<string, string>> biblioId = { // <bibtex, docbook>
	        make_pair("doi", "doi"),
	        make_pair("isbn", "isbn"),
	        make_pair("issn", "issn"),
	        make_pair("isrn", "isrn"),
	        make_pair("istc", "istc"),
	        make_pair("lccn", "libraryofcongress"),
	        make_pair("number", "pubsnumber"),
	        make_pair("url", "uri")
	};
	// Relations between documents.
	vector<pair<string, string>> relations = { // <bibtex, docbook biblioset relation>
	        make_pair("journal", "journal"),
	        make_pair("booktitle", "book"),
	        make_pair("series", "series")
	};
	// Various things that do not fit DocBook.
	vector<string> misc = { "language", "school", "note" };

	// Store the mapping between BibTeX and DocBook.
	map<string, string> toDocBookTag;
	toDocBookTag["fullnames:author"] = "SPECIFIC"; // No direct translation to DocBook: <authorgroup>.
	toDocBookTag["publisher"] = "SPECIFIC"; // No direct translation to DocBook: <publisher>.
	toDocBookTag["address"] = "SPECIFIC"; // No direct translation to DocBook: <publisher>.
	toDocBookTag["editor"] = "editor";
	toDocBookTag["institution"] = "SPECIFIC"; // No direct translation to DocBook: <org>.

	toDocBookTag["title"] = "title";
	toDocBookTag["volume"] = "volumenum";
	toDocBookTag["edition"] = "edition";
	toDocBookTag["pages"] = "artpagenums";

	toDocBookTag["abstract"] = "SPECIFIC"; // No direct translation to DocBook: <abstract>.
	toDocBookTag["keywords"] = "SPECIFIC"; // No direct translation to DocBook: <keywordset>.
	toDocBookTag["year"] = "SPECIFIC"; // No direct translation to DocBook: <pubdate>.
	toDocBookTag["month"] = "SPECIFIC"; // No direct translation to DocBook: <pubdate>.

	toDocBookTag["journal"] = "SPECIFIC"; // No direct translation to DocBook: <biblioset>.
	toDocBookTag["booktitle"] = "SPECIFIC"; // No direct translation to DocBook: <biblioset>.
	toDocBookTag["series"] = "SPECIFIC"; // No direct translation to DocBook: <biblioset>.

	for (auto const & id: biblioId)
	    toDocBookTag[id.first] = "SPECIFIC"; // No direct translation to DocBook: <biblioid>.
	for (auto const & id: relations)
	    toDocBookTag[id.first] = "SPECIFIC"; // No direct translation to DocBook: <biblioset>.
	for (auto const & id: misc)
	    toDocBookTag[id] = "SPECIFIC"; // No direct translation to DocBook: <bibliomisc>.

	// Loop over the entries. If there are no entries, add a comment to say so.
	auto vit = cites.begin();
	auto ven = cites.end();

	if (vit == ven) {
		xs << XMLStream::ESCAPE_NONE << "<!-- No entry in the bibliography. -->";
	}

	for (; vit != ven; ++vit) {
		BiblioInfo::const_iterator const biit = bibinfo.find(*vit);
		if (biit == bibinfo.end())
			continue;

		BibTeXInfo const & entry = biit->second;
		string const attr = "xml:id=\"" + to_utf8(xml::cleanID(entry.key())) + "\"";
		xs << xml::StartTag("biblioentry", attr);
		xs << xml::CR();

		// FIXME Right now, we are calling BibInfo::getInfo on the key,
		// which will give us all the cross-referenced info. But for every
		// entry, so there's a lot of repetition. This should be fixed.

		// Parse the results of getInfo and emit the corresponding DocBook tags. Interesting pieces have the form
		// "<span class="bib-STH">STH</span>", the rest of the text may be discarded.
		// Could have written a DocBook version of expandFormat (that parses a citation into HTML), but it implements
		// some kind of recursion. Still, a (static) conversion step between the citation format and DocBook would have
		// been required. All in all, both codes approaches would have been similar, but this parsing allows relying
		// on existing building blocks.

		string html = to_utf8(bibinfo.getInfo(entry.key(), buffer(), ci));
		regex tagRegex("<span class=\"bib-([^\"]*)\">([^<]*)</span>");
		smatch match;
		auto tagIt = std::sregex_iterator(html.cbegin(), html.cend(), tagRegex, regex_constants::match_default);
		auto tagEnd = std::sregex_iterator();
		map<string, string> delayedTags;

		// Read all tags from HTML and convert those that have a 1:1 matching.
		while (tagIt != tagEnd) {
			string tag = tagIt->str(); // regex_match cannot work with temporary strings.
			++tagIt;
			regex_match(tag, match, tagRegex);

			if (regex_match(tag, match, tagRegex)) {
				if (toDocBookTag[match[1]] == "SPECIFIC") {
					delayedTags[match[1]] = match[2];
				} else {
					xs << xml::StartTag(toDocBookTag[match[1]]);
					xs << from_utf8(match[2].str());
					xs << xml::EndTag(toDocBookTag[match[1]]);
				}
			} else {
				LYXERR0("The BibTeX field " << match[1].str() << " is unknown.");
				xs << XMLStream::ESCAPE_NONE << from_utf8("<!-- Output Error: The BibTeX field " + match[1].str() + " is unknown -->\n");
			}
		}

		// Type of document (book, journal paper, etc.).
		xs << xml::StartTag("bibliomisc", "role=\"type\"");
		xs << entry.entryType();
		xs << xml::EndTag("bibliomisc");
		xs << xml::CR();

		// Handle tags that have complex transformations.
		if (! delayedTags.empty()) {
			unsigned long remainingTags = delayedTags.size(); // Used as a workaround. With GCC 7, when erasing all
			// elements one by one, some elements may still pop in later on (even though they were deleted previously).
			auto hasTag = [&delayedTags](string key) { return delayedTags.find(key) != delayedTags.end(); };
			auto getTag = [&delayedTags](string key) { return from_utf8(delayedTags[key]); };
			auto eraseTag = [&delayedTags, &remainingTags](string key) {
				remainingTags -= 1;
				delayedTags.erase(key);
			};

			// Notes on order of checks.
			// - address goes with publisher if there is one, so check this first. Otherwise, the address goes with
			//   the entry without other details.

			// <publisher>
			if (hasTag("publisher")) {
				xs << xml::StartTag("publisher");
				xs << xml::CR();
				xs << xml::StartTag("publishername");
				xs << getTag("publisher");
				xs << xml::EndTag("publishername");
				xs << xml::CR();

				if (hasTag("address")) {
					xs << xml::StartTag("address");
					xs << getTag("address");
					xs << xml::EndTag("address");
					eraseTag("address");
				}

				xs << xml::EndTag("publisher");
				xs << xml::CR();
				eraseTag("publisher");
			}

			if (hasTag("address")) {
				xs << xml::StartTag("address");
				xs << getTag("address");
				xs << xml::EndTag("address");
				eraseTag("address");
			}

			// <keywordset>
			if (hasTag("keywords")) {
				// Split the keywords on comma.
				docstring keywordSet = getTag("keywords");
				vector<docstring> keywords;
				if (keywordSet.find(from_utf8(",")) == string::npos) {
					keywords = { keywordSet };
				} else {
					size_t pos = 0;
					while ((pos = keywordSet.find(from_utf8(","))) != string::npos) {
						keywords.push_back(keywordSet.substr(0, pos));
						keywordSet.erase(0, pos + 1);
					}
					keywords.push_back(keywordSet);
				}

				xs << xml::StartTag("keywordset") << xml::CR();
				for (auto & kw: keywords) {
					kw.erase(kw.begin(), std::find_if(kw.begin(), kw.end(),
					                                  [](int c) {return !std::isspace(c);}));
					xs << xml::StartTag("keyword");
					xs << kw;
					xs << xml::EndTag("keyword");
					xs << xml::CR();
				}
				xs << xml::EndTag("keywordset") << xml::CR();
				eraseTag("keywords");
			}

			// <copyright>
			// Example: http://tdg.docbook.org/tdg/5.1/biblioset.html
			if (hasTag("year")) {
				docstring value = getTag("year");
				eraseTag("year");

				// Follow xsd:gYearMonth format (http://books.xmlschemata.org/relaxng/ch19-77135.html).
				if (hasTag("month")) {
					value += "-" + getTag("month");
					eraseTag("month");
				}

				xs << xml::StartTag("pubdate");
				xs << value;
				xs << xml::EndTag("pubdate");
				xs << xml::CR();
			}

			// <institution>
			if (hasTag("institution")) {
				xs << xml::StartTag("org");
				xs << xml::CR();
				xs << xml::StartTag("orgname");
				xs << getTag("institution");
				xs << xml::EndTag("orgname");
				xs << xml::CR();
				xs << xml::EndTag("org");
				xs << xml::CR();
				eraseTag("institution");
			}

			// <biblioset>
			// Example: http://tdg.docbook.org/tdg/5.1/biblioset.html
			for (auto const & id: relations) {
				if (hasTag(id.first)) {
					xs << xml::StartTag("biblioset", "relation=\"" + id.second + "\"");
					xs << xml::CR();
					xs << xml::StartTag("title");
					xs << getTag(id.first);
					xs << xml::EndTag("title");
					xs << xml::CR();
					xs << xml::EndTag("biblioset");
					xs << xml::CR();
					eraseTag(id.first);
				}
			}

			// <authorgroup>
			// Example: http://tdg.docbook.org/tdg/5.1/authorgroup.html
			if (hasTag("fullnames:author")) {
				// Perform full parsing of the BibTeX string, dealing with the many corner cases that might
				// be encountered.
				authorsToDocBookAuthorGroup(getTag("fullnames:author"), xs, buffer());
				eraseTag("fullnames:author");
			}

			// <abstract>
			if (hasTag("abstract")) {
				// Split the paragraphs on new line.
				docstring abstract = getTag("abstract");
				vector<docstring> paragraphs;
				if (abstract.find(from_utf8("\n")) == string::npos) {
					paragraphs = { abstract };
				} else {
					size_t pos = 0;
					while ((pos = abstract.find(from_utf8(","))) != string::npos) {
						paragraphs.push_back(abstract.substr(0, pos));
						abstract.erase(0, pos + 1);
					}
					paragraphs.push_back(abstract);
				}

				xs << xml::StartTag("abstract");
				xs << xml::CR();
				for (auto const & para: paragraphs) {
					if (para.empty())
						continue;
					xs << xml::StartTag("para");
					xs << para;
					xs << xml::EndTag("para");
				}
				xs << xml::CR();
				xs << xml::EndTag("abstract");
				xs << xml::CR();
				eraseTag("abstract");
			}

			// <biblioid>
			for (auto const & id: biblioId) {
				if (hasTag(id.first)) {
					xs << xml::StartTag("biblioid", "class=\"" + id.second + "\"");
					xs << getTag(id.first);
					xs << xml::EndTag("biblioid");
					xs << xml::CR();
					eraseTag(id.first);
				}
			}

			// <bibliomisc>
			for (auto const & id: misc) {
				if (hasTag(id)) {
					xs << xml::StartTag("bibliomisc", "role=\"" + id + "\"");
					xs << getTag(id);
					xs << xml::EndTag("bibliomisc");
					xs << xml::CR();
					eraseTag(id);
				}
			}

			// After all tags are processed, check for errors.
			if (remainingTags > 0) {
				LYXERR0("Still delayed tags not yet handled.");
				xs << XMLStream::ESCAPE_NONE << from_utf8("<!-- Output Error: still delayed tags not yet handled.\n");
				for (auto const & item: delayedTags) {
					xs << from_utf8(" " + item.first + ": " + item.second + "\n");
				}
				xs << XMLStream::ESCAPE_NONE << from_utf8(" -->\n");
			}
		}

		xs << xml::EndTag("biblioentry");
		xs << xml::CR();
	}

	// Footer for bibliography.
	xs << xml::EndTag("bibliography");
}


void InsetBibtex::write(ostream & os) const
{
	params().Write(os, &buffer());
}


string InsetBibtex::contextMenuName() const
{
	return "context-bibtex";
}


} // namespace lyx
