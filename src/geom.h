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

typedef double vec2d[2];

class Point{
 public:
	double pt[2];
	
	Point ccw(Point a) {
		Point r;
		for(int i = 0; i < 2; i++) {
			double t = pt[1-i];
			r.pt[i] = i?-t:t;
		}
		return r;
	}

	Point cw(Point a) {
		Point r;
		r.pt[0] = -pt[1];
		r.pt[1] = pt[0];
		return r;
	}
	
	double L2();
/** Compute the L2 or euclidean norm of this vector */
};

typedef struct mat2 {
	float     xx,xy,yx,yy;
} mat2;

Point operator+(Point a, Point b) {
	Point r;
	for(int i = 0; i < 2; i++)
		r.pt[i] = a.pt[i]+b.pt[i];
	return r;
}

Point operator-(Point a, Point b) {
	Point r;
	for(int i = 0; i < 2; i++)
		r.pt[i] = a.pt[i]-b.pt[i];
	return r;
}

Point operator*(Point a, Point b) {
	Point r;
	for(int i = 0; i < 2; i++)
		r.pt[i] = a.pt[i]*b.pt[i];
	return r;
}

Point operator*(double s, Point b) {
	Point r;
	for(int i = 0; i < 2; i++)
		r.pt[i] = s*b.pt[i];
	return r;
}

inline double dot(Point a, Point b) {
	double d = 0;
	for(int i = 0; i < 2; i++)
		d += a.pt[i]*b.pt[i];
	
	return d;
}

inline double cross(Point a, Point b) {
	double d = 0;
	for(int i = 0; i < 2; i++)
		d = d - a.pt[i]*b.pt[1-i];
	return d;
}

Point::Normalize() {
	d = L2();
	if(d > 0.0001)
		*this = d**this;
}

#define L_VEC_AddMul(a,b,c,r) { \
	r.x=a.x+b.x*c.x; \
		r.y=a.y+b.y*c.y; \
}

#define L_VEC_SubMul(a,b,c,r) { \
	r.x=a.x-b.x*c.x; \
		r.y=a.y-b.y*c.y; \
}


#define L_VEC_Cmp(a,b) ((fabs(a.y-b.y)<0.0000001)? \
												((fabs(a.x-b.x)<0.0000001)?0:((a.x > b.x)?1:-1)): \
												((a.y > b.y)?1:-1)) 
	
#define L_VAL_Cmp(a,b) ((fabs(a-b)<0.0000001)?0:((a>b)?1:-1)) 

#define L_VEC_Normalize(d) { \
	double l=sqrt(d.x*d.x+d.y*d.y); \
		if ( l < 0.00000001 ) { \
			d.x=d.y=0; \
		} else { \
			d.x/=l; \
				d.y/=l; \
		} \
}

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
