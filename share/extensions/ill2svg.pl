#!/usr/bin/perl

# convert an illustrator file (on stdin) to svg (on stdout)
use Getopt::Std;

my %args;

# Newline characters
my $NL_DOS = "\015\012";
my $NL_MAC = "\015";
my $NL_UNX = "\012";

getopts('h:', \%args);

if ($args{h}) { usage() && exit }
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
	    #push @result, 1000 - $_[$i];
	    push @result, 1052.36218 - $_[$i];
	} else {
	    #push @result, $_[$i] - 100;
	    push @result, $_[$i];
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

sub usage {
    warn qq|Usage: ill2svg [-l "string" -h] infile > outfile
options: 
	-h print this message and exit
|;
}

sub process_line {
	chomp;
	next if /^%_/;
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
	print " <g style=\"fill: $fillcolor;\">\n";
	print "  <path d=\"$path\"/>\n";
	print " </g>\n";
	$path = '';
    } elsif (/^s$/) {
	$path .= 'z';
	$strokeparams = strokeparams ();
	#print " <g style=\"fill:none;stroke:black;stroke-opacity:1; $strokeparams\">\n";
	print " <g style=\"fill:none; $strokeparams\">\n";
	print "  <path d=\"$path\"/>\n";
	print " </g>\n";
	$path = '';
    } elsif (/^S$/) {
	$strokeparams = strokeparams ();
	#print " <g style=\"fill:none; $strokeparams\">\n";
	print " <g style=\"fill:none;stroke:black;$strokeparams;\">\n";
	print "  <path d=\"$path\"/>\n";
	print " </g>\n";
	$path = '';
    } elsif (/^1 XR$/) {
       #printf( "\nbegin\r\n" );

       if( $firstChar != 0){
         print ("</tspan>\n</text>\n");
       }

       $firstChar = 0;	
    } elsif (/^TP$/) {
       #Something do with the text;)
    } elsif (/^([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) Tp$/i) {
 
       # Text position etc;
       $cpx=$5;
       $cpy=1052.36218 - $6;         
       ## print ("x:$5 y:$6\r\n");
       
    } elsif (/^\/_([\S\s]+) ([\d\.]+) Tf$/) {
       $FontName = $1;
       $FontSize = $2;

       ## When we know font name we can render this.
       if( $firstChar != 1){
	 print ("<text x=\"$cpx\" y=\"$cpy\"");  
         print (" style=\"font-size:$FontSize;font-weight:normal;stroke-width:1;");
         print ("fill:$fillcolor;fill-opacity:1;");
         print ("font-family:$FontName;\" id=\"text$5\">\n<tspan>");
	 $firstChar = 1;
       } else {
	 print ("</tspan>\n<tspan x=\"$cpx\" y=\"$cpy\"");
	 print (" style=\"font-size:$FontSize;font-weight:normal;stroke-width:1;");
         print ("fill:$fillcolor;fill-opacity:1;");
         print ("font-family:$FontName;\" id=\"text$5\">");

       }

    } elsif (/^\(([\S\s]+)\) Tx$/) {
	# Normal text
        	print ("$1");

    } elsif (/([\d\.]+) w$/) {
	$strokewidth = $1;
    } else {
	chomp;
#	print " <!--$_-->\n";
    }
    if( $firstChar != 0){
       print ("</tspan>\n</text>\n");
    }
}


print "<svg>\n";
while (<>) {
    if (m/$NL_DOS$/) { 
	$/ = $NL_DOS; 
	foreach (split /$NL_DOS/) {
	    process_line($_);
	}
    } elsif (m/$NL_MAC$/) {
	$/ = $NL_MAC;
	foreach (split /$NL_MAC/) {
	    process_line($_);
	}
    } else {
	chomp;
	process_line($_);
    }
}
print "</svg>\n";
