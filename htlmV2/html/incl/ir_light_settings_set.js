//setLightParams - Function for setting general parameters regarding Ir light
// action - Method to call (i.e. SetManualIntensity)
// LightID - The id of the IR light to set or false if not present
// param - What parameter to set
// value - Value of parameter to set
// isAsynchronous - Set to true if asynchronous call
function setLightParam(action, LightID, param, value, isAsynchronous)
{
  var now = new Date();
  var timestamp = "?timestamp="+now.getTime();

  var url = "/vapix/services"+timestamp;
  var method = "ali:" + action;

  if( !irNameSpaces )
    initLightParams();

  var params = new SOAPClientParameters();
  if( LightID )
    params.add( "LightID", LightID );
  if( param && value )
    params.add( param, value);

  return SOAPClient.invoke( irWsdlUrl, url, irNameSpaces, method, params, (isAsynchronous ? true : false), setLightParam_onresponse );
}

//setLightParam_onresponse - Function called with response from setLightParam
function setLightParam_onresponse(responseObj)
{
  if (responseObj) {
    if (responseObj.name == "Error") {
      var errorMsg = ((responseObj.description)?responseObj.description:(responseObj.fileName)?responseObj.fileName:responseObj.message);
      alert(errorMsg);
      return false;
    }
  }//if response

  return true;
}

// If the file has been dynamically loaded, run it with the provided arguments.
if (typeof setLightParam_dynload_args != 'undefined') {
  setLightParam.apply(this, setLightParam_dynload_args);
}
