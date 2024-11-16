var Player;

function DrawTB(UseAudio, ImagePath, CamNbr, MediaRecording, HideMicSettings, ptz, enableJoystickMode, streamProfileNr, streamProfileName) // NOTE: HideMicSettings = "yes" for 207, otherwise it should be "no" (or undefined)
{
  var useMpeg = ImagePath.indexOf("h264") != -1 || ImagePath.indexOf("mpeg4") != -1;
  var useMjpeg = ImagePath.indexOf("mjpg") != -1;

  if (Player && typeof(Player.Play) != "undefined") {
    var config = "+play,+snapshot,+fullscreen";

    if ((useMjpeg || useMpeg) && MediaRecording != 0) {
      Player.EnableRecording = MediaRecording;
    }

    if (ptz == "yes" && enableJoystickMode == "true") {
      config+= ",+ptz";
    } else {
      config+= ",-ptz";
    }
    if ((useMjpeg || useMpeg) && MediaRecording != 0) {
      config+= ",+rec";
    } else {
      config+= ",-rec";
    }
    if (UseAudio == "yes") {
      if (HideMicSettings == "yes") {
        config+= ",+mute,+volume,-audiocontrol";
      } else {
        config+= ",+audiocontrol";
      }
    } else {
      config+= ",-audiocontrol";
    }

    Player.ToolbarConfiguration = config;

    if (enableJoystickMode == "false") {
      if(ptz == "yes")
        Player.UIMode = "ptz-absolute";
      else
        Player.UIMode = "none";
    }

    if (UseAudio == "yes" && (!(useMpeg && MediaRecording != 0) || (HideMicSettings != "yes" || useMpeg))) {
      initAudio(ImagePath, CamNbr, MediaRecording, ptz, streamProfileNr, streamProfileName);
    }

    if (UseAudio == "yes") {
      try {
        Player.Play();
      } catch(e) {}
    }
  }
}

function activateContextMenu()
{
  if (Player && typeof(Player.Play) != "undefined") {
    Player.EnableContextMenu = "1";
    return;
  } else {
    setTimeout('activateContextMenu()', 250);
  }
}

function initAudio(ImagePath, CamNbr, MediaRecording, ptz, streamProfileNr, streamProfileName)
{
  var confHost = getCurrentHostAndProt();
  Player.AudioConfigURL = confHost + "/axis-cgi/view/param.cgi?camera=" + CamNbr +  "&usergroup=anonymous&action=list&group=Audio,AudioSource,Properties.Audio" + ((!isNaN(streamProfileNr))? ",StreamProfile.S" + streamProfileNr : "");

  Player.AudioTransmitURL = "/axis-cgi/audio/transmit.cgi";

  if (ImagePath.indexOf("mjpg") != -1) {
    var recieveURL = "/axis-cgi/audio/receive.cgi?audio=1&camera=" + CamNbr;
    if (streamProfileName != "") recieveURL += "&streamprofile=" + streamProfileName;
    Player.AudioReceiveURL = confHost + recieveURL;
  }

  if (MediaRecording != 0)
    Player.EnableRecording = MediaRecording;
}



