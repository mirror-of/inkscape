#define __SP_METADATA_C__

/*
 * SVG <metadata> implementation
 *
 * Authors:
 *   Kees Cook <kees@outflux.net>
 *
 * Copyright (C) 2004 Kees Cook
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "attributes.h"
#include "sp-metadata.h"
#include "sp-item-group.h"
#include "xml/repr.h"
#include "document.h"

#undef DEBUG_METADATA
#ifdef DEBUG_METADATA
# define debug(f, a...) { g_print("%s(%d) %s:", \
                                  __FILE__,__LINE__,__FUNCTION__); \
                          g_print(f, ## a); \
                        }
#else
# define debug(f, a...) /**/
#endif

/* Metadata base class */

static void sp_metadata_class_init (SPMetadataClass *klass);
static void sp_metadata_init (SPMetadata *metadata);

static void sp_metadata_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_metadata_release (SPObject *object);
static void sp_metadata_set (SPObject *object, unsigned int key, const gchar *value);
static void sp_metadata_update(SPObject *object, SPCtx *ctx, guint flags);
static SPRepr *sp_metadata_write(SPObject *object, SPRepr *repr, guint flags);

static SPObjectClass *metadata_parent_class;

GType
sp_metadata_get_type (void)
{
    static GType metadata_type = 0;

    if (!metadata_type) {
        GTypeInfo metadata_info = {
            sizeof (SPMetadataClass),
            NULL, NULL,
            (GClassInitFunc) sp_metadata_class_init,
            NULL, NULL,
            sizeof (SPMetadata),
            16,
            (GInstanceInitFunc) sp_metadata_init,
            NULL,    /* value_table */
        };
        metadata_type = g_type_register_static (SP_TYPE_OBJECT, "SPMetadata", &metadata_info, (GTypeFlags)0);
    }
    return metadata_type;
}

static void
sp_metadata_class_init (SPMetadataClass *klass)
{
    //GObjectClass *gobject_class = (GObjectClass *)klass;
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;

    metadata_parent_class = (SPObjectClass*)g_type_class_peek_parent (klass);

    sp_object_class->build = sp_metadata_build;
    sp_object_class->release = sp_metadata_release;
    sp_object_class->write = sp_metadata_write;
    sp_object_class->set = sp_metadata_set;
    sp_object_class->update = sp_metadata_update;
}

static void
sp_metadata_init (SPMetadata *metadata)
{
    debug("%s: 0x%08x",__FUNCTION__,(unsigned int)metadata);
}

/*
 * \brief Reads the SPRepr, and initializes SPMetadata variables.
 *        For this to get called, our name must be associated with
 *        a repr via "sp_object_type_register".  Best done through
 *        sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_metadata_build (SPObject *object, SPDocument *document, SPRepr *repr)
{
    debug("%s: 0x%08x",__FUNCTION__,(unsigned int)object);
    if (((SPObjectClass *) metadata_parent_class)->build)
        ((SPObjectClass *) metadata_parent_class)->build (object, document, repr);

    /*
    sp_object_read_attr (object, "xlink:href");
    sp_object_read_attr (object, "attributeName");
    sp_object_read_attr (object, "attributeType");
    sp_object_read_attr (object, "begin");
    sp_object_read_attr (object, "dur");
    sp_object_read_attr (object, "end");
    sp_object_read_attr (object, "min");
    sp_object_read_attr (object, "max");
    sp_object_read_attr (object, "restart");
    sp_object_read_attr (object, "repeatCount");
    sp_object_read_attr (object, "repeatDur");
    sp_object_read_attr (object, "fill");
    */
    
}

/*
 * \brief Drops any allocated memory
 */
static void
sp_metadata_release (SPObject *object)
{
    debug("%s: 0x%08x",__FUNCTION__,(unsigned int)object);
}

/*
 * \brief Sets a specific value in the SPMetadata
 */
static void
sp_metadata_set (SPObject *object, unsigned int key, const gchar *value)
{
    debug("%s: 0x%08x",__FUNCTION__,(unsigned int)object);
    SPMetadata * metadata;

    metadata = SP_METADATA (object);

    /* see if any parents need this value */
    if (((SPObjectClass *) metadata_parent_class)->set)
        ((SPObjectClass *) metadata_parent_class)->set (object, key, value);
}

/*
 * \brief Receives update notifications
 */
static void
sp_metadata_update(SPObject *object, SPCtx *ctx, guint flags)
{
    debug("%s: 0x%08x",__FUNCTION__,(unsigned int)object);
    //SPMetadata *metadata = SP_METADATA(object);

    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something? */

    }

    if (((SPObjectClass *) metadata_parent_class)->update)
        ((SPObjectClass *) metadata_parent_class)->update(object, ctx, flags);
}

/*
 * \brief Writes it's settings to an incoming repr object, if any
 */
static SPRepr *
sp_metadata_write(SPObject *object, SPRepr *repr, guint flags)
{
    debug("%s: 0x%08x",__FUNCTION__,(unsigned int)object);
    //SPMetadata *metadata = SP_METADATA(object);

    if (flags & SP_OBJECT_WRITE_EXT) {
        if (repr) {
            // is this sane?
            sp_repr_merge (repr, SP_OBJECT_REPR (object), "id");
        } else {
            repr = sp_repr_duplicate (SP_OBJECT_REPR (object));
        }
    }

    if (((SPObjectClass *) metadata_parent_class)->write)
        ((SPObjectClass *) metadata_parent_class)->write(object, repr, flags);

    return repr;
}

/*
 * \brief Retrieves the metadata object associated with a document
 */
SPMetadata *
sp_document_metadata (SPDocument *document)
{
    SPObject *nv;

    g_return_val_if_fail (document != NULL, NULL);
    g_return_val_if_fail (SP_IS_DOCUMENT (document), NULL);

    nv = sp_item_group_get_child_by_name ((SPGroup *) document->root, NULL,
                                        "metadata");
    g_assert (nv != NULL);

    return (SPMetadata *)nv;
}


// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
