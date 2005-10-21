/*! \mainpage QOF design and developers reference

This is the new developer and design manual for the Query Object
Framework. Previous documentation has been integrated into this 
and it should always be up to date since it is generated directly 
from the source files using Doxygen.

\section changes Future changes within QOF

QOF v0.6.0 introduces \b libqof1 which will remain API compatible
until all the changes below can be folded into libqof2:

	-# All gnucash-specific code to be removed. Most of this
		has been done in v0.6.0
	-# Filename and function name conventions to be made to 
		conform to a single method:
		-# filenames: qof<module>.c|h
		-# functions: qof_<module>_<function>_ ... 
		\n e.g.
			-# gnc_date.h becomes qofdate.h
			-# qof_book_mergeInit becomes qof_book_merge_init
			-# gnc_numeric_zero becomes qof_numeric_zero
			-# gnc_set_logfile becomes qof_log_setfile.
			-# gnc-trace.c|h becomes qoflog.c|h
	-# These changes will be made by deprecating old names
		and making old files into placeholders. When libqof2
		is ready for pre-release, all deprecated elements will
		include compiler flags that will highlight the code that
		needs to be changed and placeholder files may be removed
		at this stage. All flags and deprecated code will
		then be removed in the final libqof2 release.
	-# To make this change easier, the qof.h header has been fixed
		in v0.6.0 and is now the only header file required to be
		included to use QOF. Using individual header files in 
		applications linked against QOF is now \b deprecated. All
		code that uses QOF should only use:
			-# #include <qof.h>   // or
			-# #include "qof.h"\n
		This is the \b only file guaranteed to maintain access to the
		full QOF API during the entire life of libqof1 and libqof2.

\section general General design documents.

- \ref backendapi
- \ref backenderrors
- \ref qsfmaps

General information on merging QofBooks:\n
http://code.neil.williamsleesmill.me.uk/

General information on the QSF XML backend for QOF and maps:\n
http://code.neil.williamsleesmill.me.uk/qsf.html

\section hacking Hacking on this documentation

There is the beginning of a style guide for documenting under
\ref tipshints.

The Book Merge files are an attempt to document "by the book". 
\ref BookMerge\n
Feel free to start documenting or playing with doxygen configuration. 
This main page can be found in src/doc/doxygen_main_page.c .

Each doxygen section must be within a single comment block although 
large comment blocks can be split into separate pages:
\ref stylediscussion.

This main page is just an introduction to doxygen markup, see the
Doxygen manual for the full command set.

- \ref tipshints Tips and hints for using doxygen
- \ref stylediscussion Long comments, pages, editors
- \ref reference Links to the Doxygen manual

*/
/** \page tipshints Useful tips for doxygen in C files

 - \ref index Introduction
 - \ref stylediscussion Long comments, pages, editors
 - \ref reference The Doxygen manual
 
\section tips An introduction to doxygen markup
 
\subsection Locations What to document

All declarations for:

-# typedef
-# struct
-# enum
-# functions

This will enable doxygen to link all parameter types to the declarations
every time the type is used in a function - very helpful to new developers.

\subsection Files Private files

If your declarations are in separate files, like private header files, 
a simple block can still be linked into doxygen as long as the file is
identified to doxygen using a '\\file' section:

 \\file filename.h\n
	\\brief one-liner summary of the file purpose\n
	\\author the usual copyright statement

\subsection Methods How to document

Every doxygen comment block starts with an adapted comment marker. 
You can use an extra slash /// or an extra asterisk. Blocks end
in the usual way. Doxygen accepts commands using a backslash.

To put a description with each function or structure, use '\\brief' 
End the brief description with a blank line. The rest of the documentation will
then be shown in the body of the doxygen page.

Commands may begin with \\ or @

\subsection Presentation Extras

	-# Start a line with a hyphen to start a list - the indent determines the
nesting of the list:
		- To create a numbered list, use -# e.g. for a sublist:
			-# start a numbered list
		- revert to previous list

	End the list with a blank line.
Use :: at the start of a function or structure to link to the page 
for that function in the doxygen documentation. e.g. ::qof_class_foreach

Use the param command to describe function parameters in the text.

Use the 'back reference' to document enumerator values:\n
enum testenum {\n
	enum_one **&lt; less than marker tells doxygen to use this line
		to document enum_one.

\subsection config Editing Doxygen configuration

To edit the doxygen configuration, you can use:
*
cd src/doc
*
doxywizard doxygen.cfg &

*/

/** \page stylediscussion Style discussion

- \ref index Introduction
- \ref tipshints Tips and hints for using doxygen
- \ref reference Links to the Doxygen manual

[codehelpgpg 2004-07-25] Doxygen now copes with addgroup and this page
can be handled more easily by splitting the old single comment into
repeated comments, split into pages. I've worked on doxygen files in
Kate, KWrite and XCode (MacOSX) and the comment higlighting works fine.
If you do have problems, particularly when you start a new line within
an existing comment, enter a character at the end of the last
highlighted line to refresh the highlighting. Some editors have a
specific refresh option.

[cstim 2003-03-25] The "Data Structures" page of doxygen doesn't show
anything useful for gnucash. Obviously doxygen only accepts "real" C
struct definitions for inclusion on that page. However, all gnucash
data structures are defined somewhere in private headers, and only the
typedefs are publically visible. Isn't there a way to have doxygen
show the documentation for the <i>typedefs</i> on the "Data
Structures" page? Unfortunately I don't know how.

[codehelpgpg 2004-07-25] Yes, there is a way of linking to these data
structures.
Make sure that the private header is included in the documentation by
including a
\\file command in the private header file. Then include a short doxygen
comment above the declaration. Doxygen will accept both valid C struct
definition formats.

*/

/*! \page reference Doxygen reference documentation

- \ref index Introduction
- \ref tipshints Tips and hints for using doxygen
- \ref stylediscussion Long comments, pages, editors
	
The Doxygen web site (http://www.stack.nl/~dimitri/doxygen/) has a
complete user manual.  For the impatient, here are the most
interesting sections:

- How to write grouped documentation for files, functions, variables,
etc.: http://www.stack.nl/~dimitri/doxygen/grouping.html .  Do not
forget to add a file documentation block (\@file) at the top of your
file. Otherwise, all documentation in that file will <i>not</i> appear
in the html output.

- List of the special commands you can use within your documentation
blocks: http://www.stack.nl/~dimitri/doxygen/commands.html

\section contact Contacts

\subsection web Web Site
News about GnuCash as well as the latest version can always be found at
http://www.gnucash.org/

\subsection email Email
If you have any suggestions concerning this documentation, do not hesitate to
send suggestions to gnucash-devel (see http://www.gnucash.org/en/lists.phtml
for details)

 */
