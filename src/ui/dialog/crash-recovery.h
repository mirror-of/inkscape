/** @file
 * @brief Crash Recovery dialog
 */
/* Authors:
 *   
 *
 * Copyright (C) 2013 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef SEEN_INKSCAPE_UI_DIALOG_CRASH_RECOVERY_H
#define SEEN_INKSCAPE_UI_DIALOG_CRASH_RECOVERY_H

#include <glibmm/ustring.h>
#include "ui/widget/panel.h"

class SPDocument;

namespace Inkscape {
namespace UI {
namespace Dialog {

class CrashRecovery : public UI::Widget::Panel {
public:
    CrashRecovery(gchar const *prefsPath = "/dialogs/crashrecovery");
    virtual ~CrashRecovery();

    /**
     * Helper function which returns a new instance of the dialog.
     * getInstance is needed by the dialog manager (Inkscape::UI::Dialog::DialogManager).
     */
    static CrashRecovery &getInstance();

protected:

private:
    CrashRecovery(CrashRecovery const &); // no copy
    CrashRecovery &operator=(CrashRecovery const &); // no assign
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // SEEN_INKSCAPE_UI_DIALOG_CRASH_RECOVERY_H
