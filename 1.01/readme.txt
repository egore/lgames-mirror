

                            JWordTrainer v1.01

                        Released under GNU GPL v3



Table of Contents:
    1. Introduction
    2. How to Use
    3. How to Add Words to an Existing Language
    4. How to Add a New Language
    5. Troubleshooting
    6. Feedback


1. Introduction
---------------

JWordTrainer is a generic tool to help you memorizing words for basically any 
pair of languages (the interface is always in English though). As a pure 
HTML/Javascript web application it will run in any browser on any OS
(Windows, Linux, Mac, ...).

When a language pair and vocabulary have been defined (read section 3 and 4 to 
learn more) this tool works as follows: You specify how many words from which 
input lessons you want to be asked and the programme will randomly choose that 
many. The first word is displayed, you think about it and then click a button
to show the translation. After that you may decide whether to keep the word in 
the quiz (e.g., because you were wrong, it will be asked again later) or not. 
Then another word is shown. Like this, you continue until no more words are 
left.

There are no statistics or any checks whether you really knew the word or not.
This tool is not meant for comparisons or tests but it will help you memorizing
words quickly by...
    - ... asking single words in a random order. There is no associative
      context (like position in some list), so you really have to mug up
      the words itself.
    - ... asking words again later if desired. This allows you to internalize 
      difficult words---the easy ones drop out quickly, the harder ones remain
      until you get them right.
    - ... allowing you to choose input lessons, the asked language and number 
      of words according to your needs and time at hand.
    - ... being usable anywhere: Just put it on a pendrive or upload it to some
      webspace and you can go for a few words whenever and where ever you have
      access to a computer or internet.
      
2. How to Use
-------------

This application was developed during my stay in Dhammagiri, Igatpuri, India 
for the Pali Programme 2010. Therefore, the example language pair included is 
Pali-English. How to use the programme is explained referring to this.

Basically, each language pair has one HTML-file (e.g., pali.htm) which defines 
the layout of the programme and has to be run in a browser (e.g. by 
double-clicking the file in the Explorer or entering the file path as URL in 
the browser). This will start the word trainer with the settings and vocabulary
of this language pair.

On the left-hand side you can now choose the lessons from which you want words
to be asked (Note, that you can use CTRL to select multiple entries from the
list and SHIFT to select a range of entries just like in the Explorer.), the 
number of words and in which language you want to be asked. After having made
your choice hit the "Start" button.

On the right-hand side the word is now displayed in the selected language. The
"???" below marks the position where the translation will be shown. You may 
either use the three buttons
    "Show" - show translation of the asked word
    "Next" - remove word from quiz and ask next word
    "Keep" - keep word in quiz (ask again later) and ask next word
or the key commands
    SPACE           - show translation *if not shown* OR remove word from quiz 
                      and ask next word *if translation is shown*
    ESC, "a" or "k" - keep word in quiz (ask again later) and ask next word.
Thus, as long as you know the words you can comfortably use SPACE (first to 
show translation, then to proceed to next word) as single key.

A simple search functionality is also provided. Click on the "Search" link 
below the "Lessons" heading on the left-hand side to access it. Note, that 
navigating to Search will cancel the current quiz. The search expression has
to be in the input file word format (e.g., Pali Velthius format). All files
will be searched and all words that contain the given expression as a 
substring will be displayed. 

3. How to Add Words to an Existing Language
-------------------------------------------

While HTML/Javascript gives maximum portability, it has one limitation: There 
is no way to access files. Therefore, the input files have to be manually 
edited and included to the corresponding HTML-file (e.g. pali.htm) with a 
text editor. Don't worry, this is quite easy.

For each language pair defined exists a directory (e.g., pali). This directory
contains a file named lang.js which provides language specific definitions (see
section 4 to learn more; it can be ignored when just adding words) and any 
number of word input files ending in .js (e.g., pali2010.js).

If you want to add a new word input file just copy an existing one (e.g. 
pali2010.js) and rename it (e.g., if you want to add a new file with all
exercises from "Introduction to Pali" by A.K. Warder you could name
it something like warderex.js).

