#!/usr/bin/perl
################################################################################
# file:        ute.pl
# brief:       compare output file with expected file
################################################################################

use File::Basename;

################################################################################
# var definitions
my $retcode=0;
my $register_test_program = 0;
my $gs_testcase = 0;
my $create_exp = 0;
my $create_ediff = 0;
my $debug_unifications = 0;
my $diff_tool="$ENV{'UTE_DIFF'}";
my $debug = 0;
my $use_colors = 1;
my $log_level = 2;
my $regex;
my $reason = "";
my @skip_sections = (); # Ignore sections being of no interest for session manager.  Winfried ##
my @common_permanent_unifications = (); # Unification which are written in result file.
my @unifications = (
	#'s/^(Time:).*$/\1/',
);
my @multiline_unifications = ();

################################################################################
# subrutines definition

sub read_arguments
{
# read command line parameters
	foreach my $argument (@ARGV)
	{
		# debug print "Commandline argument: $argument\n";
		if ($argument eq '-d')
		{
			if ($log_level < 6 )
			{
				$log_level = 6;
			}
			$debug = 1;
		}
		elsif ($argument eq '-ce')
		{
			$create_exp = 1;
		}
		elsif ($argument eq '-dd')
		{
			$create_ediff = 1;
			$log_level = 8;
		}
		elsif ($argument eq '-debuguni')
		{
			$debug_unifications = 1;
			$log_level = 8;
		}
		elsif ($argument eq '-nouni')
		{
			@unifications = ();
		}
		elsif ($argument eq '-n')
		{
			$use_colors = 0;
		}
	}
}

################################################################################
sub set_colors
{
	if ($use_colors)
	{
		$color_normal='[0m';
		$color_fail  ='[1;31m';
		$color_ok    ='[1;32m';
		$color_warn  ='[2;33m';
		$color_debug ="\033[1;34m";
	}
	else
	{
		$color_normal="";
		$color_fail  ="";
		$color_ok    ="";
		$color_warn  ="";
		$color_debug ="";
	}
}

################################################################################
# import own unifications
sub import_own_unifications
{
	if (-e $ENV{'UTE_TC_SUB'})
	{ require $ENV{'UTE_TC_SUB'}; import ute_unify (); }
	else
	{
		if (-e 'ute_tc_unify.pm')
		{ require ute_unify; import ute_unify (); }
	}
	if (0!=@ute_unify::own_unifications)
	{  
		push(@unifications,@ute_unify::own_unifications);
	}
	if (0!=@ute_unify::own_permanent_unifications)
	{  
		push(@common_permanent_unifications,@ute_unify::own_permanent_unifications);
	}
	if (0!=@ute_unify::own_multiline_unifications)
	{  
		push(@multiline_unifications,@ute_unify::own_multiline_unifications);
	}
	if (0!=@ute_unify::own_skip_sections)
	{  
		push(@skip_sections,@ute_unify::own_skip_sections);
	}
}

################################################################################
# unify current $result[$line] and $expected[$line] with all unifications
sub unify_pair_of_lines
{
	my ($line_1, $line_2) = @_;
	my $regex;
	foreach $regex (@unifications)
	{
		$_ = $line_1;
		eval ( $regex ) and $line_1 = $_;
		print "$@: $regex" if $@;

		$_ = $line_2;
		eval ( $regex ) and $line_2 = $_;
		print "$@: $regex" if $@;

		if ($line_1 eq $line_2)
		{ 
			last; 
		} 
	}
	($line_1, $line_2);
}

################################################################################
sub permanent_unifications ($)
{
	(my $array_ref) = @_;
	my @array = ();
	my $first_time = 0;

	foreach my $line ( @$array_ref )
	{
		if( 0 != @common_permanent_unifications )
		{
			$line =~ s/\x00$//;  # remove '^@'

			# Protect the first time stamp from "unification"
			unless ( $first_time )
			{
				$first_time = ($line =~ s/^(Time:)(.*$)/Current$1$2/);
			}
			# remove time, pointers, and handles
			foreach my $regex (@common_permanent_unifications)
			{
				$_ = $line;
				eval $regex;
				print "$@: $regex" if $@;
				$line = $_;
			}
		}
		push @array, $line;
	}
	return @array;
}

