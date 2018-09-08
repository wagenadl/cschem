#!/usr/bin/perl -w

use strict;

sub oneChar {
  my $name = shift;
  my $num = hex($name);
  return if hex($name)<32 || hex($name)>125;
  my $x0 = 200 + 40 * hex(substr($name,1,1));
  my $y0 = 200 + 40 * hex(substr($name,0,1));
  print OUT "<g id=\"char_$name\" transform=\"matrix(1,0,0,-1,$x0,$y0)\">\n";
  print COUT "    chars[$num] = {};\n";
  while (<IN>) {
    if (/polyline/) {
      print OUT $_;
      /points="\s*(.*)\s*"/ or next;
      my @pts = split(/ +/, $1);
      print COUT "    chars[$num].append({ ";
      my $x0 = -1e5;
      my $y0 = -1e5;
      for (@pts) {
	my ($x, $y) = split(/,/, $_);
	print COUT "$x,$y, " unless $x==$x0 && $y==$y0;
	$x0 = $x; $y0 = $y;
      }
      print COUT "});\n";
    }
    last if m{</g>};
  }
  print OUT "</g>\n";
}

open IN, "<phk.freebsd.dk-hacks-Wargames-index.html" or die;
open OUT, ">font.svg" or die;
open COUT, ">../font.cpp" or die;

print OUT <<"EOF";
<svg version="1.1"
 width="1000" height="1000"
 xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">
<g stroke-linecap="round" stroke-linejoin="round" fill="none" stroke="#000000">
EOF

print COUT <<"EOF";
// Extracted from phk.freebsd.dk-hacks-Wargames-index.html
#include <QMap>
#include <QVector>

typedef QVector<int> polyline;
typedef QVector<polyline> chardef;
typedef QMap<int, chardef> font;

font simpleFont() {
  font chars;
EOF

while (<IN>) {
  if (/wargames_(..).svg/) {
    oneChar($1);
  }
}

print OUT << "EOF";
</g>
</svg>
EOF

print COUT << "EOF";
  return chars;
}
EOF

close OUT;
close IN;
