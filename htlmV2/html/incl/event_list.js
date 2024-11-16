var isBrowserIE = false;
var IEVersion = 5; //Always assuming quirksmode
if ((navigator.appName == "Microsoft Internet Explorer") && (navigator.platform != "MacPPC") && (navigator.platform != "Mac68k"))
{
  isBrowserIE = true;
  if( document.documentMode )
    IEVersion = parseInt(document.documentMode);
}

var eventNameSpaces = new SOAPClientNS();
eventNameSpaces.add("aev", "http://www.axis.com/vapix/ws/event1" );
eventNameSpaces.add("tns1", "http://www.onvif.org/ver10/topics" );
eventNameSpaces.add("tnsaxis", "http://www.axis.com/2009/event/topics" );
eventNameSpaces.add("wsnt", "http://docs.oasis-open.org/wsn/b-2" );

var limitToConditions = false;
var limitToMotion = false;
var preSelectedTopicObj;

function getGroupChildNrFromTopic(topicKey)
{
  var key = strippTopic(topicKey);
  
  var almostMatch = -1;
  for( var i = 0; i < groupChildArr.length; i++ )
  {
    if( key == groupChildArr[i].topic)
      return i;

    if( key.indexOf( groupChildArr[i].topic ) == 0 )
      almostMatch = i;
  }
  return almostMatch;
}

function getGroupNrFromTopic(topicKey)
{
  var groupChildNr = getGroupChildNrFromTopic(topicKey);
  var groupNr = -1;
  if (groupChildNr >= 0)
  {
    groupNr = groupChildArr[groupChildNr].parent;
  }
  else
  {
    groupNr = findGroupNrFromUnknownTopic(topicKey);
  }

  return groupNr;
}

function findGroupNrFromUnknownTopic(topic)
{
  var strippedTopic = strippTopic(topic);
  var splitTopic = strippedTopic.split("/");
  var findKey;
  while (splitTopic.length > 0)
  {
    findKey = splitTopic.join("/");
    for (var i = 0; i < groupArr.length; i++)
    {
      for (var j = 0; groupArr[i].topicArray && j < groupArr[i].topicArray.length; j++)
      {
        if (groupArr[i].topicArray[j] == findKey)
        {
          return i;
        }
      }
    }
    splitTopic.length--;
  }

  return -1;
}

function getEventName(topicKey)
{
  var groupChildNr = getGroupChildNrFromTopic(topicKey);
  if (groupChildNr >= 0)
  {
    return groupChildArr[groupChildNr].name;
  }
  return false;
}

function setActiveInactiveLabels( aLabel, iLabel )
{
  activeLabel = aLabel;
  inactiveLabel = iLabel;
}

function getEventList( onlyConditions, selectedTopics, onlyMotion )
{
  var params = new SOAPClientParameters();

  var now = new Date();
  var timestamp = "?timestamp="+now.getTime();
  
  var wsdlUrl = "/wsdl/vapix/EventService.wsdl";
  var url = "/vapix/services"+timestamp;
  var method = "aev:GetEventInstances";

  if( onlyConditions )
  {
    limitToConditions = true;
  }

  if( onlyMotion )
  {
    limitToMotion = true;
  }

  if( selectedTopics )
  {
    preSelectedTopicObj = selectedTopics;
  }

  return SOAPClient.invoke( wsdlUrl, url, eventNameSpaces, method, params, false, loadEventList );
}

var selectedTopic = new Array();
var loadedTopics = new Array();
var topicNames = new Array();

function loadEventList( responseObj )
{
  if( responseObj )
  {
    if( responseObj.name == "Error" )
    {
      var errorMsg = ( ( responseObj.description )?responseObj.description:(responseObj.fileName)?responseObj.fileName:responseObj.message );
      alert( errorMsg );
      return false;
    }
    var selectEl = document.getElementById("triggerParent");

    var topicSet = responseObj.TopicSet;
    var topicParent, topicParentName;

    for( var topicParentKey in topicSet )
    {
      if( topicParentKey == "topic" || topicParentKey == "NiceName" )
      {
        continue;
      }
      topicParent = topicSet[ topicParentKey ];
      if( topicParent )
      {
        topicParentName = ( ( topicParent.NiceName )?topicParent.NiceName:"" );
        if( limitToMotion )
        {
          if( topicParentKey == "VideoAnalytics" )
          {
            loadMotionTopic( selectEl, topicParent, topicParentKey, topicParentName );
          }
          else if( topicParentKey == "UserAlarm" )
          {
            //render in schedulelist
            loadScheduleOptions( topicParent, ((topicParentName == "" )?topicParentKey:topicParentName ) );
          }
          else
          {
            loadTopics( selectEl, topicParent, topicParentKey, topicParentName, 0, true );
          }
        }
        else if( topicParentKey == "UserAlarm" && !limitToConditions )
        {
          //render in schedulelist
          loadScheduleOptions( topicParent, ((topicParentName == "" )?topicParentKey:topicParentName ) );
          //render
          loadTopics( selectEl, topicParent, topicParentKey, topicParentName, 0 );
        }
        else
        {
          loadTopics( selectEl, topicParent, topicParentKey, topicParentName, 0 );
        }
      }
    }

    var len = loadedTopics.length;
    for( var i = 0; i < len; i++ )
    {
      if( loadedTopics[i] )
      {
        addTopicEls( loadedTopics[i] );
      }
    }
    
    createGroups();
    hide_option_any_when_two_options();
  }
  return true;
}

function createGroups()
{
  var groupParent = document.getElementById("groupParent");
  var groupContainer = document.getElementById("groupContainer");
  var triggerParent = document.getElementById("triggerParent");
  
  triggerParent.preselected = false;
  groupParent.preselected = false;
  groupParent.onchange = function(){ eval( "toggleTopics( this, 0 )" ); };

  addSelectOption(groupParent, "", "----------", false);
  createGroupsFromSelect(triggerParent, groupParent, groupContainer, 0);
  sortSelect(groupParent);
  setPreselectedTopic();
}

function createGroupsFromSelect(selectEl, groupParent, groupContainer, level)
{
  if (!(selectEl && selectEl.options))
  {
    return;
  }
  
  var selectedIndex = selectEl.selectedIndex;
  var groupChildNr, groupNr;
  for (var i = 0; i < selectEl.options.length; i++)
  {
    var topicKey = selectEl.options[i].value;
    groupNr = -1;
    groupChildNr = getGroupChildNrFromTopic(topicKey);
    if (groupChildNr >= 0)
    {
      groupNr = groupChildArr[groupChildNr].parent;
    }
    var key = elementNameFromTopic(topicKey);
    var childSelectEl = document.getElementById( "select_" + key );
    if (groupNr == -1 && !childSelectEl)
    {
      groupNr = findGroupNrFromUnknownTopic(topicKey);
      if (groupNr == -1)
      {
        var strippedTopic = strippTopic(topicKey);
        var groupName = strippedTopic.split("/")[0];
        groupArr.push(new Group(groupName, [groupName]));
        groupNr = groupArr.length - 1;
      }
    }
    if (groupNr >= 0)
    {
      if (groupChildNr == -1 || groupChildNr >= 0 && groupChildArr[groupChildNr].display)
      {
        if(!((global_skipPTZ || !global_PTZEnabled) && groupArr[groupNr].name == "PTZ" )) {
          if (!groupArr[groupNr].collected)
          {
            groupArr[groupNr].collected = true;
            addSelectOption( groupParent, "group_" + groupNr, groupArr[groupNr].name, false );
          
            var sel = document.createElement("SELECT");
            sel.id = "select_group_" + groupNr;
            sel.className = "block large";
            sel.disabled = true;
            sel.style.display = "none";
            sel.level = level+1;
            sel.onchange = function(){ eval( "toggleTopics( this, this.level )" ); };
            groupContainer.appendChild(sel);
          
            groupArr[groupNr].el = sel;
          }
          var name = selectEl.options[i].text;
          var value = selectEl.options[i].value;
          checkAndMoveGroup(groupContainer, groupArr[groupNr].el, name, value, level+1);
        }
      }
    }
    else if (childSelectEl)
    {
      createGroupsFromSelect(childSelectEl, groupParent, groupContainer, level+1);
    }
  }
  enable_and_sort_select(selectEl);
}

