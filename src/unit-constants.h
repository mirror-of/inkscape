#ifndef INKSCAPE_UNIT_CONSTANTS_H
#define INKSCAPE_UNIT_CONSTANTS_H

#define PT_PER_IN 72.0
#define PX_PER_IN 90.0 // yet another place where the 90dpi is hard-coded
#define M_PER_IN 0.0254
#define CM_PER_IN 2.54
#define MM_PER_IN 25.4
#define MM_PER_MM 1.0
#define MM_PER_CM 10.0
#define MM_PER_M 1000.0
#define IN_PER_PT (1 / PT_PER_IN)
#define IN_PER_CM (1 / CM_PER_IN)
#define IN_PER_MM (1 / MM_PER_IN)
#define PT_PER_CM (PT_PER_IN / CM_PER_IN)
#define M_PER_PT (M_PER_IN / PT_PER_IN)
#define PT_PER_M (PT_PER_IN / M_PER_IN)
#define CM_PER_PT (CM_PER_IN / PT_PER_IN)
#define MM_PER_PT (MM_PER_IN / PT_PER_IN)
#define PT_PER_MM (PT_PER_IN / MM_PER_IN)
#define MM_PER_PX (MM_PER_IN / PX_PER_IN)
#define PT_PER_PT 1.0
#define PT_PER_PX 0.8 // yet another place where the 90dpi is hard-coded
#define IN_PER_IN 1.0

#endif /* !INKSCAPE_UNIT_CONSTANTS_H */
