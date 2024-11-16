//
// Usage:
//   function setToolTip(eid, tipHTML, maxwidth)
//     eid:      The uniqe id of the element to which the tool tip is
//               associated.
//     tipHTML:  HTML code describing the tool tip.
//     maxwidth: Max width (pixles) of the tool tip bounding box (optional).
//     Returns "true" if successful, else "false".
//
//   var TOOLTIP_MAX_WIDTH
//     The max width of the tool tip bounding box, if no "maxwidth" value is
//     passed to setToolTop(). Default is 200.
//
//   var TOOLTIP_DELAY
//     Milliseconds to wait before displaying tool tip. Default is 500.
//
// Example:
//   setToolTip("volSlider", "<b>Volume</b><br>Drag to set volume");
//
// Requirements:
//   An empty  <div> or <span> with id="toolTip" and style="visibility: hidden"
//   must be available in the document body for use as the tool tip box.
//
// Note:
//   Event handlers "onmouseover" and "onmouseout" for the the element with
//   id=eid will be replaced by setToolTip().
//

var toolTips = new Array();

var TOOLTIP_STYLE = "position: absolute; left: 10px; top: 10px; border: black 1px solid; padding: 2px; background-color: lightyellow; font-family: verdana, arial, helvetica; font-size: 10px; font-weight: normal; z-index: 100;";

var TOOLTIP_STYLE2 = "position: absolute; left: 11px; top: 10px; border: black 1px solid; padding: 2px; background-color: lightyellow; font-family: verdana, arial, helvetica; font-size: 10px; font-weight: normal; z-index: 100; white-space:nowrap; overflow:visible";

var TOOLTIP_FOLLOWMOUSE = 1;

var debug;

function createToolTip(boxId, delay, defWidth, style, offX, offY, options)
{
  debug = document.getElementById("_tip_debug");

  if (!boxId || !style || isNaN(delay) || isNaN(defWidth) || !(delay >= 0) || isNaN(offX) || isNaN(offY)) {
    alert("missing options for createToolTip()");
    return false;
  }
  var box = document.getElementById(boxId);
  if (!box) {
//    alert("No tool tip element with id=\""+boxId+"\" found!");
    return false;
  }
  var css_sel = "#" + boxId;
  if (document.styleSheets[1].addRule)
    document.styleSheets[1].addRule(css_sel, style); // IE, Chrome
  else if (document.styleSheets[1].insertRule) {
    try {
      document.styleSheets[1].insertRule(css_sel+" { "+style+" }", 0); // DOM
    } catch (e){
      
    }
  }
    
  toolTips[toolTips.length] = new ToolTip(box, Number(delay), Number(defWidth), Number(offX), Number(offY), options, toolTips.length);
//alert("Created new tool tip box \""+boxId+"\"");
  return true;
}

function setToolTip(boxId, elementId, tipHTML, maxwidth, e)
{
  for (i=0; i < toolTips.length; i++) {
    if (toolTips[i].box.id == boxId) {
      //if (!maxwidth)
      //  maxwidth = toolTips[i].defWidth;
      if (!e) {
        e = document.getElementById(elementId);
        if (!e) {
          alert("No element with id=\""+elementId+"\" found!");
          return false;
        }
      }
      if (!toolTips[i].tips[elementId]) {
        var old_out = e.onmouseout ?  e.onmouseout : function() { };
        e.onmouseout = function(e) { closeTip(e, this), old_out(e) };
        if (toolTips[i].options != TOOLTIP_FOLLOWMOUSE) {
          var old_over = e.onmouseover ?  e.onmouseover : function() { };
          e.onmouseover = function(e) { trigTip(e, this), old_over(e) };
        } else {
          var old_move = e.onmousemove ?  e.onmousemove : function() { };
          e.onmousemove = function(e) { trigTip(e, this), old_move(e) };
        }
        if (debug) debug.innerHTML = "New tool tip for element \""+elementId+"\" using box \""+boxId+"\"";
      }
      toolTips[i].tips[elementId] = [tipHTML, maxwidth];
      if (toolTips[i].tipId == elementId) {
        setTip(toolTips[i], elementId);
      }
      return true;
    }
  }
//  alert("No tool tip box with id=\""+boxId+"\" found!");
  return false;
}

function updateToolTip(boxId, elementId, tipHTML, maxwidth)
{
//  alert("update:" + tipHTML);
  for (i=0; i < toolTips.length; i++) {
    if (toolTips[i].box.id == boxId) {
      toolTips[i].tips[elementId] = [tipHTML, maxwidth];
      if (toolTips[i].tipId == elementId) {
        toolTips[i].box.innerHTML = toolTips[i].tips[elementId][0];
      }
      return true;
    }
  }
}

function ToolTip(box, delay, defWidth, offX, offY, options, index)
{
  this.box = box;
  this.box.onmouseout=releaseTip;
  this.box.onclick=releaseTip;
  this.box.onmouseover=holdTip;
  this.box.innerHTML = ""
  this.delay = delay; 
  this.defWidth = defWidth;
  this.tips = new Array();
  this.hold = false;
  this.offX = offX;
  this.offY = offY;
  this.options = options; 
  this.timer = undefined;
  this.tipId = undefined;
  this.index = index;
}

