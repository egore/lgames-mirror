/* Pali/English language-specific definitions */

/** Language names. Each word in the vocabulary input files consists 
 * of the word in these two languages separated by the hash-symbol #. */   
var jwtInputLanguageNames = new Array( "Pali", "English" );

/** The generic encodeWordToHTML() function in jwt.js returns the raw word
 * in either first language (@lang == 0) or second language (@lang == 1) 
 * (@word contains both languages separated by #-symbol) without any encoding.
 * This might not be sufficient if non-latin characters are used. In this case,
 * this function can be overwritten here to render the word correctly to HTML
 * representation. E.g., here the Pali Velthius format used in the word input
 * files is encoded to HTML unicode characters.
 * The format of @word has been checked on loading the input file. */    
function encodeWordToHTML( word, lang )
{
	if (lang == 1) {
		/* English needs no changes */
		return word.split("#")[lang];
	}
	
	var str = word.split("#")[lang];
	
	/* Velthius to HTML UTF-8 */
	str = str.replace(/aa/g,"&#x0101;"); 
	str = str.replace(/ii/g,"&#x012b;"); 
	str = str.replace(/uu/g,"&#x016b;"); 
	str = str.replace(/~n/g,"&#x00f1;");
	str = str.replace(/"n/g,"&#x1e45;");
	str = str.replace(/\.t/g,"&#x1e6d;");
	str = str.replace(/\.d/g,"&#x1e0d;");
	str = str.replace(/\.n/g,"&#x1e47;");
	str = str.replace(/\.m/g,"&#x1e43;");
	str = str.replace(/\.l/g,"&#x1e37;");
	
	/* UTF-8 characters to HTML UTF-8 */
	str = str.replace(/ā/g,"&#x0101;"); 
	str = str.replace(/ī/g,"&#x012b;"); 
	str = str.replace(/ū/g,"&#x016b;"); 
	str = str.replace(/ñ/g,"&#x00f1;");
	str = str.replace(/ṅ/g,"&#x1e45;");
	str = str.replace(/ṭ/g,"&#x1e6d;");
	str = str.replace(/ḍ/g,"&#x1e0d;");
	str = str.replace(/ṇ/g,"&#x1e47;");
	str = str.replace(/ṃ/g,"&#x1e43;");
	str = str.replace(/ḷ/g,"&#x1e37;");
	
	return str;
}

/** To simplify searching return a copy of string @str which is all lower case
 * and where all special characters have been replaced with a simple latin 
 * character, e.g., aa as a or ~n as n.
 * See also comment of generic dummy function in jwt.js. */
function toSearchFormat( str )
{
	str = str.toLowerCase();

	/* Velthius to simple latin */
	str = str.replace(/aa/g,"a"); 
	str = str.replace(/ii/g,"i"); 
	str = str.replace(/uu/g,"u"); 
	str = str.replace(/~n/g,"n");
	str = str.replace(/"n/g,"n");
	str = str.replace(/\.t/g,"t");
	str = str.replace(/\.d/g,"d");
	str = str.replace(/\.n/g,"n");
	str = str.replace(/\.m/g,"m");
	str = str.replace(/\.l/g,"l");
	
	/* UTF-8 characters to simple latin */
	str = str.replace(/ā/g,"a"); 
	str = str.replace(/ī/g,"i"); 
	str = str.replace(/ū/g,"u"); 
	str = str.replace(/ñ/g,"n");
	str = str.replace(/ṅ/g,"n");
	str = str.replace(/ṭ/g,"t");
	str = str.replace(/ḍ/g,"d");
	str = str.replace(/ṇ/g,"n");
	str = str.replace(/ṃ/g,"m");
	str = str.replace(/ḷ/g,"l");
	
	return str; 
}
