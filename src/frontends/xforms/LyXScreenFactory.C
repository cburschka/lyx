/**
 * \file LyXScreenFactory.C
 * Read the file COPYING
 *
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>
 
#include "frontends/LyXScreenFactory.h"

#include "XWorkArea.h"
#include "xscreen.h"
 
namespace LyXScreenFactory {

LyXScreen * create(WorkArea & owner)
{
	return new XScreen(static_cast<XWorkArea &>(owner));
} 
 
}