function checkAndMoveGroup(groupContainer, selectEl, name, value, level)
{
  var tmpNameArray, tmpName;
  
  var groupChildNr = getGroupChildNrFromTopic(value);
  if ( groupChildNr >= 0 && groupChildArr[groupChildNr].display && groupChildArr[groupChildNr].valueName )
  {
    var sel = document.getElementById("select_" + elementNameFromTopic(name));
    if (!sel)
    {
      sel = document.createElement("SELECT");
      sel.id = "select_" + elementNameFromTopic(name);
      sel.className = "block large";
      sel.disabled = true;
      sel.style.display = "none";
      sel.level = level+1;
      sel.onchange = function(){ eval( "toggleTopics( this, this.level )" ); };
      if ( groupChildArr[groupChildNr].option2 )
      {
        sel.title = groupChildArr[groupChildNr].option2;
      }
      document.getElementById("groupContainer").appendChild(sel);
      addSelectOption( selectEl, elementNameFromTopic(name), name, false );
    }
    
    addSelectOption( sel, value, groupChildArr[groupChildNr].valueName, false );
    enable_and_sort_select(sel);
  }
  else
  {
    addSelectOption( selectEl, value, name, false );
  }

  enable_and_sort_select(selectEl);
  var oChild = document.getElementById("select_" + elementNameFromTopic(value));
  if ( oChild )
  {
    checkAndRenameGroups(oChild);
  }
}

function checkAndRenameGroups(selectEl)
{
  var groupChildNr, topicKey;
  for (var i = 0; i < selectEl.options.length; i++)
  {
    topicKey = selectEl.options[i].value;
    groupChildNr = getGroupChildNrFromTopic(topicKey);
    if (groupChildNr >= 0 && groupChildArr[groupChildNr].valueName)
    {
      selectEl.options[i].text = groupChildArr[groupChildNr].valueName;
    }
  }
}

function setPreselectedTopic()
{
  if( !preSelectedTopicObj )
  {
    return;
  }
  
  selectedTopic = new Array();
  
  var groupNr = getGroupNrFromTopic(preSelectedTopicObj.topicExpression);
  if (groupNr >= 0)
  {
    var groupParent = document.getElementById("groupParent");
    var id = "group_" + groupNr;
    var selOk = setSelectedOption(groupParent, id);
    
    if(!selOk)
      groupParent.options[groupParent.options.length] = new Option("-", id, false, true);

    setPreselectedTopicOnSelect(id, 1);
  }
}

function setPreselectedTopicOnSelect(topicKey, level)
{
  selectedTopic.push(topicKey);
  var elementName = elementNameFromTopic(topicKey);
  var selEl = document.getElementById("select_" + elementName);
  if (selEl)
  {
    selEl.level = level;
    if( !(selEl.options.length == 1 && selEl.options[0].value.indexOf("Channel_") > 0) )
    {
      selEl.style.display = "block";
    }
    var nextTopic;
    var topicFound = false;
    for (var i = 0; i < selEl.options.length; i++)
    {
      nextTopic = selEl.options[i].value;
      if (String( preSelectedTopicObj.topicExpression ).indexOf( nextTopic ) != -1)
      {
        selEl.options[i].selected = true;
        topicFound = true;
        break;
      }
    }
    if (!topicFound)
    {
      var groupChildNr = getGroupChildNrFromTopic( preSelectedTopicObj.topicExpression );
      var elementName = elementNameFromTopic(groupChildArr[groupChildNr].topic);
      addSelectOption( selEl, elementName, groupChildArr[groupChildNr].name, true, "generalError" );
      enable_and_sort_select(selEl);
      var selectEl = document.createElement("SELECT");
      selectEl.id = "select_SI_"+elementName;
      selectEl.className = "block large";
      selectEl.disabled = true;
      selectEl.level = level + 1;
      selectEl.onchange = function(){ eval( "toggleTopics( this, this.level )" ); };
      if( !global_PTZEnabled && elementName == "Device_IO_VirtualPort" )
      {
        selectEl.style.display = "none";
      }
      addTopicEls([ selectEl ]);
      addSelectOption( selectEl, "INVALID_EVENT", txt_invalid_trigger, true, "generalError" );
    }
    setPreselectedTopicOnSelect(selEl.options[selEl.selectedIndex].value, level+1);
  }
  else
  {
    var selEl = document.getElementById("select_SI_" + elementName);
    if (selEl)
    {
      selEl.level = level;
      if( !global_PTZEnabled && elementName == "Device_IO_VirtualPort" )
      {
        selEl.style.display = "block";
      }
      if( !selEl.options[selEl.selectedIndex].preselected )
      {
        addSelectOption( selEl, "INVALID_EVENT", txt_invalid_trigger, true, "generalError" );
        if( selEl.options.length > 1 )
        {
          selEl.disabled = false;
        }
      }
      topicKey = selEl.options[selEl.selectedIndex].value;
      selectedTopic.push(topicKey);
    }
    var inputDataElSel = document.getElementById("data_input_select_state"+elementName ) ||
                        document.getElementById("data_input_select"+elementName );
    if( inputDataElSel )
    {
      inputDataElSel.level = level;
      inputDataElSel.style.display = "block";
    }

    var inputDataElTrue = document.getElementById("data_input_true"+elementName );
    var inputDataElFalse = document.getElementById("data_input_false"+elementName );
    if( inputDataElTrue && inputDataElFalse )
    {
      document.getElementById("main_label_data_input"+elementName ).style.display = "";
      inputDataElTrue.style.display = "";
      inputDataElFalse.style.display = "";
      if( !inputDataElTrue.checked && !inputDataElFalse.checked )
      {
        inputDataElTrue.checked = true;
      }
      document.getElementById("label_data_input_true"+elementName ).style.display = "";
      document.getElementById("label_data_input_false"+elementName ).style.display = "";

      inputDataElTrue.level = level;
      inputDataElFalse.level = level;
    }
  }
}

function loadScheduleOptions( topicParent, parentName )
{
  var scheduleSelect = document.getElementById("schedule");

  var topicName;
  for( var topicKey in topicParent )
  {
    if( topicKey == "topic" || topicKey == "NiceName" )
    {
      continue;
    }
    topicObj = topicParent[ topicKey ];
    if( topicObj )
    {
      topicName = ( ( topicObj.NiceName )?topicObj.NiceName:topicKey )
      nameArr = new Array( topicName );
      addScheduleOptions( scheduleSelect, topicObj, "tns1:UserAlarm/tnsaxis:"+topicKey, nameArr );
    }
  }
}

