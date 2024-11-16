/*
	Standards Compliant Script
	Alternates Table Columns
	Author : Kevin Cannon
	http://www.multiblah.com
	Last Edited: 12.12.2004
	Version 1.0

	Search through the document for tables with the "alternateRows" class,
	and set the class of even rows to "even" to appropriate rows <tr>

	Licence:
	Use as you wish, though it'd be nice if you can leave in the credit in
	the code.

	Changes:
	4/10/2004 - Added in AddLoadEvent function which piggybacks code onto
	window.onLoad

	2005-04-21 - Modified to work with Axis web pages. [pkj]
	2010-06-08 - Modified to work with Axis new emb. web pages. using <div> [annasm]
	2010-07-23 - Moved piggyback to global scripts. [annasm]
*/

// Main function, called when the page loads
function alternateRows()
{
	var i;

	if (!document.getElementById)
		return

	var mainDivs = document.getElementsByTagName("div");
	// Search through divs in document
	for (i = 0; i < mainDivs.length; i++) {
		if( String( mainDivs[i].className ).indexOf("alternateRows") != -1 ){
			rows = mainDivs[i].childNodes;
			applyClassToRows(rows, false);
		}
	}

        var tables = document.getElementsByTagName("table");
	// Search through tables in document
	for (i = 0; i < tables.length; i++) {
		// If table has the right classname
		if( String( tables[i].className ).indexOf("alternateRows") != -1 ){
			applyClassToRows(tables[i].rows, false);
		}
	}
}

// Function, which is passed a div elements, applies the class 'evenItem'
// to each even row and 'oddItem' to each odd row
function applyClassToRows(rows, row_is_odd)
{
	var i, preCls, tag, divCnt;

	// Search through rows
	for (i = 0; i < rows.length; i++) {
		if( rows[i].tagName) {
			tag = (rows[i].tagName).toUpperCase();
			if( tag == "H2" || tag == "H3" ) {
				row_is_odd = false;
				continue;
                	}
			if( tag != "DIV" && tag != "TR" ) {
				continue;
                	}
			if (rows[i].className != "reuseLast" && rows[i].className != "emptyRow" )  {
				row_is_odd = !row_is_odd;
			}

			if( tag == "TR" || String( rows[i].className ).indexOf("row") != -1  || String( rows[i].className ).indexOf("reuseLast") != -1  || String( rows[i].className ).indexOf("emptyRow") != -1 ) {
				preCls = String( rows[i].className ).replace("reuseLast", "row");
				preCls = ((preCls != "" )?" "+preCls:preCls );
				if (row_is_odd) {
					rows[i].setAttribute("className", "oddItem"+preCls);
					rows[i].setAttribute("class", "oddItem"+preCls);
				} else {
					rows[i].setAttribute("className", "evenItem"+preCls);
					rows[i].setAttribute("class", "evenItem"+preCls);
				}
			} else if( tag == "DIV" ) {
				divCnt = rows[i].getElementsByTagName("div").length;
				if( divCnt > 0 ) {
					row_is_odd = !row_is_odd;
					applyClassToRows(rows[i].childNodes, row_is_odd);
				}
			}
                }
	}
}

addLoadEvent(alternateRows);
