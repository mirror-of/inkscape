/*! \file
 *
 * Since Cython does not support namespace for function and struct, we
 * use macro to define alias for these functions and structs for
 * Verbs.
 */
#ifndef __PYB_VERB_H_
#define __PYB_VERB_H_

#include "verbs.h"
#include "helper/action.h"

#define pyb_verb_getbyid Inkscape::Verb::getbyid

#endif /* __PYB_VERB_H_ */
