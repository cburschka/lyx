/* \file Messages.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

/*
  This contains a limited parser for gettext's mo files. Several features are
  not implemented currently:
   * encoding is supposed to be UTF-8 (the charset parameter is enforced)
   * context is not handled (implemented differently in LyX)
   * plural forms are not implemented (not used for now in LyX).

  The data is loaded in a std::map object for simplicity.
 */

/*
  Format of a MO file. Source: http://www.gnu.org/software/gettext/manual/html_node/MO-Files.html

             byte
                  +------------------------------------------+
               0  | magic number = 0x950412de                |
                  |                                          |
               4  | file format revision = 0                 |
                  |                                          |
               8  | number of strings                        |  == N
                  |                                          |
              12  | offset of table with original strings    |  == O
                  |                                          |
              16  | offset of table with translation strings |  == T
                  |                                          |
              20  | size of hashing table                    |  == S
                  |                                          |
              24  | offset of hashing table                  |  == H
                  |                                          |
                  .                                          .
                  .    (possibly more entries later)         .
                  .                                          .
                  |                                          |
               O  | length & offset 0th string  ----------------.
           O + 8  | length & offset 1st string  ------------------.
                   ...                                    ...   | |
     O + ((N-1)*8)| length & offset (N-1)th string           |  | |
                  |                                          |  | |
               T  | length & offset 0th translation  ---------------.
           T + 8  | length & offset 1st translation  -----------------.
                   ...                                    ...   | | | |
     T + ((N-1)*8)| length & offset (N-1)th translation      |  | | | |
                  |                                          |  | | | |
               H  | start hash table                         |  | | | |
                   ...                                    ...   | | | |
       H + S * 4  | end hash table                           |  | | | |
                  |                                          |  | | | |
                  | NUL terminated 0th string  <----------------' | | |
                  |                                          |    | | |
                  | NUL terminated 1st string  <------------------' | |
                  |                                          |      | |
                   ...                                    ...       | |
                  |                                          |      | |
                  | NUL terminated 0th translation  <---------------' |
                  |                                          |        |
                  | NUL terminated 1st translation  <-----------------'
                  |                                          |
                   ...                                    ...
                  |                                          |
                  +------------------------------------------+

 */

#include <config.h>

#include "support/Messages.h"

#include "support/debug.h"
#include "support/docstring.h"
#include "support/lstrings.h"
#include "support/Package.h"
#include "support/unicode.h"

#include "support/lassert.h"

#include <boost/cstdint.hpp>

#include <cerrno>
#include <fstream>
#include <utility>

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

using namespace std;
using boost::uint32_t;

namespace lyx {

void cleanTranslation(docstring & trans)
{
	/*
	  Some english words have different translations, depending on
	  context. In these cases the original string is augmented by
	  context information (e.g. "To:[[as in 'From page x to page
	  y']]" and "To:[[as in 'From format x to format y']]". Also,
	  when placeholders are used, the context can indicate what will
	  be substituted for the placeholder (e.g. "%1$s[[date]], %1$s
	  [[time]]). This means that we need to filter out everything
	  in double square brackets at the end of the string, otherwise
	  the user sees bogus messages. If we are unable to honour the
	  request we just return what we got in.
	*/
	static docstring const ctx_start = from_ascii("[[");
	static docstring const ctx_end = from_ascii("]]");
	while (true) {
		size_t const pos1 = trans.find(ctx_start);
		if (pos1 != docstring::npos) {
			size_t const pos2 = trans.find(ctx_end, pos1);
			if (pos2 != docstring::npos) {
				trans.erase(pos1, pos2 - pos1 + 2);
				continue;
			}
		}
		break;
	}
}

} // lyx


#ifdef ENABLE_NLS

using namespace lyx::support;

namespace lyx {

std::string Messages::gui_lang_;


Messages::Messages(string const & l)
	: lang_(l)
{
	// strip off any encoding suffix, i.e., assume 8-bit po files
	size_t i = lang_.find(".");
	lang_ = lang_.substr(0, i);
	LYXERR(Debug::LOCALE, "language(" << lang_ << ")");

	readMoFile();
}


namespace {

// Find the code we have for a given language code. Return empty if not found.
string realCode(string code)
{
	// this loops at most twice
	while (true) {
		if (package().messages_file(code).isReadableFile())
			return code;
		if (contains(code, '_'))
			code = token(code, '_', 0);
		else
			break;
	}
	return string();
}
}


bool Messages::available(string const & c)
{
	return !realCode(c).empty();
}


string Messages::language() const
{
	return realCode(lang_);
}

namespace {

void swapInt(uint32_t & number)
{
	unsigned char * num_ar = reinterpret_cast<unsigned char *>(&number);
	swap(num_ar[0], num_ar[3]);
	swap(num_ar[1], num_ar[2]);
}


struct MoHeader
{
	// magic number = 0x950412de
	uint32_t magic;
	// file format revision = 0
	uint32_t rev;
	// number of strings
	uint32_t N;
	// offset of table with original strings
	uint32_t O;
	// offset of table with translation strings
	uint32_t T;
	// there is a hash table afterwards, but we ignore it