Then open this file with a text editor and clear it by removing all 
double-quoted lines below the line
 
    var jwtInputWords = new Array(
    
and change the caption to, e.g., "Warder Exercises". Note: Anything between /* 
and */ is a comment and will be ignored. Your file should now look like this:
 
    /* Pali/English word pairs. Pali is Velthius format. */
    var jwtInputCaption = "Warder Exercises";
    var jwtInputWords = new Array(
    );

Words are defined as actual word in first language, hash-symbol # as separator 
and actual word in second language, e.g. for Pali-English "naro # man, person".
Note: It is important to enclose the definition in double-quotes and add a 
comma at the end of the line except for the very last entry.

You can define multiple lessons within one file by using the caption-keyword:
Whenever an entry begins with "caption:" followed by some name, all word 
entries below it will be added to a new lesson by that name (until another
"caption:" entry is encountered or no more word definitions are left). If no
caption-keyword is used all words will be put into one lesson. Note: There
must not be blanks between the double-quote and the caption-keyword.

E.g., with the first three exercises of Warder's first two chapters your file
would look like this:

    /* Pali/English word pairs. Pali is Velthius format. */
    var jwtInputCaption = "Warder Exercises";
    var jwtInputWords = new Array(
    
    "caption: Lesson 01",
    "tathaagato bhaasati # the thus-gone speaks",
    "upaasako pucchati # the lay follower asks",
    "puriso eva.m vadati # the man speaks thus",
    
    "caption: Lesson 02",
    "sugato dhamma.m bhaasati # the well-gone speaks the doctrine",
    "upaasako patta.m aaharati # the lay devotee brings the bowl",
    "manussaa bhava.m icchanti # human beings desire becoming"
    
    );

Note, that there is no comma behind the last entry. The required input format 
of words should be explained either in lang.js or given as a comment in the
input files. The former is probably the better solution. For Pali it is just 
stated that the format is Velthius (thus long a is aa for example, retroflex 
t is .t and so forth).

Very important: If double quote is used within the definition (this is for  
example the case with the "n letter in Pali) it has to be "escaped" by 
backslash, e.g.,
    " a\"nguli (f) # finger, toe "

Entries to existing files are just added in the same manner.

New files must be included to the HTML file (e.g., pali.htm) for parsing.
Therefore, open the HTML file with an editor and search for the line

    <!-- LANGUAGE DEPENDENT SECTION BEGIN -->
    
Only this section differs for each language, the rest of the file is always 
the same. The first entry loads the language settings (e.g., pali/lang.js), the
other entries load the word input files.

Now, copy and paste any of the include commands in this section, e.g.,

    <script src="pali/pali2010.js" type="text/javascript">...(snip)...
    
and adjust the file name (e.g. change "pali/pali2010.js" to
"pali/warderex.js"). This will give you something like:

    <script src="pali/pali2010.js" type="text/javascript">...(snip)...
    <script src="pali/warderex.js" type="text/javascript">...(snip)...

Then refresh the page in the browser to reload the data (also required when
existing files have been changed). The new lessons will show up in the list 
(e.g., "Warder Exercises - Lesson 01").

4. How to Add a New Language
----------------------------                        

First, a new directory has to be created (e.g., german, if you want to add 
files for German-English) and a new HTML file is needed (e.g., copy pali.htm 
to german.htm).

Then copy an existing lang.js from any directory (e.g., pali/lang.js to 
german/lang.js) and adjust the language names (used for language selection, 
title, etc.) in the jwtInputLanguageNames array, e.g.:

    var jwtInputLanguageNames = new Array( "German", "English" );

Now the tricky part which might need some programming: The main purpose of 
lang.js is to provide a mapping for the words in some input format into 
something the browser displays properly (best is HTML UTF-8 encoding). 
 
But: For most languages it should be sufficient to simply use UTF-8 as encoding
for input files and to type the words straight away with special characters. 
Most browsers should be able to handle this. If it works for you, there is no
need for an encoding function and encodeWordToHTML() can be deleted leaving
only the definition of the language names in lang.js.

However, for maximum portability to all enviroments it is recommended to use
a format which conforms to ASCII (e.g. Velthius for Pali) for all input files
and map the transliterations of non-ASCII characters to HTML encoded UTF-8 
characters. pali/lang.js is a good example for this approach. How to do this 
for German-English is just outlined here as another example.   

As described in section 3 already, each word, regardless of the file encoding,
consists of the actual word in the first language, the hash-symbol # as a 
separator and the actual word in the second language. For more on adding and
editing word input files read section 3.

So let us assume some encoding is needed. E.g., for German the u-umlaut is
represented as "u in input files and has to be encoded to the corresponding
unicode character 00FC. In this case the function

    function encodeWordToHTML( word, lang )
    
has to be kept in lang.js and adjusted properly. The parameter @word is the
word as described in the paragraph above (actual word in both languages 
separated by #) and the parameter @lang is either 0 (= return encoded word in
first language) or 1 (= return encoded word in second language).  

Since in this example the second language, English, needs no changes, the first 
part of the function remains unchanged. In the second part, however, the 
replace calls have to be adjusted. The first argument defines what has to be 
replaced (/g indicating that all occurences shall be replaced), the second with
what it has to be replaced. So for replacing our u-umlaut we have to remove
all replace-calls except for one:

    str = str.replace(/"u/g,"&#x00fc;");

This will replace all occurrences of "u with &#x00fc; (the HTML encoded unicode 
character 00FC). Note, that a double-quote in the search expression does not 
have to be escaped by a backslash as it has to be in the word input files (see
section 3). To complete the encoding for German language all umlauts would 
require a similar replacement command.

After completing lang.js and defining words in input files accordingly (e.g.,
german/lesson01.js) these files have to be included to the HTML file (e.g.,
german.htm). Therefore, open this file, search for

<!-- LANGUAGE DEPENDENT SECTION BEGIN -->

and adjust the include calls in this section. E.g., to include the language 
definition and the first lesson for German: 

    <script src="german/lang.js" type="text/javascript"></script>
    <script src="german/lesson01.js" type="text/javascript">(...snip...)

Note, that the very first line for lang.js is shorter because it does not need 
a call to function addInputFile. 
  
5. Troubleshooting
------------------

Two of the largest pitfalls in editing input files are to forget to put a
backslash in front of any double-quotes in word definitions and to put a comma
after the last entry in the array. Both errors will break the file without any
obvious notice. Indicators are, that the previous lesson name in the list 
appears twice or that there are no words (because the file has not been 
processed). The error will be shown in the Error Console of Firefox though. So 
if you have any trouble with input files, use Firefox, open the Error Console 
(to be found in menu Tools) and check if there are any error messages. 
                                                                                
6. Feedback
-----------

Suggestions, comments and other feedback is very welcome: kulkanie@gmx.net


Be happy!
