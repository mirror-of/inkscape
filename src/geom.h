/*
 *  MyMath.h
 *  nlivarot
 *
 *  Created by fred on Wed Jun 18 2003.
 * moving to geom and C++ified by njh
 *
 */

#ifndef my_math
#define my_math

enum dimT { X, Y };

class Point{
 public:
	double pt[2];

	Point() {
	}

	Point(double x, double y) {
		pt[X] = x;
		pt[Y] = y;
	}

	/** Return a point like this point but rotated -90 degrees.
	    (If the y axis grows downwards and the x axis grows to the
	    right, then this is 90 degrees counter-clockwise.)
	 **/
	Point ccw() const {
		return Point(pt[Y], -pt[X]);
	}

	/** Return a point like this point but rotated +90 degrees.
	    (If the y axis grows downwards and the x axis grows to the
	    right, then this is 90 degrees clockwise.)
	 **/
	Point cw() {
		return Point(-pt[Y], pt[X]);
	}

	double L2();
/** Compute the L2 or euclidean norm of this vector */
};

Point
operator+(Point const &a, Point const &b) {
	Point r;
	for(int i = 0; i < 2; i++) {
		r.pt[i] = a.pt[i] + b.pt[i];
	}
	return r;
}

Point
operator-(Point const &a, Point const &b) {
	Point r;
	for(int i = 0; i < 2; i++) {
		r.pt[i] = a.pt[i] - b.pt[i];
	}
	return r;
}

Point
operator*(Point const &a, Point const &b) {
	Point r;
	for(int i = 0; i < 2; i++)
		r.pt[i] = a.pt[i]*b.pt[i];
	return r;
}

Point
operator*(double s, Point const &b) {
	Point ret;
	for(int i = 0; i < 2; i++) {
		ret.pt[i] = s * b.pt[i];
	}
	return ret;
}

inline double
dot(Point const &a, Point const &b) {
	double ret = 0;
	for(int i = 0; i < 2; i++) {
		ret += a.pt[i] * b.pt[i];
	}
	return ret;
}

inline double
cross(Point const &a, Point const &b) {
	double ret = 0;
	for(int i = 0; i < 2; i++)
		ret -= a.pt[i] * b.pt[1-i];
	return ret;
}

/* FIXME: What's this?  Is it intended to be yet another sp_vector_normalize variant?
   If so, shouldn't we be dividing by the length instead of multiplying? */
Point::Normalize() {
	d = L2();
	if(d > 0.0001)
		*this = d**this;
}


typedef double vec2d[2];

typedef struct mat2 {
	float     xx,xy,yx,yy;
} mat2;

#define L_VEC_AddMul(a,b,c,r) do { \
	r.x=a.x+b.x*c.x; \
		r.y=a.y+b.y*c.y; \
} while(0)

#define L_VEC_SubMul(a,b,c,r) do { \
	r.x=a.x-b.x*c.x; \
		r.y=a.y-b.y*c.y; \
} while(0)


#define L_VEC_Cmp(a,b) ((fabs(a.y - b.y) < 1e-7) \
			? ((fabs(a.x - b.x) < 1e-7)	\
			   ? 0		\
			   : ((a.x > b.x)	\
			      ? 1	\
			      : -1))	\
			: ((a.y > b.y)	\
			   ? 1		\
			   : -1))

#define L_VAL_Cmp(a,b) ((fabs(a - b) < 1e-7)	\
			? 0		\
			:((a > b)	\
			  ? 1		\
			  : -1))

/** See also sp_vector_normalize. */
#define L_VEC_Normalize(d) do { \
	double len = sqrt(d.x * d.x + d.y * d.y); \
	if (len < 0.00000001) { \
		d.x = d.y = 0; \
	} else { \
		d.x /= len; \
		d.y /= len; \
	} \
} while(0)

#define L_VEC_Distance(a,b,d) { \
	d=sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y)); \
}


#define L_VAL_Zero(a) ((fabs(a)<0.00000001)?0:((a>0)?1:-1)) 