var scheduleNames = new Array();
var recurrenceNames = new Array();

function addScheduleName( optionValue, nodeValue, nameArr )
{
  if( !scheduleNames[ optionValue ] )
  {
    scheduleNames[ optionValue ] = new Array();
    scheduleNames[ optionValue+"//." ] = new Array();
  }
  scheduleNames[ optionValue ][ nodeValue ] = nameArr;
  scheduleNames[ optionValue+"//." ][ nodeValue ] = nameArr;
  var strippedOption = strippTopic(optionValue);
  if( strippedOption != optionValue )
  {
    addScheduleName( strippedOption, nodeValue, nameArr );
  }
}

function addScheduleOptions( selectEl, topicParent, optionValue, nameArr )
{
  var messageInstance = topicParent.MessageInstance;
  if( !messageInstance )
  {
    var topicObj;
    for( var topicKey in topicParent )
    {
      if( topicKey == "topic" || topicKey == "NiceName" )
      {
        continue;
      }
      topicObj = topicParent[ topicKey ];
      if( topicObj )
      {
        topicName = ( ( topicObj.NiceName )?topicObj.NiceName:topicKey )
        nameArr[nameArr.length] = topicName;
        addScheduleOptions( selectEl, topicObj, optionValue+"/"+topicKey, nameArr );
      }
    }
  }
  else
  {
    var sourceInstance = messageInstance["SourceInstance"];
    if( sourceInstance )
    {
      var itemInstances = sourceInstance["SimpleItemInstance"];
      if( itemInstances )
      {
        if( itemInstances.constructor != Array )
        {
          itemInstances = new Array( itemInstances );
        }
        var itemInstance;
        for( var itemIndex = 0; itemIndex < itemInstances.length; itemIndex++ )
        {
          itemInstance = itemInstances[ itemIndex ];
          var valueNodes = itemInstance["Value"];
          if( valueNodes )
          {
            var itemName = itemInstance.Name;
            if( valueNodes.constructor != Array )
            {
              valueNodes = new Array( valueNodes );
            }
            var nodeValue, messagesPart;
            var isSelected = false;
            var optionSelected = false;

            var nodeObj;

            if( preSelectedTopicObj && String( preSelectedTopicObj.topicExpression ).indexOf( optionValue ) != -1 )
            {
              isSelected = true;
            }
            
            valueNodes.sort( sortByNiceName );

            for( var i = 0; i < valueNodes.length; i++ )
            {
              optionSelected = false;
              nodeObj = valueNodes[i];
              if( nodeObj.constructor == Object )
              {
                nodeValue = nodeObj["#text"];

                var displayName = ( nodeObj.NiceName )?nodeObj.NiceName:nodeValue;
                var itemId = nameArr.concat( displayName );
                if( messageInstance.isProperty == "true" )
                {
                  if( preSelectedTopicObj && preSelectedTopicObj.messages && preSelectedTopicObj.messages[ itemName ] && preSelectedTopicObj.messages[ itemName ] == nodeValue )
                  {
                    optionSelected = true;
                  }
                  messagesPart = 'boolean(//SimpleItem[@Name="'+itemName+'" and @Value="'+nodeValue+'"]) and boolean(//SimpleItem[@Name="active" and @Value="1"])';

                  addScheduleName( optionValue, nodeValue, itemId );
                  addSelectOption( selectEl, optionValue+"#"+messagesPart, displayName, optionSelected );
                  enable_and_sort_select(selectEl);
                }
                else
                {
                  if( !recurrenceNames[ optionValue ] )
                  {
                    recurrenceNames[ optionValue ] = new Array();
                    recurrenceNames[ optionValue+"//." ] = new Array();
                    recurrenceNames[ strippTopic(optionValue) ] = new Array();
                  }
                  recurrenceNames[ optionValue ][ nodeValue ] = itemId;
                  recurrenceNames[ optionValue+"//." ][ nodeValue ] = itemId;
                  recurrenceNames[ strippTopic(optionValue) ][ nodeValue ] = itemId;
                }
              }
            }
          }
        }
      }
    }
  }
}

function addTopicEls( arr )
{
  var len = arr.length;
  var groupContainer = document.getElementById("subGroupContainer");
  for( var i = 0; i < len; i++ )
  {
    if( arr[i] )
    {
      groupContainer.appendChild(arr[i]);
    }
  }
}

function loadMotionTopic( selectEl, topicParent, topicParentKey, topicParentName )
{
  var topicObj;
  var topicName, topicValue, topicText;

  for( var topicKey in topicParent )
  {
    if( topicKey == "topic" || topicKey == "NiceName" )
    {
      continue;
    }

    if( topicKey != "MotionDetection" )
    {
      continue;
    }

    topicObj = topicParent[ topicKey ];
    if( topicObj )
    {
      topicName = ( ( topicObj.NiceName )?topicObj.NiceName:"" );
      topicText = topicKey;

      if( topicParentName == "" && topicName == "" )
      {
        topicText = topicParentKey+" - "+topicKey;
      }
      else if( topicParentName != "" && topicName != "" )
      {
        topicText = topicParentName+" - "+topicName;
      }
      else if( topicName != "" )
      {
        topicText = topicName;
      }

      var tmpStr = String( topicParentKey ).replace( /_/, "/tnsaxis:" );
      tmpStr = String( tmpStr ).replace( /_/g, "/" );

      topicValue = "tns1:"+tmpStr+"/tnsaxis:"+topicKey;
      selectedTopic[ 0 ] = "group_" + getGroupNrFromTopic(topicValue);
      selectedTopic[ 1 ] = topicValue;

      topicNames[ topicValue ] = new NameObject( topicText );
      addSubTopicElements( topicParentKey+"_"+topicKey, topicValue, topicName, topicObj, 1 );
      addSelectOption( selectEl, topicValue, topicText, true );
      enable_and_sort_select(selectEl);
      break;
    }
  }
}

function NameObject( name )
{
  this.Name = name;
  this.Messages = new Array();
  return this;
}

function loadTopics( selectEl, topicParent, topicParentKey, topicParentName, level, namesOnly )
{
    var retVal = false;
    var topicObj;
    if( !loadedTopics[ level ] )
    {
      loadedTopics[ level ] = new Array();
    }
    var topicName, optionValue, optionText, isSelected;

    for( var topicKey in topicParent )
    {
      if( topicKey == "topic" || topicKey == "NiceName" )
      {
        continue;
      }

      topicObj = topicParent[ topicKey ];
      if( topicObj )
      {
        topicName = ( ( topicObj.NiceName )?topicObj.NiceName:"" );
        
        tmpStr = String( topicParentKey ).replace( /_/, "/tnsaxis:" );
        tmpStr = String( tmpStr ).replace( /_/g, "/" );

        optionValue = "tns1:"+tmpStr+( (level == 0)?"/tnsaxis:":"/" )+topicKey;

        optionText = getEventName(optionValue);
        if( topicKey.indexOf("Channel_") == 0 && image_names )
        {
          var img_nbr = topicKey.replace("Channel_", "");
          if ( images_enabled )
          {
            if ( !images_enabled[img_nbr-1] )
            {
              continue;
            }
            else if ( topicParentKey.indexOf("PTZController") == 0 && ptz_enabled && !ptz_enabled[img_nbr-1] )
            {
              continue;
            }
          }
          optionText = getImageName( img_nbr );
        }
        if (!optionText)
        {
          if (topicName != "")
          {
            optionText = topicName;
          }
          else
          {
            optionText = topicKey;
          }
        }

        if( addSubTopicElements( topicParentKey+"_"+topicKey, optionValue, topicName, topicObj, level+1, namesOnly ) )
        {
          if( !topicNames[ optionValue ] )
          {
            topicNames[ optionValue ] = new NameObject( optionText );
          }
          else
          {
            topicNames[ optionValue ].Name = optionText;
          }
          if( !namesOnly )
          {
            if( topicKey.indexOf("Channel_") == 0 && image_names.length == 1 )
            {
              selectEl.style.display = "none";
            }
            addSelectOption( selectEl, optionValue, optionText, false );
            enable_and_sort_select(selectEl);
          }
          retVal = true;
        }
      }
    }

    return retVal;
}
/*
*/
function getSimpleItemNiceName(itemName, value)
{
  for(var i=0; i < simpleItemArr.length; i++)
  {
    if(simpleItemArr[i].itemName == itemName && simpleItemArr[i].value == value)
      return simpleItemArr[i].niceName;
  }
  return value;
}