function trigTip(e, obj)
{
  if (!e) e = window.event;
//  alert("delayTip e:"+e+" type:" +(e ? e.type : undefined)+" target:"+obj+" id:"+(obj ? obj.id : undefined));
  if (!obj.id) {
//    alert("No id of element "+obj+"!");
    return true;
  }
  for (i=0; i < toolTips.length; i++) {
    if (toolTips[i].tips[obj.id]) {
      if (toolTips[i].options == TOOLTIP_FOLLOWMOUSE) {
        // get mouse coord
        setMousePos(toolTips[i], e)
        if (toolTips[i].tipId == obj.id)
          displayTip(toolTips[i]);
      }
      if (toolTips[i].tipId != obj.id) {
//        alert("Delay tip for "+obj.id)
        if (debug) debug.innerHTML = "Delay tip for "+obj.id;
        if (toolTips[i].options != TOOLTIP_FOLLOWMOUSE) {
          setElementPos(toolTips[i], obj);
        }
        window.clearTimeout(toolTips[i].timer);
        var call = "setTip(toolTips[" +i+ "], \"" +obj.id+ "\")";
        toolTips[i].timer = window.setTimeout(call, toolTips[i].delay);
      }
      return true;
    }
  }
  alert("No tooltip for element "+obj.id+"!");
  return true;
}

function closeTip(e, obj)
{
  if (!obj.id) {
//    alert("No id of element "+obj+"!");
    return true;;
  }
  for (i=0; i < toolTips.length; i++) {
    if (toolTips[i].tips[obj.id]) {
//      alert("close box "+obj.id+", hold:"+toolTips[i].hold);
      if (!toolTips[i].hold) {
        window.clearTimeout(toolTips[i].timer);
        toolTips[i].tipId = undefined;
        toolTips[i].box.style.visibility='hidden';
      }
      return true; 
    }
  }
//  alert("No tooltip for element "+obj.id+"!");
  return true;
}

function holdTip()
{
  for (i=0; i < toolTips.length; i++)
  {
    if (toolTips[i].box.id == this.id)
    {
      toolTips[i].hold = true;
      toolTips[i].box.style.visibility = '';
//      alert("hold box "+this.id);
      return true;
    }
  }
//  alert("No tooltip for element "+this.id+"!");
  return false;
}

function releaseTip()
{
  for (i=0; i < toolTips.length; i++)
  {
    if (toolTips[i].box.id == this.id)
    {
      toolTips[i].hold = false;
      toolTips[i].tipId = undefined;
      toolTips[i].box.style.visibility = 'hidden';
//      alert("relese box "+this.id);
      return true;
    }
  }
//  alert("No tooltip for element "+this.id+"!");
  return false;
}

function setTip(toolTip, tipId)
{
//  alert("setTip");
  toolTip.box.innerHTML = toolTip.tips[tipId][0];
  toolTip.tipId = tipId;
  //toolTip.box.style.width = '';
  toolTip.box.style.left = '';
  toolTip.box.style.right = '';
  toolTip.box.style.top = '';
  toolTip.box.style.bottom = '';
  if (toolTip.tips[toolTip.tipId][1])
    toolTip.box.style.width = toolTip.tips[toolTip.tipId][1] +"px";
  toolTip.box.style.height = '';
  toolTip.box.style.visibility = '';
  displayTip(toolTip);
}

