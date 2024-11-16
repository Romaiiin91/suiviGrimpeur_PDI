
  function launch(url) {
    var w = 480, h = 340;
  
    if (document.all) {
      /* the following is only available after onLoad */
      w = document.body.clientWidth;
      h = document.body.clientHeight;
    }
    else if (document.layers) {
      w = window.innerWidth;
      h = window.innerHeight;
    }
  
    var popW = 475, popH = 590;
    var leftPos = (w-popW)/2, topPos = (h-popH)/2;
  
    self.name = "opener";
    remote = document.open(url, 'helpWin', "resizable,scrollbars,status,width=" + popW + ",height="+popH+",left="+leftPos+",top="+topPos+"");
  
    //Fix for IE6 to solve problem with video stopping when opening help
    try {
      if ((typeof(useAMC) != "undefined")&&(useAMC == "yes") &&(navigator.appVersion.indexOf("MSIE 6") != -1) && (typeof(stopStartStream) == "function") && (typeof(imagepath) == "string"))
        stopStartStream(imagepath);
    } catch (e) {}
  }

  
  function openPopUp(thePage, theName, theWidth, theHeight, centerWindow)
  {
    theWidth = Number( theWidth ) + 10;
    theHeight = Number( theHeight ) + 20;

    var someFeatures = 'scrollbars=yes,toolbar=0,location=no,directories=0,status=0,menubar=0,resizable=1,width=' + theWidth + ',height=' + theHeight;
    if (centerWindow)
    {
      var theLeft = (screen.width - theWidth)/2;
      var theTop = (screen.height - theHeight)/2;
      someFeatures += ",left=" + theLeft + ",top=" + theTop;
    }

    var aPopUpWin = document.open(thePage, theName, someFeatures);

    if (navigator.appName == "Netscape" && aPopUpWin != null) {
      aPopUpWin.focus();
    }
  }

  function showStatus(msg)
  {
    window.status = msg
    return true
  }

  // Piggy-back function onto onLoad event ......................................
  function addLoadEvent(func)
  {
    var oldonload = window.onload;
    if (typeof window.onload != 'function') {
      window.onload = func;
    } else {
      
      window.onload = function()
      {
        if( oldonload )
        {
          oldonload();
        }
        func();
      }
    }
  }
