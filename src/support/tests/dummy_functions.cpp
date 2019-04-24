#include <config.h>

#include "LyXRC.h"

#include "support/Messages.h"

using namespace std;

namespace lyx {
	// Dummy verbose support
	bool verbose = false;

	// Dummy LyXRC support
	LyXRC lyxrc;

	// Keep the linker happy on Windows
	void lyx_exit(int) {}

	// Dummy language support
	Messages const & getGuiMessages()
	{
		static Messages lyx_messages;

		return lyx_messages;
	}
	Messages const & getMessages(string const &)
	{
		static Messages lyx_messages;

		return lyx_messages;
	}

} // namespace lyx
