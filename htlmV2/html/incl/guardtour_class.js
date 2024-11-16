//----------------------------------
// Class for handling guard tours
//
//----------------------------------

var _guardtour_preset_request = AxisConnectionFactory.createAjaxConnection(AxisConnection.msXml.v3);
_guardtour_preset_request.setOverrideMimeType( 'text/plain' );

var _guardtour_recorded_request = AxisConnectionFactory.createAjaxConnection(AxisConnection.msXml.v3);
_guardtour_recorded_request.setOverrideMimeType( 'text/plain' );
_guardtour_recorded_request.setOverrideMimeType( 'text/xml' );

var _guardtour_object;

function guardtour_item(itemType, id, name, status, mode, camnbr, delay, tbs)
{
  this.itemType = itemType; // "G" for Preset tours, "R" for Recorded tours
  this.id = id;
  this.name = name;
  this.status = status;     // Possible values are "running", "stopped" and "recording"
  this.mode = mode;         // Possible values are "sequential", "random" and "recorded"
  this.camnbr = camnbr;
  this.delay = delay;
}

function guardtour_list(camnbr, onGuardtoursLoaded)
{
  _guardtour_object = this;
  this.camnbr = camnbr;
  this.onGuardtoursLoaded = onGuardtoursLoaded;

  this._list = new Array();
  this.nbrOfRecordedToursInProduct = 0;
  this.nbrOfPresetToursInProduct = 0;
  this.isLoadingPresetTours = "no";
  this.isLoadingRecordedTours = "no";
  this.use_preset = "yes";
  this.use_recorded = "yes";

  this.load = function(use_preset, use_recorded)
  {
    if (use_preset == "yes" || use_preset == "no")
    {
      _guardtour_object.use_preset = use_preset;
    }
    if (use_recorded == "yes" || use_recorded == "no")
    {
      _guardtour_object.use_recorded = use_recorded;
    }
    _guardtour_object._list = new Array();
    if (_guardtour_object.use_preset == "yes")
    {
      _guardtour_object.isLoadingPresetTours = "loading";
    }
    if (_guardtour_object.use_recorded == "yes")
    {
      _guardtour_object.isLoadingRecordedTours = "loading";
    }
    if (_guardtour_object.use_preset == "yes")
    {
      _guardtour_object.load_preset_tours("yes");
    }
    if (_guardtour_object.use_recorded == "yes")
    {
      _guardtour_object.load_recorded_tours("yes");
    }
  }

  this.load_preset_tours = function(keep_list)
  {
    if (keep_list != "yes")
    {
      _guardtour_object._list = new Array();

    }
    var now = new Date();
    _guardtour_object.isLoadingPresetTours = "loading";
    _guardtour_object.nbrOfPresetToursInProduct = 0;
    AxisConnectionFactory.sendAsync(["GET", "/axis-cgi/param.cgi?action=list&responsecharset=utf8&group=GuardTour&timestamp=" + now.getTime()], "", _guardtour_object._load_preset_tours_onreadystatechange, _guardtour_preset_request);
  }

  this._load_preset_tours_onreadystatechange = function()
  {
    if( _guardtour_preset_request.getReadyState() == 4 )
    {
      var responseText = _guardtour_preset_request.getResponseText();
      if (_guardtour_preset_request.getStatus() == 200 && responseText && responseText.indexOf("# Error:") < 0)
      {
        var parameters = responseText.replace(/\r/ig, "").split("\n");
        var tours = new Array();
        for (var i = 0; i < parameters.length && parameters[i]; i++)
        {
          var tmp = parameters[i].split("=")
          var parameter_arr = tmp[0].split(".");
          var parameter_value = tmp[1];
          var parameter_group = parameter_arr[2];
          if( !tours[parameter_group] )
          {
            tours[parameter_group] = new guardtour_item("G", parameter_group.substr(1));
          }
          switch(parameter_arr[3])
          {
            case "Name":
              tours[parameter_group].name = parameter_value;
              break;
            case "Running":
              tours[parameter_group].status = (parameter_value == "yes" ? "running" : "stopped");
              break;
            case "CamNbr":
              tours[parameter_group].camnbr = parameter_value;
              break;
            case "RandomEnabled":
              tours[parameter_group].mode = (parameter_value == "yes" ? "random" : "sequential");
              break;
            case "TimeBetweenSequences":
              tours[parameter_group].delay = parameter_value;
              break;
          }
        }
        for(var key in tours )
        {
          _guardtour_object.nbrOfPresetToursInProduct++;
          _guardtour_object._list.push(tours[key]);
        }
      }
      _guardtour_object.isLoadingPresetTours = "loaded";
      if (_guardtour_object.isLoadingRecordedTours != "loading" && _guardtour_object.onGuardtoursLoaded)
      {
        _guardtour_object.onGuardtoursLoaded();
      }
    }
  }

  this.load_recorded_tours = function(keep_list)
  {
    if (keep_list != "yes")
    {
      _guardtour_object._list = new Array();
    }
    var now = new Date();
    _guardtour_object.isLoadingRecordedTours = "loading";
    _guardtour_object.nbrOfRecordedToursInProduct = 0;
    AxisConnectionFactory.sendAsync(["GET", "/axis-cgi/recordedtour/list.cgi?schemaversion=1&timestamp=" + now.getTime()], "", _guardtour_object._load_recorded_tours_onreadystatechange, _guardtour_recorded_request);
  }
  
  this._load_recorded_tours_onreadystatechange = function()
  {
    if( _guardtour_recorded_request.getReadyState() == 4 )
    {
      var responseXML = _guardtour_recorded_request.getResponseXml();
      if (_guardtour_recorded_request.getStatus() == 200 && responseXML)
      {
        var success = responseXML.getElementsByTagName("Success")[0];
        if (success)
        {
          var recordings = responseXML.getElementsByTagName("RecordingInformation");
          _guardtour_object.nbrOfRecordedToursInProduct = recordings.length;
          var id, name, status, camnbr, delay;
          for( var i = 0; i < _guardtour_object.nbrOfRecordedToursInProduct; i++)
          {
            for( var j = 0; j < recordings[i].childNodes.length; j++)
            {
              switch(recordings[i].childNodes[j].nodeName)
              {
                case "RecordingId":
                  id = recordings[i].childNodes[j].text;
                  break;
                case "NiceName":
                  name = recordings[i].childNodes[j].text;
                  break;
                case "Status":
                  status = recordings[i].childNodes[j].text;
                  status = (status == "playing" ? "running" : status);
                  break;
                case "Camera":
                  camnbr = recordings[i].childNodes[j].text;
                  break;
                case "DefaultLoopDelay":
                  delay = recordings[i].childNodes[j].text;
                  break;
              }
            }

            _guardtour_object._list.push( new guardtour_item("R", id, name, status, "recorded", camnbr, delay ) );
          }
        }
      }
      _guardtour_object.isLoadingRecordedTours = "loaded";
      if (_guardtour_object.isLoadingPresetTours != "loading" && _guardtour_object.onGuardtoursLoaded)
      {
        _guardtour_object.onGuardtoursLoaded();
      }
    }
  }

  this.get_guardtour_list = function()
  {
    return this._list;
  }

  this.get_guardtour = function(itemType, id)
  {
    var i = this._get_guardtour_list_item_index(itemType, id);
    if (i >= 0)
    {
      return this._list[i];
    }
    return false;
  }
  
  this.get_guardtour_by_name = function(name)
  {
    for (var i = 0; i < this._list.length; i++)
    {
      if (this._list[i].name == name)
      {
        return this._list[i];
      }
    }
    return false;
  }

  this._get_guardtour_list_item_index = function(itemType, id)
  {
    for (var i = 0; i < this._list.length; i++)
    {
      if (this._list[i].itemType == itemType &&
          this._list[i].id == id)
      {
        return i;
      }
    }
    return -1;
  }
