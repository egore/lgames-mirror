/* Copyright 2010 Michael Speck, Published under GNU GPL v3 */

/** Variables and functions for JWordTrainer. */

var jwtInput = []; /* array of all input lessons */
var jwtLanguageNames = [ "Lang1", "Lang2" ]; /* overwritten by input */
var jwtQuestionMode = 0; /* 0 = first language, 1 = second */
var jwtWords = []; /* all (remaining) words to be asked */
var jwtNumWords = 0; /* number of unsolved entries */
var jwtCurrentWordIdx = -1; /* currently asked word */ 
var jwtLastWordIdx = -1; /* index of last asked word */
var jwtSolutionIsShown = 0; /* whether solution is visible */
var jwtQuizStarted = 0; /* whether any action occured (show/keep/next) */
var jwtDialogueId = "main"; /* id of currently displayed window */

/** Use language settings from lang.js (loaded as hardcoded file
 * in HTML before calling this on loading page---ugly!!!). */  
function useInputLanguageSettings()
{
	var obj = document.getElementById("selQuestionMode");
	 
	jwtLanguageNames = jwtInputLanguageNames;
	obj.options[0].text = jwtLanguageNames[0];
	obj.options[1].text = jwtLanguageNames[1];
}

/** Update list of input files (is called on page load). */
function updateInputList()
{
	var i, obj = document.getElementById("selInput");
	
	obj.options.length = 0;
	for (i = 0; i < jwtInput.length; i++) {
		selected = jwtInput[i]["selected"];
		opt = new Option(jwtInput[i]["caption"],i,0,selected);
		obj.options[obj.options.length] = opt;
	}
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
		lesson["words"].push(jwtInputWords[i]);
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
	
	if ( askedLang == 0 )
		title = jwtLanguageNames[0] + " - " + jwtLanguageNames[1];
	else 
		title = jwtLanguageNames[1] + " - " + jwtLanguageNames[0];
	document.getElementById("spanWordHeader").innerHTML = 
				"<b>" + title + "</b><br><font size=-1>(" + 
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
	obj = document.getElementById("spanWord");
	obj.innerHTML = encodeWordToHTML( jwtWords[jwtCurrentWordIdx], jwtQuestionMode );
	obj = document.getElementById("spanWordSolution");
	obj.innerHTML = "???";
	jwtSolutionIsShown = 0;
}

/** Show solution of word (if any currently selected) in object
 * spanWordSolution. */
function showCurrentWordSolution()
{
	if (jwtNumWords == 0 || jwtCurrentWordIdx == -1)
		return;
		
	jwtQuizStarted = 1;
		
	document.getElementById("spanWordSolution").innerHTML = 
				encodeWordToHTML( jwtWords[jwtCurrentWordIdx], 
						jwtQuestionMode==1?0:1 );
	jwtSolutionIsShown = 1;
	
	/* unfocus, otherwise focus on click will interfere with key handler */
	document.getElementById("butShowWord").blur();	
}

