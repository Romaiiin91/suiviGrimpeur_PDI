

function Language(pkg, file)
{
  this._pkg = pkg;
  this._file = file;
  this._isOutput = true;
  this._isIncl = false;
    
  this._uploadedLang = null;
  this._defaultLang = null;
  this._ajax = AxisConnectionFactory.createAjaxConnection();
  
  if( !Language_CacheGet(this._pkg) )
  {
    for(var i=0; i < 2; i++)
    {
      if(i == 0)
        this._action = "/language/uploaded/language_"+this._pkg+".xml";
      else
        this._action = "/language/default/language_"+this._pkg+".xml";

      var now = new Date();
      var timestamp = now.getTime();
      
      var responseXMLResult = AxisConnectionFactory.getResponseXmlWith(["GET", this._action+"?timestamp="+timestamp, false], "", this._ajax);

      if(i == 0) 
        this._uploadedLang = this._ajax.getStatus() == 200 ? responseXMLResult.selectSingleNode("//"+file) : null;
      else 
        this._defaultLang = responseXMLResult.selectSingleNode("//"+file);
    }
    Language_CacheSet(this._pkg, this._defaultLang, this._uploadedLang);
  }
  else
  {
    var cacheObj = Language_CacheGet(this._pkg);
    this._uploadedLang = (cacheObj.responseUpl != null ?(cacheObj.responseUpl).selectSingleNode("//"+file):null);
    this._defaultLang = cacheObj.responseDef.selectSingleNode("//"+file);
  }
}