################################################################################
# users want to create unified <*exp.unified> and <*out.unified> files (-d option)
# users want to create unified <*exp.unified> and <*out.unified> files and want see differences in detail (-dd option)
sub store_unified_files
{
	if (( $create_ediff and $retcode > 0 ) or $debug or $debug_unifications )
	{
		my ($res_name, $exp_name, $uni_result, $uni_expected ) = @_;
		my $unified_result = $res_name;
		my $unified_expected = $exp_name;    
		$unified_result =~ s/\/(.*)$/\/$1.unified/; 
		$unified_expected =~ s/\/(.*)$/\/$1.unified/;

		print "${color_debug}~${color_normal} \tunified output file:   $unified_result\n";
		print "${color_debug}~${color_normal} \tunified expected file: $unified_expected\n";

		open (UNI_RESULT, ">$unified_result")  || die "${color_fail}~${color_normal} Cannot override $unified_result file";
		open (UNI_EXPECTED, ">$unified_expected")  || die "${color_fail}~${color_normal} Cannot override $unified_expected file";
		print UNI_RESULT @$uni_result;
		print UNI_EXPECTED @$uni_expected;
		close(UNI_RESULT);
		close(UNI_EXPECTED);

		if ( $debug_unifications )
		{
			print ("$diff_tool $res_name $unified_result $unified_expected $exp_name &\n");
			system("$diff_tool $res_name $unified_result $unified_expected $exp_name &");
		}
		elsif ( $create_ediff ) 
		{
			print ("$diff_tool $unified_result $unified_expected &\n");
			system("$diff_tool $unified_result $unified_expected &");
		}
	}
}

################################################################################
sub skip_events ($$)
{
	my ( $content_ref, $regex_ref ) = @_;
	my @result = ();
	my @buffer = ();
	my $ignore_event = 0;
	foreach ( @$content_ref )
	{
		if ( $_ =~ m/^Event: / )
		{
			push @result, @buffer  unless $ignore_event;
			@buffer = ();
			$ignore_event = 0;
		}
		foreach $regex ( @$regex_ref )
		{
			if ( eval $regex )
			{
				$ignore_event = 1;
			}
		}
		push @buffer, $_;
	}

	push   @result, @buffer  unless $ignore_event;
	return @result;
}

################################################################################
sub skip_sections ($$)
{
	my ( $content_ref, $regex_ref ) = @_;
	my @array = ();
	my $ignore_section = 0;

	Line:foreach ( @$content_ref )
	{
		if ( $ignore_section )
		{
			$ignore_section-- if m/^=============/;
			# if end of section detected, resume copying array with next entry
			next Line;
		}
		else
		{
			my $regex;
			foreach $regex ( @$regex_ref )
			{
				# compare current line, but skip following lines until end of section
				# $ignore_section = eval $regex and print "nonreg_skip_section found: $regex\n" and next Line;
				if ( eval $regex )
				{
					$ignore_section = 2;
					next Line;
				}
			}
		}
		push @array, $_;
	}
	return @array;
}

################################################################################
# Main
################################################################################

my $expected_name = shift;
my $result_name   = shift;
my $testcase_name = $result_name;

read_arguments();
set_colors();
import_own_unifications();

if ( $ENV{'DIFF'} )
{
	$diff_tool=$ENV{'DIFF'};
}


################################################################################
# Process result file

open(RESULT,   "<$result_name")   || die "${color_fail}~${color_normal}   Cannot open result file \"$result_name\"";
my   @result = <RESULT>;
close(RESULT);

# It shall be possible to use .out file as input for script create_expected,
# therefore the following manipulations have to be done even if .exp does not
# exist, or if .out and .exp have different length.                (Winfried)

@result = &permanent_unifications( \@result );

