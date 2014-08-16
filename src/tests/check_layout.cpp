#include <config.h>

#include "../support/FileName.h"
#include "../support/filetools.h"
#include "../Format.h"
#include "../LayoutFile.h"
#include "../LaTeXFeatures.h"
#include "../Lexer.h"
#include "../support/Messages.h"
#include "../support/os.h"
#include "../support/Package.h"

#include <cstdlib>
#include <iostream>


namespace lyx {
namespace frontend {
namespace Alert {
void warning(docstring const & title, docstring const & message, bool const &)
{
	LYXERR0(title);
	LYXERR0(message);
}
}
}

bool LaTeXFeatures::isAvailable(std::string const &)
{
	return false;
}

Formats formats;
bool Formats::isZippedFile(support::FileName const &) const
{
	return false;
}
}


using namespace lyx::support;
using namespace lyx;

using namespace std;


bool test_Layout(string const & input, string const & output)
{
	FileName const ifn(makeAbsPath(input));
    LayoutFileList & l = LayoutFileList::get();
	LayoutFileIndex i = l.addLocalLayout(ifn.onlyFileName(), ifn.onlyPath().absFileName());
	if (i.empty()) {
		cerr << "Could not read layout file " << input << ".layout.\n";
		return false;
	}
	LayoutFile const & f = l[i];
	ostream * os = NULL;
	if (output == "-")
		os = &cout;
	else if (!output.empty())
		os = new ofstream(output.c_str());
	bool success = true;
	for (TextClass::const_iterator it = f.begin(); it != f.end(); ++it) {
		if (os)
			it->write(*os);
		ostringstream oss;
		it->write(oss);
		istringstream iss(oss.str());
		// swallow "Style Standard" line
		string line;
		if (getline(iss, line)) {
			Lexer lex;
			lex.setStream(iss);
			Layout test;
			test.setName(it->name());
			if (test.read(lex, f)) {
				// Caution: operator==() is incomplete!
				// Testing test == *it does not make much sense
				// therefore.
				// It does not work either for styles with
				// non-empty obsoletedby if the obsoletedby
				// style was modified after the obsolete style
				// was defined: Layout::write() writes only the
				// final version, but the obsolete style was
				// set to the version at the time it was
				// defined, and therefore they differ. See e.g.
				// the obsolete style AMS which is replaced by
				// Subjectclass in amsdefs.inc, and
				// Subjectclass is modified later in siamltex.layout.
				ostringstream osstest;
				test.write(osstest);
				if (osstest.str() != oss.str()) {
					cerr << "Round trip for style "
					     << to_utf8(it->name())
					     << " failed:\n"
					     << osstest.str() << oss.str();
					success = false;
				}
			} else {
				cerr << "Could not read style "
				     << to_utf8(it->name()) << ".\n";
				success = false;
			}
		} else {
			cerr << "Could not read first line for style "
			     << to_utf8(it->name()) << ".\n";
			success = false;
		}
	}
	if (output != "-")
		delete os;
	return success;
}


int main(int argc, char * argv[])
{
	os::init(argc, argv);
	lyxerr.setStream(cerr);
	if (argc < 2 || argc > 3) {
		cerr << "Usage: " << argv[0] << " <input layout file> [<output layout file>]\n";
		return EXIT_FAILURE;
	}
	FileName const exe(makeAbsPath(os::internal_path(os::utf8_argv(0))));
	string const lyxexe = addName(exe.onlyPath().absFileName(), "lyx");
	init_package(lyxexe, string(), string());
	if (test_Layout(argv[1], argc > 2 ? argv[2] : ""))
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
}