function addSubTopicElements( parentId, parentValue, parentName, topicObj, level, namesOnly )
{
  var retVal = false;
  if( !loadedTopics[ level ] )
  {
    loadedTopics[ level ] = new Array();
  }
  var messageInstance = topicObj["MessageInstance"];
  if( !messageInstance )
  {
    if( !namesOnly )
    {
      var selectEl = document.createElement("SELECT");
      selectEl.id = "select_"+parentId;
      if( preSelectedTopicObj && String( preSelectedTopicObj.topicExpression ).indexOf(parentValue) != -1 )
      {
        selectEl.preselected = true;
      }
      else
      {
        selectEl.preselected = false;
      }
      selectEl.style.display = "none";
      selectEl.className = "block large";
      selectEl.disabled = true;
      selectEl.level = level+1;
      selectEl.onchange = function(){ eval( "toggleTopics( this, this.level );") };
    }
    if( loadTopics( selectEl, topicObj, parentId, parentName, level, namesOnly ) )
    {
      if( !namesOnly )
      {
        loadedTopics[ level ][ loadedTopics[ level ].length ] = selectEl;
      }
      retVal = true;
    }
    return retVal;
  }

  var onlySelect = false;
  var sourceInstance = messageInstance["SourceInstance"];
  var sourceItemInstances;
  if( sourceInstance )
  {
    sourceItemInstances = sourceInstance["SimpleItemInstance"];
  }
  var dataInstance = messageInstance["DataInstance"];
  var dataItemInstances;
  if( dataInstance )
  {
    dataItemInstances = dataInstance["SimpleItemInstance"];
    if (limitToConditions && !dataItemInstances.isPropertyState)
    {
      return false;
    }
  }
  if( sourceInstance && sourceItemInstances )
  {
    if( sourceItemInstances.constructor != Array )
    {
      sourceItemInstances = new Array( sourceItemInstances );
    }
    var itemInstance;
    for( var itemIndex = 0; itemIndex < sourceItemInstances.length; itemIndex++ )
    {
      itemInstance = sourceItemInstances[ itemIndex ];
      var valueNodes = itemInstance["Value"];
      if( valueNodes )
      {
        var itemName = itemInstance.Name;

        if( !limitToConditions && parentId == "UserAlarm_Recurring_Interval" ||
            limitToConditions && parentId.indexOf("UserAlarm") >= 0)
        {
          onlySelect = true;
          itemName = parentId;
          valueNodes = "";
        }
        if( onlySelect || valueNodes.constructor != Array && valueNodes.constructor != Object )
        {
          if( !namesOnly && dataItemInstances && dataItemInstances.isPropertyState )
          {
            //render hidden value
            var inputEl = document.createElement("INPUT");
            inputEl.type = "hidden";
            inputEl.id = "input_"+parentId;
            inputEl.name = itemName;
            inputEl.value = valueNodes;
            document.getElementById("groupContainer").appendChild(inputEl);
          }
        }
        else
        {
          if( !namesOnly )
          {
            //render select
            var selectEl = document.createElement("SELECT");
            selectEl.id = "select_SI_"+parentId;
            selectEl.className = "block large";
            selectEl.disabled = true;
            selectEl.level = level + 1;
            selectEl.onchange = function(){ eval( "toggleTopics( this, this.level )" ); };
            selectEl.name = itemName;
            if( !global_PTZEnabled && parentId == "Device_IO_VirtualPort" )
            {
              selectEl.style.display = "none";
            }
          }
          if( !topicNames[ parentValue ] )
          {
            topicNames[ parentValue ] = new NameObject( "" );
          }
          var nodeValue;
          var nodeName;
          var isSelected = ( !limitToMotion )?false:true;
          var optionSelected = false;
          
          if( valueNodes.constructor != Array )
          {
            valueNodes = new Array( valueNodes );
          }
          var len = valueNodes.length;
          var nodeObj, niceName;

          if( preSelectedTopicObj && String( preSelectedTopicObj.topicExpression ).indexOf(parentValue) != -1 )
          {
            isSelected = true;
          }
          valueNodes.sort( sortByNiceName );

          for( var i = 0; i < len; i++ )
          {
            nodeObj = valueNodes[i];
            if( nodeObj.constructor != Object )
            {
              nodeValue = nodeObj;
              nodeName = nodeObj;
            }
            else
            {
              nodeValue = nodeObj["#text"];
              nodeName = (( nodeObj.UserString )?nodeObj.UserString:nodeValue);
              nodeName = (( nodeObj.NiceName )?nodeObj.NiceName:nodeName );
            }
            niceName = getSimpleItemNiceName(itemName, nodeName);
            addTopicOptionElement( level, limitToMotion, (i == 0), parentValue, itemName, nodeValue, nodeName, selectEl, isSelected, namesOnly );
          }
          if( !namesOnly )
          {
            var groupChildNr = getGroupChildNrFromTopic(parentValue);
            if (groupChildNr >= 0)
            {
              var lbl = groupChildArr[groupChildNr].option1;
              if (lbl)
              {
                selectEl.title = lbl;
              }
            }
            if( !global_PTZEnabled && parentId == "Device_IO_VirtualPort" )
            {
              selectEl.style.display = "none";
            }
            else
            {
              selectEl.style.display = (isSelected ? "" : "none");
            }
            loadedTopics[ level ][ loadedTopics[ level ].length ] = selectEl;
          }
        }
      }
    }
  }

  if( !limitToConditions )
  {
    retVal = true;
  }

  if( !onlySelect && dataInstance )
  {
    if( dataItemInstances )
    {
      if( dataItemInstances.constructor != Array )
      {
        dataItemInstances = new Array( dataItemInstances );
      }
      var itemInstance;
      var statesContainerEl = document.getElementById("statesContainer");
      for( var itemIndex = 0; itemIndex < dataItemInstances.length; itemIndex++ )
      {
        itemInstance = dataItemInstances[ itemIndex ];
        if( limitToConditions )
        {
          retVal = true;
        }

        var itemType = itemInstance.Type;
        if( itemType != "xsd:boolean" && !namesOnly && itemInstance["Value"] && dataItemInstances.isPropertyState )
        {
          var value = itemInstance["Value"];
          var itemName = itemInstance.Name;
          var inputEl = document.createElement("INPUT");
          inputEl.type = "hidden"
          inputEl.id = "data_input_"+parentId;
          inputEl.name = itemName;
          inputEl.value = value;
      
          statesContainerEl.appendChild(inputEl);
        }
        else if (itemType != "xsd:boolean" && itemInstance["Value"] && itemInstance["Value"].constructor == Array)
        {
          var itemName = itemInstance.Name;
          if( !topicNames[ parentValue ] )
          {
            topicNames[ parentValue ] = new NameObject( "" );
          }
          topicNames[ parentValue ].ActiveName = itemName;

          if( !namesOnly )
          {
            var displayEl = "none";
            if( ( preSelectedTopicObj && String( preSelectedTopicObj.topicExpression ).indexOf(parentValue) != -1 ) )
            {
              displayEl = "";
            }
            
            var selectEl = document.createElement("SELECT");
            if (dataItemInstances.isPropertyState)
            {
              selectEl.id = "data_input_select_state"+parentId;
            }
            else
            {
              selectEl.id = "data_input_select"+parentId;
            }

            selectEl.className = "block large";
            selectEl.level = level + 1;
            selectEl.name = itemName;
            selectEl.style.display = displayEl;
            selectEl.disabled = true;

            var isSelected;
            for( var val in itemInstance["Value"])
            {
              isSelected = false;
              if( displayEl == "" && preSelectedTopicObj && preSelectedTopicObj.messages && preSelectedTopicObj.messages[ itemName ] && preSelectedTopicObj.messages[ itemName ] == itemInstance["Value"][val] )
              {
                isSelected = true;
              }
              addSelectOption( selectEl, itemInstance["Value"][val], itemInstance["Value"][val], isSelected );
            }
            enable_and_sort_select(selectEl);

            statesContainerEl.appendChild(selectEl);
          }
        }
        else if( itemType == "xsd:boolean")
        {
          var itemName = itemInstance.Name;
          if( !topicNames[ parentValue ] )
          {
            topicNames[ parentValue ] = new NameObject( "" );
          }
          topicNames[ parentValue ].ActiveName = itemName;

          if( !namesOnly )
          {
            var display = false;
            if( ( preSelectedTopicObj && String( preSelectedTopicObj.topicExpression.split("//.")[0] ) == parentValue ) || limitToMotion )
            {
              display = true;
            }
            
            var boolean_label = ((itemInstance.NiceName)?itemInstance.NiceName:itemName);
            var boolean_active = activeLabel;
            var boolean_inactive = inactiveLabel;
            
            var groupChildNr = getGroupChildNrFromTopic(parentValue);
            if (groupChildNr >= 0)
            {
              if (groupChildArr[groupChildNr].boolean_label)
                boolean_label = groupChildArr[groupChildNr].boolean_label;
              if (groupChildArr[groupChildNr].boolean_active)
                boolean_active = groupChildArr[groupChildNr].boolean_active;
              if (groupChildArr[groupChildNr].boolean_inactive)
                boolean_inactive = groupChildArr[groupChildNr].boolean_inactive;
            }
            var mainLabelEl = document.createElement("LABEL");
            mainLabelEl.id = "main_label_data_input"+parentId;
            mainLabelEl.innerHTML = boolean_label+":";
            mainLabelEl.style.display = (display ? "" : "none");
            statesContainerEl.appendChild(mainLabelEl);
            
            var selected = true;
            if( display )
            {
              var preVal;
              if( preSelectedTopicObj && preSelectedTopicObj.messages && preSelectedTopicObj.messages[ itemName ] )
              {
                preVal = preSelectedTopicObj.messages[ itemName ];
              }

              if( preVal == "0" )
              {
                selected = false;
              }
            }
            createStatefullOption(display, selected,  itemName, parentId, limitToMotion, statesContainerEl, boolean_active,   "1", "true"+parentId);
            createStatefullOption(display, !selected, itemName, parentId, limitToMotion, statesContainerEl, boolean_inactive, "0", "false"+parentId);
          }
        }
      }
    }
  }
  return retVal;
}
function createStatefullOption(isDisplayed, isSelected, itemName, parentId, limitToMotion, container, label, value, id)
{
var inputEl;

if (isBrowserIE && IEVersion < 9)
{
  var inputEl_str = '<input type="radio" name="' + itemName + '"'  + (isSelected?' checked':'') + ' />';
  inputEl = document.createElement(inputEl_str);
}
else
{
  inputEl = document.createElement("INPUT");
  inputEl.type = "radio";
  inputEl.name = "data_input_" + parentId + "_" + itemName;
  inputEl.checked = isSelected;
}
inputEl.className = "radio";
inputEl.id = "data_input_"+id;
inputEl.value = value;
inputEl.style.display = ( isDisplayed ? "" : "none");
if( parentId == "Device_Status_Boot" ||
    parentId == "Device_Status_SystemInitializing" )
{
  inputEl.disabled = true;
}
if( limitToMotion )
{
  inputEl.disabled = true;
}
container.appendChild(inputEl);

var labelEl = document.createElement("LABEL");
labelEl.id = "label_data_input_"+id;
labelEl.htmlFor = "data_input_"+id;
labelEl.className = "radio";
labelEl.innerHTML = label;
labelEl.style.display = ( isDisplayed ? "" : "none");
container.appendChild(labelEl);

inputEl.label_data_input = labelEl;
}

