/**
 * \file importer.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "importer.h"
#include "converter.h"
#include "format.h"
#include "frontends/LyXView.h"
#include "funcrequest.h"
#include "lyx_cb.h"

#include "support/filetools.h"

#include "frontends/Alert.h"

#include "gettext.h"
#include "BufferView.h"
#include "buffer_funcs.h"


namespace lyx {

using support::bformat;
using support::changeExtension;
using support::makeDisplayPath;

using std::find;
using std::string;
using std::vector;


bool Importer::Import(LyXView * lv, string const & filename,
		      string const & format, ErrorList & errorList)
{
	docstring const displaypath = makeDisplayPath(filename);
	lv->message(bformat(_("Importing %1$s..."), displaypath));

	string const lyxfile = changeExtension(filename, ".lyx");

	string loader_format;
	vector<string> loaders = Loaders();
	if (find(loaders.begin(), loaders.end(), format) == loaders.end()) {
		for (vector<string>::const_iterator it = loaders.begin();
		     it != loaders.end(); ++it) {
			if (converters.isReachable(format, *it)) {
				string const tofile =
					changeExtension(filename,
						formats.extension(*it));
				if (!converters.convert(0, filename, tofile,
							filename, format, *it, errorList))
					return false;
				loader_format = *it;
				break;
			}
		}
		if (loader_format.empty()) {
			frontend::Alert::error(_("Couldn't import file"),
				     bformat(_("No information for importing the format %1$s."),
					 formats.prettyName(format)));
			return false;
		}
	} else {
		loader_format = format;
	}


	if (loader_format == "lyx") {
		lv->loadLyXFile(lyxfile);
	} else {
		Buffer * const b = newFile(lyxfile, string(), true);
		if (b)
			lv->setBuffer(b);
		else
			return false;
		bool as_paragraphs = loader_format == "textparagraph";
		string filename2 = (loader_format == format) ? filename
			: changeExtension(filename,
					  formats.extension(loader_format));
		insertAsciiFile(lv->view(), filename2, as_paragraphs);
		lv->dispatch(FuncRequest(LFUN_MARK_OFF));
	}

	// we are done
	lv->message(_("imported."));
	return true;
}


vector<Format const *> const Importer::GetImportableFormats()
{
	vector<string> loaders = Loaders();
	vector<Format const *> result =
		converters.getReachableTo(loaders[0], true);
	for (vector<string>::const_iterator it = loaders.begin() + 1;
	     it != loaders.end(); ++it) {
		vector<Format const *> r =
			converters.getReachableTo(*it, false);
		result.insert(result.end(), r.begin(), r.end());
	}
	return result;
}


vector<string> const Importer::Loaders()
{
	vector<string> v;
	v.push_back("lyx");
	v.push_back("text");
	v.push_back("textparagraph");
	return v;
}


} // namespace lyx
