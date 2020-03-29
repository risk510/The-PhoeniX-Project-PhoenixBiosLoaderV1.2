#!/usr/bin/perl
system("rm -f bigsin.h");
open(LOGFILE, ">>bigsin.h");
my $i;
print LOGFILE "float sine_table[360000] = {0.0f, \n";
for($i=0.001;$i<360;$i+=0.001)
{
	my $sinof = sin($i);
	if ($sinof eq 0.958915723868494) {
	} else {
		print LOGFILE $sinof;
		print LOGFILE "f, \n";
	}
}
print LOGFILE "};\n";
close(LOGFILE);
