/*
 * Authors:
 *
 * Copyright (C) 2012 Authors
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include "crash-recovery.h"
#include "preferences.h"
#include "verbs.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

CrashRecovery &CrashRecovery::getInstance()
{
    return *new CrashRecovery();
}

CrashRecovery::CrashRecovery(gchar const *prefsPath) :
    Inkscape::UI::Widget::Panel("", prefsPath, SP_VERB_DIALOG_CRASH_RECOVERY, "", false)
{
}

CrashRecovery::~CrashRecovery()
{
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape







