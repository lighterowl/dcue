#!/usr/bin/env perl

use strict;
use warnings;
use utf8;

open(my $fh, '<:crlf:utf8', $ARGV[0]);
binmode(\*STDOUT, ':utf8');
while(<$fh>)
{
  chomp;
  print 'R"cue(' . $_ . ')cue"sv' . ",\n";
}
close($fh);
