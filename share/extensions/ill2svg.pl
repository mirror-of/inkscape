#!/usr/bin/perl

# convert an illustrator file (on stdin) to svg (on stdout)

$color = "#000";

sub cmyk_to_css {
    my ($c, $m, $y, $k) = @_;
    my ($r, $g, $b);

    $r = 1 - ($k + $c);
    if ($r < 0) { $r = 0; }
    $g = 1 - ($k + $m);
    if ($g < 0) { $g = 0; }
    $b = 1 - ($k + $y);
    if ($b < 0) { $b = 0; }
    return sprintf ("#%02x%02x%02x", 255 * $r, 255 * $g, 255 * $b);
}

sub nice_float {
    my ($x) = @_;

    my $result = sprintf ("%.3f", $x);
    $result =~ s/0*$//;
    $result =~ s/\.$//;
    return $result;
}

sub xform_xy {
    my ($x, $y) = @_;
    my @result = ();

    for my $i (0..$#_) {
	if ($i & 1) {
	    push @result, 1000 - $_[$i];
	} else {
	    push @result, $_[$i] - 100;
	}
    }
    return join ' ', map { nice_float ($_) } @result;
}

sub strokeparams {
    my $result = "stroke:$strokecolor";
    if ($strokewidth != 1) {
	$result .= "; stroke-width:$strokewidth";
    }
    return $result;
}

print "<svg>\n";
while (<>) {
    if (/^([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) k$/) {
	$fillcolor = cmyk_to_css ($1, $2, $3, $4);
    } elsif (/^([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) K$/) {
	$strokecolor = cmyk_to_css ($1, $2, $3, $4);
    } elsif (/^([\d\.]+) g$/) {
	$fillcolor = cmyk_to_css (0, 0, 0, 1 - $1);
    } elsif (/^([\d\.]+) G$/) {
	$strokecolor = cmyk_to_css (0, 0, 0, 1 - $1);
    } elsif (/^([\d\.]+) ([\d\.]+) m$/) {
	$path .= 'M'.xform_xy($1, $2);
	$cpx = $1;
	$cpy = $2;
    } elsif (/^([\d\.]+) ([\d\.]+) l$/i) {
	$path .= 'L'.xform_xy($1, $2);
	$cpx = $1;
	$cpy = $2;
    } elsif (/^([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) v$/i) {
	$path .= 'C'.xform_xy($cpx, $cpy, $1, $2, $3, $4);
	$cpx = $3;
	$cpy = $4;
    } elsif (/^([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) y$/i) {
	$path .= 'C'.xform_xy($1, $2, $3, $4, $3, $4);
	$cpx = $3;
	$cpy = $4;
    } elsif (/^([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) c$/i) {
	$path .= 'C'.xform_xy($1, $2, $3, $4, $5, $6);
	$cpx = $5;
	$cpy = $6;
    } elsif (/^b$/) {
	$path .= 'z';
	$strokeparams = strokeparams ();
	print " <g style=\"fill: $fillcolor; $strokeparams\">\n";
	print "  <path d=\"$path\"/>\n";
	print " </g>\n";
	$path = '';
    } elsif (/^B$/) {
	$strokeparams = strokeparams ();
	print " <g style=\"fill: $fillcolor; $strokeparams\">\n";
	print "  <path d=\"$path\"/>\n";
	print " </g>\n";
	$path = '';
    } elsif (/^f$/i) {
	$path .= 'z';
	print " <g style=\"fill: $fillcolor\">\n";
	print "  <path d=\"$path\"/>\n";
	print " </g>\n";
	$path = '';
    } elsif (/^s$/) {
	$path .= 'z';
	$strokeparams = strokeparams ();
	print " <g style=\"$strokeparams\">\n";
	print "  <path d=\"$path\"/>\n";
	print " </g>\n";
	$path = '';
    } elsif (/^S$/) {
	$strokeparams = strokeparams ();
	print " <g style=\"$strokeparams\">\n";
	print "  <path d=\"$path\"/>\n";
	print " </g>\n";
	$path = '';
    } elsif (/([\d\.]+) w$/) {
	$strokewidth = $1;
    } else {
	chomp;
#	print " <!--$_-->\n";
    }
}
print "</svg>\n";
