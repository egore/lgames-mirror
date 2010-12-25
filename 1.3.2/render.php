<?php
/* General render functions */

/* Render beginning of page (HTML+logo+link bar+title). $class is CSS 
 * class of main table (empty= no class). $title may be null. $pagetitle
 * (window title) may be null (is just 'OCC' then). */
function renderPageBegin($pagetitle,$class,$links,$title)
{
	global $theme;

	if ($pagetitle==null)
		$pagetitle='Online Chess Club';

	echo '<HTML><HEAD><TITLE>'.$pagetitle.'</TITLE><LINK rel=stylesheet type="text/css" href="images/'.$theme.'/style.css"></HEAD><BODY>';
	echo '<DIV align="center">';
	echo '<P><IMG alt="" src="images/'.$theme.'/logo.jpg"><BR>';
	if ($links) {
		echo '[ ';
		$i=0;
		foreach ($links as $name=>$url) {
			echo '<A href="'.$url.'">'.$name.'</A>';
			$i++;
			if ($i<count($links))
				echo ' | ';
		}
		echo ' ]';
	}
	if ($title) {
		echo '<BR><IMG src="images/spacer.gif" height=5><BR>';
		echo '<B>'.$title.'</B>';
	}
	echo '</P>';
	if (!empty($class))
		echo '<TABLE class="'.$class.'" border=0 cellspacing=0 cellpadding=0><TR><TD align="center">';
	else
		echo '<TABLE border=0 cellspacing=0 cellpadding=0><TR><TD align="center">';
}

/* Render end of page (footer+HTML). If $credit is not empty display it. 
 * Show build-time if $btstart is set (in session.php). This is a good
 * place here, since it is most likely the last function to be called on a page. */
function renderPageEnd($credits)
{
	global $btstart;

	echo '<HR width=100%>';
	echo '<TABLE width=100% border=0><TR><TD class="tiny" valign="top">';
	echo 'Online Chess Club v1.3.2<BR>Published under GNU GPL</TD>';
	echo '<TD valign="top" align="right" class="tiny">';
	echo '&copy; 2003-2007 Michael Speck<BR><A class="tiny" href="http://lgames.sf.net">http://lgames.sf.net</A>';
	echo '</TD></TR></TABLE>';
	if (!empty($credits))
		echo '<P class="tiny">'.$credits.'</P>';
	if (!empty($btstart))
		echo '<DIV align="right" class="tiny">Build-time: '.sprintf("%.3f",1000*(microtime(true)-$btstart)).' msecs</DIV>';
	echo '</TD></TR></TABLE></DIV>';
	echo '</BODY></HTML>';
}

/* Render functions for various components of chess page */

/* Render command form which contains information about players and last 
 * moves and all main buttons (shown when nescessary). The final command 
 * is mapped to hidden field 'cmd' on submission. Show previous command 
 * result $cmdres if set or last move if any. Fill move edit with $move
 * if set (to restore move when notes were saved). */
