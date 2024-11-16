
  var progressDiv = null;
  var progressLength = 192;
  var progressTimer = null;
  var isProgress;

  function progress_init( progress_text, prog_top, prog_width, isProgressBar )
  {
    var top = 200;
    if(prog_top)
    top = prog_top;
    var width = 400;
    if(prog_width)
      width = prog_width;
    
    if(isProgressBar == null || isProgressBar == "undefined")
      isProgress = true;
    else
      isProgress = isProgressBar;
    
    var padding = 40;
    var left = parseInt((document.body.clientWidth - width - 2 * padding)/2, 10);
  
    progressDiv = document.createElement("DIV");
    progressDiv.style.display = "none";
    progressDiv.align = "center";
    progressDiv.style.backgroundColor = "transparent";
    progressDiv.style.position = "absolute";
    progressDiv.style.left = 0;
    progressDiv.style.top = 0;
    progressDiv.style.zIndex = "500";
    progressDiv.style.width = "100%";
    progressDiv.style.height = "100%";
    progressDiv.innerHTML = "<br />";

    document.body.appendChild( progressDiv );
  
    var div = document.createElement("DIV");
    div.style.top = top+"px";
    div.style.left = left+"px";
    div.style.position = "absolute";
    div.style.width = width+"px";
    div.style.border = "#3366cc solid 1px";
    div.style.backgroundColor = "#3366cc";
    div.style.margin = "0px 0px";
    if(isProgress)
      div.style.padding = (Number(padding)+12)+"px "+padding+"px";
    else
      div.style.padding = (Number(padding)+12)+"px "+padding+"px "+"10px "+padding+"px";
    div.style.zIndex = "501";
    div.style.opacity = 0.6;
    div.style.filter = "alpha(opacity=60)";
    div.innerHTML = "<br /><br /><br />";

    progressDiv.appendChild( div );
  
    var divContent = document.createElement("DIV");
    divContent.style.top = top+"px";
    divContent.style.left = left+"px";
    divContent.style.position = "absolute";
    divContent.style.width = width+"px";
    divContent.style.border = "black solid 1px";
    divContent.style.backgroundColor = "transparent";
    divContent.style.margin = "0px 0px";
    divContent.style.padding = padding+"px";
    divContent.style.zIndex = "502";
    divContent.innerHTML = "<span id='progressText'>"+( ( progress_text )?progress_text:"" )+"</span><br /><br />";

    progressDiv.appendChild( divContent );

    if(isProgress)
    {
      var imgDiv = document.createElement("DIV");
      imgDiv.id = "progressImg";
      imgDiv.style.left = "2px";
      imgDiv.style.position = "relative";
      imgDiv.style.width = progressLength+"px";
      imgDiv.style.height = "16px";
      imgDiv.style.margin = "0px 0px";
      imgDiv.style.padding = "0px 0px";

      imgDiv.style.backgroundColor = "#FFF";
      imgDiv.style.backgroundImage = "url(/pics/progress.gif)";
      imgDiv.style.backgroundRepeat = "no-repeat";
      imgDiv.style.backgroundPosition = "-"+ progressLength+"px bottom";
      
      imgDiv.style.zIndex = "503";

      divContent.appendChild( imgDiv );
      
      imgDiv = document.createElement("DIV"); 
      imgDiv.style.top = "-20px";
      imgDiv.style.position = "relative";
      imgDiv.style.width = "200px";
      imgDiv.style.height = "24px";
      imgDiv.style.margin = "0px 0px";
      imgDiv.style.padding = "0px 0px";
      imgDiv.style.backgroundColor = "transparent";
      imgDiv.style.backgroundImage = "url(/pics/progressBg.gif)";
      imgDiv.style.backgroundRepeat = "no-repeat";

      imgDiv.style.zIndex = "504";

      divContent.appendChild( imgDiv );
    }

  }

  function progress_start( progress_text, getPercentFunction, finishedProgressFunction, prog_top, prog_width, isProgressBar )
  {
    if( progressDiv == null || isProgress != isProgressBar)
    {
      progress_init(  progress_text, prog_top, prog_width, isProgressBar  );
    }
    else
    {
      //reset progress and set new text
      if(isProgress)
        document.getElementById( "progressImg" ).style.backgroundPosition = "-"+progressLength+"px bottom";
      document.getElementById("progressText").innerHTML = ( ( progress_text )?progress_text:"" );
    }

    if( document.Player )
    {
      document.Player.style.visibility = "hidden";
    }
    progressDiv.style.display = "block";

    progressTimer = window.setTimeout( 'progress_update( \''+getPercentFunction+'\', \''+finishedProgressFunction+'\' )', 1000 );
  }

  function progress_stop( finishedProgressFunction )
  {
    if( progressTimer )
    {
      window.clearTimeout( progressTimer );
    }

    if( progressDiv != null )
    {
      progressDiv.style.display = "none";
    } 

    if( document.Player )
    {
      document.Player.style.visibility = "visible";
    }

     eval( finishedProgressFunction );
  }

  function progress_disableInput(typeToDisable)
  {
    var inputs = document.getElementsByTagName("INPUT");
    if( typeToDisable )
    {
      typeToDisable = typeToDisable.toLowerCase();
    }
    var cntInputs = inputs.length
    for( var i = 0; i < cntInputs; i++)
    {
      if( !typeToEnable || typeToEnable == inputs[i].type.toLowerCase() )
      {
        inputs[i].oldDisabled = inputs[i].disabled;
        inputs[i].disabled = true;
      }
    }
  }
  function progress_enableInput(typeToEnable)
  {
    var inputs = document.getElementsByTagName("INPUT");
    if( typeToEnable )
    {
      typeToEnable = typeToEnable.toLowerCase();
    }
    
    var cntInputs = inputs.length;
    for( var i = 0; i < cntInputs; i++ )
    {
      if( !typeToEnable || typeToEnable == inputs[i].type.toLowerCase() )
      {
        inputs[i].disabled = inputs[i].oldDisabled;
      }
    }
  }

  function progress_update( getPercentFunction, finishedProgressFunction )
  {
    if( progressTimer )
    {
      window.clearTimeout( progressTimer );
    }

    var progressPercent = eval( getPercentFunction );

    if(isProgress)
    {
      if( progressPercent <= 100 )
      {
        var bgPx = parseInt( progressLength * ( progressPercent/100 ), 10 ) - progressLength;
        document.getElementById( "progressImg" ).style.backgroundPosition = bgPx+"px bottom";
      }
    }

    if( progressPercent == 100 )
    {
      progressTimer = window.setTimeout( 'progress_stop( \''+finishedProgressFunction+'\' )', 1000 );
    }
    else
    {
      progressTimer = window.setTimeout( 'progress_update( \''+getPercentFunction+'\', \''+finishedProgressFunction+'\' )', 1000 );
    }
  }
