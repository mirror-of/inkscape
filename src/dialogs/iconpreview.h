#ifndef SEEN_ICON_PREVIEW_H
#define SEEN_ICON_PREVIEW_H
/*
 * A simple dialog for previewing icon representation.
 *
 * Authors:
 *   Jon A. Cruz
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004,2005 The Inkscape Organization
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

namespace Inkscape {
namespace UI {
namespace Dialogs {


/**
 * A dialog that displays an icon preview
 */
class IconPreview
{

public:


    /**
     * Constructor
     */
    IconPreview() {};


    /**
     * Factory method
     */
    static IconPreview *create();

    /**
     * Destructor
     */
    virtual ~IconPreview() {};


    /**
     * Show the dialog
     */
    virtual void show() = 0;

    /**
     * Do not show the dialog
     */
    virtual void hide() = 0;

    /**
     * Get a shared singleton instance
     */
    static IconPreview *getInstance();

    /**
     * Show the instance above
     */
    static void showInstance();
};


} //namespace Dialogs
} //namespace UI
} //namespace Inkscape



#endif // SEEN_ICON_PREVIEW_H