function renderCommandForm($game,$cmdres,$move)
{
	$i_move=$game['curmove'];
	$i_plyr='Black';
	if ($game['curplyr']=='w') {
		$i_plyr='White';
		$i_move++;
	} else if ($game['curstate']=='D')
		$i_move++;
	echo '<P align="center" class="large">';
	echo '<B>'.$game['white'].'</B> - <B>'.$game['black'].'</B><BR>';
	echo 'Move '.$i_move.' ('.$i_plyr.')</P>';
	echo '<FORM name="commandForm" method="post">';
	echo '<INPUT type="hidden" name="cmd" value="">';
	echo '<INPUT type="hidden" name="comment" value="">';
	echo '<INPUT type="hidden" name="privnotes" value="">';
	if ($game['p_mayabort'] && !$game['p_maymove']) {
		/* Info that game is very old + abort form */
		echo '<P class="warning">You may abort this game since your opponent did not move for more than four weeks (it is not counted then).<BR>';
		echo '<DIV align="center"><INPUT type="submit" value="Abort Game" onClick="if (confirmAbort()) {document.commandForm.cmd.value=\'abort\'; gatherCommandFormData(); return true;} else return false;"></DIV>';
		echo '</P>';
	}
	if ($cmdres || ($game['archived']==0 && $game['lastmove']!='x' &&
			($game['curstate']=='D' || $game['curstate']=='?'))) {
		/* Info about last move + possible undo */
		if (!empty($cmdres)) 
			$info=$cmdres;
		else {
			if ($game['curplyr']=='b')
				$info='White\'s';
			else
				$info='Black\'s';
			$info=$info.' last move: '.$game['lastmove'];
		}
		echo '<P class=info><B>'.$info.'</B>';
		if ($game['p_mayundo'])
			echo '&nbsp;&nbsp;<INPUT type="submit" value="Undo" onClick="if (confirmUndo()) {document.commandForm.cmd.value=\'undo\'; gatherCommandFormData(); return true;} else return false;">';
		echo '</P>';
	}
	if ($game['p_maymove'] && $game['curstate']=='D') {
		/* Info if draw is offered + draw form */
		if (empty($info)) {
			$info=$game['p_opponent'].' has offered a draw.';
			echo '<P class=info><B>'.$info.'</B></P>';
		}
		echo '<TABLE width=100%><TR><TD align=left>';
		echo '<INPUT class=warning type="submit" value="Accept Draw" onClick="onClickAcceptDraw()">&nbsp;&nbsp;';
		echo '</TD><TD align=right>';
		echo '<INPUT type="submit" value="Refuse Draw" onClick="onClickRefuseDraw()">';
		echo '</TD></TR></TABLE>';
	} else if ($game['p_maymove'] && $game['curstate']=='?') {
		/* Normal move form */
		echo '<TABLE width=100%><TR><TD align=left>';
		echo '<INPUT id=moveButton type="button" value="Move" onClick="onClickMove()">';
		echo '</TD><TD align=center>';
		echo '<INPUT type="button" value="Offer Draw" onClick="onClickOfferDraw()">';
		echo '</TD><TD align=right>';
		if ($game['p_mayabort'])
			echo '<INPUT class=warning type="button" value="Abort Game" onClick="onClickAbortGame()">';
		else
			echo '<INPUT class=warning type="button" value="Resign" onClick="onClickResign()">';
		echo '</TD></TR></TABLE>';
		echo '<INPUT type="hidden" size=10 name="move" value="'.$move.'">';
		echo '<script language="Javascript">checkMoveButton(); highlightMove(window.document.commandForm.move.value)</script>';
	} else {
		/* User may not move. Show proper info and possibly archive/refresh form */
		if ($game['curstate']=='D' || $game['curstate']=='?') {
			echo '<P align="center">';
			echo '<INPUT type="submit" value="Refresh Board" onClick="document.commandForm.cmd.value=\'\'; gatherCommandFormData(); return true;"></P>';
		} else if (empty($info)) {
			if ($game['curstate']=='-')
				$info='draw';
			else if ($game['curstate']=='w')
				$info=$game['white'].' won';
			else if ($game['curstate']=='b')
				$info=$game['black'].' won';
			$info='Game result: '.$info;
			echo '<P><B style="color: 8888ff">'.$info.'</B></P>';
			if ($game['p_mayarchive'])
				echo '<P align="center"><INPUT type="submit" value="Archive Game" onClick="document.commandForm.cmd.value=\'archive\'; gatherCommandFormData(); return true;"></P>';
		}
	}
	echo '</FORM>';
}

/* Render browser form which contains title about who is playing and browsing
 * buttons to move forward/backward in history. */
function renderBrowserForm($game)
{
	global $theme;

	echo '<P align="center" class="large">';
	echo '<B>'.$game['white'].'</B> - <B>'.$game['black'].'</B><BR>';
	if ($game['curstate']!='?' && $game['curstate']!='D') {
		if ($game['curstate']=='-')
			$res='draw';
		else if ($game['curstate']=='w')
			$res='White won';
		else 
			$res='Black won';
		echo '<FONT class=info><b>Game result: '.$res.'</b></FONT>';
	}
	echo '</P><P align="center">';
	echo '<A href="first" onClick="return gotoMove(0);">';
	echo '<IMG alt="" src="images/'.$theme.'/h_first.gif" border=0></A>';
	echo '<IMG width=2 height=2 alt="" src="images/spacer.gif">';
	echo '<A href="prev" onClick="return gotoMove(cur_move-1);">';
	echo '<IMG alt="" src="images/'.$theme.'/h_backward.gif" border=0></A>';
	echo '<IMG width=2 height=2 alt="" src="images/spacer.gif">';
	echo '<IMG name="colorpin" alt="" src="images/h_white.gif">';
	echo '<IMG name="digit1" alt="" src="images/'.$theme.'/d0.gif">';
	echo '<IMG name="digit2" alt="" src="images/'.$theme.'/d1.gif">';
	echo '<IMG alt="" src="images/'.$theme.'/h_right.gif">';
	echo '<IMG width=2 height=2 alt="" src="images/spacer.gif">';
	echo '<A href="next" onClick="return gotoMove(cur_move+1);">';
	echo '<IMG alt="" src="images/'.$theme.'/h_forward.gif" border=0></A>';
	echo '<IMG width=2 height=2 alt="" src="images/spacer.gif">';
	echo '<A href="last" onClick="return gotoMove(move_count-1);">';
	echo '<IMG alt="" src="images/'.$theme.'/h_last.gif" border=0></A>';
	echo '</P>';
}

