/**
* Class for making ajax-calls cross-browser.
* Could be extended to handle other calls, such as WebSockets.
*
* Example usage:
*   var myAjax = new AxisConnection();
*   myAjax.open("GET", "/language/default/language_basic.xml?timestamp="+timestamp, false);
*   myAjax.send("");
*   var result = myAjax.getResponseXml();
*
* Alt.
*   var myXhr = AxisConnection.createRequest();
*   var myAjax = new AxisConnection();
*   myAjax.open(["GET", "/language/default/language_basic.xml?timestamp="+timestamp, false], myXhr);
*   myAjax.send("", myXhr);
*   var result = myAjax.getResponseXml(myXhr);
*
* Note, XMLHttpRequest/ActiveXObject is stored internally when creating an instance of AxisConnection with "new".
* Example:
*   var myAjax = new AxisConnection();
*   myAjax.ajax;
*
* @class AxisConnection
* @constructor
*
* @param arguments[0] Specifies which index to use in order to create a specific msxml version, for use when creating ActiveXObject
* @type {number}
* @optional
*/
function AxisConnection () {
  if (typeof arguments[0] === "number" && (arguments[0] >= 0 && arguments[0] < AxisConnection.msXml.types.length))
    this.client.supports.msXmlVersion = arguments[0];

  this.init();
}

/**
* Stores Microsoft XML document- and XMLHttp types and version numbers
* Used to determine what version is supported by the user's client.
* Also used for the occasions when a specific MSXML version needs to be set.
*
* Important - If types are changed, please remember to update version number properties (v6, v3 etc.)
*
* Note, this construct exists because properties in object literals are not guaranteed to be in consistent order.
*
* @property msXml
* @type {Object}
*
* @property v6 Holds array index for MSXML Version 6.0
* @type {number} 
*
* @property v3 Holds array index for MSXML Version 3.0
* @type {number} 
*
* @property v2 Holds array index for MSXML Version 2
* @type {number} 
*/
AxisConnection.msXml = {
  types: [
    { http: "MSXML2.XmlHttp.6.0", dom: "MSXML2.DOMDocument.6.0" },
    { http: "MSXML2.XmlHttp.3.0", dom: "MSXML2.DOMDocument.3.0" },
    { http: "MSXML2.XmlHttp", dom: "MSXML2.DOMDocument" }
  ],
  v6: 0,
  v3: 1,
  v2: 2
}

/**
* Contains various log levels. Used in conjunction with logStatus.
*
* @property logLevel
* @return {string} log level
*/
AxisConnection.logLevel = {
  none: "none",
  trace: "trace",
  debug: "debug",
  info: "info",
  warn: "warn",
  error: "error",
  fatal: "fatal"
}

/**
* Determines what logStatus this instance of AxisConnection has.
* Used by print-function to determine how to output userinfo on the client.
* To change logStatus, use logLevel, e.g.
*   AxisConnection.logStatus = AxisConnection.logLevel.debug;
* Reload page after change.
*
* @property logStatus Current log status.
* @default logLevel.none
* @return {string} log level
*/
AxisConnection.logStatus = AxisConnection.logLevel.none;

/**
* Helper object
* Among other things, stores information about what technologies the user's client supports, print-function
* Can be extended to supply more information about supported client technologies and more.
*
* @property client
*
* @property client.supports.activeX 
* @return {Boolean} Returns true if ActiveXObject is supported
*
* @property msXmlVersion
* @default null
* @return {number} Returns the number in the array: "msXml", of what http and dom-type the client supports. E.g. AxisConnection.msXml.types[this.client.supports.msXmlVersion].http
* 
* @property client.supports.xmlHttp 
* @return {Boolean} Returns true if XMLHttpRequest is supported
*
* @property client.supports.webSocket 
* @return {Boolean} Returns true if WebSocket is supported
*
* @property client.print Prints information in console or in alert-popup, depending on value of AxisConnection.logStatus. For use when debugging.
*/
AxisConnection.prototype.client = {
  supports: {
    activeX: typeof ActiveXObject != "undefined",
    msXmlVersion: null,
    xmlHttp: typeof XMLHttpRequest != "undefined",
    webSocket: typeof WebSocket != "undefined" //for future implementation purposes
  },
  print: 
    (AxisConnection.logStatus != AxisConnection.logLevel.none && console && typeof console.log === "function") ? 
      function () { console.log(arguments[0]); } : 
        function () { window.alert(arguments[0]); }
}


/**
* Stores the XMLHttpRequest or ActiveXObject (depending on what the client supports), for internal use.
*
* @property ajax
* @type {XMLHttpRequest|ActiveXObject}
* @default null
*/
AxisConnection.prototype.ajax = null;