function addTopicOptionElement( level, limitToMotion, isFirst, parentValue, itemName, nodeValue, nodeName, selectEl, isSelected, namesOnly )
{
  topicNames[ parentValue ].Messages[ itemName+":"+nodeValue ] = nodeName;
  if( !namesOnly )
  {
    var optionSelected = false;
    if( isSelected )
    {
      if( preSelectedTopicObj && preSelectedTopicObj.messages && preSelectedTopicObj.messages[ itemName ] == nodeValue )
      {
        optionSelected = true;
      }
      else if( !preSelectedTopicObj && limitToMotion && isFirst )
      {
        optionSelected = true;
      }
    }
    if( parentValue.indexOf("PTZPresets") > 0)
    {
      var selectedSource = parentValue.slice( parentValue.lastIndexOf("/")+1 ).replace(/Channel_/ig, "");
      if( isNaN( selectedSource ) )
      {
        selectedSource = "1";
      }
      selectedSource = parseInt(selectedSource, 10) - 1;
      var oHomePos = document.getElementsByName("root_PTZ_Preset_P" + selectedSource + "_HomePosition");
      if( nodeValue != -1 && oHomePos && oHomePos[0] && oHomePos[0].value == nodeValue )
      {
        nodeName += " (H)";
      }
    }
    else
    {
      nodeName = getOptionName( parentValue, nodeValue, nodeName );
    }
    addSelectOption( selectEl, nodeValue, nodeName, optionSelected );
    selectEl.options[ selectEl.length - 1 ].preselected = optionSelected;
    enable_and_sort_select(selectEl);
  }
}