	// Change the endianness of header data
	void swapEnd();
};


void MoHeader::swapEnd()
{
	swapInt(magic);
	swapInt(rev);
	swapInt(N);
	swapInt(O);
	swapInt(T);
}

struct StringTable
{
	// string length
	uint32_t length;
	// string offset
	uint32_t offset;

	// Change the endianness of string stable data
	void swapEnd();
};


void StringTable::swapEnd()
{
	swapInt(length);
	swapInt(offset);
}


} // namespace anon

bool Messages::readMoFile()
{
	// FIXME:remove
	if (lang_.empty()) {
		LYXERR0("No language given, nothing to load.");
		return false;
	}

	string const code = realCode(lang_);
	if (code.empty()) {
		LYXERR(Debug::LOCALE, "Cannot find translation for language " << lang_);
		return false;
	}

	string const filen = package().messages_file(code).toSafeFilesystemEncoding();

	// get file size
	struct stat buf;
	if (stat(filen.c_str(), &buf)) {
		LYXERR0("Cannot get information for file " << filen);
		return false;
	}

	vector<char> moData(buf.st_size);

	ifstream is(filen.c_str(), ios::in | ios::binary);
	if (!is.read(&moData[0], buf.st_size)) {
		LYXERR0("Cannot read file " << filen);
		return false;
	}

	MoHeader * header = reinterpret_cast<MoHeader *>(&moData[0]);

	bool doSwap = false;
	if (header->magic == 0xde120495) {
		header->swapEnd();
		doSwap = true;
	}

	if (header->magic != 0x950412de) {
		LYXERR0("Wrong magic number for file " << filen
			<< ".\nExpected 0x950412de, got 0x" << std::hex << header->magic);
		return false;
	}

	StringTable * orig = reinterpret_cast<StringTable *>(&moData[0] + header->O);
	StringTable * trans = reinterpret_cast<StringTable *>(&moData[0] + header->T);
	// First the header
	if (doSwap) {
		// Handle endiannness change
		orig[0].swapEnd();
		trans[0].swapEnd();
	}
	string const info = string(&moData[0] + trans[0].offset, trans[0].length);
	size_t pos = info.find("charset=");
	if (pos != string::npos) {
		pos += 8;
		string charset;
		size_t pos2 = info.find("\n", pos);
		if (pos2 == string::npos)
			charset = info.substr(pos);
		else
			charset = info.substr(pos, pos2 - pos);
		charset = ascii_lowercase(trim(charset));
		if (charset != "utf-8") {
			LYXERR0("Wrong encoding " << charset << " for file " << filen);
			return false;
		}
	} else {
		LYXERR0("Cannot find encoding encoding for file " << filen);
		return false;
	}

	for (size_t i = 1; i < header->N; ++i) {
		if (doSwap) {
			// Handle endiannness change
			orig[i].swapEnd();
			trans[i].swapEnd();
		}
		// Note that in theory the strings may contain NUL characters.
		// This may be the case with plural forms
		string const ostr(&moData[0] + orig[i].offset, orig[i].length);
		docstring tstr = from_utf8(string(&moData[0] + trans[i].offset,
						  trans[i].length));
		cleanTranslation(tstr);
		trans_map_[ostr] = tstr;
		//lyxerr << ostr << " ==> " << tstr << endl;
	}

	return true;
}

docstring const Messages::get(string const & m) const
{
	if (m.empty())
		return docstring();

	TranslationMap::const_iterator it = trans_map_.find(m);
	if (it != trans_map_.end())
		return it->second;
	else {
		docstring res = from_utf8(m);
		cleanTranslation(res);
		return res;
	}
}

} // namespace lyx

#else // ENABLE_NLS
// This is the dummy variant.

namespace lyx {

std::string Messages::gui_lang_;

Messages::Messages(string const & /* l */) {}

docstring const Messages::get(string const & m) const
{
	docstring trans = from_ascii(m);
	cleanTranslation(trans);
	return trans;
}

std::string Messages::language() const
{
	return string();
}

bool Messages::available(string const & /* c */)
{
	return false;
}

} // namespace lyx

#endif
