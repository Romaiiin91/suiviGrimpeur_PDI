function getPaddedString(theStr, theMaxLength, theNBIndicator, skipTagReplacement, strNBIndicator, countAllChars )
{
  var str = "";
  var isTag = false;
  var ch = '';
  var len = 0;
  var strLen = theStr.length;

  if( !strNBIndicator )
  {
    strNBIndicator = "&nbsp;";
  }

  if( skipTagReplacement)
  {
     strLen = ( theStr.replace(/<.*?>/g, "") ).length;
  }
  for(var i = 0; i < theStr.length; i++)
  {
    ch = theStr.charAt(i); 

    if( skipTagReplacement)
    {
      if( countAllChars )
      {
        if (len >= theMaxLength)
        {
          continue;
        }
        else if (len >= theMaxLength - 3)
        {
          str += ".";
          len++;
          continue;
        }
      }

      if (ch == '<')
      {
        isTag = true;
        str += ch;
        if( countAllChars )
        {
          len++;
        }
        continue;
      }
      if (ch == '>')
      {
        isTag = false;
        str += ch;
        if( countAllChars )
        {
          len++;
        }
        continue;
      }
      if (isTag)
      {
        str += ch;
        if( countAllChars )
        {
          len++;
        }
        continue;
      }
    }

    if (strLen > theMaxLength)
    {
      if (len >= theMaxLength)
      {
        continue;
      }
      else if (len >= theMaxLength - 3)
      {
        str += ".";
        len++;
        continue;
      }
    }

    if (ch == ' ')
    {
      if (theNBIndicator)
        str += strNBIndicator;
      else
        str += " ";
    }
    else if (ch == '<')
    {
      str += "&lt;";
    }
    else if (ch == '>')
    {
      str += "&gt;";
    }
    else
    {
      str += ch;
    }
    len++;
  }

  for (i = len; i < theMaxLength; i++)
  {
    if (theNBIndicator)
    {
      str += strNBIndicator;
    }
    else
    {
      str += " ";
    }
  }

  return str;
}

function writeListHeaders( headerElement, headers )
{
  if( !headerElement )
    return;

  var strArray = new Array();

  for( var key in headers )
  {
    strArray[ strArray.length ] = getPaddedString( key, headers[key], "&nbsp;" );
  }
  headerElement.innerHTML = strArray.join( "&nbsp;&nbsp;" )
}

function writeListHeadersWithClass( headerElement, headers )
{
  if( !headerElement )
    return;

  var strArray = new Array();

  for(var i=0; i < headers.length; i++)
  {
    strArray[ strArray.length ] = getPaddedString( headers[i].text, headers[i].length, "&nbsp;" );
  }

  headerElement.innerHTML = strArray.join( "&nbsp;&nbsp;" )
}

function addSelectOption( selectEl, nodeValue, nodeText, isSelected, cls, beforeIndex )
{
  var op = document.createElement("OPTION");
  op.value = nodeValue;
  op.text = nodeText;
  op.selected = isSelected;
  if( cls )
  {
    op.className = cls;
  }
  if( beforeIndex && beforeIndex < selectEl.length )
  {
    selectEl.options.add( op, beforeIndex );
  }
  else
  {
    try {
      selectEl.options.add( op );
    }
    catch(e) {}
  }
}

function removeSelectOption( selectEl, nodeValue )
{
  var obj;
  var len = selectEl.length;
  for( var i = 0; i < len; i++ )
  {
    if( selectEl.options[ i ].value == nodeValue )
    {
      obj = { optionText:selectEl.options[ i ].text, optionValue:selectEl.options[ i ].value, beforeIndex:i };
      selectEl.remove( i );
    }
  }
  return obj;
}

function clearSelect( el )
{
  try {
    while( el.length > 0 )
    {
      el.remove( 0 );
    }
  } catch(e) {}
}

function setSelectedOption( selectEl, val )
{
  var retVal = false;
  var len = selectEl.length;
  for( var i = 0; i < len; i++ )
  {
    selectEl.options[ i ].selected = (( selectEl.options[ i ].value ==  val )?true:false);
    
    if( selectEl.options[ i ].selected == true)
      retVal = true;
  }
  
  return retVal;
}

function checkSelectedOption( selectEl, emptyAlertTxt )
{
  if( selectEl.selectedIndex < 0 )
  {
    alert( emptyAlertTxt );
    return false;
  }
  if( selectEl.options[ selectEl.selectedIndex ].value == "" )
  {
    alert( emptyAlertTxt );
    return false;
  }
  return true;
}

function sortByObjectName( firstItem, lastItem )
{
  var retVal = 0;
  //both needs value to be compared
  if( firstItem.Name && lastItem.Name )
  {
    retVal = ( firstItem.Name ).localeCompare( lastItem.Name );
  }
  return retVal;
}