/**
* Stores the result (if any) from the latest ajax send()-call.
*
* @property sendResult
* @type {mixed}
* @default null
*/
AxisConnection.prototype.sendResult = null;

/**
* Helper-function, used to determine if a variable is of type Array.
* 
* @method isArray
* @param {Object} _object An object to test
* @return {Boolean} Returns true on success
*/
AxisConnection.isArray = function (_object) {
  if (Array.isArray)
    return Array.isArray(_object);
  else
    return Object.prototype.toString.call(_object) === "[object Array]";
}

/**
* Returns the name of the current function.
* Used to refer to the function where the error occurred.
*
* Note, this method will only work properly for named functions.
*   e.g. AxisConnection.myNamedFunction = function myNamedFunction () { ... }
* 
* @method getFunctionName
* @param {Function} arguments[0] Argument 1
* @return {String} Returns "function" followed by the function-name
*/
AxisConnection.getFunctionName = function () {
  return arguments[0].callee.toString().match(/function\s+([^\s\(]+)/)[0];
}

/**
* Returns the name of the class and the current function.
* Used in Error-messages to refer to the parent class and the function name where the error occurred.
*
* Note, this method will only work for class object created with keyword new.
*   e.g. var myAjax = new AxisConnection();
* 
* @method getErrorOrigin
* @param {Function} arguments[0] Argument 1
* @return {String} Returns concatenated String with parent class "function" followed by the function-name
*/
AxisConnection.prototype.getErrorOrigin = function () {
  var errorOrigin =  ". Occurs in " + this.constructor.name + ", " + AxisConnection.getFunctionName(arguments[0]);
  return errorOrigin;
}

/**
* Initializes the class.
* Stores the ajax object internally.
* 
* @method init
* @return {XMLHttpRequest|ActiveXObject} The ajax object which to perform ajax operations on
*/
AxisConnection.prototype.init = function init() {
  //Note, order is important to facilitate functionality for IE10
  if (this.client.supports.activeX) {
    this.ajax = this.createAxo();
    return this.ajax;
  }
  else if (this.client.supports.xmlHttp) {
    this.ajax = this.createXhr();
    return this.ajax;
  }
  else {
    throw new Error("Your client doesn't support an XML HTTP Request" + this.getErrorOrigin(arguments));
  }
}

/**
* Creates an ajax object.
* Used if you simply want the ajax object (not the AxisConnection-object).
* 
* @method createRequest
* @return {XMLHttpRequest|ActiveXObject} Ajax object
*/
AxisConnection.createRequest = function () {
  var axisCon = new AxisConnection();
  return axisCon.ajax;
}

/**
* Creates an ActiveXObject object.
*
* @method createAxo
* @return {ActiveXObject}
*/
AxisConnection.prototype.createAxo = function createAxo() {
  var axo = null;
  for (var i = this.client.supports.msXmlVersion || 0; i < AxisConnection.msXml.types.length; i++) {
    try { //Determine which version of activeXObject to use
      axo = new ActiveXObject(AxisConnection.msXml.types[i].http);
      this.client.supports.msXmlVersion = i;
      break;
    }
    catch(e) {}
  }
  if (!axo)
    throw new Error("Could not create ActiveXObject" + this.getErrorOrigin(arguments)); //Perhaps incorrect msxml http-types?

  return axo;
}

/**
* Creates an XMLHttpRequest object.
*
* @method createXhr
* @return {XMLHttpRequest}
*/
AxisConnection.prototype.createXhr = function createXhr() {
  var xhr = new XMLHttpRequest();

  if (!xhr)
      throw new Error("Could not create XMLHttpRequest" + this.getErrorOrigin(arguments));

  return xhr;
}

/**
* Invokes the open-method on the ajax object.
* 
* @method open
* @param {Array} arguments[0] Array with open-parameters
* @param {ActiveXObject|XMLHttpRequest} arguments[1] The ajax-object
*
* Or
*
* @param {Array} arguments[0] Array with open-parameters
*
* Or
*
* @param {String} arguments[0] method
* @param {String} arguments[1] url
* @param {String} arguments[2] async
* @optional
*
* @param {String} arguments[3] username
* @optional
*
* @param {String} arguments[4] password
* @optional
*/
AxisConnection.prototype.open = function open() {
  var ajax, openParams = [];

  // e.g. myAxisCon.open( [<method>,<url>,<async>...], ajax )
  if (arguments.length === 2 && AxisConnection.isArray(arguments[0]) && arguments[1]) {
    openParams = arguments[0];
    ajax = arguments[1];
  }
  // e.g. myAxisCon.open( [<method>,<url>,<async>...] )
  else if (arguments.length === 1 && AxisConnection.isArray(arguments[0]) && this.ajax) {
    openParams = Array.prototype.slice.call(arguments[0]);
    ajax = this.ajax;   
  }
  // e.g. myAxisCon.open( <method>,<url>... ) 
  else if (arguments.length >= 2 && !AxisConnection.isArray(arguments[0]) && this.ajax) {
    openParams = Array.prototype.slice.call(arguments);
    ajax = this.ajax;
  }
  else {
    throw new Error("Incorrect parameter(s)" + this.getErrorOrigin(arguments));
  }
  
  switch (openParams.length) {
    case 2:
      //method, url
      ajax.open(openParams[0], openParams[1]);
      break;
    case 3:
      //method, url, async
      ajax.open(openParams[0], openParams[1], openParams[2]); 
      break;
    case 4:
      //method, url, async, username
      ajax.open(openParams[0], openParams[1], openParams[2], openParams[3]);
      break;
    case 5:
      //method, url, async, username, password
      ajax.open(openParams[0], openParams[1], openParams[2], openParams[3], openParams[4]);
      break;
    default:
  }
}

/**
* Invokes the send-method on the ajax object.
* 
* @method send
* @param arguments[0] send-method parameter
* @param {ActiveXObject|XMLHttpRequest} arguments[1] ajax-object
* @optional
* @return {mixed} returns result (if any) from the send-call
*/
AxisConnection.prototype.send = function send() {
  var sendParam = arguments[0];
  var ajax = arguments[1] || this.ajax;

  if (!ajax)
    throw new Error("Incorrect parameter(s)" + this.getErrorOrigin(arguments));

  try {
    this.sendResult = ajax.send(sendParam);
    return this.sendResult || null;
  }
  catch (e) { delete e; }
}

/**
* Returns the responseXML-object.
* 
* @method getResponseXml
* @param {ActiveXObject|XMLHttpRequest} arguments[0] ajax-object
* @optional
* @return {Object} returns result from the responseXML-property
*/
AxisConnection.prototype.getResponseXml = function getResponseXml() {
  var ajax = arguments[0] || this.ajax;
  if (!ajax)
    throw new Error("Incorrect parameter(s)" + this.getErrorOrigin(arguments));

  if (this.client.supports.activeX) {
    return this.getMsXmlDoc(ajax);
  } 
  else if (this.client.supports.xmlHttp) {
    return ajax.responseXML;
  }
}

/**
* Returns the responseXML-object for IE.
* Used internally and should not be nessessary to call manually.
* 
* @method getMsXmlDoc
* @param {ActiveXObject|XMLHttpRequest} arguments[0] ajax-object
* @optional
* @return {responseXML} returns result from the ajax-call
*/
AxisConnection.prototype.getMsXmlDoc = function getMsXmlDoc() {
  var ajax = arguments[0] || this.ajax;
  var msXml = AxisConnection.msXml;
  var msXmlVersion = this.client.supports.msXmlVersion;
  var msXmlDoc = null;
  if (ajax && msXml && typeof msXmlVersion === "number" && msXmlVersion >= 0) {
    if (ajax.responseXML && (typeof ajax.responseXML.selectNode == "undefined" || typeof ajax.responseXML.selectSingleNode == "undefined") ) {
      try { //create MS XML Doc
        msXmlDoc = new ActiveXObject(msXml.types[msXmlVersion].dom);
        var xmlString = ajax.responseXML.xml ? ajax.responseXML.xml : ajax.responseText;
        msXmlDoc.loadXML(xmlString);
      }
      catch (e) { throw new Error("Could not create Microsoft XML Document" + this.getErrorOrigin(arguments)); }

      if (msXmlDoc.parseError.errorCode != 0)
        throw new Error("Parsing error: " + msXmlDoc.parseError.reason + this.getErrorOrigin(arguments));
    }
    else {
      throw new Error("responseXML is missing necessary functions" + this.getErrorOrigin(arguments)); //perhaps wrong msxml doctype?
    }
    return msXmlDoc;
  }
  else
    throw new Error("Incorrect parameter(s)" + this.getErrorOrigin(arguments));
}

/**
* Returns the responseText-object of the ajax-object.
* 
* @method getResponseText
* @param {ActiveXObject|XMLHttpRequest} arguments[0] ajax-object
* @optional
* @return {String} responseText from the ajax-call
*/
AxisConnection.prototype.getResponseText = function getResponseText() {
  var ajax = arguments[0] || this.ajax;
  if (!ajax)
    throw new Error("No ajax object" + this.getErrorOrigin(arguments));

  return ajax.responseText;
}

/**
* Returns the response-object of the ajax-object.
* 
* @method getResponse
* @param {ActiveXObject|XMLHttpRequest } arguments[0] ajax-object
* @optional
* @return {response} response from the ajax-call
*/
AxisConnection.prototype.getResponse = function getResponse() {
  var ajax = arguments[0] || this.ajax;
  if (!ajax)
    throw new Error("No ajax object" + this.getErrorOrigin(arguments));

  return ajax.response;
}

/**
* Invokes the getAllResponseHeaders-method of the ajax-object.
* 
* @method getAllResponseHeaders
* @param {ActiveXObject|XMLHttpRequest } arguments[0] ajax-object
* @optional
* @return {response} result
*/
AxisConnection.prototype.getAllResponseHeaders = function getAllResponseHeaders() {
  var ajax = arguments[0] || this.ajax;
  if (!ajax)
    throw new Error("No ajax object" + this.getErrorOrigin(arguments));

  return ajax.getAllResponseHeaders();
}

/**
* Returns readyState of the ajax-object.
* 
* @method getReadyState
* @param {ActiveXObject|XMLHttpRequest } arguments[0] ajax-object
* @optional
* @return {number} readystate from the ajax-call
*/
AxisConnection.prototype.getReadyState = function getReadyState() {
  var ajax = arguments[0] || this.ajax;
  if (!ajax)
    throw new Error("No ajax object" + this.getErrorOrigin(arguments));

  return ajax.readyState;
}

/**
* Sets the supplied function to onreadystatechange-event, of the ajax-object.
* 
* @method setOnreadystatechange
* @param {function} arguments[0] function/handler
* @param {ActiveXObject|XMLHttpRequest } arguments[1] ajax-object
* @optional
*/
AxisConnection.prototype.setOnreadystatechange = function setOnreadystatechange() {
  var func = arguments[0];
  var ajax = arguments[1] || this.ajax;
  if (!func || !ajax)
    throw new Error("Incorrect parameter(s)" + this.getErrorOrigin(arguments));

  try {
    ajax.onreadystatechange = func;
  } 
  catch (e) {}
}

/**
* Returns HTTP status code.
* 
* @method getStatus
* @param {ActiveXObject|XMLHttpRequest } arguments[0] ajax-object
* @optional
* @return {number} status from the ajax-call
*/
AxisConnection.prototype.getStatus = function getStatus() {
  var ajax = arguments[0] || this.ajax;
  if (!ajax)
    throw new Error("No ajax object" + this.getErrorOrigin(arguments));

  return ajax.status;
}

/**
* Returns the value of the statusText property, of the ajax-object.
* 
* @method getStatusText
* @param {ActiveXObject|XMLHttpRequest } arguments[0] ajax-object 
* @optional
* @return {number} statusText from the ajax-call
*/
AxisConnection.prototype.getStatusText = function getStatusText() {
  var ajax = arguments[0] || this.ajax;
  if (!ajax)
    throw new Error("No ajax object" + this.getErrorOrigin(arguments));

  return ajax.statusText;
}


/**
* Invokes the abort metod, of the ajax-object.
* 
* @method abort
* @param {ActiveXObject|XMLHttpRequest } arguments[0] ajax-object 
* @optional
*/
AxisConnection.prototype.abort = function abort() {
  var ajax = arguments[0] || this.ajax;
  if (!ajax)
    throw new Error("No ajax object" + this.getErrorOrigin(arguments));

  ajax.abort();
}

/**
* Sets the overrideMimeType of the ajax-object, with the supplied mimeType.
* 
* @method setOverrideMimeType
* @param {string} arguments[0] Mime type
* @param {ActiveXObject|XMLHttpRequest } arguments[1] ajax-object 
* @optional
* @return {Boolean} Returns true if overrideMimeType was successfully set, otherwise false
*/
AxisConnection.prototype.setOverrideMimeType = function setOverrideMimeType() {
  var mimeType = arguments[0];
  var ajax = arguments[1] || this.ajax;
  if (!mimeType || !ajax)
    throw new Error("Incorrect parameter(s)" + this.getErrorOrigin(arguments));

  if (typeof ajax.overrideMimeType === "function") {
    ajax.overrideMimeType(mimeType);
    return true;
  }
  else {
    return false;
  }
}

/**
* Factory class for creating ajax-objects.
* Useful if the implementation of ajax-objects and -handling changes in the future.
*
* Example usage:
*   var myAjaxConnection = AxisConnectionFactory.createAjaxConnection();
*   myAjaxConnection.open("GET", "/language/default/language_basic.xml?timestamp="+timestamp, false);
*   myAjaxConnection.send("");
*   var myResponseXml = myAjaxConnection.getResponseXml();
*
* Alt 1.
*   var myResponseXml = AxisConnectionFactory.getResponseXmlWith(["GET", "/language/default/language_basic.xml?timestamp="+timestamp, false], null);

* Alt 2.
*   var myAjaxConnection = AxisConnectionFactory.createAjaxConnection();
*   var myResponseText = AxisConnectionFactory.getResponseTextWith(["GET", "/language/default/language_basic.xml?timestamp="+timestamp, false], null, myAjaxConnection);
*
* Alt 3.
*   var myAjaxObject = AxisConnectionFactory.createZXmlHttpRequest();
*   Note, requires the zXml lib to be loaded beforehand.
*
* @class AxisConnectionFactory
* @method createAjaxConnection 
* @return {AxisConnection} The AxisConnection-object
*
* @method getResponseXmlWith
* @param {Array} arguments[0] open-parameters
* @param arguments[1] send-parameter
* @param {AxisConnection} arguments[2] AxisConnection on which to perform ajax-operations on
* @optional
* @return {responseXML} The responseXML-object
*
* @method getResponseTextWith
* @param {Array} arguments[0] open-parameters
* @param arguments[1] send-parameter
* @param {AxisConnection} arguments[2] AxisConnection on which to perform ajax-operations on
* @optional
* @return {responseText} The responseText value
*
* @method sendAsync
* @param {Array} arguments[0] open-parameters
* @param arguments[1] send-parameter
* @param arguments[2] callback function to attach to onreadystatechange-event or null if no callback function.
* @param {AxisConnection} arguments[3] AxisConnection on which to perform ajax-operations on
* @optional
*
* @method sendSync
* @param {Array} arguments[0] open-parameters
* @param arguments[1] send-parameter
* @param {AxisConnection} arguments[2] AxisConnection on which to perform ajax-operations on
* @optional
*
* @method createZXmlHttpRequest
* @return {ActiveXObject|XMLHttpRequest } The ajax-object
*/
var AxisConnectionFactory = {
  createAjaxConnection: function () {
    var msXmlTypeVersion = arguments[0];

    if (msXmlTypeVersion)
      return new AxisConnection(msXmlTypeVersion);
    else
      return new AxisConnection();
  },
  getResponseXmlWith: function () {
    var openParams = arguments[0];
    var sendParams = arguments[1];
    var axisCon = arguments[2] || new AxisConnection();
    if (!openParams || !axisCon)
      throw new Error("Parameter-fault in getResponseXmlWith()");

    openParams[2] = false;
    axisCon.open(openParams);
    axisCon.send(sendParams);
    if (axisCon.getStatus() == 200) {
      return axisCon.getResponseXml();
    }
    else {
      return null;
    }

    if (!arguments[2])
      delete axisCon;    
  },
  getResponseTextWith: function () {
    var openParams = arguments[0];
    var sendParams = arguments[1];
    var axisCon = arguments[2] || new AxisConnection();
    if (!openParams || !axisCon)
      throw new Error("Parameter-fault in getResponseTextWith()");

    openParams[2] = false;
    axisCon.open(openParams);
    axisCon.send(sendParams);
    if (axisCon.getStatus() == 200) {
      return axisCon.getResponseText();
    }
    else {
      return null;
    }

    if (!arguments[2])
      delete axisCon;
  },
  sendAsync: function () {
    var openParams = arguments[0];
    var sendParams = arguments[1];
    var callback = arguments[2] || null;
    var axisCon = arguments[3] || new AxisConnection();
    if (!openParams || !axisCon)
      throw new Error("Parameter-fault in sendAsync()");

    openParams[2] = true;
    axisCon.open(openParams);

    if (callback)
      axisCon.setOnreadystatechange(callback);

    axisCon.send(sendParams);

    if (!arguments[3])
      delete axisCon;
  },
  sendSync: function () {
    var openParams = arguments[0];
    var sendParams = arguments[1];
    var axisCon = arguments[2] || new AxisConnection();
    if (!openParams || !axisCon)
      throw new Error("Parameter-fault in sendSync()");

    openParams[2] = false;
    axisCon.open(openParams);
    axisCon.send(sendParams);

    if (!arguments[2])
      delete axisCon;
  },
  createZXmlHttpRequest: function () {
    if (zXmlHttp && zXmlHttp.createRequest) {
      return zXmlHttp.createRequest();
    }
    else {
      throw new Error("Could not create zXmlHttpRequest-object"); //Perhaps missing dependecy
    }
  }
