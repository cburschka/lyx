/**
 * \file kill.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "lyxlib.h"

#include <sys/types.h>
#include <csignal>

int lyx::kill(int pid, int sig)
{
	return ::kill(pid, sig);
}