Language.prototype.init = function(isOutput, isIncl) 
{
  if(isOutput == false)
    this._isOutput = false;
    
  if(isIncl == true)
    this._isIncl = true;

  if(this._isOutput)
  {
    var data = "";
    var dynamicButtontext = "";
    var dynamicTitletext = "";
    
    if(this._uploadedLang != null && this._uploadedLang.childNodes.length != this._defaultLang.childNodes.length)
    {
      for(var i=0; i < this._defaultLang.childNodes.length; i++)
      {
        data = "";
        if(this._defaultLang.childNodes[i].nodeType == 1)
        {
          if(this._defaultLang.childNodes[i].getAttribute("type") == "text")
          {
            if( document.getElementById( this._defaultLang.childNodes[i].nodeName ) )
            {
              if( this._uploadedLang.selectSingleNode(this._defaultLang.childNodes[i].nodeName) != null)
              {
                data = this.preventInjections( (this._uploadedLang.selectSingleNode(this._defaultLang.childNodes[i].nodeName).firstChild.data) );
                document.getElementById( this._defaultLang.childNodes[i].nodeName ).innerHTML = data;
              }
              else
              {
                data = this.preventInjections( (this._defaultLang.childNodes[i].firstChild.data) );
                document.getElementById( this._defaultLang.childNodes[i].nodeName ).innerHTML = data;
              }
            }
          }
          else if(this._defaultLang.childNodes[i].getAttribute("type") == "button")
          {
            if( document.getElementById( this._defaultLang.childNodes[i].getAttribute("buttonid") ) )
            {
              dynamicButtontext = document.getElementById( this._defaultLang.childNodes[i].getAttribute("buttonid") ).value;
            
              if(this._uploadedLang.selectSingleNode(this._defaultLang.childNodes[i].nodeName) != null)
              {
                if(dynamicButtontext == this._uploadedLang.selectSingleNode(this._defaultLang.childNodes[i].nodeName).firstChild.data)
                  dynamicButtontext = "";
                document.getElementById( this._defaultLang.childNodes[i].getAttribute("buttonid") ).value = (this._uploadedLang.selectSingleNode(this._defaultLang.childNodes[i].nodeName).firstChild.data)+dynamicButtontext;
              }
              else
              {
                if(dynamicButtontext == this._defaultLang.childNodes[i].firstChild.data)
                  dynamicButtontext = "";
                document.getElementById( this._defaultLang.childNodes[i].getAttribute("buttonid") ).value = (this._defaultLang.childNodes[i].firstChild.data)+dynamicButtontext;
              }
            }
          }
          else if(this._defaultLang.childNodes[i].getAttribute("type") == "image")
          {
            if( document.getElementById( this._defaultLang.childNodes[i].getAttribute("imageid") ) )
            {
              if(this._uploadedLang.selectSingleNode(this._defaultLang.childNodes[i].nodeName) != null)
              {
                document.getElementById( this._defaultLang.childNodes[i].getAttribute("imageid") ).alt = (this._uploadedLang.selectSingleNode(this._defaultLang.childNodes[i].nodeName).firstChild.data);
                document.getElementById( this._defaultLang.childNodes[i].getAttribute("imageid") ).title = (this._uploadedLang.selectSingleNode(this._defaultLang.childNodes[i].nodeName).firstChild.data);
              }
              else
              {
                document.getElementById( this._defaultLang.childNodes[i].getAttribute("imageid") ).alt = (this._defaultLang.childNodes[i].firstChild.data);
                document.getElementById( this._defaultLang.childNodes[i].getAttribute("imageid") ).title = (this._defaultLang.childNodes[i].firstChild.data);
              }
            }
          }
          else if(this._defaultLang.childNodes[i].getAttribute("type") == "title")
          {
            if(document.title)
              dynamicTitletext = " - "+document.title;
          
            if(this._uploadedLang.selectSingleNode(this._defaultLang.childNodes[i].nodeName) != null)
            {
              document.title = this._uploadedLang.selectSingleNode(this._defaultLang.childNodes[i].nodeName).firstChild.data+dynamicTitletext;
            }
            else
            {
              document.title = this._defaultLang.childNodes[i].firstChild.data+dynamicTitletext;
            }
          }
        }
      }
    }
    else
    {
      var xmlLang = this._uploadedLang;
      if(xmlLang == null)
        xmlLang = this._defaultLang;
        
      for(var i=0; i < xmlLang.childNodes.length; i++)
      {
        data = "";
        if(xmlLang.childNodes[i].nodeType == 1)
        {
          if(xmlLang.childNodes[i].getAttribute("type") == "text")
          {
            if( document.getElementById( xmlLang.childNodes[i].nodeName ) )
            {
              data = this.preventInjections( (xmlLang.childNodes[i].firstChild.data) );
              document.getElementById( xmlLang.childNodes[i].nodeName ).innerHTML = data;
            }
          }
          else if(xmlLang.childNodes[i].getAttribute("type") == "button")
          {
            if( document.getElementById( xmlLang.childNodes[i].getAttribute("buttonid") ) )
            {
              dynamicButtontext = document.getElementById( xmlLang.childNodes[i].getAttribute("buttonid") ).value;
              
              if( dynamicButtontext == (xmlLang.childNodes[i].firstChild.data) )
                dynamicButtontext = "";
              
              document.getElementById( xmlLang.childNodes[i].getAttribute("buttonid") ).value = (xmlLang.childNodes[i].firstChild.data)+dynamicButtontext;
            }
          }
          else if(xmlLang.childNodes[i].getAttribute("type") == "image")
          {
            if( document.getElementById( xmlLang.childNodes[i].getAttribute("imageid") ) )
            {
              document.getElementById( xmlLang.childNodes[i].getAttribute("imageid") ).alt = (xmlLang.childNodes[i].firstChild.data);
              document.getElementById( xmlLang.childNodes[i].getAttribute("imageid") ).title = (xmlLang.childNodes[i].firstChild.data);
            }
          }
          else if(xmlLang.childNodes[i].getAttribute("type") == "title")
          {
            if(document.title)
              dynamicTitletext = " - "+document.title;
          
            document.title = xmlLang.childNodes[i].firstChild.data+dynamicTitletext;
          }
        }
      }
    }
    
    if( typeof globalLanguageVars == 'function' && this._isIncl == false)
    {
      globalLanguageVars();
    }
  }
}

Language.prototype.getText = function(id, isParhand)
{
  if(isParhand)
    id = id+"PHTxt";

  var data = "";
  var type = "";
  if( this._uploadedLang != null && this._uploadedLang.getElementsByTagName(id).length == 1 )
  {
    type = this._uploadedLang.getElementsByTagName(id)[0].getAttribute("type");
    data = (this._uploadedLang.getElementsByTagName(id)[0].childNodes[0].nodeValue);
  }
  else if( this._defaultLang.getElementsByTagName(id).length == 1 )
  {
    type = this._defaultLang.getElementsByTagName(id)[0].getAttribute("type");
    data = (this._defaultLang.getElementsByTagName(id)[0].childNodes[0].nodeValue);
  }
  else if(!isParhand || this._defaultLang.getElementsByTagName(id).length > 1)
  {
    throw new Error("Language.getText(): Duplicate instances/Not possible to find the given id \""+id+"\" in \""+this._file+".shtml\"");
  }
  
  if(type == "msgAsText")
    data = this.preventInjections(data);
  return data;
}