function getOptionName(parentValue, nodeValue, nodeName )
{
  var n;
  if ( parentValue.indexOf("PTZReady") > 0 ||
       parentValue.indexOf("Move") > 0 ||
       parentValue.indexOf("Tampering") > 0 ||
       parentValue.indexOf("Connections") > 0 ||
       parentValue.indexOf("IO/VirtualPort") > 0 )
  {
    n =  getImageName( parseInt(nodeValue, 10) );
  }
  else if( parentValue.indexOf("TriggerLevel") > 0)
  {
    n =  audio_name_default.replace("#", nodeValue);
  }
  else if( parentValue.indexOf("IO/Port") > 0)
  {
    if( nodeValue != "-1" )
      n =  parseInt(nodeValue, 10) + 1;
    else
      n =  txtAny;
  }
  else
  {
    n = nodeName;
  }

  return n;
}

function toggleTopics( mainSelect, level )
{
  hidePreTopics( level );
  showTopic( mainSelect, level );
}

function showTopic( mainSelect, level )
{
  var selectedIndex = ( mainSelect.selectedIndex >= 0 )?mainSelect.selectedIndex:0;
  var selectedValue = mainSelect.options[ selectedIndex ].value;
  var selEl;

  selectedTopic[ level ] = selectedValue;
  //show selected topic
  if( selectedValue )
  {
    selEl = toggleTopicInput( selectedValue, true, level );
    if( selEl)
    {
      showTopic( selEl, level+1 );
    }
  }
}

function toggleTopicInput( selectedValue, showTopicInput, level )
{
  var displayValue = (showTopicInput ? "" : "none" );
  var elementName = elementNameFromTopic( selectedValue );
  var selEl = document.getElementById("select_"+elementName);
  if( !selEl )
  {
    selEl = document.getElementById("select_SI_"+elementName);
  }
  if( selEl )
  {
    if( !(selEl.options.length == 1 && selEl.options[0].value.indexOf("Channel_") > 0) )
    {
      selEl.level = level+1;
      if( showTopicInput && !global_PTZEnabled && elementName == "Device_IO_VirtualPort" )
      {
        displayValue = "none";
      }
    }
    selEl.style.display = displayValue;
  }

  var inputDataElSel = document.getElementById("data_input_select_state"+elementName ) ||
                       document.getElementById("data_input_select"+elementName );
  if( inputDataElSel )
  {
    inputDataElSel.style.display = displayValue;
  }

  var inputDataElTrue = document.getElementById("data_input_true"+elementName );
  var inputDataElFalse = document.getElementById("data_input_false"+elementName );
  if( inputDataElTrue && inputDataElFalse )
  {
    document.getElementById("main_label_data_input"+elementName ).style.display = displayValue;
    inputDataElTrue.style.display = displayValue;
    inputDataElFalse.style.display = displayValue;
    if( showTopicInput&& !inputDataElTrue.checked && !inputDataElFalse.checked )
    {
      inputDataElTrue.checked = true;
    }
    document.getElementById("label_data_input_true"+elementName ).style.display = displayValue;
    document.getElementById("label_data_input_false"+elementName ).style.display = displayValue;
  }

  return selEl;
}

function hidePreTopics( level )
{
  var len = selectedTopic.length;
  for( var i = len; i > level; i-=1 )
  {
    var topic = selectedTopic.pop();
    //hide selected topic
    if( topic )
    {
      toggleTopicInput( topic, false, level );
    }
  }
}

function getSelectedTopicFilter()
{
  var valueIndex = selectedTopic.length - 1;
  var selectedValue = !selectedTopic[ valueIndex ] ? "" : selectedTopic[ valueIndex ];
  var topicExpress;

  var isCondition = false;
  var oSchedule = document.getElementById("schedule");
  var selectedSchedule = oSchedule.options[oSchedule.selectedIndex].value;

  var conditionValue;

  var messages = new Array();
  var elementName = elementNameFromTopic( selectedValue );
  var selectEl, messageValue;

  while( selectedValue.indexOf("tns1:") == -1 && valueIndex > 0 )
  {
    valueIndex--;
    messageValue = selectedValue;
    selectedValue = selectedTopic[ valueIndex ];
    elementName = elementNameFromTopic( selectedValue );
    selectEl = document.getElementById("select_"+elementName );
    if( !selectEl )
    {
      selectEl = document.getElementById("select_SI_"+elementName );
      if( !selectEl )
      {
        selectEl = document.getElementById(elementName );
      }
      if( selectEl )
      {
        messageValue = selectEl.options[ selectEl.selectedIndex ].value;
      }
    }
    messages[messages.length] = 'boolean(//SimpleItem[@Name="'+selectEl.name+'" and @Value="'+messageValue+'"])';
  }

  var inputDataElSel = document.getElementById("data_input_select_state"+elementName );
  if( inputDataElSel)
  {
    isCondition = true;
    messages[messages.length] = 'boolean(//SimpleItem[@Name="'+inputDataElSel.name+'" and @Value="'+inputDataElSel.options[inputDataElSel.selectedIndex].value+'"])';
  }

  inputDataElSel = document.getElementById("data_input_select"+elementName );
  if( inputDataElSel)
  {
    messages[messages.length] = 'boolean(//SimpleItem[@Name="'+inputDataElSel.name+'" and @Value="'+inputDataElSel.options[inputDataElSel.selectedIndex].value+'"])';
  }

  var inputDataElTrue = document.getElementById("data_input_true"+elementName );
  var inputDataElFalse = document.getElementById("data_input_false"+elementName );
  if( inputDataElTrue && inputDataElFalse )
  {
    var actualName = (inputDataElTrue.name).replace("data_input_" + elementName + "_", "");
    if( inputDataElTrue.checked )
    {
      isCondition = true;
      messages[messages.length] = 'boolean(//SimpleItem[@Name="'+actualName+'" and @Value="1"])';
    }
    else if ( inputDataElFalse.checked )
    {
      isCondition = true;
      messages[messages.length] = 'boolean(//SimpleItem[@Name="'+actualName+'" and @Value="0"])';
    }
  }
  else
  {
    var inputDataElHidden = document.getElementById("data_input_"+elementName );
    if( inputDataElHidden )
    {
      isCondition = true;
      messages[messages.length] = 'boolean(//SimpleItem[@Name="'+inputDataElHidden.name+'" and @Value="'+inputDataElHidden.value+'"])';
    }
  }

  if (!isCondition && (selectedValue === "tns1:UserAlarm/tnsaxis:Recurring/Interval" || selectedValue.length === 0) && selectedSchedule != "") {
    isCondition = true;
  }

  var returnObj;
  if( messages.length > 0 )
  {
    returnObj = getTopicFilterObject( selectedValue, messages.join(' and ') );
  }
  else
  {
    returnObj = getTopicFilterObject( selectedValue );
  }
  return { topicFilter:returnObj, isCondition:isCondition };
}

function getTopicFilterObject( topicStr, messageStr )
{
  var topicFilterObject = new Array();
  var topicObj = new Object();

  var strDialect = "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet";
  strDialect = strDialect.setIsAttribute( true );
  topicObj.Dialect = strDialect;

  topicStr = convertTopicValue(topicStr);

  topicStr = topicStr.setIsObjectNodeValue( true );
  topicObj.value = topicStr;

  topicFilterObject["wsnt:TopicExpression"] = topicObj;

  if( messageStr )
  {
    var messageObj = new Object();
    strDialect = "http://www.onvif.org/ver10/tev/messageContentFilter/ItemFilter";
    strDialect = strDialect.setIsAttribute( true );
    messageObj.Dialect = strDialect;

    messageStr = messageStr.setIsObjectNodeValue( true );
    messageObj.value = messageStr;

    topicFilterObject["wsnt:MessageContent"] = messageObj;
  }
  return topicFilterObject;
}

