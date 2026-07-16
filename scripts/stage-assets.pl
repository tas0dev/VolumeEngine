use strict;
use warnings;
use Cwd qw(abs_path);
use File::Basename qw(dirname);
use File::Path qw(make_path remove_tree);

sub main {
	if (@ARGV != 2) {
		die "usage: stage-assets.pl <source> <destination>\n";
	}

	my ($source_argument, $destination) = @ARGV;
	my $source = abs_path($source_argument);

	if (!defined($source) || !-d $source) {
		die "asset directory does not exist: $source_argument\n";
	}

	make_path(dirname($destination));

	if (-l $destination || -f $destination) {
		unlink($destination)
			or die "failed to remove $destination: $!\n";
	} elsif (-d $destination) {
		remove_tree($destination);
	}

	symlink($source, $destination)
		or die "failed to create symlink $destination -> $source: $!\n";

	return 0;
}

exit main();