/********************************************************************************
*                                                                               *
*                      O n - L i n e   H e l p   T e x t                        *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2004 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This program is free software; you can redistribute it and/or modify          *
* it under the terms of the GNU General Public License as published by          *
* the Free Software Foundation; either version 2 of the License, or             *
* (at your option) any later version.                                           *
*                                                                               *
* This program is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
* GNU General Public License for more details.                                  *
*                                                                               *
* You should have received a copy of the GNU General Public License             *
* along with this program; if not, write to the Free Software                   *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.    *
*********************************************************************************
* $Id: help.cpp,v 1.7 2004/05/15 17:15:30 fox Exp $                             *
********************************************************************************/
#include "help.h"


// Help text
const char help[]=
"\n"
"\n"
"                                                      A . d . i . e\n"
"\n"
"                                         The ADvanced Interactive Editor\n"
"\n"
"\n"
"\n"
"\n"
"Introduction.\n"
"\n"
"Adie is an extremely fast and convenient programming text editor written using the FOX Toolkit.\n"
"Besides being a nice text editor, Adie is also an extremely convenient file viewer, "
"supporting a plethora of methods to move from one file to the next.\n\n"
"For each visited file, Adie remembers where in that file you were last looking, and "
"which special places in that file have been bookmarked, so you can quickly return to frequently "
"visited places.\n\n"
"Project browsing is made easy by optionally displaying a File/Directory browser side by side "
"with the text so files may be visited by means of a single click.  Files may also be opened "
"simply by highlighting a file name, compiler warning message, or #include directive and hitting "
"a single button; Adie will search for the file in the same directory as an already "
"loaded file, or in a number of predefined directories (like for example include directories).\n"
"\n"
"\n"
"General Features.\n"
"\n"
"  - Programmable syntax coloring and text styles.\n"
"  - Active or current text line highlighting.\n"
"  - Optional line number display.\n"
"  - Preferences are configurable with convenient dialogs.\n"
"  - Text size only limited by available memory.\n"
"  - Persistent bookmarks may be set for each file.\n"
"  - Persistent scroll position is remembered for each visited file.\n"
"  - Wheel mouse scroll support.\n"
"  - Online help built in.\n"
"\n"
"File Access Features.\n"
"\n"
"  - File Dialog supporting bookmarked directories.\n"
"  - Dropping a file into Adie opens the file.\n"
"  - Single-click access to file using File/Directory browser.\n"
"  - Visit recently edited files.\n"
"  - Jump to file and line number of highlighted compiler warnings or errors.\n"
"  - Customizable list of file patterns; show only source files, for example.\n"
"  - Open a file by highlighting a #include directive.\n"
"  - Customizable list of directories to search for header files.\n"
"  - Detect if file changed outside of editor to prevent accidental overwriting.\n"
"\n"
"Editing Features.\n"
"\n"
"  - Unlimited undo/redo capability.\n"
"  - Cut and paste to clipboard support.\n"
"  - X11 primary selection support using middle-mouse paste.\n"
"  - Drag-and-drop support for text move or copy.\n"
"  - Brace matching, statement block, and expression block selection.\n"
"  - Up to 10 persistent bookmarks may be set in each file.\n"
"  - Search and replace history is stored persistently.\n"
"  - Auto-indent feature.\n"
"  - Strip trailing spaces, strip carriage-returns for DOS files.\n"
"  - Indent or unindent selected text; clean indentation.\n"
"  - Search for highlighted text.\n"
"  - Goto line number, goto highlighted line number, goto matching brace, goto begin/end of block.\n"
"  - Fixed word wrap column, or continuous word wrap mode.\n"
"  - Mouse wheel or right-mouse scroll support.\n"
"  - Customizable word-delimiters to select words by double-clicks.\n"
"  - Insert files or extract highlighted text to file.\n"
"\n"
"Syntax Coloring Features.\n"
"\n"
"  - Syntax coloring is programmable, using PERL-like regular expressions.\n"
"  - Syntax patterns are keyed to language or file extension.\n"
"  - Text styles including foreground, background, selection colors, as well as underlining and strike-throughs.\n"
"  - Syntax expressions, text styles, and language modes can be set up from convenient dialogs.\n"
"  - Color fallback mechanism to minimize style setup hassles.\n"
"\n"
"\n"
"Starting a New Document.\n"
"\n"
"To start a new document, invoke the New menu, or press on the New button on "
"the toolbar.  If the current document has not yet been saved, you will be prompted "
"to either save or abandon the current text.\n"
"\n"
"\n"
"Opening and Saving Files the Old Fashioned Way.\n"
"\n"
"To open a file, you can invoke the Open menu, or press on the Open button on "
"the toolbar.  This will bring up the standard File Dialog, which allows you "
"to select a file.\n"
"To save a file, you can either invoke the Save or the Save As menu option. "
"The former saves the file back to the same filename, while the latter prompts "
"for an alternative filename.\n\n"
"You can quickly navigate to the desired file by typing the first few letters of "
"the filename and then pressing Enter (Return); use Backspace to move up one directory "
"level.  Control-H moves to your home directory, Control-W moves back to the current "
"working directory.\n\n"
"A nice convenience of the File Dialog is the ability to set bookmarks, so once "
"bookmarked, you can quickly move back to a previously visited directory.\n"
"\n"
"\n"
"Opening Files Using the File/Directory Browser.\n"
"\n"
"An alternative method to open files is the File/Directory Browser. "
"You can display the File/Directory Browser by invoking the File Browser option under "
"the View menu.  "
"To open a file using the File/Directory Browser, simply click on the file. "
"If there are many files, you may want to limit the number of files displayed "
"by specifying a file pattern in the Filter typein field.\n"
"The pattern is can be any regular file wildcard expression such as \"*.cpp\".  "
"By default, the File/Directory Browser shows all files, i.e. the pattern is \"*\".\n"
"You can switch patterns by means of the combo box under file File Browser; "
"additional patterns for the combo box (and File Dialog) can be specified "
"in the Preferences Dialog.\n"
"\n"
"\n"
"Opening Recently Visited Files.\n"
"\n"
"The recent file menu shows files which have been recently visited.  "
"You can quickly get back to a file you've been editing simply by selecting "
"one of these recent files.\n"
"\n"
"\n"
"Opening Files by Drag and Drop.\n"
"\n"
"Using a file browser such as PathFinder or other Konqueror or other XDND "
"compatible file browsers, you can simply drop a file into the text pane and "
"have Adie read this file.\n"
"Of course, the File/Directory browser supports drag and drop also, so you "
"can also drag a file from the File/Directory browser into the Text Window.\n"
"\n"
"\n"
"Opening a Selected Filename\n"
"\n"
"Selecting any filename, possibly in another application, and invoking the "
"\"Open Selected\" option causes Adie to open the selected file. "
"When the selected filename is of the form:\n"
"\n"
"   #include \"filname.h\"\n"
"\n"
"or:\n"
"\n"
"   #include <filename.h>\n"
"\n"
"then Adie will search for this file in a sequence of include directories, "
"otherwise it will search in the same directory as the currently loaded file.  "
"You can specify the list of include directories to search with the \"Include Path\" "
"option.\n"
"When the selected filename is of the form:\n"
"\n"
"   filename.cpp:177\n"
"\n"
"then Adie will not only load the filename, but also jump to the given line "
"number.  If this file has already been loaded, Adie will simply jump to the "
"given line number in the current file.  "
"This option is very useful when fixing compiler errors.\n"
"\n"
"\n"
"Mouse Selection.\n"
"\n"
"You can move the cursor by simply clicking on the desired location with the left mouse button. "
"To highlight some text, press the mouse and drag the mouse while holding the left button. "
"To select text a word at a time, you can double-click and drag; "
"to select text a line at a time, you can triple-click and drag.\n"
"Performing a shift-click extends the selection from the last cursor location to the "
"current one.\n"
"When selecting words, words are considered to extend from the clicked position up to "
"a blank or word-delimiting character.  The latter may depend on the programming language, "
"and so Adie offers a way to change the set of delimiter characters.\n"
"\n"
"\n"
"Scrolling Text.\n"
"\n"
"Using the right mouse button, you can grab the text and scroll it. "
"a right mouse drag is a very convenient way to scroll the text buffer by small amount "
"as the scrolling is exactly proportional to the mouse movement.\n"
"You can of course also use the scroll bars.  Because scrolling becomes awkward when "
"dealing with large amounts of text, you can do a fine scroll or vernier-scroll by "
"holding the shift or control keys while moving the scroll bars.\n\n"
"Adie can also take advantage of a wheel mouse; simply point the mouse inside the "
"text area and use the wheel to scroll it up and down.  Holding the Control-key while "
"operating the wheel makes the scrolling go faster, by smoothly scrolling one page at "
"a time.  To scroll horizontally, simply point the mouse at the horizontal scroll bar.\n"
"In fact, any scrollable control (including the File/Directory Browser), can be scrolled by "
"simply pointing the cursor over it and using the mouse wheel.\n"
"You can adjust the number of lines scrolled for each wheel notch by means of the "
"Preferences dialog.\n"
"\n"
"\n"
"The Clipboard.\n"
"\n"
"After selecting some text, you can cut or copy this text to the clipboard. "
"A subsequent paste operation will then insert the contents of the clipboard at "
"the current cursor location.\n"
"If some text has been selected in another application, then you can paste this "
"text by placing the cursor at the right spot in your text and invoking the paste "
"command.\n"
"\n"
"\n"
"The Primary Selection.\n"
"\n"
"When text is selected anywhere (possibly in another application), Adie can "
"paste this text into the current text buffer by means of the middle mouse button "
"or by pressing the wheel-button if you have a wheel mouse.  Note that while holding "
"the button, the insertion point can be moved by moving the mouse:- Adie will "
"only insert the text when you release the button.\n"
"\n"
"\n"
"Text Drag and Drop.\n"
"\n"
"After selecting some text, you can drag this text to another location by pressing the "
"middle mouse button; because Adie is fully drag and drop enabled, you can not only "
"drag a selection from one place to another inside the text buffer, but also between different "
"Adie applications, or even from Adie to another drag and drop enabled application "
"or vice-versa.\n"
"Within the same text window, the drag defaults to a text-movement.  You can change this to "
"a text copy by holding down the Control key while you're dragging.\n"
"Between one text window and another, the drag defaults to a copy operation "
"you can change this to a text movement by holding down the Shift key while dragging.\n"
"\n"
"\n"
"Undo and Redo.\n"
"\n"
"Adie support unlimited (well, the limit is large...) undo and redo capability.  "
"Each time you insert, remove, or replace some text, Adie remembers what you did.\n"
"If you make a mistake, you can undo the last command, and the one before that, and so on. "
"Having invoked undo many times, it is sometimes desirable to invoke the redo command, i.e. "
"to perform the original editing operation again.  Thus, you can move backward or forward in "
"time.\n"
"However if, after undoing several commands, you decide edit the buffer in a different way, then "
"you will no longer be able to redo the undone commands:- you have now taken a different path.\n"
"When you first load a file, or just after you save it, Adie remembers that this version of "
"the text was special; while subsequent editing commands can be undone individually, you can always "
"quickly return to this special version of the text by means of the revert command.\n"
"\n"
"\n"
"Search and Replace.\n"
"\n"
"Adie uses the standard Search Dialog for searching strings. The search dialog offers"
"forward and backward searches (relative to the current cursor location), and three "
"search modes:\n"
"\n"
"Exact. In the Exact mode, a search looks for a verbatim occurrence of the search string in "
"the text.\n"
"\n"
"Ignore Case. In the Ignore Case mode, a search looks for a verbatim occurrence also, but while "
"disregarding the uppercase/lowercase distinctions.\n"
"\n"
"Expression. In the Expression mode, the the search performs a full regular expression match.  "
"The regular expression syntax being used is very similar to PERL and is as follows:\n"
"\n"
"  ^\tMatch begin of line\n"
"  $\tMatch end of line\n"
"  .\tMatch any character\n"
"  |\tAlternation\n"
"  ( ... )\tGrouping sub pattern\n"
"  (?i ... )\tMatch sub pattern case insensitive\n"
"  (?I ... )\tMatch sub pattern case sensitive\n"
"  (?n ... )\tMatch sub pattern with newlines\n"
"  (?N ... )\tMatch sub pattern with no newlines\n"
"  (?: ... )\tNon-capturing parentheses\n"
"  (?= ... )\tZero width positive lookahead\n"
"  (?! ... )\tZero width negative lookahead\n"
"  []\tCharacter class\n"
"\n"
"  *\tMatch 0 or more [greedy]\n"
"  +\tMatch 1 or more [greedy]\n"
"  ?\tMatch 0 or 1 [greedy]\n"
"  {}\tMatch 0 or more [greedy]\n"
"  {n}\tMatch n times [greedy]\n"
"  {,m}\tMatch no more than m times [greedy]\n"
"  {n,}\tMatch n or more [greedy]\n"
"  {n,m}\tMatch at least n but no more than m times [greedy]\n"
"\n"
"  *?\tMatch 0 or more [lazy]\n"
"  +?\tMatch 1 or more [lazy]\n"
"  ??\tMatch 0 or 1 [lazy]\n"
"  {}?\tMatch 0 or more times [lazy]\n"
"  {n}?\tMatch n times [lazy]\n"
"  {,m}?\tMatch no more than m times [lazy]\n"
"  {n,}?\tMatch n or more [lazy]\n"
"  {n,m}?\tMatch at least n but no more than m times [lazy]\n"
"\n"
"  \\a\tAlarm, bell\n"
"  \\e\tEscape character\n"
"  \\t\tTab\n"
"  \\f\tForm feed\n"
"  \\n\tNewline\n"
"  \\r\tReturn\n"
"  \\v\tVertical tab\n"
"  \\cx\tControl character\n"
"  \\033\tOctal\n"
"  \\x1b\tHex\n"
"  \\w\tWord character [a-zA-Z_0-9]\n"
"  \\W\tNon-word character\n"
"  \\l\tLetter [a-zA-Z]\n"
"  \\L\tNon-letter\n"
"  \\s\tSpace\n"
"  \\S\tNon-space\n"
"  \\d\tDigit [0-9]\n"
"  \\D\tNon-digit\n"
"  \\h\tHex digit [0-9a-fA-F]\n"
"  \\H\tNon-hex digit\n"
"  \\b\tWord boundary\n"
"  \\B\tWord interior\n"
"  \\u\tSingle uppercase character\n"
"  \\U\tSingle lowercase character\n"
"  \\p\tPunctuation (not including '_')\n"
"  \\P\tNon punctuation\n"
"  \\<\tBegin of word\n"
"  \\>\tEnd of word\n"
"  \\A\tMatch only beginning of string\n"
"  \\Z\tMatch only and end of string\n"
"  \\1...\\9\tBack reference\n"
"\n"
"\n"
"Keyboard Bindings.\n"
"\n"
"The following table lists the keyboard bindings.\n"
"\n"
"Key:\tAction:\n"
"====\t=======\n"
"\n"
"Up\tMove cursor up.\n"
"Shift+Up\tMove cursor up and extend selection.\n"
"Down\tMove cursor down.\n"
"Shift+Down\tMove cursor down and extend selection.\n"
"Left\tMove cursor left.\n"
"Shift+Left\tMove cursor left and extend selection.\n"
"Right\tMove cursor right.\n"
"Shift+Right\tMove cursor right and extend selection.\n"
"Home\tMove cursor to begin of line.\n"
"Shift+Home\tMove cursor to begin of line and extend selection.\n"
"Ctl+Home\tMove cursor to top of text.\n"
"End\tMove cursor to end of line.\n"
"Ctl+End\tMove cursor to bottom of text.\n"
"Shift+End\tMove cursor to end of line and extend selection.\n"
"Page Up\tMove cursor up one page.\n"
"Shift+Page Up\tMove cursor up one page and extend selection.\n"
"Page Down\tMove cursor down one page.\n"
"Shift+Page Down\tMove cursor down one page and extend selection.\n"
"Insert\tToggle between insert mode and overstrike mode.\n"
"Delete\tDelete character after cursor, or text selection.\n"
"Back Space\tDelete character before cursor, or text selection.\n"
"Ctl+A\tSelect all text.\n"
"Ctl+X\tCut selected text to clipboard.\n"
"Ctl+C\tCopy selected text to clipboard.\n"
"Ctl+V\tPaste text from clipboard.\n"
"\n"
"\n"
"Changing Font.\n"
"\n"
"You can change font by invoking the Font Selection Dialog from the Font menu. "
"The Font Dialog displays four list boxes showing the font Family, Weight, Style, "
"and Size of each font. \n"
"You can narrow down the number of fonts displayed by selecting a specific character "
"set, setwidth, pitch, and whether or not scalable fonts are to be listed only.\n"
"The All Fonts checkbutton causes all fonts to be listed. Use this feature if you "
"need to select old-style X11 bitmap fonts.\n"
"The Preview window shows a sample of text in the selected font.\n"
"\n"
"\n"
"Changing Preferences.\n"
"\n"
"Since there is no accounting for tastes, Adie can be extensively configured "
"by means of the Preferences dialog. "
"The Preferences dialog is comprised of a number of subpanes which allow you to "
"change colors, editor modes, file patterns, and word delimiters.\n"
"\n"
"Changing Colors.\n"
"\n"
"The colors subpane allows you to change the colors used in the File/Directory browser, "
"and the Text Window.\n"
"You can simply drag colors from one color well to another, or you can double-click "
"on a color well and bring up the Color Dialog.\n"
"The Color Dialog offers a number of ways to create a new color, either by selecting one of "
"the pre-defined color wells, by mixing a custom color in RGB, HSV, or CMYK color space, "
"or by selecting a named color from a list.\n"
"\n"
"\n"
"Changing Editor Settings.\n"
"\n"
"The editor subpane is used to change various modes of the editor:\n"
"\n"
"Word Wrapping.  This enables word wrapping mode for the Text Window; when "
"word wrapping is turned on, text flowed to stay within the wrap margins. \n"
"\n"
"Fixed Wrap Margin.  When this feature is enabled, the text is flowed to stay within "
"a fixed wrap column; when disabled, the Text Window is in continuous wrapping mode, "
"and the text is flowed to fit the size of the Text Window.  This option has no effect "
"if the Word Wrapping option is off.\n"
"\n"
"Wrap Margin.  This specifies the column at which paragraphs are to be wrapped; the "
"wrap margin only has effect if the fixed wrap margin option is selected.\n"
"\n"
"Tab Columns.  This specifies the tab spacing, in terms of spaces.\n"
"\n"
"Strip Carriage Returns.  When this option is selected, MS-Windows/MS-DOS text files with "
"lines ending in CRLF are transformed into LF only files upon read; this is mostly of interest "
"when loading DOS files on a UNIX system; when Adie runs under MS-Windows, files are normally "
"automatically translated back and forth.\n"
"\n"
"Strip Trailing Spaces.  When selected, Adie will remove all white-space after the last "
"printable character when reading a file.\n"
"\n"
"Auto Indent.  This causes spaces or tabs to be inserted to match the previous line; "
"this option is meant for programmers.\n"
"\n"
"Brace Match Time.  When entering a brace, parenthesis, or bracket, the cursor temporarily "
"jumps to the matching brace and pauses there for a bit before hopping back to the insert "
"position.  If the brace match time is set to 0, then this feature is disabled.\n"
"\n"
"Insert Tab Characters.  When this option is selected, hard tab characters are inserted when "
"entering the tab key.  If the option is not selected then entering the tab key will insert "
"the corresponding number of spaces into the text buffer.\n"
"\n"
"Mouse Wheel Lines.  This is the number of lines scrolled when the mouse wheel is rotated by one "
"notch.  When holding the control key while using the wheel, the text scrolls by one page "
"at a time; when holding the alt key, it scrolls one pixel at a time.\n"
"\n"
"Save View of File.  When this option is selected, the current view or scroll position is "
"saved so that a subsequent visit to this file can immediately jump back to the same view.\n"
"Disabling this option will cause all remembered views to be forgotten.\n"
"\n"
"Save Bookmarks.  When selected, all currently set bookmarks will be saved for this file; "
"a subsequent visit to this file will restore the bookmarks.\n"
"Disabling this option will cause all remembered bookmarks to be forgotten.\n"
"\n"
"\n"
"Changing File Patterns.\n"
"\n"
"This subpane allows you to enter a list of file patterns, one pattern on each line. "
"These patterns are used in the File/Directory browser and the File Dialog.  They are "
"especially useful in the File/Directory browser as it allows you to cause the File/Directory "
"browser to only show those file types you want to see (e.g. only source files).\n"
"Each line of a pattern has the format:\n"
"\n"
"   patternname (patternlist)\n"
"\n"
"Where patternname is the name of the pattern (e.g. \"C Source\") and the patternlist is "
"a comma separated list of patterns (for example \"*.h,*.c\").\n"
"The patternname is optional.\n"
"Some examples from my own setup of Adie (you can paste these from this help window "
"if you want) are shown below:\n"
"\n"
"  All Files (*)\n"
"  All Source (*.cpp,*.cxx,*.cc,*.C,*.c,*.hpp,*.hxx,*.hh,*.H,*.h,*.y,*.l)\n"
"  C++ Source Files (*.cpp,*.cxx,*.cc,*.c,*.C)\n"
"  C++ Header Files (*.h,*.hpp,*.hxx,*.hh,*.H)\n"
"  C Source Files (*.c)\n"
"  C Header Files (*.h)\n"
"  Python Files (*.py)\n"
"  Perl Files (*.pl)\n"
"  Ruby Files (*.rb)\n"
"  Lex (*.l)\n"
"  Yacc (*.y)\n"
"  Object (*.o)\n"
"  X Pixmap (*.xpm)\n"
"  X Bitmap (*.xbm)\n"
"\n"
"Some details on the allowable wild-card patterns:\n"
"\n"
"  ?\tMatches single character.\n"
"  *\tMatches zero or more characters.\n"
"  [abc]\tMatches a single character, which must be a, b, or c.\n"
"  [^abc]\tMatches a single character, which must be anything other than a, b, or c.\n"
"  [!abc]\tDitto.\n"
"  [a-zA-Z]\tMatches single character, which must be one of a-z or A-Z.\n"
"  [^a-zA-Z]\tMatches single character, which must be anything other than a-z or A-Z.\n"
"  [!a-zA-Z]\tDitto.\n"
"  pat1|pat2\tMatches either pat1 or pat2.\n"
"  pat1,pat2\tDitto.\n"
"  (pat1|pat2)\tMatches either pat1 or pat2; patterns may be nested.\n"
"  (pat1,pat2)\tDitto.\n"
"\n"
"\n"
"Changing Delimiters.\n"
"\n"
"Different programming language have different notions about identifiers.  Adie "
"accomodates for this by allowing you to specify which characters (besides white space, "
"of course) are used to delimit identifiers or tokens in the programming language. "
"The default set of delimiters is \"!\"#$%&'()*+,-./:;<=>?@[\\]^`{|}~\".\n"
"\n"
"\n"
"Other Configuration Issues.\n"
"\n"
"Because Adie is an editor written to use the FOX Toolkit, there are various other "
"items, common to all FOX Toolkit programs, which may be configured either by hand or "
"using some other control panel.\n"
"A few items of particular interest are list below:\n"
"\n"
"  [SETTINGS]\n"
"  typingspeed=800\n"
"  clickspeed=400\n"
"  scrollspeed=80\n"
"  scrolldelay=600\n"
"  blinkspeed=500\n"
"  animspeed=10\n"
"  menupause=400\n"
"  tippause=800\n"
"  tiptime=3000\n"
"  dragdelta=6\n"
"  wheellines=1\n"
"  bordercolor=black\n"
"  basecolor=AntiqueWhite3\n"
"  hilitecolor=AntiqueWhite\n"
"  shadowcolor=AntiqueWhite4\n"
"  backcolor=AntiqueWhite1\n"
"  forecolor=black\n"
"  selforecolor=AntiqueWhite\n"
"  selbackcolor=#aea395\n"
"  tipforecolor=yellow\n"
"  tipbackcolor=black\n"
"  normalfont=\"[lucidatypewriter] 90 700 1 1 0 1\"\n"
"  iconpath = /usr/share/icons:/home/jeroen/icons\n"
"\n"
"These settings can be either placed in $HOME/.foxrc (and thus affect all FOX programs), "
"or in $HOME/.foxrc/FoxTest/Adie (only applying to Adie).\n"
"File types may be bound to a command, mime-type, and icons using statements like "
"the one below:\n"
"\n"
"  [FILETYPES]\n"
"  cpp = \"/usr/local/bin/textedit %s &;C++ Source File;c_src.xpm;mini/c_src.xpm\"\n"
"  /home/jeroen = \";Home Directory;home.xpm;mini/home.xpm;application/x-folder\"\n"
"  defaultfilebinding = \"/usr/local/bin/textedit %s &;Document;document.xpm;mini/document.xpm\"\n"
"  defaultexecbinding = \";Application;exec.xpm;mini/exec.xpm;application/x-executable-file\"\n"
"  defaultdirbinding = \";Folder;folder.xpm;mini/folder.xpm;application/x-folder\"\n"
"\n"
"This example shows how the extension \".cpp\" is bound to the program \"textedit\" "
"and is associated with two icons, a big icon \"c_src.xpm\" and a small icon \"mini/c_src.xpm\", "
"which are to be found in the directories determined by \'iconpath\", in this case, "
"\"/usr/share/icons\" or \"/home/jeroen/icons\".\n\n"
"It also binds two icons \"home.xpm\" and \"mini/home.xpm\" to the home directory "
"\"/home/jeroen\".\n\n"
"Finally, it assigns icons, commands, and mime-types to unbound documents, executables, "
"and directories, overriding the built-in icons of the FOX Toolkit.\n"
;
