# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>

# This file is the counterpart of stdstruct.inc
# It is desirable, as far as possible, to have the same look and feel for
# related layouts in latex and docbook.

Format 85
Input stdstruct.inc

Style Abstract
	LatexType             Environment
	LatexName             abstract
	DocBookAbstract       true
	DocBookInInfo         always
	DocBookWrapperTag     abstract
	DocBookWrapperMergeWithPrevious    true
	DocBookTag            para
End

NoStyle Bibliography
