#!/usr/bin/perl
#
# Copyright (c) 2011 Antony Sidwell.
#
# A not-particularly-efficient script to build the rephial.org website.
#
# Uses a series of fragmentary HTML, Markdown files and templates to
# build the site as static files ready for serving.  
#
# Knocked up to reduce reliance on esoteric server setups and make 
# decentralised editing of the site more straightforward.
#
# TODO:
# - Don't process files we aren't going to output later.
#
use strict;
use warnings;
use File::stat;
use Markdown;
	


# Get the last modified time for this script - if the script is
# updated we want to regenerate the output pages.
open(my $F, $0);
my $script_lastmod = stat($F)->mtime;
close($F);
		
# Loads a source file into a hash table.
#
# The file format is no of more lines of the form '<variable> = "<string value>"'
# followed by lines of content.
#
# The returned table has a key "content" with the value being the content,
# and a number of other key, value pairs extracted from the initial lines.
# It also has a key _lastmod with the modification time since the epoch of the
# source file.
sub load_source {
	my ($src_file) = @_;
	my $state = "vars";
	my %vars;
	
	open(my $F, $src_file) or die $!;
	while (<$F>) {
		my ($line) = $_;
		if ($state eq "vars") {
			if ($line =~ /^\s*([^ ]+)\s*=\s*"(.*)"\s*$/) {
				$vars{$1} = $2;
			} else {
				$state = "content";
			}
		}
		
		if ($state eq "content") {
			$vars{"content"} .= $line;
		}
	}
	$vars{"_lastmod"} = $script_lastmod;
	
	if ($vars{"_lastmod"} < stat($F)->mtime) {
		$vars{"_lastmod"} = stat($F)->mtime;
	}
	close($F);

	# If the source is in Markdown format, convert it to HTML
	if (defined($vars{"markdown"}) && $vars{"markdown"} eq "true") {
		$vars{"content_markdown"} = $vars{"content"};
		$vars{"content"} = Markdown::Markdown($vars{"content"}, {empty_element_suffix=>">", heading_ids=>0});
		# Need to downgrade header levels to fit the page structure
		$vars{"content"} =~ s/<h3 id/<h5 id/g;
		$vars{"content"} =~ s/h3>/h5>/g;
		$vars{"content"} =~ s/<h2 id/<h4 id/g;
		$vars{"content"} =~ s/h2>/h4>/g;
		$vars{"content"} =~ s/<h1 id/<h3 id/g;
		$vars{"content"} =~ s/h1>/h3>/g;
	}
	
	# If some variables are not defined, set them to defaults
	if (!defined($vars{"siteurl"})) { $vars{"siteurl"} = "http://rephial.org/"; }
	if (!defined($vars{"title"})) { $vars{"title"} = "Home of Angband"; }
	
	return %vars;
}

# Takes a template filename and a "vars" hash table, and replaces strings of the form "<@key@>" in the
# template with the corresponding value in the hash table.
#
# Returns hash table with "content" containing the new content and "_lastmod" updated.
sub apply_template {
	my ($template_file, %vars) = @_;
	my $newcontent;
	
	open(my $F, $template_file) or die $!;
	while (<$F>) {
		my $line = $_;
		while (my($k, $v) = each(%vars)) {
			$line =~ s/<\@$k\@>/$v/g;
		}
		$line =~ s/<\@[^ ]*\@>//g;
		$newcontent .= $line;
	}
	if (!defined($vars{"_lastmod"}) || ($vars{"_lastmod"} < stat($F)->mtime)) {
		$vars{"_lastmod"} = stat($F)->mtime;
	}
	close $F;
	
	$vars{"content"} = $newcontent;
	return %vars;
}

#
# We put all the output page tables into a hash table structure, with the keys the 
# output leaf name.
#
my $output_files = {};

# First work on the main site pages
my $dir = "src";
opendir(DIR, $dir) or die $!;	
while (my $file = readdir(DIR)) {
	next unless (-f "$dir/$file");
	
	my %table = load_source("$dir/$file");
	%table = apply_template("template.html", %table);
	$output_files->{$file} = \%table;
}
closedir(DIR);

# Then do the release pages
$output_files->{"release"} = {};
$dir = "src/releases";
opendir(DIR, "$dir") or die $!;
my @releases = ();
my $newest_mod = 0;
	
while (my $file = readdir(DIR)) {
	next unless (-f "$dir/$file");
	my %table = load_source("$dir/$file");
	$table{"filename"}= $file;
	if  (!defined $table{"release_name"}) {
		$table{"release_name"} = $file;
	}
	$table{"title"} = "Angband Releases: $table{'release_name'}";

        if ($table{"_lastmod"} > $newest_mod) {
            $newest_mod = $table{"_lastmod"}
        };	

        push(@releases, \%table);
}

closedir(DIR);

# Build an html list of releases (with links) for inclusion in the release pages.
@releases = sort {($b->{"release_code"} or $b->{'filename'}) cmp ($a->{'release_code'} or $a->{'filename'})} @releases;

for my $release (@releases) {
	# A couple of variables for the various templates before they are applied.
	$release->{'release_list'} = "";	
	for my $r (@releases) {
		if ($r->{'filename'} eq $release->{'filename'}) {
			$release->{'release_list'} .= "<li>$r->{'release_name'}";
		} else {
			$release->{'release_list'} .= "<li><a href='$r->{'filename'}'>$r->{'release_name'}</a>";
		}
	}
	$release->{'releasetab'} = "class='selected'";
	
	%$release = apply_template("release_template.html", %$release);
	%$release = apply_template("template.html", %$release);
        if ($newest_mod > $release->{'_lastmod'}) {
            $release->{'_lastmod'} = $newest_mod;
        }
	$output_files->{"release"}->{$release->{'filename'}} = $release;
}


# We now have a filled "output_files" structure ready to go.
sub write_files {
	my ($output_dir, $files) = @_;
	
	if (not -d $output_dir) {
		print "Creating directory '$output_dir'.\n";
		mkdir($output_dir);
	}
	
	for my $file (keys %$files) {
            my $output_path = "$output_dir/$file.html";
		if ($files->{$file}{"title"}) {
			my $F;
			# Check if our new file is newer than the one already there.
			if (not (open($F, $output_path)) || stat($F)->mtime < $files->{$file}{"_lastmod"}) {
#				print "Writing $output_path\n";
				open(OUT, ">".$output_path) or die $!;
				print OUT $files->{$file}{"content"};
				close(OUT);
			}
			close($F);
		}
		else {
			write_files("$output_dir/$file", $files->{$file});
		}
	}
}

write_files("output", $output_files);

# Nasty hack to configure lighttpd to not need .html extensions
# for any of the generated files, and to make /releases/ link
# to the latest release.
#
# This stage is how you know it's related to Angband's traditional
# coding standards. :)
sub write_mappings {
	my ($OUT, $path, $files) = @_;
	
	for my $file (keys %$files) {
		my $fullpath = "$path/$file";
		if ($files->{$file}{"title"}) {
                    print $OUT "\"^$fullpath\$\" => \"$fullpath.html\",\n";
                } else {
                    write_mappings($OUT, $fullpath, $files->{$file});
                }
        }
}

my $OUT;
open($OUT, ">mappings") or die $!;
print $OUT "url.rewrite-once  = (\n";
# Output the "root" links
print $OUT "\"^/release/\$\" => \"/release/$releases[0]->{filename}.html\",\n";
print $OUT ")\n";
close $OUT;

exit 0;
