/*****************************************************************************\

 Javascript "SOAP Client" library

 @version: 2.4 - 2007.12.21
 @author: Matteo Casati - http://www.guru4.net/

\*****************************************************************************/

/*****************************************************************************\

  AXIS Modifications:

  SOAPClientParameters 
    - added empty method

  SOAPClientNS 
    - new object used for adding additional namespaces
  
  SOAPClient.invoke 
    - new parameters wsdl_url and namespaces
  
  SOAPClient._loadWsdl
    - new parameters wsdl_url and namespaces

  SOAPClient._onLoadWsdl
     - new parameters wsdl_url and namespaces

  SOAPClient._XMLResponse
    - handles mime type problem (we get the responses as text/plain not the correct text/xml )

  SOAPClient._sendSoapRequest
     - new parameter namespaces

  SOAPClient._node2object
    - modified to resolve attributes as well

  SOAPClient._extractValue
     - new parameter attr
     - modified to handle attributes
 

\*****************************************************************************/

function SOAPClientParameters()
{
	var _pl = new Array();
	this.add = function(name, value)
	{
		_pl[name] = value;
		return this;
	}
	this.toXml = function()
	{
		var xml = "";
		var v = "";
		for(var p in _pl)
		{
			switch( typeof(_pl[p]))
			{
				case "string":
				case "number":
				case "boolean":
				case "object":
					v = SOAPClientParameters._serialize(_pl[p])
					xml += "<" + p + v.a +">" + v.s + "</" + p + ">";
					break;
				default:
					break;
			}
		}
		
		if( xml.indexOf("<_>") != -1)
		{
		  xml = xml.replace("<_>", "");
		  xml = xml.replace("</_>", "");
		}

		return xml;
	}
	this.empty = function()
	{
		if( _pl.length > 0 )
			return true;
		return false;
	}

	this.get = function(name)
	{
		return _pl[name];
	}

}

function SOAPClientNS()
{
	var _ns = new Array();
	this.add = function(name, url)
	{
		_ns[name] = url;
		return this;
	}
	this.toXmlnsString = function()
	{
		var tmpArr = new Array();
		for(var n in _ns)
		{
			tmpArr[tmpArr.length]= "xmlns:" + n + "=\"" + _ns[n] + "\"";
		}
		return tmpArr.join(" ");
	}
	this.empty = function()
	{
		if( _ns.length > 0 )
			return true;
		return false;
	}
}

