// This file is a modeling file for coverity

namespace lyx {

// Tell coverity that this function always exits
void doAssertWithCallstack(bool value)
{
	if (!value) {
		 __coverity_panic__();
	}
}

}
