#ifndef __NR_TYPE_POS_DEF_H__
#define __NR_TYPE_POS_DEF_H__

class NRTypePosDef {
public:
	unsigned int italic : 1;
	unsigned int oblique : 1;
	unsigned int weight : 8;
	unsigned int stretch : 8;
	/* These can probably be made sensible sizes rather than bitfields; for the moment we'll
	   keep the old definition. */

public:
	NRTypePosDef(char const *description);

};

/* Constants extracted mechanically from string constructor code. */
#define NR_POS_WEIGHT_THIN		 32
#define NR_POS_WEIGHT_EXTRA_LIGHT	 64
#define NR_POS_WEIGHT_ULTRA_LIGHT	 64
#define NR_POS_WEIGHT_LIGHT		 96
#define NR_POS_WEIGHT_BOOK		128
#define NR_POS_WEIGHT_NORMAL		128
#define NR_POS_WEIGHT_MEDIUM		144
#define NR_POS_WEIGHT_SEMI_BOLD		160
#define NR_POS_WEIGHT_SEMIBOLD		160
#define NR_POS_WEIGHT_DEMI_BOLD		160
#define NR_POS_WEIGHT_DEMIBOLD		160
#define NR_POS_WEIGHT_BOLD		192
#define NR_POS_WEIGHT_ULTRA_BOLD	224
#define NR_POS_WEIGHT_EXTRA_BOLD	224
#define NR_POS_WEIGHT_BLACK		255

#define NR_POS_STRETCH_NARROW		 64
#define NR_POS_STRETCH_CONDENSED	 64
#define NR_POS_STRETCH_NORMAL		128
#define NR_POS_STRETCH_WIDE		192

#endif /* __NR_TYPE_POS_DEF_H__ */
