#ifndef __NR_OBJECT_H__
#define __NR_OBJECT_H__

/*
 * RGBA display list system for sodipodi
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr_config.h>

typedef NRULong NRType;

#define NR_TYPE_OBJECT (nr_object_get_type ())
#define NR_OBJECT(o) (NR_CHECK_INSTANCE_CAST ((o), NR_TYPE_OBJECT, NRObject))
#define NR_IS_OBJECT(o) (NR_CHECK_INSTANCE_TYPE ((o), NR_TYPE_OBJECT))

typedef struct _NRObject NRObject;
typedef struct _NRObjectClass NRObjectClass;

#define NR_TYPE_ACTIVE_OBJECT (nr_active_object_get_type ())
#define NR_ACTIVE_OBJECT(o) (NR_CHECK_INSTANCE_CAST ((o), NR_TYPE_ACTIVE_OBJECT, NRActiveObject))
#define NR_IS_ACTIVE_OBJECT(o) (NR_CHECK_INSTANCE_TYPE ((o), NR_TYPE_ACTIVE_OBJECT))

typedef struct _NRActiveObject NRActiveObject;
typedef struct _NRActiveObjectClass NRActiveObjectClass;

#define nr_return_if_fail(expr) if (!(expr) && nr_emit_fail_warning (__FILE__, __LINE__, "?", #expr)) return
#define nr_return_val_if_fail(expr,val) if (!(expr) && nr_emit_fail_warning (__FILE__, __LINE__, "?", #expr)) return (val)

unsigned int nr_emit_fail_warning (const unsigned char *file, unsigned int line, const unsigned char *method, const unsigned char *expr);

#ifndef NR_DISABLE_CAST_CHECKS
#define NR_CHECK_INSTANCE_CAST(ip, tc, ct) ((ct *) nr_object_check_instance_cast (ip, tc))
#else
#define NR_CHECK_INSTANCE_CAST(ip, tc, ct) ((ct *) ip)
#endif

#define NR_CHECK_INSTANCE_TYPE(ip, tc) nr_object_check_instance_type (ip, tc)
#define NR_OBJECT_GET_CLASS(ip) (((NRObject *) ip)->klass)

NRType nr_type_is_a (NRType type, NRType test);

void *nr_object_check_instance_cast (void *ip, NRType tc);
unsigned int nr_object_check_instance_type (void *ip, NRType tc);

NRType nr_object_register_type (NRType parent,
				      unsigned char *name,
				      unsigned int csize,
				      unsigned int isize,
				      void (* cinit) (NRObjectClass *),
				      void (* iinit) (NRObject *));

/* NRObject */

struct _NRObject {
	NRObjectClass *klass;
	unsigned int refcount;
};

struct _NRObjectClass {
	NRType type;
	NRObjectClass *parent;

	unsigned char *name;
	unsigned int csize;
	unsigned int isize;
	void (* cinit) (NRObjectClass *);
	void (* iinit) (NRObject *);

	void (* finalize) (NRObject *object);
};

NRType nr_object_get_type (void);

/* Dynamic lifecycle */

NRObject *nr_object_new (NRType type);
NRObject *nr_object_delete (NRObject *object);

NRObject *nr_object_ref (NRObject *object);
NRObject *nr_object_unref (NRObject *object);

/* Automatic lifecycle */

NRObject *nr_object_setup (NRObject *object, NRType type);
NRObject *nr_object_release (NRObject *object);

/* NRActiveObject */

typedef struct _NRObjectListener NRObjectListener;
typedef struct _NRObjectCallbackBlock NRObjectCallbackBlock;
typedef struct _NRObjectEventVector NRObjectEventVector;

struct _NRObjectEventVector {
	void (* dispose) (NRObject *object, void *data);
};

struct _NRObjectListener {
	const NRObjectEventVector *vector;
	unsigned int size;
	void *data;
};

struct _NRObjectCallbackBlock {
	unsigned int size;
	unsigned int length;
	NRObjectListener listeners[1];
};

struct _NRActiveObject {
	NRObject object;
	NRObjectCallbackBlock *callbacks;
};

struct _NRActiveObjectClass {
	NRObjectClass parent_class;
};

NRType nr_active_object_get_type (void);

void nr_active_object_add_listener (NRActiveObject *object, const NRObjectEventVector *vector, unsigned int size, void *data);
void nr_active_object_remove_listener_by_data (NRActiveObject *object, void *data);

#endif