/** Show "no more words" message. */
function showEndMessage()
{
	document.getElementById("spanWord").innerHTML = "No more words!";	
	document.getElementById("spanWordSolution").innerHTML = 
						"Press Start for more.";
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

/** Consider current word to be solved, remove it and ask for
 * a new one (if any remains). */
function removeCurrentWord()
{
	if (jwtNumWords == 0 || jwtCurrentWordIdx == -1)
		return;
		
	jwtQuizStarted = 1;

	/* XXX Ops, I don't know how to delete from array so 
	 * let's just copy last word to same slot and decrease
	 * word count (now you know why there is numWords instead
	 * of array length :-) since order doesn't matter to us. */
	jwtWords[jwtCurrentWordIdx] = jwtWords[jwtNumWords-1];
	jwtNumWords--;
	
	jwtLastWordIdx = jwtCurrentWordIdx;
	jwtCurrentWordIdx = -1;
	askNewWord();
	
	/* unfocus, otherwise focus on click will interfere with key handler */
	document.getElementById("butNextWord").blur();	
}

/** Set jwtNumWantedWords and jwtQuestionMode according to selected
 * values. Select words and start questioning. */   
function restartTrainer()
{
	var numWantedWords;
	
	/* unfocus, otherwise focus on click will interfere with key handler */
	document.getElementById("butStart").blur();
		
	numWantedWords = document.getElementById("selNumWords").value;
	jwtQuestionMode = document.getElementById("selQuestionMode").value;
	
	if (selectWords(numWantedWords) == false)
		return false;

	askNewWord();
	jwtQuizStarted = 0;
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

/** Show search dialogue (replace contents of main frame). If quiz has started,
 * ask for confirmation first, since this will end the current quiz. */  
function showSearchDialogue()
{
	if ( jwtQuizStarted && jwtNumWords > 0 )
		if ( !confirm("Navigating to search page will cancel " +
						"current quiz. Continue?") )
			return;

	/* set new dialogue id */			
	jwtDialogueId = "search";
	
	/* replace HTML code of main frame */
	document.getElementById("tdMainFrame").innerHTML =
		"<table border=1 cellpadding=10 id=tabtest>" +
		"<tr><td align=center><span id=spanSearchHeader></span></td></tr>" +
		"<tr><td align=center><input type=text size=20 id=txtSearchExpr>" +
		"&nbsp;<input type=button value=\"Search\" onClick=\"searchWords(0);\"><br>" +
		"</td></tr>" +
		"<tr><td align=center><span id=spanSearchResult>&nbsp;</span></td></tr>" +
		"</table>";
	
	/* set title */
	document.getElementById("spanSearchHeader").innerHTML = 
		"<b>Word Search (" + jwtLanguageNames[0] + " / " + 
		jwtLanguageNames[1] + ")</b><br>" +
		"<font size=-1>[ <a href='#' onClick='location.reload();'>Back to Word Trainer</a> ]</font>";

	/* focus input */
	document.getElementById("txtSearchExpr").focus();			
}

/** Search in all input files for all words that contain expression from object 
 * txtSearchExpr and display all matches in object spanSearchResult. The 
 * expression is converted to search format und matched against the search 
 * keys in each lesson. On matching the actual word is shown however. 
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
	resultCode = "<u>Results for '" + expr + 
			"':</u><br><br><table width=100% border=1 cellpadding=4>"; 
	for (i = 0; i < jwtInput.length; i++) {
		firstMatchInFile = 1;
		for (j = 0; j < jwtInput[i][searchArrayName].length; j++)
			if (jwtInput[i][searchArrayName][j].indexOf(expr) != -1) {
				numMatches++;
				if ( firstMatchInFile ) {
					firstMatchInFile = 0;
					resultCode += "<tr><td colspan=2 align=center><b>" + 
							jwtInput[i]["caption"] + 
							"</b></td></tr>";
				}
				resultCode += "<tr><td width=50%>" + 
						encodeWordToHTML(jwtInput[i]["words"][j],0) + 
						"</td><td width=50%>" +
						encodeWordToHTML(jwtInput[i]["words"][j],1) + 
						"</td>";
			}
	}
	if ( numMatches == 0 )
		resultCode += "<tr><td>No matches</td></tr>";		
	resultCode += "</table>"; 

	document.getElementById("spanSearchResult").innerHTML = resultCode;		
}

/** Handle key event @ev. In main dialogue map to buttons, in search dialogue
 * start search on Enter key. */
function handleKeyCommand( ev )
{
	var keyCode;

	/* some browsers don't pass event as argument properly */	
	if (ev == null)
		ev = window.event;

	/* get the keycode */
	if ( ev.which )
		keyCode = ev.which;
	else
		keyCode = ev.keyCode;

	/* start search? */
	if (jwtDialogueId == "search") {
		if (keyCode == 13)
			searchWords();
		return;
	}
	
	/* handle key command */
	if (keyCode == 32) {
		if ( jwtSolutionIsShown )
			removeCurrentWord();
		else
			showCurrentWordSolution();
	} else if (keyCode == 27 || keyCode == 97 || keyCode == 107)
		pushBackCurrentWord();
}

/** Called on page load. Apply language settings from lang.js (e.g.,
 * set names in language select), update list of input files, start
 * questioning with a default number of words. */
function onPageLoad()
{
	useInputLanguageSettings();
	updateInputList();
	selectAllInput(1);
	restartTrainer();
	
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
