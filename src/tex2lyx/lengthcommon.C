// This file is here sine the master lengthcommon.C contains gettext
// markers, and also that this file does not always exist (in the linked
// case). So it is possible for po/POTFILES.in to get out of sync.
// The compile will then fail. So to make sure that this file
//(tex2lyx/lengthcommon.C) is not present in POTFILES.in we do a
// include trick. (Lgb)
#include "../lengthcommon.C"
