#include <config.h>

#include <cerrno>
#include <unistd.h>

#include "support/lyxlib.h"

static inline
char * l_getcwd(char * buffer, size_t size)
{
#ifndef __EMX__
	return ::getcwd(buffer, size);
#else
	return ::_getcwd2(buffer, size);
#endif
}


// Returns current working directory
string const lyx::getcwd()
{
  	int n = 256;	// Assume path is less than 256 chars
	char * err;
  	char * tbuf = new char[n];
  	
  	// Safe. Hopefully all getcwds behave this way!
  	while (((err = l_getcwd(tbuf, n)) == 0) && (errno == ERANGE)) {
		// Buffer too small, double the buffersize and try again
    		delete[] tbuf;
    		n = 2 * n;
    		tbuf = new char[n];
  	}

	string result;
	if (err) result = tbuf;
	delete[] tbuf;
	return result;
}
