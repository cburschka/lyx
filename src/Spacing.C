#include <config.h>

#include <stdio.h>
#include "Spacing.h"

/// how can I put this inside of Spacing (class)
static
char const *spacing_string[] = {"single", "onehalf", "double", "other"};


void Spacing::writeFile(FILE *file)
{
	if (getSpace() == Spacing::Other) {
		fprintf(file, "\\spacing %s %.2f \n",
			spacing_string[getSpace()],
			getValue());
	} else {
		fprintf(file, "\\spacing %s \n",
			spacing_string[getSpace()]);
	}	
}