/** See convert functions in action_rule_setup and action_rules for more info - search for "convert" - this function is used for saving **/
function convertTopicValue(topicStr)
{
  var convertArr = new Array();
  convertArr.push( { oldExp: "tns1:Storage/tnsaxis:Disruption", newExp: "tnsaxis:Storage/Disruption"} );
  convertArr.push( { oldExp: "tns1:Storage/tnsaxis:Recording", newExp: "tnsaxis:Storage/Recording"} );
  convertArr.push( { oldExp: "tns1:CameraApplicationPlatform/tnsaxis:", newExp: "tnsaxis:CameraApplicationPlatform/"} );

  for(var i=0; i < convertArr.length; i++)
  {
    if(topicStr.indexOf(convertArr[i].oldExp) > -1)
    {
      return topicStr.replace(convertArr[i].oldExp, convertArr[i].newExp);
    }
  }
  
  return topicStr;
}

function getSelectedConditionTopicFilter()
{
  var valueIndex = selectedTopic.length - 1;
  var selectedValue = selectedTopic[ valueIndex ];
  var topicExpress;

  var conditionValue;

  var messages = new Array();
  var elementName = elementNameFromTopic( selectedValue );
  var selectEl, messageValue;
  var names = new Array();

  while( selectedValue.indexOf("tns1:") == -1 && valueIndex > 0 )
  {
    valueIndex--;
    messageValue = selectedValue;
    selectedValue = selectedTopic[ valueIndex ];
    elementName = elementNameFromTopic( selectedValue );
    selectEl = document.getElementById("select_"+elementName );
    if( !selectEl )
    {
      selectEl = document.getElementById("select_SI_"+elementName );
      messageValue = selectEl.options[ selectEl.selectedIndex ].value;
    }
    names[ names.length ] = selectEl.options[ selectEl.selectedIndex ].text;
    messages[messages.length] = 'boolean(//SimpleItem[@Name="'+selectEl.name+'" and @Value="'+messageValue+'"])'; 
  }

  var inputDataElTrue  = document.getElementById("data_input_true"+elementName );
  var inputDataElFalse = document.getElementById("data_input_false"+elementName );
  var isActive = false;
  if( inputDataElTrue && inputDataElFalse )
  {
    var actualName = (inputDataElTrue.name).replace("data_input_" + elementName + "_", "");

    if( inputDataElTrue.checked )
    {
      messages[messages.length] = 'boolean(//SimpleItem[@Name="' + actualName + '" and @Value="1"])';
      isActive = true;
    }
    else if( inputDataElFalse.checked )
    {
      messages[messages.length] = 'boolean(//SimpleItem[@Name="' + actualName + '" and @Value="0"])';
    }
  }
  
  var inputDataElSel = document.getElementById("data_input_select_state"+elementName );
  if( inputDataElSel)
  {
    isActive = true;
    messages[messages.length] = 'boolean(//SimpleItem[@Name="'+inputDataElSel.name+'" and @Value="'+inputDataElSel.options[inputDataElSel.selectedIndex].value+'"])';
  }

  inputDataElSel = document.getElementById("data_input_select"+elementName );
  if( inputDataElSel)
  {
    isActive = true;
    messages[messages.length] = 'boolean(//SimpleItem[@Name="'+inputDataElSel.name+'" and @Value="'+inputDataElSel.options[inputDataElSel.selectedIndex].value+'"])';
  }

  while( valueIndex > 0 )
  {
    valueIndex--;
    elementName = elementNameFromTopic( selectedTopic[ valueIndex ] );
    selectEl = document.getElementById("select_"+elementName );
    if( !selectEl )
    {
      selectEl = document.getElementById("select_SI_"+elementName );
    }

    if( selectEl )
    {
      names[ names.length ] = selectEl.options[ selectEl.selectedIndex ].text;
    }
  }

  selectEl = document.getElementById("groupParent");
  if( selectEl )
  {
    names[ names.length ] = selectEl.options[ selectEl.selectedIndex ].text;
  }

  var returnObj = new Object();
  returnObj.names = names.reverse();
  returnObj.isActive = isActive;
  returnObj.topic = selectedValue;

  if( messages.length > 0 )
  {
    returnObj.message = messages.join(' and ');
  }
  return returnObj;
}

function getSelectedConditionProperties( selectedConditionTopic, messages, checkForSchedule )
{
  var messageArray = parseMessageStr( messages );
  var names = new Array();

  var selectedSchedule;
  var active = "0";
  var selectEl = document.getElementById("groupParent");

  if( selectedConditionTopic.indexOf("UserAlarm") == -1 )
  {
    names = getConditionNames( selectedConditionTopic, messageArray );
    var topicConditionName = selectedConditionTopic.replace("//.", "");
    if( topicNames[ topicConditionName ] && topicNames[ topicConditionName ].ActiveName )
    {
      if( messageArray[ topicNames[ topicConditionName ].ActiveName ] )
      {
        active = ( ( messageArray[ topicNames[ topicConditionName ].ActiveName ] == "1")?"1":"0" );
      }
    }
  }
  else if( scheduleNames[ selectedConditionTopic ] && messageArray["id"] )
  {
    names = scheduleNames[ selectedConditionTopic ][ messageArray["id"] ];
    active = ( ( messageArray["active"] == "1")?"1":"0" );
    
    if( checkForSchedule && active == "1" )
    {
      selectedSchedule = {topic:selectedConditionTopic, id:messageArray["id"]};
    }
  }

  return {names:names, isActive:active, schedule:selectedSchedule };
}

function getConditionNames( selectedConditionTopic, messages )
{
  var foundNames = new Array();
  selectedConditionTopic = selectedConditionTopic.replace(/\/\/\./g, "");
  var groupChildNr = getGroupChildNrFromTopic( selectedConditionTopic );
  var selectedConditions = selectedConditionTopic.split("/");
  var len = selectedConditions.length;

  if( groupChildNr >= 0 )
  {
    foundNames.push( groupArr[ groupChildArr[ groupChildNr ].parent ].name );
    foundNames.push( groupChildArr[ groupChildNr ].name );
    var label_y = groupChildArr[ groupChildNr ].boolean_active;
    var label_n = groupChildArr[ groupChildNr ].boolean_inactive;
    if( label_y && label_n && topicNames[ selectedConditionTopic ] && topicNames[ selectedConditionTopic ].ActiveName  && messages )
    {
      foundNames.push( topicNames[ selectedConditionTopic ].ActiveName == "1" ? label_y : label_n );
    }
  }
  else if( len > 1 )
  {
    var findCondition = selectedConditions[0];
    for( var i = 1; i < len; i++ )
    {
      findCondition = findCondition+"/"+selectedConditions[i];
      if( topicNames[ findCondition ] )
      {
        foundNames[ foundNames.length ] = topicNames[ findCondition ].Name;
      }
    }
    if( topicNames[ findCondition ].Messages && messages )
    {
      for( var val in messages )
      {
        if( topicNames[ findCondition ].Messages[ val+":"+messages[val] ] )
        {
          foundNames[ foundNames.length ] = topicNames[ findCondition ].Messages[ val+":"+messages[val] ];
        }
      }
    }
  }

  return foundNames;
}