SOAPClientParameters._serialize = function(o)
{
	var s = "";
	var a = "";
        var type;
	var v;
	
	switch(getObjectType(o))
	{
	case "string":
		if( o.getIsAttribute() == true )
		{
			a += o.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
      a = a.replace(/"/g, "&quot;").replace(/'/g, "&apos;");
			a = a.setIsTheAttribute( true );
		}
		else
		{
			s += o.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
			if( o.getIsObjectNodeValue() == true )
			{
				s = s.setIsObjectNodeValue( true ); 
			}
		}
		break;
	case "number":
	case "boolean":
		if( o.getIsAttribute() == true )
		{
			a += o;
			a = a.setIsAttribute( true ); 
		}
		else
		{
			s += o;
			if( o.getIsObjectNodeValue() == true )
			{
				s = s.setIsObjectNodeValue( true ); 
			}
		}
		break;
	case "DateTime":
		// Date
		var year = o.getFullYear();
		var month = (o.getMonth() + 1); month = (month.length == 1) ? "0" + month : month;
		var date = o.getDate(); date = (date.length == 1) ? "0" + date : date;
		var hours = o.getHours(); hours = (hours.length == 1) ? "0" + hours : hours;
		var minutes = o.getMinutes(); minutes = (minutes.length == 1) ? "0" + minutes : minutes;
		var seconds = o.getSeconds(); seconds = (seconds.length == 1) ? "0" + seconds : seconds;
		var milliseconds = o.getMilliseconds();
		var tzminutes = Math.abs(o.getTimezoneOffset());
		var tzhours = 0;
		while(tzminutes >= 60)
		{
			tzhours++;
			tzminutes -= 60;
		}
		tzminutes = (tzminutes.length == 1) ? "0" + tzminutes : tzminutes;
		tzhours = (tzhours.length == 1) ? "0" + tzhours : tzhours;
		var timezone = ((o.getTimezoneOffset() < 0) ? "+" : "-") + tzhours + ":" + tzminutes;
		if( o.getIsAttribute() == true )
		{
			a += year + "-" + month + "-" + date + "T" + hours + ":" + minutes + ":" + seconds + "." + milliseconds + timezone;
		}
		else
		{
			s += year + "-" + month + "-" + date + "T" + hours + ":" + minutes + ":" + seconds + "." + milliseconds + timezone;
			if( o.getIsObjectNodeValue() == true )
			{
				s = s.setIsObjectNodeValue( true ); 
			}
		}
		break;
	case "array":
		// Array
		for(var p in o)
		{
			type = getObjectType( o[p] );
			if(!isNaN(p))   // linear array
			{
				if( o[p].getIsAttribute && o[p].getIsAttribute() == true )
				{
					v = SOAPClientParameters._serialize(o[p]);
					a += " " + type + "=\"" + v.a + "\"";
					a = a.setIsAttribute( true ); 
				}
				else
				{
					switch(type)
					{
						case "string":
						case "number":
						case "boolean":
						case "DateTime":
						case "Object":
						case "array":
							v = SOAPClientParameters._serialize(o[p])
							if( v.a.getIsAttribute() )
							{
								a += v.a;
							}
							else
							{
								s +=  v.s;
							}
						break;
						default:
							v = SOAPClientParameters._serialize(o[p])
							if( v.a.getIsAttribute() )
							{
								s += "<" + type + v.a + ">" + v.s + "</" + type + ">"
							}
							else if( v.s.getIsObjectNodeValue() )
							{
								s +=  v.s;
							}
							else
							{
								s += "<" + type + ">" + v.s + "</" + type + ">"
							}	
						break;
					}	
				}
			}
			else	// associative array
			{
				switch(type)
				{
					case "string":
					case "number":
					case "boolean":
					case "DateTime":
						v = SOAPClientParameters._serialize(o[p]);
						if( v.a.getIsAttribute() )
						{
							a += " " + p + "=\"" + v.a + "\""
							a = a.setIsAttribute( true );
						}
						else if( v.s.getIsObjectNodeValue() )
						{
							s +=  v.s;
						}
						else
						{
							s += "<" + p + ">" + v.s + "</" + p + ">"
						}
					break;
					default:
						v = SOAPClientParameters._serialize(o[p]);
						if( v.a.getIsAttribute() )
						{
							s += "<" + p + v.a + ">" + v.s + "</" + p + ">"
						}
						else
						{
							s += "<" + p + ">" + v.s + "</" + p + ">"
						}
					break;	
				}
			}
		}
	break;
	// Object or custom function
	default:
		for(var p in o)
		{
			v = SOAPClientParameters._serialize(o[p]);
			if( v.a.getIsAttribute() && v.s != "" )
			{
				s += "<" + p + v.a + ">" + v.s + "</" + p + ">";
			}
			else if( v.a.getIsAttribute() )
			{
				a += " " + p + "=\"" + v.a + "\"";
				a = a.setIsAttribute( true );
			}
			else if( v.s.getIsObjectNodeValue() )
			{
				s +=  v.s;
			}
			else
			{
				s += "<" + p + ">" + v.s + "</" + p + ">"
			}
		}
		break;
	}
	return { s:s, a:a };
}

function SOAPClient() {}

SOAPClient.username = null;
SOAPClient.password = null;

SOAPClient.invoke = function(wsdl_url, url, namespaces, method, parameters, async, callback)
{
	if(async)
		SOAPClient._loadWsdl(wsdl_url, url, namespaces, method, parameters, async, callback);
	else
		return SOAPClient._loadWsdl(wsdl_url, url, namespaces, method, parameters, async, callback);
}

// private: wsdl cache
SOAPClient_cacheWsdl = new Array();

// private: invoke async
SOAPClient._loadWsdl = function(wsdl_url, url, namespaces, method, parameters, async, callback)
{
	// load from cache?
	var wsdl = SOAPClient_cacheWsdl[wsdl_url];
	if(typeof(wsdl) != "undefined")
		return SOAPClient._sendSoapRequest(url, namespaces, method, parameters, async, callback, wsdl);
	// get wsdl
	var xmlHttp = SOAPClient._getXmlHttp();
	xmlHttp.open("GET", wsdl_url, async);

	if(async)
	{
		xmlHttp.onreadystatechange = function()
		{
			if(xmlHttp.readyState == 4)
				SOAPClient._onLoadWsdl(wsdl_url, url, namespaces, method, parameters, async, callback, xmlHttp);
		}
	}
	xmlHttp.send(null);
	if (!async)
		return SOAPClient._onLoadWsdl(wsdl_url, url, namespaces, method, parameters, async, callback, xmlHttp);
}
SOAPClient._onLoadWsdl = function(wsdl_url, url, namespaces, method, parameters, async, callback, req )
{
	var wsdl = SOAPClient._XMLResponse( req );
	SOAPClient_cacheWsdl[wsdl_url] = wsdl;	// save a copy in cache
	return SOAPClient._sendSoapRequest( url, namespaces, method, parameters, async, callback, wsdl);
}
SOAPClient._sendSoapRequest = function( url, namespaces, method, parameters, async, callback, wsdl)
{
	// get namespace
	var ns = (wsdl.documentElement.attributes["targetNamespace"] + "" == "undefined") ? wsdl.documentElement.attributes.getNamedItem("targetNamespace").nodeValue : wsdl.documentElement.attributes["targetNamespace"].value;

	// build SOAP request
	var sr =
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>" +
		"<soap:Envelope " +
		"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" " +
		"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" " +
                (( namespaces && !namespaces.empty() )?namespaces.toXmlnsString()+" ":"") +
		"xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\">" +
		"<soap:Body>" +
		(( parameters.empty() )?"<" + method + " xmlns=\"" + ns + "\"/>":
		"<" + method + " xmlns=\"" + ns + "\">" + parameters.toXml() + "</" + method + ">") +
		"</soap:Body></soap:Envelope>";
	// send request
	var xmlHttp = SOAPClient._getXmlHttp();
	if (SOAPClient.userName && SOAPClient.password){
		xmlHttp.open("POST", url, async, SOAPClient.userName, SOAPClient.password);
		// Some WS implementations (i.e. BEA WebLogic Server 10.0 JAX-WS) don't support Challenge/Response HTTP BASIC, so we send authorization headers in the first request
		xmlHttp.setRequestHeader("Authorization", "Basic " + SOAPClient._toBase64(SOAPClient.userName + ":" + SOAPClient.password));
	}
	else
		xmlHttp.open("POST", url, async);
	var soapaction = ((ns.lastIndexOf("/") != ns.length - 1) ? ns + "/" : ns) + method.substring( method.indexOf(":")+1, method.length);
	xmlHttp.setRequestHeader("SOAPAction", soapaction);
	xmlHttp.setRequestHeader("Content-Type", "text/xml; charset=utf-8");
	if(async)
	{
		xmlHttp.onreadystatechange = function()
		{
			if(xmlHttp.readyState == 4)
				SOAPClient._onSendSoapRequest(method, async, callback, wsdl, xmlHttp);
		}
	}
	xmlHttp.send(sr);
	if (!async)
		return SOAPClient._onSendSoapRequest(method, async, callback, wsdl, xmlHttp);
}

SOAPClient._onSendSoapRequest = function(method, async, callback, wsdl, req)
{
	var o = null;
	var method = method+"Response";

	var xmlDoc = SOAPClient._XMLResponse( req );
	var nd = SOAPClient._getElementsByTagName(xmlDoc, method); //FF, IE, Opera use "tev:XXXX"

	if(nd.length == 0)
	{
		//Seperate namespace and actual method
		method = method.substring( method.indexOf(":")+1, method.length);
		nd = SOAPClient._getElementsByTagName(xmlDoc, method);	// Safari and Google Chrome cant handle "tev:XXXX", only "XXXX"
	}

	if(nd.length == 0)
	{
		if(xmlDoc.getElementsByTagName("faultcode").length > 0)
		{
			var reasonText = xmlDoc.getElementsByTagName("faultstring")[0].childNodes[0].nodeValue;
			if( !reasonText && xmlDoc.getElementsByTagName("faultstring")[0].childNodes[0].textContent )
				reasonText = xmlDoc.getElementsByTagName("faultstring")[0].childNodes[0].textContent;

			if(async || callback)
				o = new Error(500, reasonText);
			else
				throw new Error(500, reasonText);
		}
		else if(xmlDoc.getElementsByTagName("fault").length > 0 || xmlDoc.getElementsByTagName("Fault").length > 0 )
		{
			var reasonText = xmlDoc.getElementsByTagName("Reason")[0].childNodes[0].nodeValue;
			if( !reasonText && xmlDoc.getElementsByTagName("Reason")[0].childNodes[0].textContent )
				reasonText = xmlDoc.getElementsByTagName("Reason")[0].childNodes[0].textContent;

			var detailNode = xmlDoc.getElementsByTagName("Detail")[0];
			if( detailNode )
			{
				var detailText = detailNode.childNodes[0].nodeValue;
				if( !detailText && detailNode.childNodes[0].textContent )
					detailText = detailNode.childNodes[0].textContent;
				
				//While solving Trouble #18953, it was found that chrome returned null
				//for errors instead of the correct message.
				if( !detailText && detailNode.childNodes[0].tagName )
					detailText = detailNode.childNodes[0].tagName;
				if( !detailText && detailNode.childNodes[0].nodeName )
					detailText = detailNode.childNodes[0].nodeName;

				if( detailText )
					reasonText += "\n"+detailText;
					
			  if(reasonText == "\n" || reasonText == "")
			    reasonText = detailNode.tagName;
			}

			if(async || callback)
			{
				o = new Error(500, reasonText);
				if( o.message == "" || o.message == "500" )
				{
					o.message = reasonText;
				}
			}
			else
				throw new Error(500, reasonText);
		}
		else if(xmlDoc.getElementsByTagName("SOAP-ENV:Fault").length > 0)
		{
			var reasonText = "";
			var reasonNode = xmlDoc.getElementsByTagName("SOAP-ENV:Reason")[0];
			do
			{
				if( reasonNode.childNodes[0].nodeValue )
					reasonText = reasonNode.childNodes[0].nodeValue;
				else
					reasonNode = reasonNode.childNodes[0];
			}
			while( reasonNode && reasonNode.hasChildNodes() && reasonText == "" )
			
			var detailNode = xmlDoc.getElementsByTagName("SOAP-ENV:Detail")[0];
			if( detailNode )
			{
				var detailText = "";
				do
				{
					if( detailNode.childNodes[0].nodeValue )
						detailText = detailNode.childNodes[0].nodeValue;
					else
						detailNode = detailNode.childNodes[0];
				}
				while( detailNode && detailNode.hasChildNodes() && detailText == "" )
				reasonText += "\n"+detailText;
				
				if(reasonText == "\n")
			    reasonText = detailNode.tagName;
			}
			if(async || callback)
				o = new Error(500, reasonText);
			else
				throw new Error(500, reasonText);
		}
	}
	else
	{
		o = SOAPClient._soapresult2object(nd[0], wsdl);
	}	
	if(callback)
		callback(o, xmlDoc);
	if(!async)
		return o;
}
SOAPClient._soapresult2object = function(node, wsdl)
{
	var wsdlTypes = SOAPClient._getTypesFromWsdl(wsdl);
	return SOAPClient._node2object(node, wsdlTypes);
}
SOAPClient._node2object = function(node, wsdlTypes)
{
	// null node
	if(node == null)
		return null;
	// text node
	if(node.nodeType == 3 || node.nodeType == 4)
		return SOAPClient._extractValue(node, wsdlTypes, false);
	// leaf node
	if (node.childNodes.length == 1 && node.attributes.length == 0 && (node.childNodes[0].nodeType == 3 || node.childNodes[0].nodeType == 4))
		return SOAPClient._node2object(node.childNodes[0], wsdlTypes);
	var isarray = SOAPClient._getTypeFromWsdl(node.nodeName, wsdlTypes).toLowerCase().indexOf("arrayof") != -1;
	// object node
	if(!isarray)
	{
		var obj = null;
		if(node.hasChildNodes() || ( node.attributes && node.attributes.length > 0 ) )
		{
                        var localName;
                        obj = new Object();
			
                        for(var i = 0; i < node.childNodes.length; i++)
			{
				var p = SOAPClient._node2object(node.childNodes[i], wsdlTypes);
                                localName = node.childNodes[i].nodeName;
                                localName = localName.substring( localName.indexOf(":")+1, localName.length);
                                if( obj[localName] )
                                {
                                    if( obj[localName].constructor != Array )
                                    {
                                      tmp = obj[localName];
                                      obj[localName] = new Array();
                                      obj[localName].push( tmp );
                                    }
                                    obj[localName].push(p);
                                }
                                else
                                {
                                  obj[localName] = p;
                                }
                        }
                        if( node.attributes && node.attributes.length > 0)
                        {
                          for(var i = 0; i < node.attributes.length; i++)
		          {
                                localName = node.attributes[i].nodeName;
                                localName = localName.substring( localName.indexOf(":")+1, localName.length);
				obj[localName] = SOAPClient._extractValue(node.attributes[i], wsdlTypes, true);
                          }
                        }
                        
		}
		return obj;
	}
	// list node
	else
	{
		// create node ref
		var l = new Array();
		for(var i = 0; i < node.childNodes.length; i++)
			l[l.length] = SOAPClient._node2object(node.childNodes[i], wsdlTypes);
		return l;
	}
	return null;
}
SOAPClient._extractValue = function(node, wsdlTypes, attr)
{
	var value = node.nodeValue;
	switch(SOAPClient._getTypeFromWsdl( ((attr)?node.nodeName:node.parentNode.nodeName), wsdlTypes).toLowerCase())
	{
		default:
		case "xs:string":
			return (value != null) ? value + "" : "";
		case "xs:boolean":
			return value + "" == "true";
		case "xs:int":
		case "xs:long":
			return (value != null) ? parseInt(value + "", 10) : 0;
		case "xs:double":
			return (value != null) ? parseFloat(value + "") : 0;
		case "xs:datetime":
			if(value == null)
				return null;
			else
			{
				value = value + "";
				value = value.substring(0, (value.lastIndexOf(".") == -1 ? value.length : value.lastIndexOf(".")));
				value = value.replace(/T/gi," ");
				value = value.replace(/-/gi,"/");
				var d = new Date();
				d.setTime(Date.parse(value));
				return d;
			}
	}
}
SOAPClient._getTypesFromWsdl = function(wsdl)
{
	var wsdlTypes = new Array();
	// IE, FF, Opera
	var ell = wsdl.getElementsByTagName("xs:element");
	var useNamedItem = true;
	// Safari, Chrome
	if(ell.length == 0)
	{
		ell = wsdl.getElementsByTagName("element");
		useNamedItem = false;
	}
	for(var i = 0; i < ell.length; i++)
	{
		if(useNamedItem)
		{
			if(ell[i].attributes.getNamedItem("name") != null && ell[i].attributes.getNamedItem("type") != null)
				wsdlTypes[ell[i].attributes.getNamedItem("name").nodeValue] = ell[i].attributes.getNamedItem("type").nodeValue;
		}
		else
		{
			if(ell[i].attributes["name"] != null && ell[i].attributes["type"] != null)
				wsdlTypes[ell[i].attributes["name"].value] = ell[i].attributes["type"].value;
		}
	}
	return wsdlTypes;
}

SOAPClient._getTypeFromWsdl = function(elementname, wsdlTypes)
{
	var type = wsdlTypes[elementname] + "";
	return (type == "undefined") ? "" : type;
}
// private: utils
SOAPClient._getElementsByTagName = function(doc, tagName)
{
	try
	{
		// trying to get node omitting any namespaces (latest versions of MSXML.XMLDocument)
		return doc.selectNodes(".//*[local-name()=\""+ tagName +"\"]");
	}
	catch (ex) {}
	// old XML parser support
	return doc.getElementsByTagName(tagName);
}
// private: xmlhttp factory
SOAPClient._getXmlHttp = function()
{
	try
	{
		if(window.XMLHttpRequest)
		{
			var req = new XMLHttpRequest();
			// some versions of Moz do not support the readyState property and the onreadystate event so we patch it!
			if(req.readyState == null)
			{
				req.readyState = 1;
				req.addEventListener("load",
									function()
									{
										req.readyState = 4;
										if(typeof req.onreadystatechange == "function")
											req.onreadystatechange();
									},
									false);
			}
			return req;
		}
		if(window.ActiveXObject)
			return new ActiveXObject(SOAPClient._getXmlHttpProgID());
	}
	catch (ex) {}
	throw new Error("Your browser does not support XmlHttp objects");
}
SOAPClient._getXmlHttpProgID = function()
{
	if(SOAPClient._getXmlHttpProgID.progid)
		return SOAPClient._getXmlHttpProgID.progid;
	var progids = ["Msxml2.XMLHTTP.5.0", "Msxml2.XMLHTTP.4.0", "MSXML2.XMLHTTP.3.0", "MSXML2.XMLHTTP", "Microsoft.XMLHTTP"];
	var o;
	for(var i = 0; i < progids.length; i++)
	{
		try
		{
			o = new ActiveXObject(progids[i]);
			return SOAPClient._getXmlHttpProgID.progid = progids[i];
		}
		catch (ex) {};
	}
	throw new Error("Could not find an installed XML parser");
}

SOAPClient._toBase64 = function(input)
{
	var keyStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
	var output = "";
	var chr1, chr2, chr3;
	var enc1, enc2, enc3, enc4;
	var i = 0;

	do {
		chr1 = input.charCodeAt(i++);
		chr2 = input.charCodeAt(i++);
		chr3 = input.charCodeAt(i++);

		enc1 = chr1 >> 2;
		enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
		enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
		enc4 = chr3 & 63;

		if (isNaN(chr2)) {
			enc3 = enc4 = 64;
		} else if (isNaN(chr3)) {
			enc4 = 64;
		}

		output = output + keyStr.charAt(enc1) + keyStr.charAt(enc2) +
		keyStr.charAt(enc3) + keyStr.charAt(enc4);
	} while (i < input.length);

	return output;
}

SOAPClient._XMLResponse = function( req )
{
  var xmlDoc = req.responseXML;
  if( req.responseXML == null || req.responseXML.xml == "")
  {
	if( String( req.responseText ).indexOf("<?xml") != -1 )
	{
		if (window.DOMParser)
	{
		var parser=new DOMParser();
		xmlDoc=parser.parseFromString(req.responseText,"text/xml");
	}
	else // Internet Explorer
	{
		xmlDoc=new ActiveXObject("Microsoft.XMLDOM");
		xmlDoc.loadXML(req.responseText);
	}
	}
  }
  return xmlDoc;
}

function getObjectType( o )
{
	var matches = o.constructor.toString().match(/function\s+(\w*)\s*\(/i);
	var type = ( ( matches && matches.length >= 2 )?matches[1]:"" );
	switch(type)
	{
		case "":
			type = typeof(o); break;
		case "String":
			type = "string"; break;
		case "Number":
			type = "number"; break;
		case "Boolean":
			type = "boolean"; break;
		case "Date":
			type = "DateTime"; break;
		case "Array":
			type = "array"; break;
	}
	return type;
}

String.prototype.getIsAttribute = function () 
{
	return( this.isAttribute ); 
};

String.prototype.setIsAttribute = function (isAttribute)
{
	this.isAttribute = isAttribute; 
	return this;
};

//The function below is used as a fix for the ticket #57179. Safari 7 has some problems when
//using the same name of the function with variables of different type as a string wasn't
//converted to the string object when using setIsAttribute() function
String.prototype.setIsTheAttribute = function (isAttribute)
{
	this.isAttribute = isAttribute; 
	return this;
};

String.prototype.getIsObjectNodeValue = function () 
{
	return( this.isObjectNodeValue ); 
};

String.prototype.setIsObjectNodeValue = function (isObjectNodeValue)
{
	this.isObjectNodeValue = isObjectNodeValue; 
	return this;
};

Number.prototype.getIsAttribute = function () 
{
	return( this.isAttribute );
};

Number.prototype.setIsAttribute = function (isAttribute)
{
	this.isAttribute = isAttribute; 
	return this;
};

Number.prototype.getIsObjectNodeValue = function () 
{
	return( this.isObjectNodeValue ); 
};

Number.prototype.setIsObjectNodeValue = function (isObjectNodeValue)
{
	this.isObjectNodeValue = isObjectNodeValue; 
	return this;
};

Boolean.prototype.getIsAttribute = function () 
{
	return( this.isAttribute ); 
};

Boolean.prototype.setIsAttribute = function (isAttribute)
{
	this.isAttribute = isAttribute; 
	return this;
};

Boolean.prototype.getIsObjectNodeValue = function () 
{
	return( this.isObjectNodeValue ); 
};

Boolean.prototype.setIsObjectNodeValue = function (isObjectNodeValue)
{
	this.isObjectNodeValue = isObjectNodeValue; 
	return this;
};

Date.prototype.getIsAttribute = function () 
{
	return( this.isAttribute ); 
};

Date.prototype.setIsAttribute = function (isAttribute)
{
	this.isAttribute = isAttribute; 
	return this;
};

Date.prototype.getIsObjectNodeValue = function () 
{
	return( this.isObjectNodeValue ); 
};

Date.prototype.setIsObjectNodeValue = function (isObjectNodeValue)
{
	this.isObjectNodeValue = isObjectNodeValue; 
	return this;
};