Language.prototype.preventInjections = function(data)
{
  if( data.toLowerCase().indexOf("<script") != -1 || data.toLowerCase().indexOf("</script") != -1 )
  {
    data = data.replace(/</g, "&lt;"); //try to prevent injections and broken tags
    data = data.replace(/>/g, "&gt;");
  }
  return data;
}

Language.prototype.getLanguageCode = function(langInUse, isAMC)
{
  var currentLang = langInUse.toLowerCase();
  var type = "";
  var now = new Date();
  var timestamp = now.getTime();
  var url = "/axis-cgi/view/language_viewer.cgi?action=type&timestamp="+timestamp;

  var type = AxisConnectionFactory.getResponseTextWith(["GET", url, false], "");
  
  if( type )
  {

    if(type == "")
      return "";
    
    var langArr = new Array();
    langArr["english"] = { amc: "eng", axis: "en" };
    langArr["portuguese"] = { amc: "por-br", axis: "pt" };
    langArr["chinese"] = { amc: "chi", axis: "zh" };
    langArr["french"] = { amc: "fre", axis: "fr" };
    langArr["german"] = { amc: "ger", axis: "de" };
    langArr["italian"] = { amc: "ita", axis: "it" };
    langArr["japanese"] = { amc: "jpn", axis: "ja" };
    langArr["spanish"] = { amc: "spa", axis: "es" };
    langArr["korean"] = { amc: "kor", axis: "ko" };
    langArr["russian"] = { amc: "rus", axis: "ru" };
    
    var choosenLangCode = "";
    
    if(type == "default" || type == "unknown" || type == "mini" || langArr[currentLang] == null)
    {
      if(isAMC)
        choosenLangCode = langArr["english"].amc;
      else
        choosenLangCode = langArr["english"].axis;
    }
    else if(type == "uploaded" || "bultin")
    {
      if(isAMC)
        choosenLangCode = langArr[currentLang].amc;
      else
        choosenLangCode = langArr[currentLang].axis;
    }
    
    return choosenLangCode;
  }
}

Language.prototype.addOptionsToList = function(list, opArr, addFirst)
{
  var op;
  for(var i=0; i < opArr.length; i++)
  {
    op = document.createElement('option');
    op.value = opArr[i].val;
    op.text = opArr[i].text;
    if(opArr[i].selected == true)
      op.selected = true;
      
    if(addFirst && addFirst == true && list.options.length > 0)
    {
      try
      {
        list.add(op, list.options[i]);
      }
      catch(err)
      {
        list.options.add(op, i );
      }
    }
    else
      list.options.add( op );
  }
}

Language.prototype.translateOptionsInList = function(list, opArr) {
  for (var i=0; i<list.length; i++) {
    for (var j=0; j<opArr.length; j++) {
      if (list[i].value === opArr[j].val) {
        list[i].text = opArr[j].text;
      }
    }
  }
}

Language.prototype.createOptionArrayFromList = function(listToConvert)
{
  var options = listToConvert.options;

  var retArr = new Array();
  var selectText = "";
  
  for(var i=0; i < options.length; i++)
  {
    if( isNaN( options[i].value.charAt(0) ) )
      selectText = this.getText(options[i].value, true);
    else
      selectText = this.getText( (options[i].text.toLowerCase()).replace(/ /g,""), true);
    if( selectText == "")
      selectText = options[i].text;
    retArr.push( {val: options[i].value, text: selectText, selected: options[i].selected} );
  }
  
  return retArr;
}

var Language_cacheArr = new Array();
function Language_CacheSet(pkg, responseDef, responseUpl)
{
  if( !Language_cacheArr[pkg] )
    Language_cacheArr[pkg] = ( {responseDef: responseDef, responseUpl: responseUpl} );
}

function Language_CacheGet(pkg)
{
  if( Language_cacheArr[pkg] )
    return Language_cacheArr[pkg];
  else
    return false;
}