/* Render move history and chessmen difference. 
 * $list: w1,b1,w2,b2,... 
 * If $browsing is set ignore $diff (create empty slots instead) and show full 
 * history with javascript links. Otherwise show only few last moves. */
function renderHistory($list,$diff,$browsing)
{
	global $theme;

	if (count($list)==0)
		return;

	echo '<P>';
	echo '<TABLE width=100% border=0 cellpadding=1 cellspacing=1 class="textFrame">';

	echo '<TR><TD class="textFrameData">';
	$num=floor((count($list)+1)/2);
	/* Show only few last moves if not browsing */
	if (!$browsing && $num>12) {
		$start=floor($num-12);
		echo '... ';
	} else
		$start=0;
	for ($i=1+$start,$j=$start*2;$i<=$num;$i++,$j+=2) {
		echo '<B>'.$i.'.</B>&nbsp;';
		if ($browsing) {
			$jspos=$j;
			echo '<A href="'.$jspos.'" onClick="return gotoMove('.$jspos.');">'.$list[$j].'</A> ';
			$jspos++;
			echo '<A href="'.$jspos.'" onClick="return gotoMove('.$jspos.');">'.$list[$j+1].'</A> ';
		} else
			echo $list[$j].' '.$list[1+$j].' ';
	}
	echo '</TD></TR>';
	if ($browsing) {
		echo '<TR><TD class="textFrameData">';
		for ($i=0; $i<15; $i++)
			echo '<IMG name="tslot'.$i.'" src="images/'.$theme.'/sempty.gif">';
		echo '</TD></TR>';
	} else if (!empty($diff)) {
		$names=array('pawn','knight','bishop','rook','queen');
		echo '<TR><TD class="textFrameData">';
		/* White first */
		$src=null;
		for ($i=0;$i<5;$i++)
			if ($diff[$i]>0)
				for ($j=0;$j<$diff[$i];$j++) {
					$src='images/'.$theme.'/sw'.$names[$i].'.gif';
					echo '<IMG src="'.$src.'">';
				}
		if ($src != null)
			echo '<IMG src="images/'.$theme.'/sempty.gif">';
		/* Black second */
		for ($i=0;$i<5;$i++)
			if ($diff[$i]<0)
				for ($j=0;$j>$diff[$i];$j--) {
					$src='images/'.$theme.'/sb'.$names[$i].'.gif';
					echo '<IMG src="'.$src.'">';
				}
		echo '</TD></TR>';
	}
	echo '</TABLE>';
	echo '</P>';
}

/* Render chess board. 
 * $board: 1dim chess board (a1=0,...,h8=63) with color/chessmen ('bQ','wP',...)
 * $pc: playercolor ('w' or 'b' or empty)
 * $active: may move (add javascript calls for command assembly)
 * If $board is null create empty board for history browser.
 */
