/*
 * This is what gets executed to initialize all of the modules.  For
 * the internal modules this invovles executing their initialization
 * functions, for external ones it involves reading their .spmodule
 * files and bringing them into Sodipodi.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2003 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __MODULES_INIT_H__
#define __MODULES_INIT_H__


void sp_modules_init (void);


#endif /* SP_MODULES_INIT_H__ */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
