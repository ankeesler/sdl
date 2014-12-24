#!/usr/bin/perl

# Test for basic SDL cli.

use strict;
use warnings;

use Mcgoo;

my $EXEC = "./build/sdl";

Mcgoo::Announce;

# Check to make sure we get usage with no args.
my @outputs = `$EXEC`;
chomp @outputs;
Mcgoo::Expect @outputs, "Usage: sdl.*";

# Check to make sure no args to -n throws error.
#@outputs = `$EXEC -n`;
#chomp @outputs;
#Mcgoo::Expect $outputs[0], "Error:.*option n.";
#Mcgoo::Expect $outputs[1], "Usage: sdl.*";

# Check to make sure putting a non-positive arg to -n gives error.
#@outputs = `$EXEC -n -2`;
#chomp @outputs;
#Mcgoo::Expect $outputs[0], "Error: .* 1.";

Mcgoo::Summarize;
