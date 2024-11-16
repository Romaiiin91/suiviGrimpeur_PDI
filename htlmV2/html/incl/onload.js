// Replace onload events with DOMContentLoaded, if supported.
if ( document.addEventListener )
{
  var onloadDone = false;
  
  function DCLCallback()
  {
    if ( onloadDone )
    {
      return;
    }
    onloadDone = true;

    if ( window.onload )
    {
      var func = window.onload;
      window.onload = null;
      func();
    }
  }

  function OnloadCallback()
  {
    onloadDone = true;
  }   

  window.addEventListener("load", OnloadCallback);
  document.addEventListener("DOMContentLoaded", DCLCallback);
}
