#ifndef __TILEDIALOG_H__
#define __TILEDIALOG_H__
/*
 * A simple dialog for creating grid type arrangements of selected objects
 *
 * Authors:
 *   Bob Jamison ( based off trace dialog)
 *   John Cliff
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004 Bob Jamison
 * Copyright (C) 2004 John Cliff
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

namespace Inkscape {
namespace UI {
namespace Dialogs {


/**
 * A dialog that displays log messages
 */
class TileDialog
{

public:


    /**
     * Constructor
     */
    TileDialog() {  };


    /**
     * Factory method
     */
    static TileDialog *create();

    /**
     * Destructor
     */
    virtual ~TileDialog() {};


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
    static TileDialog *getInstance();

    /**
     * Show the instance above
     */
    static void showInstance();

};


} //namespace Dialogs
} //namespace UI
} //namespace Inkscape




#endif /* __TILEDIALOG_H__ */