#define L_VEC_Cross(a,b,r) { \
	r=a.x*b.x+a.y*b.y; \
}

#define L_VEC_Dot(a,b,r) { \
	r=a.x*b.y-a.y*b.x; \
}


#define	L_MAT(m,a,b) {c[0][0].Set(ica.x);c[0][1].Set(icb.x);c[1][0].Set(ica.y);c[1][1].Set(icb.y);};

#define	L_MAT_Set(m,a00,a10,a01,a11) {m.xx=a00;m.xy=a01;m.yx=a10;m.yy=a11;};

#define L_MAT_SetC(m,a,b) {m.xx=a.x;m.xy=b.x;m.yx=a.y;m.yy=b.y;};

#define L_MAT_SetL(m,a,b) {m.xx=a.x;m.xy=a.y;m.yx=b.x;m.yy=b.y;};

#define L_MAT_Init(m) {m.xx=m.xy=m.yx=m.yy=0;};
	
#define L_MAT_Col(m,no,r) { \
		if ( no == 0 ) { \
			r.x=m.xx; \
			r.y=m.yx; \
		} \
		if ( no == 0 ) { \
			r.x=m.xy; \
			r.y=m.yy; \
		} \
	}; 

#define L_MAT_Row(m,no,r) { \
		if ( no == 0 ) { \
			r.x=m.xx; \
			r.y=m.xy; \
		} \
		if ( no == 0 ) { \
			r.x=m.yx; \
			r.y=m.yy; \
		} \
	};
	
#define L_MAT_Det(m,d) {d=m.xx*m.yy-m.xy*m.yx;};
	
#define L_MAT_Neg(m) {m.xx=-m.xx;m.xy=-m.xy;m.yx=-m.yx;m.yy=-m.yy;};

#define L_MAT_Trs(m) {double t=m.xy;m.xy=m.yx;m.yx=t;};

#define L_MAT_Inv(m) { \
	double d; \
	L_MAT_Det(m,d); \
	m.yx=-m.yx; \
	m.xy=-m.xy; \
	double t=m.xx;m.xx=m.yy;m.yy=t; \
	m.xx/=d; \
	m.xy/=d; \
	m.yx/=d; \
	m.yy/=d; \
};

#define L_MAT_Cof(m) { \
			m.yx=-m.yx; \
				m.xy=-m.xy; \
					double t=m.xx;m.xx=m.yy;m.yy=t; \
};

#define L_MAT_Add(u,v,m) {m.xx=u.xx+v.xx;m.xy=u.xy+v.xy;m.yx=u.yx+v.yx;m.yy=u.yy+v.yy;};

#define L_MAT_Sub(u,v,m) {m.xx=u.xx-v.xx;m.xy=u.xy-v.xy;m.yx=u.yx-v.yx;m.yy=u.yy-v.yy;};

#define L_MAT_Mul(u,v,m) { \
	mat2d r; \
	r.xx=u.xx*v.xx+u.xy*v.yx; \
	r.yx=u.yx*v.xx+u.yy*y.yx; \
	r.xy=u.xx*v.xy+u.xy*v.yy; \
	r.yy=u.yx*v.xy+u.yy*v.yy; \
	m=r; \
}

#define L_MAT_MulC(u,v,m) {m.xx=u.xx*v;m.xy=u.xy*v;m.yx=u.yx*v;m.yy=u.yy*v;};

#define L_MAT_DivC(u,v,m) {m.xx=u.xx/v;m.xy=u.xy/v;m.yx=u.yx/v;m.yy=u.yy/v;};
	
#define L_MAT_MulV(m,v,r) { \
	vec2d t; \
	t.x=m.xx*v.x+m.xy*v.y; \
	t.y=m.yx*v.x+m.yy*v.y; \
	r=t; \
};

#define L_MAT_TMulV(m,v,r) { \
	vec2d t; \
	t.x=m.xx*v.x+m.yx*v.y; \
	t.y=m.xy*v.x+m.yy*v.y; \
	r=t; \
};

#endif