function displayTip(toolTip)
{
  var bound = getBounds();
  if (debug) debug.innerHTML = "bound:"+bound.x+","+bound.y+","+bound.w+","+bound.h;
  var tipPos = getPos(toolTip.box);

  if (!toolTip.tips[toolTip.tipId][1] && (tipPos.w > toolTips.defWidth))
  {
    // The box is wider than default width
    if (debug) debug.innerHTML += "<br>shrink "+tipPos.w+"-&gt;"+toolTip.tips[toolTip.tipId][1];
    toolTip.box.style.width = MAXWIDTH +"px";
    tipPos = getPos(toolTip.box);
  }
  if (debug) debug.innerHTML += "<br>tip:"+tipPos.x+","+tipPos.y+","+tipPos.w+","+tipPos.h;
  if (debug) debug.innerHTML += "<br>place at "+toolTip.posX+"("+toolTip.offX+"),"+toolTip.posY+"("+toolTip.offY+")";

  // Try placing box at specified offset
  if (toolTip.offX < 0)
  {
    // Place box to the left of position
    tipPos.x = toolTip.posX + toolTip.offX - tipPos.w;
    toolTip.box.style.left = tipPos.x +"px";
  }
  else if (toolTip.offX > 0)
  {
    // Place box to the right of position
    tipPos.x = toolTip.posX + toolTip.offX;
    toolTip.box.style.left = tipPos.x +"px";
  }
  else
  {
    // Place box centered on position
    tipPos.x = toolTip.posX - Math.floor(tipPos.w/2);
    toolTip.box.style.left = tipPos.x +"px";
  }
  tipPos = getPos(toolTip.box);
  if (toolTip.offY < 0)
  {
    // Place box above position
    tipPos.y = toolTip.posY + toolTip.offY - tipPos.h;
    toolTip.box.style.top = tipPos.y +"px";
  }
  else if (toolTip.offY > 0)
  {
    // Place box below position
    tipPos.y = toolTip.posY + toolTip.offY;
    toolTip.box.style.top = tipPos.y +"px";
  }
  else
  {
    // Place box centered on position
    tipPos.y = toolTip.posY - Math.floor(tipPos.h/2);
    toolTip.box.style.top = tipPos.y +"px";
  }
  tipPos = getPos(toolTip.box);
  if (debug) debug.innerHTML += "<br>tip:"+tipPos.x+","+tipPos.y+","+tipPos.w+","+tipPos.h;

  // Make sure box is not outside window boundaries.
  if (true)
  {
    if (tipPos.x < bound.x)
    {
      // Outside left boundry
      tipPos.x = bound.x;
      if (debug) debug.innerHTML += "<br>Move right to "+tipPos.x+"px";
    }
    if ((tipPos.x + tipPos.w) >= (bound.x + bound.w))
    {
      // Outside right boundry
      tipPos.x = (bound.x + bound.w) - tipPos.w;
      if (debug) debug.innerHTML += "<br>Move left to "+tipPos.x+"px";
    }
    toolTip.posX = tipPos.x;
    toolTip.box.style.left = toolTip.posX +'px';

    tipPos = getPos(toolTip.box);
    if (tipPos.y < bound.y)
    {
      // Outside top boundry
      tipPos.y = bound.y;
      if (debug) debug.innerHTML += "<br>Move down to "+tipPos.y+"px";
    }
    if ((tipPos.y + tipPos.h) > (bound.y + bound.h))
    {
      // Outside bottom boundry
      tipPos.y = (bound.y + bound.h) - tipPos.h;
      if (debug) debug.innerHTML += "<br>Move up to "+tipPos.y+"px";
    }
    toolTip.posY = tipPos.y;
    toolTip.box.style.top =  toolTip.posY +'px';
    if (debug) debug.innerHTML += "<br>tip:"+tipPos.x+","+tipPos.y+","+tipPos.w+","+tipPos.h;
  }
  //toolTip.timer = window.setTimeout("displayTip(toolTips["+toolTip.index+"])", 300);
}

function Box(x,y,w,h)
{
  this.x=x;
  this.y=y;
  this.w=w;
  this.h=h;
}


function getBounds()
{
  // www.quirksmode.org
  var box = new Box(0,0,0,0);
  if (document.body)
  {
    box.w = document.body.clientWidth;
    box.h = document.body.clientHeight;
  }
  else if (document.documentElement && !isNaN(document.documentElement.clientWidth))
  // Explorer 6 Strict Mode
  {
    box.w = document.documentElement.clientWidth;
    box.h = document.documentElement.clientHeight;
  }
  if (!isNaN(document.body.scrollLeft))
  // all other Explorers
  {
    if (document.body.scrollLeft > 0)
      box.x = document.body.scrollLeft;
    if (document.body.scrollTop > 0)
      box.y = document.body.scrollTop;
  }
  if (!isNaN(self.pageYOffset))
  // all except Explorer
  {
    if (self.pageXOffset > 0)
      box.x = self.pageXOffset;
    if (self.pageYOffset > 0)
      box.y = self.pageYOffset;
  }
  if (document.documentElement && !isNaN(document.documentElement.scrollLeft))
  // Explorer 6 Strict
  {
    if (document.documentElement.scrollLeft > 0)
      box.x = document.documentElement.scrollLeft;
    if (document.documentElement.scrollTop > 0)
      box.y = document.documentElement.scrollTop;
  }
  return box;
}

function getPos(obj)
{
  // www.quirksmode.org
  //alert("obj:"+obj);
  var box = new Box(0,0,0,0);
  box.w=obj.offsetWidth;
  box.h=obj.offsetHeight;
  if (obj.offsetParent)
  {
    while (obj.offsetParent)
    {
      box.x += obj.offsetLeft;
      box.y += obj.offsetTop;
      obj = obj.offsetParent;
    }
  }
  else if (!NaN(obj.x))
  {
    box.x += obj.x;
    box.y += obj.y;
  }
  return box;
}


// Set position to center of element
function setElementPos(toolTip, element)
{
  var ePos = getPos(element);
  toolTip.posX = ePos.x + Math.floor(ePos.w/2);
  toolTip.posY = ePos.y + Math.floor(ePos.h/2);
  //alert("setElementPos :"+toolTip.posX+","+toolTip.posY);
}


// set position to mouse
function setMousePos(toolTip, e)
{
  // www.quirksmode.org
  if (e.pageX && e.pageY)
  {
    toolTip.posX = e.pageX;
    toolTip.posY = e.pageY;
  }
  else if (e.clientX)
  {
    var bound = getBounds();
    toolTip.posX = (e.clientX + bound.x);
    toolTip.posY = (e.clientY + bound.y);
  }
  //alert("setMousePos :"+toolTip.posX+","+toolTip.posY);
}
