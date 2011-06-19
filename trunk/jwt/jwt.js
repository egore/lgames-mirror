/* Copyright 2011 Michael Speck, Published under GNU GPL v3 */

/** Variables and functions for JWordTrainer. */

var jwtInput = []; /* array of all input lessons */
var jwtLanguageNames = [ "Lang1", "Lang2" ]; /* overwritten by input */
var jwtNumWantedWords = 30; /* number of words to ask */
var jwtQuestionMode = 0; /* 0 = first language, 1 = second */
var jwtWords = []; /* all (remaining) words to be asked */
var jwtNumWords = 0; /* number of unsolved entries */
var jwtCurrentWordIdx = -1; /* currently asked word */ 
var jwtLastWordIdx = -1; /* index of last asked word */
var jwtSolutionIsShown = 0; /* whether solution is visible */
var jwtQuizStarted = 0; /* whether any action occured (show/keep/next) */
var jwtDialogId = "main"; /* id of currently displayed window */

/** Use language settings from lang.js (loaded as hardcoded file
 * in HTML before calling this on loading page---ugly!!!). */  
function useInputLanguageSettings()
{
	jwtLanguageNames = jwtInputLanguageNames;
}

/** Show program name and languages in span spanTitle. */
function updateTitle()
{
	var title;
	var obj = document.getElementById("spanTitle");

	title = "JWordTrainer - (" + jwtLanguageNames[0] + " / " + 
						jwtLanguageNames[1] + ")";
	if (obj) 
		obj.innerHTML = title;
}

/** Select/Unselect all input files. @selected is 0 or 1. */
function selectAllInput( selected )
{
	var i, obj = document.getElementById("selInput");
	
	for (i = 0; i < jwtInput.length; i++) {
		obj.options[i].selected = selected;
		jwtInput[i]["selected"] = selected;
	}
}

/** Create and return empty lesson object with caption @caption. */
function createEmptyLesson( caption )
{
	var obj = new Object();
	
	obj["words"] = new Array();
	obj["searchkeys"] = new Array(); /* words in search format */
	obj["caption"] = caption;
	obj["selected"] = 0;
	
	return obj;
}

/** Check syntax of word (must contain a #-symbol and both subentries must
 * be non-empty). For format of @word see comment for encodeWordToHTML(). 
 * Return 0 on success or -1 on error and show error message. */
function checkWordSyntax( word )
{
	var first, last;
	
	first = word.indexOf('#');
	last = word.lastIndexOf('#');
	if (first == -1) {
		alert("\"" + word +"\" has no separator #.");
		return -1;
	} else if (first != last ) {
		alert("\"" + word +"\" has more than one separator #.");
		return -1;
	} else if (first == 0) {
		alert("\"" + word +"\" has no definition in first language.");
		return -1;
	} else if (last == word.length - 1) {
		alert("\"" + word +"\" has no definition in second language.");
		return -1;
	}
	return 0;
}

/** A helper function to trim a string. Javascript does not provide this. */
function trimString( str )
{
	return str.replace (/^\s+/, '').replace (/\s+$/, '');
}

/** Add input to jwtInput (loaded as hardcoded file in HTML before calling
 * this---ugly!!!). jwtInputWords is list of all words, jwtInputCaption is
 * caption of file.
 * If word entry has special format "caption: NEWCAP" then a new lesson is
 * started with combination of jwtInputCaption and NEWCAP as caption. Empty
 * lessons are silently dropped. */ 