function renderBoard($board,$pc,$active)
{
	global $theme;
	
	/* show white at bottom if not playing */
	if (empty($pc))
		$pc='w';

	/* build chessboard */
	echo '<TABLE class="boardFrame"><TR><TD>';
	echo '<TABLE class="board">';
	if ($pc=='w') {
		$index=56;
		$pos_change = 1;
		$line_change = -16;
	} else {
		$index=7;
		$pos_change = -1;
		$line_change = 16;
	}
	for ($y=0;$y<9;$y++) {
		echo '<TR>';
		for ($x=0;$x<9;$x++) {
    			if ($y==8) {
				/* number at bottom */
				if ($x>0) {
					if ( $pc == 'w' )
						$c = chr(96+$x);
					else
						$c = chr(96+9-$x);
					echo '<TD align="center"><IMG height=4 src="images/spacer.gif"><BR><B class="boardCoord">'.$c.'</B></TD>';
				} else
					echo '<TD></TD><TD></TD>';
			} else if ($x==0) {
				/* number on the left */
				if ( $pc == 'w' )
					$i = 8-$y;
				else
					$i = $y+1;
				echo '<TD><B class="boardCoord">'.$i.'</B></TD><TD><IMG width=4 src="images/spacer.gif"></TD>';
			} else {
				/* normal tile */
				if ($board) {
					$entry=$board[$index];
					$color=substr($entry,0,1);
					$name=strtolower(getCMName($entry[1]));
				}
				if ((($y+1)+($x))%2==0)
					$class='boardTileWhite';
				else
					$class='boardTileBlack';
				if ($board==null) {
					echo '<TD class="'.$class.'"><IMG name="b'.$index.'" src="images/'.$theme.'/empty.gif"></TD>';
				} else if ($name!='empty') {
					if ($active) {
						if ($pc!=$color)
							$cmdpart=sprintf('x%s',i2bc($index));
						else
							$cmdpart=sprintf('%s%s',$board[$index][1],i2bc($index));
						echo '<TD id="btd'.$index.'" class="'.$class.'"><A href="" onClick="return assembleCmd(\''.$cmdpart.'\');"><IMG border=0 src="images/'.$theme.'/'.$color.$name.'.gif"></A></TD>';
        				} else
						echo '<TD class="'.$class.'"><IMG src="images/'.$theme.'/'.$color.$name.'.gif"></TD>';
				} else {
					if ($active) {
						$cmdpart=sprintf('-%s',i2bc($index));
						echo '<TD id="btd'.$index.'" class="'.$class.'"><A href="" onClick="return assembleCmd(\''.$cmdpart.'\');"><IMG border=0 src="images/'.$theme.'/empty.gif"></A></TD>';
					} else
						echo '<TD class="'.$class.'"><IMG src="images/'.$theme.'/empty.gif"></TD>';
				}
				$index += $pos_change;
			}
		}
		$index += $line_change;
		echo "</TR>";
	}
	echo "</TABLE></TD></TR></TABLE>";
}

/* Render private notes formular
 * $notes: current unencrypted contents
 */
function renderPrivateNotes($uid,$oid)
{
	global $theme;

	$notes=ioLoadPrivateNotes($uid,$oid);

	echo '<script language="Javascript">';
	echo 'function gatherPNotesFormData() {';
	echo 'fm=document.pnotesForm;';
	echo 'if (document.commentForm && document.commentForm.comment)';
	echo '  fm.commentbackup.value=document.commentForm.comment.value;';
	echo 'if (document.commandForm && document.commandForm.move)';
	echo '  fm.movebackup.value=document.commandForm.move.value;';
	echo '} </script>';
	echo '<FORM method="post" name="pnotesForm">';
	echo '<INPUT type="hidden" name="commentbackup" value="">';
	echo '<INPUT type="hidden" name="movebackup" value="">';
	echo 'Notes:<FONT class="warning"> (encrypted)</FONT>&nbsp;';
	echo '<INPUT type="image" src="images/'.$theme.'/savenotes.gif" onClick="gatherPNotesFormData(); return true;"><BR>';
	echo '<TEXTAREA cols=35 rows=3 name="privnotes">'.$notes.'</TEXTAREA>';
	echo '</FORM>';
}

/* Render chatter and add comment formular if wanted
 * $game: game context 
 * $comment: current comment */
function renderChatter($game, $comment)
{
	echo '<TABLE width=100% border=0 cellspacing=0 cellpadding=0><TR><TD>';
	echo '<TABLE width=100% border=0 cellspacing=1 cellpadding=1 class="textFrame"><TR><TD class="textFrameData">';
	for ($i=count($game['chatter'])-1;$i>=0;$i--)
		echo '<I>'.$game['chatter'][$i].'</I><BR>';
	echo '</TD></TR></TABLE>';
	echo '</TD></TR><TR><TD align="center">';
	if ($game['p_maymove']) {
		echo '<FORM name="commentForm" method="post">';
		echo '<TEXTAREA cols=80 rows=2 name="comment">'.$comment.'</TEXTAREA>';
		echo '</FORM>';
	}
	echo '</TD></TR></TABLE>';
}

?>
