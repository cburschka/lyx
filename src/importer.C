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

#include "bufferlist.h"
#include "support/filetools.h"
#include "frontends/Alert.h"
#include "gettext.h"
#include "BufferView.h"


using namespace lyx::support;

using std::vector;
using std::find;


extern BufferList bufferlist;
extern void InsertAsciiFile(BufferView *, string const &, bool);


bool Importer::Import(LyXView * lv, string const & filename,
		      string const & format)
{
	string const displaypath = MakeDisplayPath(filename);
	lv->message(bformat(_("Importing %1$s..."), displaypath));

	string const lyxfile = ChangeExtension(filename, ".lyx");

	string loader_format;
	vector<string> loaders = Loaders();
	if (find(loaders.begin(), loaders.end(), format) == loaders.end()) {
		for (vector<string>::const_iterator it = loaders.begin();
		     it != loaders.end(); ++it) {
			if (converters.isReachable(format, *it)) {
				if (!converters.convert(0, filename, filename,
							format, *it))
					return false;
				loader_format = *it;
				break;
			}
		}
		if (loader_format.empty()) {
			Alert::error(_("Couldn't import file"),
				     bformat(_("No information for importing the format %1$s."),
				         formats.prettyName(format)));
			return false;
		}
	} else
		loader_format = format;


	if (loader_format == "lyx") {
		lv->view()->loadLyXFile(lyxfile);
	} else {
		lv->view()->newFile(lyxfile, string(), true);
		bool as_paragraphs = loader_format == "textparagraph";
		string filename2 = (loader_format == format) ? filename
			: ChangeExtension(filename,
					  formats.extension(loader_format));
		InsertAsciiFile(lv->view().get(), filename2, as_paragraphs);
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
