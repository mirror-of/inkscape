Radial::Radial(NR::Point const &p)
{
	r = NR::L2(p);
	if (r > 0) {
		a = NR::atan2 (p);
	} else {
		a = HUGE_VAL; //undefined
	}
}

Radial::operator NR::Point() const
{
	if (a == HUGE_VAL) {
		return NR::Point(0,0);
	} else {
		return r*NR::Point(cos(a), sin(a));
	}
}
