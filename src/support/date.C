#include <config.h>

#include <time.h>

#include "support/lyxlib.h"

char * lyx::date()
{
	time_t tid;
	if ((tid = ::time(0)) == static_cast<time_t>(-1))
		return 0;
	else
		return ::ctime(&tid);
}
