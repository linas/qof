#!/bin/perl

# Copyright 2005 Neil Williams <linux@codehelp.co.uk>
# This file is part of QOF.
# QOF is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

use strict;
use POSIX;     # for setlocale()
use gettext;

setlocale(LC_MESSAGES, "");
bindtextdomain("qof", "");
textdomain("qof");

use vars qw( %options %texts $xmlmiddle $language);

print <<XMLHEAD;
<?xml version="1.0" encoding="UTF-8"?>
<qofconfig xmlns="http://qof.sourceforge.net/" >
  <backend name="QSF Backend Version 0.1">
XMLHEAD

$language = gettext("en-gb"); 

$options{'map_file'} = qq/string/;
$texts{'map_file'} = gettext("Where the find the QSF maps.");
$options{'defaults'} = qq/boolean/;
$texts{'defaults'} = gettext("Use customised defaults.");

print "    <provider xml:lang=\"$language\">\n";
foreach $xmlmiddle (keys %options) {
print "        <option type=\"$options{$xmlmiddle}\" name=\"$xmlmiddle\">
            $texts{$xmlmiddle}
        </option>\n";
}
print "    </provider>\n";

print <<XMLEND;
  </backend>
</qofconfig>
XMLEND

=pod
=head1 XML Configuration options for backends.

=head2 QSF Backend Version 0.1 configuration script.

 Change the backend name before using for any other backend!

 The file exists to create translatable configuration files for
 QOF backends in XML. Gettext absorbs the marked strings in this file
 and inserts translated versions when the file is built.

 The file should be built in the same directory as the backend
 to which it relates and installed to a standard XML location. 
 like F</usr/share/xml/qof/>.

 note to translators: The $language variable
 is the XML language tag,
 it may not be the same as the one in C.
 e.g. English(GB) is en-gb not en_GB.
 The XML version generally uses a hyphen 
 instead of an underscore and is case insensitive.
 F<http://www.w3.org/TR/REC-xml/#sec-lang-tag>
 F<http://www.ietf.org/rfc/rfc3066.txt>

$language = gettext("en-gb"); 

=head2 Development pointers

 each key in %options must also exist in %texts
 each value in %options must be one of the allowed
 types for a KvpValue:
 B<gint64>, B<double>, B<gnc_numeric>, B<string>, B<guid>,
 B<date>
 These are equivalent to:
 B<KVP_TYPE_GINT64>, B<KVP_TYPE_DOUBLE>, B<KVP_TYPE_NUMERIC>,
 B<KVP_TYPE_STRING>, B<KVP_TYPE_GUID>, B<KVP_TYPE_TIMESPEC>
 Other KVP values like KVP_TYPE_BINARY, KVP_TYPE_GLIST are not 
 supported and KVP_TYPE_FRAME is not possible.

 The generated XML can be validated against the schema
 by downloading from:
 http://qof.sourceforge.net/backend-schema.xsd.xml
 and running:
 F<xmllint --schema backend-schema.xsd.xml file.xml>

=head2 Translating Perl with gettext.

 The following statement initializes the locale handling
 code in the C library. Normally, it causes it to read
 in the environment variables that determine the current
 locale.

 The first parameter is the category you would
 like to initialize locale information for. You can use
 LC_ALL for this, which will set locale information for
 all categories, including LC_CTYPE, LC_TIME, LC_NUMERIC,
 etc..

 I recommend that you set only LC_MESSAGES (text strings)
 or LC_CTYPE (character sets) and LC_TIME (time
 conventions) too at most. You may find that if you set
 LC_NUMERIC or some other categories, you will start
 outputting decimal numbers with strange thousand separators
 and decimal points and they will be unparseable in
 other countries.

 The second parameter is the locale name. If it is an
 empty string, then this information will be fetched from
 environment variables.

 Note that setlocale() will cause every part of your
 program to start operating in the new, non default (C)
 locale, including C library functions. So don't be
 surprised if POSIX::ctime returns "Montag, 22. Juli 1996,
 12:08:25 Uhr EDT" instead of "Mon Jul 22 12:08:25 EDT 1996"
 If you set LC_TIME or LC_ALL using setlocale().

setlocale(LC_MESSAGES, "");

 Decide on a unique identifier that will distinguish your
 program's text strings in the LC_MESSAGES database. This
 would usually be the name of your program

 By default, locale information is found in OS dependant
 system directories such as /usr/lib/locale, or any directory
 found in the $PATH-like environment variable $NLSPATH.
 I recommend that you do _not_ install files in /usr. If
 your program is installed in a directory tree such as
 F</opt/my_package_name/>{bin,man,lib,etc}, then you could
 use F</opt/my_package_name/locale> to store locale information
 specific to your program, or you could put in somewhere
 in F</usr/local/lib>.

 Wherever you put it, if it is not one of the default
 directories, you will need to call bindtextdomain() to
 tell the library where to find your files. The first
 parameter is your database's identifier that you chose
 above.

bindtextdomain("qof", "");

 Now set the default text domain to yours. If you do not
 call this, then the default is "messages".

textdomain("qof");

 That's it for the initializations

=cut