# write modified content back into result file
open (RESULT, ">$result_name")  || die "${color_fail}~${color_normal} Cannot override result file $result_name";
print RESULT @result;
close(RESULT);

# store result with permanent_unifications only
my @result_before_multi = @result;

if (@multiline_unifications != 0 )
{
	$result_name_old = $result_name;
	$result_name_after_multiline = fileparse($result_name) ;
	$result_name_after_multiline = "/vobs/ims_do/tmp/".$result_name_after_multiline.".munif";
	$expected_name_old = $expected_name;
	$expected_name_after_multiline = fileparse($expected_name) ;
	$expected_name_after_multiline = "/vobs/ims_do/tmp/".$expected_name_after_multiline.".munif";

	if ($log_level >= 8)
	{
		printf "${color_ok}~${color_normal}   Result multi  : %s\n", $result_name_after_multiline;
		printf "${color_ok}~${color_normal}   Expected multi: %s\n", $expected_name_after_multiline;
	}

	$regexik = join(';',@multiline_unifications);
	if ($log_level >= 8)
	{
		printf "${color_debug}~${color_normal} system \"perl -p -e \"undef \$\/; $regexik\" $result_name > $result_name_after_multiline\"\n";
		printf "${color_debug}~${color_normal} system \"perl -p -e \"undef \$\/; $regexik\" $expected_name > $expected_name_after_multiline\"\n";
	}
	system "perl -p -e \"undef \$\/; $regexik\" $result_name > $result_name_after_multiline";
	system "perl -p -e \"undef \$\/; $regexik\" $expected_name > $expected_name_after_multiline";
	$result_name = $result_name_after_multiline;
	$expected_name = $expected_name_after_multiline;
}

# re-read result content from temporary file (if available then it contains changes caused by multiline unifications)
open(RESULT,   "<$result_name")   || die "${color_fail}~${color_normal}   Cannot open result file \"$result_name\"";
@result = <RESULT>;
close(RESULT);


my $include;
my $include_res;

@differences_list = ();
@differences_out = ();
@differences_exp = ();

