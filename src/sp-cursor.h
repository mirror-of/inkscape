#ifndef SP_CURSOR_H
#define SP_CURSOR_H

#include <gdk/gdk.h>

void sp_cursor_bitmap_and_mask_from_xpm (GdkBitmap **bitmap, GdkBitmap **mask, gchar **xpm);
GdkCursor * sp_cursor_new_from_xpm (gchar ** xpm, gint hot_x, gint hot_y);

#endif
