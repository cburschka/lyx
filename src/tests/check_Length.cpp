#include <config.h>

#include "../Length.h"
#include "../LyXRC.h"
#include "../support/debug.h"

#include <iostream>


using namespace lyx;
using namespace std;


void test_inPixels()
{
	// want to see non-zero SP
	lyxrc.zoom = 100000;
	lyxrc.dpi = 72;
	for (int i = Length::BP; i <= Length::UNIT_NONE; ++i) {
		Length const l(2342, static_cast<Length::UNIT>(i));
		cout << l.inPixels(250) << endl;
	}
}


void test_inBP()
{
	for (int i = Length::BP; i <= Length::UNIT_NONE; ++i) {
		Length const l(2342, static_cast<Length::UNIT>(i));
		cout << l.inBP() << endl;
	}
}


int main(int, char **)
{
	// Connect lyxerr with cout instead of cerr to catch error output
	lyx::lyxerr.setStream(cout);
	test_inPixels();
	test_inBP();
}