for (my $line = 0; $line <= $#result; $line++)
{
	if ($result[$line] =~ /^Event: Switching to included file.*$/)
	{
		$include = $result[$line+3];
		chop ($include);
		$include_res = $result[$line+5];
		chop ($include_res);

		if ($include_res eq "")
		{
			if ($log_level >= 4)
			{
				print "\n${color_ok}~${color_normal}   INCLUDE Event found - results from include file are in current (main) result file\n";
			}
		}
		else
		{
			$num_inc++;
			if ($log_level >= 4)
			{
				printf "\n${color_ok}~${color_normal}   INCLUDE Event found - include level %d\n", $num_inc;
			}
			$num_inc--;
		}
	}
}
################################################################################
# Process expected file

# open and read expected file
my  @expected = ();
my  $expected_exists = 0;

# read expected content from temporary file (if available then it contains changes caused by multiline unifications)
if (open(EXPECTED, "<$expected_name"))
{
	$expected_exists = 1;
	# read complete files
	@expected = <EXPECTED>;
	close(EXPECTED);
}

if (@multiline_unifications != 0 )       
{
	unlink $result_name_after_multiline; $result_name = $result_name_old;
	unlink $expected_name_after_multiline; $expected_name = $expected_name_old;
}

if ( $create_exp or $expected_exists==0 or (scalar(@expected) == 0) )
{
	if (scalar(@expected) == 0)
	{
		$status = "ERROR";
		$retcode=1;
		if ($expected_exists)
		{
			$reason = "Expected file is empty";
		}
		else
		{
			$reason = "Expected file does not exist";
		}

		if ($log_level >= 4)
		{
			printf "\n${color_fail}~${color_normal} %-60s%-26s%-20s\n", $testcase_name, "${color_fail}$status${color_normal}", $reason;
		}
	}

	print "${color_warn}~ Creating exp file: $expected_name${color_normal}\n";

	open (EXPECTED, ">$expected_name")  || die "${color_fail}~${color_normal} Cannot override exp file $expected_name";
	print EXPECTED @result_before_multi;
	close(EXPECTED);

	exit $retcode;
}

################################################################################
# proceed unification

if (scalar(@result) != scalar(@expected))
{
	$status = "ERROR";
	$reason = "different length. Maybe some line in exp file is missing.";
	$retcode=2;
	if ($log_level >= 4)
	{
		printf "\n${color_fail}~${color_normal} %-60s%-26s%-20s\n", $testcase_name, "${color_fail}$status${color_normal}", $reason;
	}
	if ($create_ediff==0) 
	{
		print "\n${color_ok}~${color_normal} You can see these differences in more comfortable way using option -dd.\n\n";
	}
}

my $ignore_section = 0;
my $differences = 0;

for (my $line = 0; $line <= $#result or $line <= $#expected; $line++)
{
	if ($result[$line] ne $expected[$line])
	{
		# print  $result[$line],  $expected[$line];
		($result[$line],  $expected[$line]) = unify_pair_of_lines $result[$line], $expected[$line];
		# print  $result[$line],  $expected[$line];

		if ($result[$line] ne $expected[$line])
		{
			push(@differences_list, " [out ".($line+1)."]\t$result[$line] [exp ".($line+1)."]\t$expected[$line]\n");
			push(@differences_out, "$result[$line]");
			push(@differences_exp, "$expected[$line]");
		}
	}
}

if (@differences_list."" == 0)
{
	$status = "Successful";
	$reason = "";
	if ($log_level >= 4)
	{
		printf "\n${color_ok}~${color_normal} %-60s%-26s%-20s\n", $testcase_name, "${color_ok}$status${color_normal}", $reason;
	}
}
else
{
	@differences_out_sorted = ();
	@differences_exp_sorted = ();
	@differences_out_sorted = sort @differences_out;
	@differences_exp_sorted = sort @differences_exp;
	@sort_diff = ();
	for ($diff = 0; $diff < @differences_list.""; $diff++)
	{
		if ( $differences_out_sorted[$diff] ne $differences_exp_sorted[$diff] )
		{
			push(@sort_diff,"\t$differences_out_sorted[$diff]\t$differences_exp_sorted[$diff]\n");
		}
	} 
	if (@sort_diff."" == 0)
	{  
		$status = "Successful";
		$reason = @differences_list==1 ? "1 sorted difference" : @differences_list." sorted differences";
		if ($log_level >= 4)
		{
			printf "\n${color_warn}~${color_normal} %-60s%-26s%-20s\n", $testcase_name, "${color_warn}$status${color_normal}", $reason;
			if ($log_level >= 8)
			{
				print @differences_list;
				print "Sorted out diffs:\n";
				print @differences_out_sorted;
				print "\nSorted exp diffs:\n";
				print @differences_exp_sorted;
				print "\n\nDiff of Sorted diffs: is EMPTY\n\n";
			}
		}
	}
	else
	{ 
		$status = "ERROR";
		$reason = @differences_list==1 ? "1 difference" : @differences_list." differences";
		$retcode=3;
		if ($log_level >= 4)
		{
			printf "\n${color_fail}~${color_normal} %-60s%-26s%-20s\n", $testcase_name, "${color_fail}$status${color_normal}", $reason;
			if ($log_level >= 6)
			{
				print @differences_list;
				if ($log_level >= 8)
				{
					print "Sorted out diffs:\n";
					print @differences_out_sorted;
					print "\nSorted exp diffs:\n";
					print @differences_exp_sorted;
					print "\n\nDiff of Sorted diffs:\n";
					print @sort_diff;
				}
			}
		}
		if ($create_ediff==0) 
		{
			print "\n${color_ok}~${color_normal} You can see these differences in more comfortable way using option -dd.\n\n";
		}
	}
}

store_unified_files $result_name, $expected_name, \@result, \@expected;

# Errors of the testcase itself have already been logged

exit $retcode;

################################################################################
# End Main
################################################################################
# vim: ts=2 sw=2 et fdm=marker
