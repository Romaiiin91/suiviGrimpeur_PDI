//Global variable which contains all parameters fetched regarding Ir light
var ir;

var irNameSpaces;
var now = new Date();
var irWsdlUrl = "/wsdl/vapix/LightService.wsdl?timestamp=" + now.getTime();

//getLightParams - Function for fetching general parameters regarding Ir light
// action - Method to call (i.e. GetLightInformation)
// LightID - The id of the IR light to fetch or false if not present
// get_as - What string that the parameters are stored in ir
// get_from_arr - Array containing the properties needed to fetch the right parameter
// isAsynchronous - Set to true if asynchronous call
// callback - Function to call when the data is fetched and available
function getLightParams(action, LightID, get_as, get_from_arr, isAsynchronous, callback)
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

  var onresponse = new getLightParams_onresponse(get_as, get_from_arr, callback);

  return SOAPClient.invoke( irWsdlUrl, url, irNameSpaces, method, params, (isAsynchronous ? true : false), onresponse.handler );
}

//getLightParams_onresponse - class for response from getLightParams
function getLightParams_onresponse(get_as, get_from_arr, callback)
{
  this._get_as = get_as;
  this._get_from_arr = get_from_arr;
  this._callback = callback;

  // Get a static context. 'this' in a function changes meaning when passing the function around.
  var ctx = this;

  this.handler = function(responseObj)
  {
    if (responseObj) {
      if (responseObj.name == "Error") {
        var errorMsg = ((responseObj.description)?responseObj.description:(responseObj.fileName)?responseObj.fileName:responseObj.message);
        alert(errorMsg);
        return false;
      }
      var retval = responseObj;
      for (var i = 0; i < ctx._get_from_arr.length && retval; i++)
      {
        retval = retval[ctx._get_from_arr[i]];
      }
      ir[ctx._get_as] = retval;
      if (ctx._callback) {
        ctx._callback(ir);
      }
    }//if response

    return true;
  };
}

// Used when dynamically loading the setLightParam library
var setLightParam_dynload_args = undefined;

//setLightParam_dynload - function that dynamically loads the setLightParam
// library when needed. Will require reauthentication from the user if he/she
// is not an operator (ie. not allowed to read that file).
// NOTE: This function will work asynchronously, regardless of the value of the
// isAsynchronous parameter to setLightParam. If you need synchronous calling,
// load the setLightParam library yourself.
function setLightParam_dynload()
{
  setLightParam_dynload_args = arguments;
  var tag = document.createElement("script");
  tag.type="text/javascript";
  tag.src="/incl/ir_light_settings_set.js";
  tag.language="javascript";
  document.body.appendChild(tag);
}

// Make sure setLightParam is in global scope.
var setLightParam;

// Provide setLightParam using setLightParam_dynload if not already loaded.
if (typeof setLightParam == 'undefined') {
  setLightParam = setLightParam_dynload;
}

function initLightParams()
{
  ir = new Array();
  
  irNameSpaces = new SOAPClientNS();
  irNameSpaces.add("ali", "http://www.axis.com/vapix/ws/light" );
  irNameSpaces.add("wsnt", "http://docs.oasis-open.org/wsn/b-2" );
  irNameSpaces.add("tns1", "http://www.onvif.org/ver10/topics" );
}
