#!/usr/bin/perl

# Test checking out logging print outs.
# There are two parts to this test.
#   1. A C unit test that logs stuff.
#   2. This perl tests that checks the file dump is correct.
# The file for the dump is defined in the makefile.

use strict;
use warnings;

use Mcgoo;

my $MAKE_TARGET = "run-log-test";
my $FILENAME = "tuna.sdl";

Mcgoo::Announce;

# Run the C unit test.
`make $MAKE_TARGET`;
Mcgoo::Expect $?, 0;

# Check the output file.
Mcgoo::FileLineExists $FILENAME, "CAPTURE";
Mcgoo::FileLineExists $FILENAME, "{";
Mcgoo::FileLineExists $FILENAME, "TX";
Mcgoo::FileLineExists $FILENAME, "RX";
Mcgoo::FileLineExists $FILENAME, "0x01, 0x02, 0x03, 0x04";
Mcgoo::FileLineExists $FILENAME, "0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,";
Mcgoo::FileLineExists $FILENAME, "}";

Mcgoo::Summarize;


