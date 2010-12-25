<? /* Start game page. */
include 'verifysession.php';
include 'io.php';
include 'render.php';

/* If color and opponent is submitted create new game and redirect to board page. */
if (isset($_POST['color']) && isset($_POST['opponent'])) { 
	if ($_POST['color']=='w') {
		$white=$uid;
		$black=$_POST['opponent'];
		$comment=null;
	} else {
		$white=$_POST['opponent'];
		$black=$uid;
		$comment=$_POST['comment'];
	}
	$gid=ioCreateGame($white,$black,$comment);
	header('location:board.php?gid='.$gid);
}

/* JS confirm dialog */
echo '<script language="Javascript">function confirm_new() {';
echo 'return confirm("Are you sure you want to open a new game?");';
echo '}</script>';

renderPageBegin('OCC - New Game',null,array(
	'My Games'=>'index.php',
	'Help'=>'help.php',
	'Logout'=>'logout.php'),
	'New Game');
?>
<FORM onSubmit="return confirm_new();" action="newgame.php" method="POST">
<TABLE border=0>
<TR><TD>Color:</TD><TD align=right>
<SELECT name="color">
  <OPTION value="w">White</OPTION>
  <OPTION value="b">Black</OPTION>
</SELECT>
</TD></TR>
<TR><TD>Opponent:</TD><TD align=right>
<SELECT name="opponent">
<?
$users=ioLoadUserList();
foreach($users as $usr)
	if ($usr!=$uid)
		echo '<OPTION value="'.$usr.'">'.$usr.'</OPTION>';
?>
</SELECT>
</TD></TR>
<TR><TD colspan=2>Comment (if Black):</TD></TR>
<TR><TD colspan=2><TEXTAREA cols=40 rows=3 name="comment"></TEXTAREA>
</TD></TR>
<TR><TD colspan=2 align="center">
<INPUT type="submit" name="submit" value="Open New Game">
</TD></TR>
</TABLE>
</FORM>

<?
renderPageEnd(null);
?>

