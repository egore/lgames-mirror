/* Copyright 2011 Michael Speck, Published under GNU GPL v3 */

/** User interface */

/** Show dialog in the main frame (id=tdMain) by setting its innerHTML to
 * @content. Store @id of current dialog in global var jwtDialogId. Return
 * true on success, false otherwise. */
function showDlg( id, content )
{
	var obj = document.getElementById("tdMain");

	if (obj == null)
		return false; /* oops? */
	
	obj.innerHTML = content;
	jwtDialogId = id;
	return true;
}

/***************** Main Dialog ***********************/

var mainDlgContent = 
"<table border=0 cellspacing=4 cellpadding=2>" + 
"<tr><td align=center><b>Lessons:</b></td></tr>" + 
"<tr><td align=center>" + 
"	<select id=\"selInput\" multiple size=8 onChange=\"updateInputSelection();\"></select>" + 
"</td></tr>" + 
"<tr><td align=center>" + 
"	<input type=button onClick=\"selectAllInput(1);\" value=\"Select All\">&nbsp;&nbsp;" + 
"	<input type=button onClick=\"selectAllInput(0);\" value=\"Select None\">" + 
"</td></tr>" + 
"<tr><td align=center><hr></td></tr>" + 
"<tr><td align=center><select id=selNumWords></select></td></tr>" + 
"<tr><td align=center><select id=selQuestionMode></select></td></tr>" + 
"<tr><td align=center>" + 
"	<input type=button id=butStart value=\"Start Quiz\" onClick=\"showQuizDlg();\">" + 
"</td></tr>" + 
"<tr><td align=center><hr></td></tr>" + 
"<tr><td align=center>" + 
"	<input type=button id=butStart value=\"Search Words\" onClick=\"showSearchDlg();\">" + 
"</td></tr>" + 
"</table>"; 

/** Show the main lesson selection dialog in the main frame. */
function showMainDlg()
{
	var obj, i;
	
	showDlg("main", mainDlgContent);

	/* update list of input files */
	if ((obj = document.getElementById("selInput")) != null) {
		obj.options.length = 0;
		for (i = 0; i < jwtInput.length; i++) {
			selected = jwtInput[i]["selected"];
			opt = new Option(jwtInput[i]["caption"],i,0,selected);
			obj.options[obj.options.length] = opt;
		}
	}

	/* add choice for how many words to be asked */
	if ((obj = document.getElementById("selNumWords")) != null) {
		var numArr = new Array(10, 20, 30, 50, 100, 150, 200);
		obj.options.length = 0;
		for (i = 0; i < numArr.length; i++)
			obj.options[i] = new Option(
						"Ask " + numArr[i] + " Words",
						numArr[i], 0, 0);
		obj.options[obj.options.length] = new Option("Ask All Words",
								-1, 0, 0);
		for (i = 0; i < obj.options.length; i++)
			if (obj.options[i].value == jwtNumWantedWords) {
				obj.options[i].selected = true;
				break;
			}
	}
	
	/* add choice for language in which words are asked */
	if ((obj = document.getElementById("selQuestionMode")) != null) {
		obj.options.length = 0;
		obj.options[0] = new Option("in " + jwtLanguageNames[0],0,0,0);
		obj.options[1] = new Option("in " + jwtLanguageNames[1],1,0,0);
		if (jwtQuestionMode == 0)
			obj.options[0].selected = true;
		else
			obj.options[1].selected = true;
	}
}

/***************** Quiz Dialog ***********************/

var quizDlgContent = 
"<table border=0 cellspacing=4 cellpadding=2> " +
"<tr><td align=center><span id=spanWordHeader></span></td></tr>" +
"<tr><td align=center><hr></td></tr>" +
"<tr><td align=center class=word><span id=spanWord></span></td></tr>" +
"<tr><td align=center>&nbsp;</td></tr> " +
"<tr><td align=center class=solution><span id=spanWordSolution></span></td></tr>" +
"<tr><td align=center>&nbsp;</td></tr> " +
"<tr><td align=center>" +
"	<table border=0 cellpadding=0 cellspacing=0>" +
"	<tr><td>" +
"		<input type=\"Button\" id=\"butPushBackWord\" value=\"Keep\" onClick=\"pushBackCurrentWord();\">" +
"	</td><td>" +
"		&nbsp;&nbsp;&nbsp;" +
"		<input type=\"Button\" id=\"butShowWord\" value=\"Show\" onClick=\"showCurrentWordSolution();\">" +
"		&nbsp;&nbsp;&nbsp;" +
"	</td><td>" +
"		<input type=\"Button\" id=\"butNextWord\" value=\"Next\" onClick=\"removeCurrentWord();\">" +
"	</td></tr>" +
"	</table>" +
"</td></tr>" +
"<tr><td align=center><hr></td></tr>" +
"<tr><td align=center>" + 
"	<input type=button value=\"Go Back\" onClick=\"showMainDlg();\">" +
"	&nbsp;&nbsp;&nbsp;&nbsp;" + 
"	<input type=button id=butRestartQuiz value=\"Restart\" onClick=\"restartTrainer();\">" +
"</td></tr>" +
"</table>";

/** Show quiz dialog and start quiz. Before replacing main dialog, store the
 * quiz settings to global vars. */
function showQuizDlg()
{
	if (jwtDialogId == "main") {
		jwtNumWantedWords = document.getElementById("selNumWords").value;
		jwtQuestionMode = document.getElementById("selQuestionMode").value;
	}
	
	showDlg("quiz",quizDlgContent);
	restartTrainer();
}

/***************** Search Dialog *********************/

var searchDlgContent = 
"<table border=0 cellspacing=4 cellpadding=2>" + 
"<tr><td align=center><b>Enter Word (or a Part):</b></td></tr>" + 
"<tr><td align=center><input type=text size=20 id=txtSearchExpr></td></tr>" +
"<tr><td align=center>" + 
"	<input type=button value=\"Go Back\" onclick=\"showMainDlg();\">" + 
"	&nbsp;&nbsp;&nbsp;" +
"	<input type=button value=\"Search\" onClick=\"searchWords(0);\">" + 
"</td></tr>" + 
"<tr><td align=center><hr></td></tr>" + 
"<tr><td align=center><span id=spanSearchResult>&nbsp;</span></td></tr>" +
"</table>"; 

/** Show the search dialog in the main frame. */ 
function showSearchDlg()
{
	/* store settings in global vars before navigating away */
	if (jwtDialogId == "main") {
		jwtNumWantedWords = document.getElementById("selNumWords").value;
		jwtQuestionMode = document.getElementById("selQuestionMode").value;
	}

	showDlg("search", searchDlgContent);
	document.getElementById("txtSearchExpr").focus(); /* focus input */
}