function addInputFile()
{
	var i, lesson;

	lesson = createEmptyLesson( jwtInputCaption );
	
	for (i = 0; i < jwtInputWords.length; i++ ) {
		if (jwtInputWords[i].indexOf("caption:") == 0) {
			if (lesson["words"].length > 0)
				jwtInput[jwtInput.length] = lesson;
			lesson = createEmptyLesson( jwtInputCaption + " - " +
						jwtInputWords[i].substr(8));
			continue;
		}
		if (checkWordSyntax(jwtInputWords[i]) < 0)
			continue;
		/* remove all whitespaces before separator # before pushing to
		 * word list */
		lesson["words"].push(jwtInputWords[i].replace(/\s*#/, '#'));
		lesson["searchkeys"].push(toSearchFormat(jwtInputWords[i]));
	}
	if (lesson["words"].length > 0)
		jwtInput[jwtInput.length] = lesson;

	delete jwtInputWords;
	delete jwtInputCaption;
}

/* Randomly select @num words from all files in jwtInput that are
 * marked as selected and store in jwtWords. Return true on success or
 * false on error. */  
function selectWords( num )
{
	var numAllWords = 0;
	var inputIdxs = []; /* indices of used objects in jwtInput */
	var wordIdxs = []; /* indices of wanted words in word list */
	var i, j, k, idx;
	
	/* get indices of selected input objects and count total words */
	for (i = 0; i < jwtInput.length; i++)
		if (jwtInput[i]["selected"]) {
			inputIdxs[inputIdxs.length] = i;
			numAllWords += jwtInput[i]["words"].length;
		}
	if (numAllWords == 0) {
		if (inputIdxs.length == 0)
			alert("Please select at least one input file!");
		else
			alert("There are no words in the selected files. " +
					"Maybe the data is corrupted?");
		return false;
	}
	
	/* check range of num (-1 == all words) */
	if (num < 0 || num > numAllWords)
		num = numAllWords;
		
	/* fill array with indices and jumble around a bit */
	for (i = 0; i < numAllWords; i++)
		wordIdxs[i] = i;
	for (i = 0; i < numAllWords; i++) {
		j = Math.floor(Math.random() * numAllWords); 
		k = wordIdxs[i];
		wordIdxs[i] = wordIdxs[j];
		wordIdxs[j] = k;				 
	}
		
	/* Indices should be pretty well mixed now so just
	 * select first @num entries. */
	jwtWords = [];
	for (i = 0; i < num; i++) {
		idx = wordIdxs[i];
		j = 0;
		while (idx >= jwtInput[inputIdxs[j]]["words"].length) {
			idx -= jwtInput[inputIdxs[j]]["words"].length;
			j++;
		}
		jwtWords[i] = jwtInput[inputIdxs[j]]["words"][idx];
	}
	jwtNumWords = jwtWords.length;
	
	return true;
}

/** @word contains actual word in both languages separated by #-symbol. This
 * function returns the word in either first (@lang == 0) or second language
 * (@lang == 1). No encoding is done. This is a generic function meant to be 
 * overloaded by the actual language if need be.
 * For an example see pali/lang.js. 
 * The format of @word has been checked on loading the input file. */    
function encodeWordToHTML( word, lang )
{
	return word.split("#")[lang];
}

/** Update header for asked word (language, how many remaining, etc) */
function updateWordHeader( askedLang )
{
	var title;
	var obj = document.getElementById("spanWordHeader");
	
	if ( askedLang == 0 )
		title = jwtLanguageNames[0] + " - " + jwtLanguageNames[1];
	else 
		title = jwtLanguageNames[1] + " - " + jwtLanguageNames[0];
	if (obj)
		obj.innerHTML = "<b>" + title + "</b><br><font size=-1>(" + 
				jwtNumWords + " words remaining)" + 
				"</font>";
}

/** Select one unsolved word randomly, show it in object spanWord 
 * and also adjust the word header in object spanWordHeader. */ 
function askNewWord()
{
	var obj = null;

	/* check that there are words */
	if (jwtNumWords == 0) {
		showEndMessage();
		return;
	}
	
	/* For now language to be asked is fixed. */
	updateWordHeader( jwtQuestionMode );
	
	/* Select word index from array. If it equals last selection
	 * than choose previous word. */
	if (jwtCurrentWordIdx != -1)
		jwtLastWordIdx = jwtCurrentWordIdx;
	jwtCurrentWordIdx = Math.floor(Math.random() * jwtNumWords);
	if (jwtCurrentWordIdx == jwtLastWordIdx) {
		jwtCurrentWordIdx--;
		if (jwtCurrentWordIdx == -1)
			jwtCurrentWordIdx = jwtNumWords - 1;
	}  					
	
	/* Show word */
	if ((obj = document.getElementById("spanWord")))
		obj.innerHTML = encodeWordToHTML( jwtWords[jwtCurrentWordIdx], 
							jwtQuestionMode );
	if ((obj = document.getElementById("spanWordSolution")))
		obj.innerHTML = "???";
	jwtSolutionIsShown = 0;
}

/** Show solution of word (if any currently selected) in object
 * spanWordSolution or (if already shown) remove it from quiz and
 * show new word. */
function showCurrentWordSolution()
{
	if (jwtNumWords == 0 || jwtCurrentWordIdx == -1)
		return;
		
	jwtQuizStarted = 1;
	
	if ( jwtSolutionIsShown ) {
		/* XXX Ops, I don't know how to delete from array so 
		 * let's just copy last word to same slot and decrease
		 * word count (now you know why there is numWords instead
		 * of array length :-) since order doesn't matter to us. */
		jwtWords[jwtCurrentWordIdx] = jwtWords[jwtNumWords-1];
		jwtNumWords--;
		
		jwtLastWordIdx = jwtCurrentWordIdx;
		jwtCurrentWordIdx = -1;
		askNewWord();
	} else {
		document.getElementById("spanWordSolution").innerHTML = 
				encodeWordToHTML( jwtWords[jwtCurrentWordIdx], 
						jwtQuestionMode==1?0:1 );
		jwtSolutionIsShown = 1;
	}
	
	/* unfocus, otherwise focus on click will interfere with key handler */
	document.getElementById("butShowWord").blur();	
}

/** Show "no more words" message. */
function showEndMessage()
{
	document.getElementById("spanWord").innerHTML = "No more words!";	
	document.getElementById("spanWordSolution").innerHTML = 
						"Press Restart for more.";
	updateWordHeader( jwtQuestionMode );
}

/** Keep word is list and select new word if any. */
function pushBackCurrentWord()
{
	if (jwtNumWords == 0 || jwtCurrentWordIdx == -1)
		return;
		
	jwtQuizStarted = 1;
	
	/* actually to keep it nothing has to be done :-) */	
	
	jwtLastWordIdx = jwtCurrentWordIdx; 
	jwtCurrentWordIdx = -1;
	askNewWord();
	
	/* unfocus, otherwise focus on click will interfere with key handler */
	document.getElementById("butPushBackWord").blur();	
}

/** Select words and start questioning. */   
function restartTrainer()
{
	var obj;
	
	if (selectWords(jwtNumWantedWords) == false)
		return false;

	askNewWord();
	jwtQuizStarted = 0;
	
	/* if restart button was used focus would interfere with key command */
	if ((obj = document.getElementById("butRestartQuiz")))
		obj.blur();

	return true; 	 
}

/** Update jwtInput[]["selected"] according to actual selection of
 * object selInput. Called on clicking selInput. */
function updateInputSelection()
{
	var i, obj = document.getElementById("selInput");

	for (i = 0; i < jwtInput.length; i++)
		jwtInput[i]["selected"] = obj.options[i].selected;
}

/** Helper function to render a match result. If @lesson is not null, a new
 * lesson has been entered and this is its caption. @match is the word in 
 * input file format with both languages separated by a hash symbol. 
 * Return the rendered string. */
function renderMatch( lesson, match )
{
	var renderStr = "";

	if (lesson)
		renderStr += "<tr><td class=matchTitle>"+ lesson +"</td></tr>";
	if (match) 
		renderStr += "<tr><td><b>" + 
				encodeWordToHTML(match, 0) + ": </b>" +
				encodeWordToHTML(match, 1) + "</td></tr>";
	return renderStr;
}

/** Search in all input files for all words that contain expression from object 
 * txtSearchExpr and display all matches in object spanSearchResult. The 
 * expression is converted to search format und matched against the search 
 * keys in each lesson. On matching the actual word is shown. 
 * If @exact is true, do not use search key but word in input format. */
function searchWords( exact )
{
	var expr = "";
	var i, j;
	var resultCode = "";
	var firstMatchInFile;
	var numMatches = 0;
	var searchArrayName = "words";
	
	/* get expression from HTML object */
	expr = document.getElementById("txtSearchExpr").value;
	if (!exact) {
		expr = toSearchFormat(expr);
		searchArrayName = "searchkeys";
	}
	
	/* search all files and add result to list */
	resultCode = "<table border=0 cellpadding=2>"; 
	for (i = 0; i < jwtInput.length; i++) {
		firstMatchInFile = 1;
		for (j = 0; j < jwtInput[i][searchArrayName].length; j++)
			if (jwtInput[i][searchArrayName][j].indexOf(expr) != -1) {
				numMatches++;
				if (firstMatchInFile) {
					firstMatchInFile = 0;
					resultCode += renderMatch(
							jwtInput[i]["caption"], 
							jwtInput[i]["words"][j]);
				} else 
					resultCode += renderMatch(null, 
							jwtInput[i]["words"][j]);
			}
	}
	if ( numMatches == 0 )
		resultCode += "<tr><td align=center>No match.</td></tr>";		
	resultCode += "</table>"; 

	document.getElementById("spanSearchResult").innerHTML = resultCode;		
}

/** Handle key event @ev. In main dialogue map to buttons, in search dialogue
 * start search on Enter key. */
function handleKeyCommand( ev )
{
	var keyCode;

	/* do nothing in main dialog */
	if (jwtDialogId == "main")
		return;

	/* some browsers don't pass event as argument properly */	
	if (ev == null)
		ev = window.event;

	/* get the keycode */
	if ( ev.which )
		keyCode = ev.which;
	else
		keyCode = ev.keyCode;

	/* start search? */
	if (jwtDialogId == "search") {
		if (keyCode == 13)
			searchWords();
		return;
	}
	
	/* handle key command */
	if (keyCode == 32)
		showCurrentWordSolution();
	else if (keyCode == 27 || keyCode == 97 || keyCode == 107)
		pushBackCurrentWord();
}

/** Called on page load. Apply language settings and show main dialog. */
function onPageLoad()
{
	useInputLanguageSettings();
	
	updateTitle();
	showMainDlg();
	selectAllInput(1);
	
	/* set handler for key press events */
	document.onkeypress = handleKeyCommand;
}

/** Return a copy of string @str in a representation that is used to match any
 * search expression (which has also been converted to this format) when
 * searching the lessons for words.
 * This is a dummy function for any real implementation in lang.js. */
function toSearchFormat( str )
{
	return str; 
}
