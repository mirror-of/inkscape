/*
 * Code for handling extensions (i.e., scripts)
 *
 * Authors:
 *   Bryce Harrington <bryce@osdl.org>
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2003 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __MODULES_EXTENSION_H__
#define __MODULES_EXTENSION_H__

#include "module.h"

void          extension_load         (SPModule *module);
void          extension_unload       (SPModule *module);
GtkDialog *   extension_input_prefs  (SPModule * module,
                                      const guchar * filename);
SPDocument *  extension_open         (SPModule * module,
                                      const guchar * filename);
GtkDialog *   extension_output_prefs (SPModule * module);
void          extension_save         (SPModule * module,
                                      SPDocument * doc,
                                      const guchar * filename);
GtkDialog *   extension_filter_prefs (SPModule * module);
void          extension_filter       (SPModule * module,
                                      SPDocument * document);

#endif /* __MODULES_EXTENSION_H__ */