function InstallFilter(prodName, ID, CLSID_filter, ver_filter, cab, installText1, text, installText2, installText3, installText4)
{
  var versionInfo = getInstalledInfo( CLSID_filter );
  var installedVersion = versionInfo.installedVersion;
  var amcInstalled = versionInfo.amcInstalled;

  var newVersion = true;
  ver_filter = ver_filter.replace(/&#44;/g, ",");

  if( amcInstalled )
  {
    newVersion = compareVersions( installedVersion, ver_filter )
  }

  if( amcInstalled)
  {
    var createdElement;
    
    if (newVersion)
    {
      var text = installText1 + "&nbsp;" + installText2 + "&nbsp;" + installText3 + "<br>" +ID + "&nbsp;" + installText4;
      createdElement = createInstallObject(ID, CLSID_filter, cab, ver_filter, text);
    }
    else
    {
      createdElement = createInstallObject(ID);
    }
    
    filterinstallocation.appendChild(createdElement);
  }
}

function InstallLanguage(prodName, ID, CLSID_lang, ver_lang, cab, installText1, installText2, installText3, installText4)
{
  var versionInfo = getInstalledInfo( CLSID_lang );
  var installedVersion = versionInfo.installedVersion;
  var amcInstalled = versionInfo.amcInstalled;

  var newVersion = true;
  ver_lang = ver_lang.replace(/&#44;/g, ",");

  if( amcInstalled )
  {
    newVersion = compareVersions( installedVersion, ver_lang );
  }

  if( amcInstalled)
  {
    var createdElement;
  
    if (newVersion)
    {
      var text = installText1 + "&nbsp;" + installText2 + "&nbsp;" + installText3 + "<br>" +ID + "&nbsp;" + installText4;
      createdElement = createInstallObject(ID, CLSID_lang, cab, ver_lang, text);
    }
    else
    {
      createdElement = createInstallObject(ID);
    }
    
    filterinstallocation.appendChild(createdElement);
  }
}

function createInstallObject(id, clsid, cab, version, text)
{
  var createdElement;

  if(clsid && cab && version && text)
  {
    var element = '<object id="'+ id + '" height="0" width="0" classid="CLSID:' + clsid + '" codebase="/activex/' + cab + '#Version=' + version + '"><br><b>' + id + '</b>&nbsp;' + text + '<br></object>';
    
    try
    {
      createdElement = document.createElement(element);
    }
    catch(e)
    {
      createdElement = document.createElement("object");
      createdElement.setAttribute("id", id);
      createdElement.style.width = "0px";
      createdElement.style.height = "0px";
      createdElement.classid = "CLSID:" + clsid;
      createdElement.codebase = "/activex/" + cab + "#Version=" + version;
      
      document.write('<object id="'+ id + '" height="0" width="0" classid="CLSID:' + clsid + '" codebase="/activex/' + cab + '#Version=' + version + '"><br><b>' + id + '</b>&nbsp;' + text + '<br></object>');
    }
  }
  else
  {
    var element = '<object id="' + id + '" height="0" width="0" style="display: none"></object>';
    
    try
    {
      createdElement = document.createElement(element);
    }
    catch(e)
    {
      createdElement = document.createElement("object");
      createdElement.setAttribute("id", id);
      createdElement.style.width = "0px";
      createdElement.style.height = "0px";
      createdElement.style.display = "none";
    }
  }
  return createdElement;
}

function InstallDecoder(prodName, ID, CLSID_AMC, cab, ver_AMC, authorized, notAuthorizedText, authorizedText, installDecoderText1, installDecoderText2)
{
  var versionInfo = getInstalledInfo( CLSID_AMC );
  var installedVersion = versionInfo.installedVersion;

  ver_AMC = ver_AMC.replace(/&#44;/g, ",");
  var newerVersion = compareVersions( installedVersion, ver_AMC );

  var o = '<object id="'+ ID + '" height="0" width="0" classid="CLSID:' + CLSID_AMC + '" codebase="' + cab + '#version=' + ver_AMC + '" standby="Loading Axis Media Control components...">';
  if (newerVersion) {
    if (authorized != "yes") {
      o += '<br><p><b>' + notAuthorizedText + '</b><br>';
    } else {
      o += '<br><p><a href="javascript:launch(\'';
      if (ID == "AAC Decoder") {
        o += '/incl/aac_license.shtml\')">';
      } else if (ID == "H.264 Decoder") {
        o += '/incl/license_h264.shtml\')">';
      } else {
        o += '/incl/license.shtml\')">';
      }
      o += authorizedText + '</a><br>';
    }
    if (installedVersion == "0,0,0,0") {
      o += '<br>' + installDecoderText1;
      if (authorized == "yes") {
        o += '<br>' + installDecoderText2;
      }
    } else {
      o += '<br>' + installDecoderText2;
    }
  }
  document.write(o + '<br></object>');
}


function drawMpeg4Dec(height, width, ImagePath, CLSID_AMC, cab, ver_AMC)
{
  document.write('<object id="Decoder" height="' + height + '" width="' + width + '" border="0" classid="CLSID:' + CLSID_AMC + '" codebase="/activex/decoder/' + cab + '#version=' + ver_AMC + '" standby="Loading Axis Media Control components..."><br></object>');
}

function drawAacDec(height, width, ImagePath, CLSID_AAC_dec, cab, ver_AAC_dec)
{
  document.write('<object id="aac" height="' + height + '" width="' + width + '" border="0" classid="CLSID:' + CLSID_AAC_dec + '" codebase="/activex/decoder/' + cab + '#version=' + ver_AAC_dec + '" standby="Loading Axis Media Control components..."></object>');
}

// NOTE: extra = "yes" -> 207, extra = "9999" means centermode (zoom=9999) is activated in 212PTZ, extra = "recording" -> recording playback, otherwise extra should be "no" or undefined.
function DrawAMC(prodName, ID, height, width, ImagePath, CLSID_AMC, cab, ver_AMC, ShowAMCToolbar, ptzgui, useWithCam, CamNbr, UseRel, ShowRelCross, ShowSVG, UseMotion, UseAudio, rtspPort, external, installText1, text, installText2, installText3, installText4, mediaRecording, extra, enableAreaZoom, maintainAspectRatio, minimap, languageCode )
{
  var host = getHost(ImagePath);
  var confHost = getCurrentHostAndProt();


  var mpeg4 = ImagePath.indexOf("videocodec=mpeg4") != -1;
  var h264 = ImagePath.indexOf("videocodec=h264") != -1;
  var mjpg = ImagePath.indexOf("videocodec=mjpg") != -1;
  var mpeg2 = ImagePath.indexOf("videocodec=mpeg2") != -1;
  var multicast = ImagePath.indexOf("sdp") != -1;

  var o = '<object id="Player" height="' + height + '" width="' + width + '" border="0" classid="CLSID:' + CLSID_AMC + '" codebase="/activex/' + cab + '#version=' + ver_AMC + '" standby="Loading Axis Media Control components...">';


  if (ImagePath.indexOf("://") > 0 || external == "yes") {
    o += '<param name="MediaURL" value="' + ImagePath + '">';
  } else if ((mpeg4 || h264) && external == "no") {
    o += '<param name="MediaURL" value="' + host + ':' + rtspPort + '' + ImagePath + '">';
  } else {
    o += '<param name="MediaURL" value="' + host + ImagePath + '">';
  }
  var MediaType;
  if (mpeg4) {
    MediaType = "mpeg4"
  } else if (h264) {
    MediaType = "h264"
  } else if (mpeg2) {
    if (multicast)
      MediaType = "mpeg2-multicast"
    else
      MediaType = "mpeg2-unicast"
  } else {
    MediaType = "mjpeg"
  }
  if (languageCode !== "undefined" && languageCode !== null) {
    o += '<param name="Language" value="' + languageCode + '">';
  }
  o += '<param name="MediaType" value="' + MediaType + '">';
  o += '<param name="Volume" value="' + (extra == "yes"?'70':'1') + '">'; // 207
    o += '<param name="ShowStatusBar" value="' + (ShowAMCToolbar == "yes"?'1':'0') + '">';
  o += '<param name="ShowToolbar" value="' + (ShowAMCToolbar == "yes"?'1':'0') + '">';
  o += '<param name="AutoStart" value="' + (UseAudio == "yes"?'0':'1') + '">';
  o += '<param name="StretchToFit" value="' + (((UseMotion == "yes") || (external == "yes"))?'0':'1') + '">';

  if ((ptzgui == "yes") && (useWithCam == "yes")) {
    extra += ""
    var ptzControlUrl = confHost + '/axis-cgi/com/ptz.cgi?camera=' + CamNbr + (extra == "9999"?'&zoom=9999':'')
    if (ImagePath.indexOf("rotation") != -1) {
      var startIndex = ImagePath.indexOf("rotation=") + 9;
      var endIndex = ImagePath.indexOf("&", startIndex);
      var rot = ImagePath.substring(startIndex, endIndex);
      ptzControlUrl += "&imagerotation=" + rot;
    }

    o += '<param name="PTZControlURL" value="' + ptzControlUrl + '">';
    if (minimap == "yes") {
      o += '<param name="PTZMapURL" value="/axis-cgi/jpg/image.cgi?camera=1&resolution=800x600&overlayimage=0&text=0&date=0&clock=0&rotation=0&mirror=0">';
        o += '<param name="UIMode" value="ptzmap">';
    }
  }

  o += '<br><b>' + ID + '</b>&nbsp;' + installText1 + '&nbsp;' + text + '&nbsp;' + installText2 + '&nbsp;' + installText3 + '<br>' + ID + '&nbsp;' + installText4;
  document.write(o + '<br><br></object>');

  Player = document.getElementById("Player");
  if (Player.ShowToolbar) { // This line is essential when installing AMC or else volume and mute buttons will show until reload
    if (ShowAMCToolbar == "yes") {
      if (extra == "yes") { // = 207
        Player.ToolbarConfiguration = "+play,+snapshot,+fullscreen,+mute,+volume";
      }
    }
  }
  var installedVersion = "0,0,0,0";
  var requiredVersion = "5,5,5,0";
  var tooOldVersion = false;
  try {
    installedVersion = Player.GetVersionPart(CLSID_AMC,0);
    installedVersion += ',' + Player.GetVersionPart(CLSID_AMC,1);
    installedVersion += ',' + Player.GetVersionPart(CLSID_AMC,2);
    installedVersion += ',' + Player.GetVersionPart(CLSID_AMC,3);
  }
  catch(e) {}
  instVerArray = installedVersion.split(",");
  verReqArray  = requiredVersion.split(",");
  for (i=0; i<instVerArray.length; i++) {
    if (parseInt(verReqArray[i], 10) > parseInt(instVerArray[i], 10)) {
      tooOldVersion = true;
      break;
    } else if (parseInt(verReqArray[i], 10) < parseInt(instVerArray[i], 10)) {
      break;
    }
  }
  if (!tooOldVersion) {
    Player.MaintainAspectRatio = ( maintainAspectRatio && maintainAspectRatio != "")?((maintainAspectRatio == "yes")?"true":"false"):"true";
  }

  if (h264 && (width * height > 3100000)) {
    Player.H264VideoRenderer=0x1000;
  }

  if (UseMotion == "yes") {
    Player.UIMode = "MDConfig";
    var motionConfURL = confHost + "/axis-cgi/operator/param_authenticate.cgi?ImageSource=" + [CamNbr - 1]
    var rot = 0;
    var mir = "no";
    if (ImagePath.indexOf("rotation") != -1) {
      var startIndex = ImagePath.indexOf("rotation=") + 9;
      var endIndex = ImagePath.indexOf("&", startIndex);
      rot = ImagePath.substring(startIndex, endIndex);
      motionConfURL += "&rotation=" + rot;
    }
    if (ImagePath.indexOf("mirror") != -1) {
      var startIndex = ImagePath.indexOf("mirror=") + 7;
      var endIndex = ImagePath.indexOf("&", startIndex);
      mir = ImagePath.substring(startIndex, endIndex);

      if( mir == "0" || mir == "1" )
        motionConfURL += "&mirror=" + mir;
    }
    Player.MotionConfigURL = motionConfURL
    Player.MotionDataURL = confHost + "/axis-cgi/motion/motiondata.cgi";
    Player.stretchToFit = true;
  }

  if (ShowSVG == "yes") {
    Player.SvgDataURL = confHost + "/axis-cgi/iv/stream.cgi?channel=9";
  }

  activateContextMenu()

  if ((enableAreaZoom == "yes") && (useWithCam == "yes")) {
    Player.EnableJoystick = "True";
    Player.EnableAreaZoom = "True";
  }

  if (extra == "recording") {
    Player.EnableReconnect = "false"; // Don't start playback again when the recording has reached the end.
    // Don't display "Video lost" popup baloon when the recording has reached the end.
    // Display login dialog when necessary
    Player.Popups = 1;
  }


}


function getHost(ImagePath)
{
  var host;
  var indexOfAddr;

  indexOfAddr = ImagePath.indexOf("://");
  if (indexOfAddr > 0) {
    host = ImagePath.substring(0,indexOfAddr + 3);
  } else {
    if (ImagePath.indexOf("videocodec=mpeg4") >= 0 || ImagePath.indexOf("videocodec=h264") >= 0) {
      host = "rtsp://" + getIPv6HostName(location.hostname);
    } else {
      host = getCurrentHostAndProt();
    }
  }
  return host;
}

function getCurrentHostAndProt()
{
  return location.protocol + "//" + getIPv6HostName(location.hostname) + (location.port > 0 ?':' + location.port : "");
}

function getIPv6HostName(hostname)
{
  if (hostname.split(":").length > 1)
    hostname = "[" + hostname + "]"
  return hostname;
}

function getInstalledInfo( CLSID_AMC )
{
  var version = "0,0,0,0";
  var hasAMC = false;
  try
  {
    version = document.Player.GetVersionPart(CLSID_AMC,0);
    version += ',' + document.Player.GetVersionPart(CLSID_AMC,1);
    version += ',' + document.Player.GetVersionPart(CLSID_AMC,2);
    version += ',' + document.Player.GetVersionPart(CLSID_AMC,3);
    hasAMC = true;
  }
  catch(e) 
  {
  }

  return {installedVersion:version, amcInstalled:hasAMC };
}

function compareVersions( installedVersion, requestedVersion )
{
  var isNewer = false;

  var instVerArray = installedVersion.split(",");
  var verReqArray  = requestedVersion.split(",");
  var len = instVerArray.length;

  for( var i=0; i<len; i++ )
  {
    if( parseInt(verReqArray[i], 10) > parseInt(instVerArray[i], 10) )
    {
      isNewer = true;
      break;
    } 
    else if( parseInt(verReqArray[i], 10) < parseInt(instVerArray[i], 10) )
    {
      break;
    }
  }

  return isNewer;
}

function checkAMCVersion( requiredVersion, CLSID_AMC )
{
  var versionInfo = getInstalledInfo( CLSID_AMC );
  var installedVersion = versionInfo.installedVersion;
  return !( compareVersions( installedVersion, requiredVersion ) );
}