function parseMessageStr( str )
{
  var messageArr = new Array();
  var re = /boolean\(\/\/SimpleItem\[@Name="([^"]*)" and @Value="([^"]*)"\]\)/ig;
  var theName, theValue, matchArr;
  do
  {
    matchArr = re.exec(str);

    if( matchArr && matchArr.length > 2 )
    {
      theName = matchArr[ 1 ];
      theValue = matchArr[ 2 ];
      if( theName && theValue )
      {
        messageArr[ theName ] = theValue;
      }
    }
  }
  while( matchArr )

  return messageArr;
}

function elementNameFromTopic( topic )
{
    var elementName = ( topic ).replace( /\/tnsaxis:/g, "_" );
    elementName = ( elementName ).replace( /\/\/\.$/, "" );
    elementName = ( elementName ).replace( /\//g, "_" );
    elementName = ( elementName ).replace( "tns1:", "" );

    return elementName;
}

function strippTopic(topic)
{
  return topic.replace(/tns1:/ig, "").replace(/tnsaxis:/ig, "").replace(/\/\/\./g, "");
}

function sortByNiceName( firstItem, lastItem )
{
  var firstCompareValue = "";
  var lastCompareValue = "";

  if( firstItem.NiceName )
  {
    firstCompareValue = firstItem.NiceName;
  }
  else if( firstItem.UserString )
  {
    firstCompareValue = firstItem.UserString;
  }
  else if( firstItem["#text"] )
  {
    firstCompareValue = firstItem["#text"];
  }
  else if (firstItem)
  {
    firstCompareValue = firstItem;
  }

  if( lastItem.NiceName )
  {
    lastCompareValue = lastItem.NiceName;
  }
  else if( lastItem.UserString )
  {
    lastCompareValue = lastItem.UserString;
  }
  else if( lastItem["#text"] )
  {
    lastCompareValue = lastItem["#text"];
  }
  else if (lastItem)
  {
    lastCompareValue = lastItem;
  }
  return ( ( firstCompareValue ).localeCompare( lastCompareValue ) );
}

function sortSelect( obj )
{
  var tmp  = new Array();
  var len = obj.options.length;
  var selectedIndex = obj.selectedIndex;
  
  for (i = 0; i < len; i++) {
    tmp[i] = new Array(obj.options[i].text.toLowerCase(), new Object(obj.options[i]), i == selectedIndex);
  }
  if (tmp[0][0].replace(/[0-9]/g,"").length > 0)
  {
    tmp.sort();
  }
  else
  {
    tmp.sort(sortInt);
  }


  obj.options.length = 0;

  var newSelectedIndex = -1;
  for (i = 0; i < len; i++)
  {
    obj.options.add(tmp[i][1]);
    if (tmp[i][2])
    {
      newSelectedIndex = i;
    }
  }
  obj.selectedIndex = newSelectedIndex;
}

function sortInt(a, b)
{
  a = parseInt(a, 10);
  b = parseInt(b, 10);
  if (a < b)
    return -1;
  else if (a == b)
    return 0;
  else
    return 1;
}

var selects_using_any = new Array();
function enable_and_sort_select(o)
{
  if (o.options.length != 1)
  {
    for ( var i = 0; i < o.options.length; i++ )
    {
      if (o.options[i].value == "-1" || o.options[i].value == "Any")
      {
        selects_using_any.push(o);
        break;
      }
    }
    o.disabled = false;
    sortSelect(o);
  }
}

function hide_option_any_when_two_options()
{
  for (var i = 0; i < selects_using_any.length; i++)
  {
    var s = selects_using_any[i];
    if (s.options.length == 2)
    {
      if (s.options[0].value == "-1" || s.options[0].value == "Any")
      {
        s.options[1].selected = true;
        s.disabled = true;
      }
      else if (s.options[1].value == "-1" || s.options[1].value == "Any")
      {
        s.options[0].selected = true;
        s.disabled = true;
      }
    }
  }
}

var ajaxRequest;
function sendAction(action, method, asText)
{
  if( !ajaxRequest )
    ajaxRequest = AxisConnectionFactory.createAjaxConnection();
  AxisConnectionFactory.sendSync([method, action], null, ajaxRequest);

  var result = (ajaxRequest.getStatus() == 200 ? ( asText )?ajaxRequest.getResponseText(): ajaxRequest.getResponseXml() : null);
  return result;
}

function getRecipientParamValue(params, name)
{
  for( var i = 0; i < params.length; i++ )
  {
    if (params[i].Name == name)
    {
      return params[i].Value;
    }
  }
  return "";
}

function getImageName( img_nbr )
{
  img_nbr = parseInt( img_nbr, 10 ) - 1;
  if( image_names[img_nbr] && image_names[img_nbr].length > 0 )
  {
    return image_names[img_nbr];
  }
  else
  {
    return image_name_default.replace( "#", img_nbr );
  }
}

function checkValidEvent(event)
{
  if (!event.MessageContent || !event.MessageContent["#text"])
  {
    return {isValid: true, error:""};
  }

  var topicKey = strippTopic( event.TopicExpression["#text"] );
  var message;
  if( event.MessageContent && event.MessageContent["#text"] )
  {
    message = parseMessageStr( event.MessageContent["#text"] );
  }
  if (topicKey == "VideoAnalytics/MotionDetection")
  {
    var re = /<OPTION VALUE="M([^"]*)">M([^"]*)<\/OPTION>/ig;

    var matchArr, nr;
    do
    {
      matchArr = re.exec(checkValidEvent_existingMotionWindowStr);

      if( matchArr && matchArr.length > 2 )
      {
        nr = matchArr[ 1 ];
        if( nr && nr == message["window"] )
        {
          return {isValid: true, error:""};
        }
      }
    }
    while( matchArr )

    return {isValid: false, error: checkValidEvent_error_MD_not_found};
  }
  else if( topicKey.indexOf("PTZController") == 0 && !global_PTZEnabled )
  {
    return {isValid: false, error:checkValidEvent_error_PTZ_disabled};
  }
  else if( topicKey.indexOf("PTZController/PTZPresets") == 0 )
  {
    var selectedSource = topicKey.slice( topicKey.lastIndexOf("/")+1 ).replace(/Channel_/ig, "");
    if( isNaN( selectedSource ) )
    {
      selectedSource = "1";
    }
    selectedSource = parseInt(selectedSource, 10) - 1;
    
    var presetStr = "root_PTZ_Preset_P" + selectedSource + "_Position_P" + message["PresetToken"] + "_Name";
    var o = document.getElementsByName( presetStr );
    if (o.length == 0 && message["PresetToken"] != "-1")
    {
      return {isValid: false, error:checkValidEvent_error_Preset_not_found};
    }
  }
  else if( topicKey == "UserAlarm/Recurring/Interval" )
  {
    if( !scheduleNames[topicKey] || !message["id"] || !scheduleNames[topicKey][message["id"]] )
    {
      return {isValid: false, error:checkValidEvent_error_Schedual_not_found};
    }
  }
  else if( topicKey == "UserAlarm/Recurring/Pulse" )
  {
    if ( !recurrenceNames[topicKey] || !message["id"] || !recurrenceNames[topicKey][message["id"]] )
    {
      return {isValid: false, error:checkValidEvent_error_Recurrence_not_found};
    }
  }
  return {isValid: true, error:""};
}

function isValidFloat(value, min, max)
{
  var f = parseFloat(value);
  return f == value && ( isNaN(min) || f >= min ) && ( isNaN(max) || f <= max );
}
