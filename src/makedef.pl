#! perl

print "####### Generating 'inkscape.def' #######\n";

open (OUTFILE, ">inkscape.def");
print OUTFILE "LIBRARY\tinkscape.dll\n";
print OUTFILE "EXPORTS\n";
open (PIPE, "i686-pc-mingw32-nm libinkscape.a | sort | uniq |");
while(<PIPE>)
   {
      if ($_ =~ /T _/)
      {
         $line = $_;
         $line =~ s/.* T _//;
         print OUTFILE $line;
      }
      elsif  ($_ =~ /B _/)
      {
         $line = $_;
         $line =~ s/.* B _//;
         print OUTFILE $line;
      }
   }
close (PIPE);
close (OUTFILE);
