#ifndef __SCRIPTDIALOG_H__
#define __SCRIPTDIALOG_H__
/*
 * This dialog is for launching scripts whose main purpose if
 * the scripting of Inkscape itself.
 *
 * Authors:
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004 The Inkscape Organization
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */



namespace Inkscape {
namespace UI {
namespace Dialogs {


/**
 * A script editor, loader, and executor
 */
class ScriptDialog
{

    public:
    

    /**
     * Constructor
     */
    ScriptDialog() {};


    /**
     * Factory method
     */
    static ScriptDialog *create();

    /**
     * Destructor
     */
    virtual ~ScriptDialog() {};


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
    static ScriptDialog *getInstance();

    /**
     * Show the instance above
     */
    static void showInstance();


    

}; // class ScriptDialog


} //namespace Dialogs
} //namespace UI
} //namespace Inkscape




#endif /* __DEBUGDIALOG_H__ */

