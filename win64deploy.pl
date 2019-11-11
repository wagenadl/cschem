#!/usr/bin/perl -w

use strict;
use Cwd;
use File::Copy;
use File::Path;

######################################################################
# EXTERNAL PATHS
my $qt_path = "c:/Qt";
my $qbin_path = $qt_path . "/5.12.5/mingw73_64/bin";
my $qssl_path = $qt_path . "/Tools/QtCreator/bin";
#my $openssl_path = "c:/OpenSSL-Win32";
my $msvc_path = "c:/Program Files (x86)/Microsoft Visual Studio 14.0/VC";

######################################################################
# INTERNAL PATHS
my $cschem_buildpath = "build-cschem-Desktop_Qt_5_12_5_MinGW_64_bit-Release/release";
my $cpcb_buildpath = "build-cpcb-Desktop_Qt_5_12_5_MinGW_64_bit-Release/release";
my $release_path = "release-cschem-x64";

######################################################################

my @envpath = split(/:/, $ENV{PATH});
my @usepath;
for (@envpath) {
	push @usepath, $_ unless /Anaconda/;
}
$ENV{PATH} = join(":", @usepath);	

$msvc_path =~ s/\//\\/g;
$ENV{VCINSTALLDIR} = $msvc_path;

die "cschem executable not found" unless -f "$cschem_buildpath/cschem.exe";
die "cpcb executable not found" unless -f "$cpcb_buildpath/cpcb.exe";


File::Path::remove_tree($release_path) if -d $release_path;
File::Path::make_path($release_path);

system("$qbin_path/windeployqt --dir $release_path "
       . " --compiler-runtime $cschem_buildpath/cschem.exe")
  and die "Failed to get cschem deployment";

system("$qbin_path/windeployqt --dir $release_path "
       . " --compiler-runtime $cpcb_buildpath/cpcb.exe")
  and die "Failed to get cpcb deployment";

File::Copy::copy("$cschem_buildpath/cschem.exe", "$release_path/");
File::Copy::copy("$cpcb_buildpath/cpcb.exe", "$release_path/");

sub sslcopy {
  my $pth = shift;
  my $fn = shift;
  print "$fn not found in $pth - no https support\n" unless -f "$pth/$fn";
  File::Copy::copy("$pth/$fn", "$release_path/"); 
}

#sslcopy($qssl_path, "ssleay32.dll", "$release_path/");
#sslcopy($qssl_path, "libeay32.dll", "$release_path/");
#sslcopy($openssl_path, "libcrypto-1_1.dll", "$release_path/");
#sslcopy($openssl_path, "libssl-1_1.dll", "$release_path/");

print "Now run 'cschem-x64.iss' using Inno Setup.\n";
