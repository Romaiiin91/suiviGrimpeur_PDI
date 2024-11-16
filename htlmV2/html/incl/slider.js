///////////////////////////////////////////////////////////////////////////////
// To use a slider, some elements with specific id's are required.
//
//  Example:
//  <!-- "SliderGroup" is the "common" id which is used for all elements which are expected -->
//  <img id="SliderGroup-left">		<!-- Left arrow / decrease - horizontal -->
//  <img id="SliderGroup-right">	<!-- Right arrow / increase - horizontal -->
//  <img id="SliderGroup-down">		<!-- Down arrow / decrease - vertical -->
//  <img id="SliderGroup-up">		<!-- Up arrow / increase - vertical -->
//  <img id="SliderGroup-bg">		<!-- Background -->
//  <img id="SliderGroup-handle">	<!-- The handle -->
//  isValidText()						<!-- A function for validation of text entered -->
//
//  <!-- Global variable -->
//  SliderGroupSlider = new Slider("SliderGroup", LowValue, HighValue, CurrentValue, ValidatioFunction, Orientation, Enabled);
//
// The function image_preview_onChange() is called on change if it exists.
// If a slider change event callback is set (with the setSliderOnChange method)
// then it is called when the slider changes. If the callback does not exist
// but a function slider_onChange("SliderGroup") do, then is is called.
//
// ----------------------------------------------------------------------------
//
// Some usable functions
// ---------------------
// moveTo() - Sync slider with value in text field.
// moveX(X) - Horisontal slider, move slider X steps.
// moveY(Y) - Vertical slider, move slider Y steps.
// boundary(value) - Get value inside slider boundary.
// enable() - Enable the slider.
// disable() - Disable the slider.
// setActual(value) - Shows a marker on the slider indicating an "actual" value.
// removeActual() - Hides the marker on the slider indicating an "actual" value.
// update(min, max, value) - Updates limits and current value.
// refresh() - Refresh slider (moves handle to current value).
// setSliderOnChange(callback) - Set callback for slider change events.
// 
///////////////////////////////////////////////////////////////////////////////

function Slider(cname, min, max, begin, validateFunction, orientation, enabled)
{
  var isDragging = false;
  /*
   * Return number of pixels to top
   */
  function getRealTop(imgElem) {
    try {
      yPos = eval(imgElem).offsetTop;
      tempEl = eval(imgElem).offsetParent;
      while (tempEl != null) {
        yPos += tempEl.offsetTop;
        tempEl = tempEl.offsetParent;
      }
    } catch(e) {yPos = 1;}
    return yPos;
  }

  /*
   * Return number of pixels to left side
   */
  function getRealLeft(imgElem) {
    try {
      xPos = eval(imgElem).offsetLeft;
      tempEl = eval(imgElem).offsetParent;
      while (tempEl != null) {
        xPos += tempEl.offsetLeft;
        tempEl = tempEl.offsetParent;
      }
    } catch(e) {xPos = 1;}
    return xPos;
  }

  /*
  * Returns if the scrollbar present for browser
  */
  function getScrollBarState() {
    var result = {vScrollbar: true, hScrollbar: true};
    try {
      var root = document.documentElement === null? document.body: document.documentElement;
      result.vScrollbar = root.scrollHeight > root.clientHeight;
      result.hScrollbar = root.scrollWidth > root.clientWidth;
    } catch(e) {}
    return(result);
  }

  this.startDrag = function(e) {
    var coord;
    var obj = this.obj;

    if (!obj.isEnabled)
      return false;

    var h = obj.handle;

    if (!h)
      return false;
    isDragging = true;

    // Set coordinate event origioned from and prevent triggering the background.
    if (typeof(event) == "undefined") {
      coord = obj.isHorizontal ? e.clientX : e.clientY;
      e.stopPropagation();
    } else {
      coord = obj.isHorizontal ? event.screenX : event.y;
      event.cancelBubble=true
    }

    h.last_pos = coord - (obj.isHorizontal ? getRealLeft(h) : getRealTop(h));

    // After dragging has started, the mouse pointer is not necessarily
    // located over the handle. Therefor keep track in the entire document.

    document.obj = obj;
    document.onmousemove = obj.doDrag;
    document.onmouseup = obj.endDrag;

    return false;
  }

  this.doDrag = function(e) {
    var coord;
    var obj = this.obj;

    if (!obj.isEnabled)
      return false;

    var h = obj.handle;

    if (!h)
      return false;

    if (obj.isHorizontal) {
      if (typeof(event) == "undefined")
        coord = e.clientX;
      else
        coord = event.screenX;

      obj.value = obj.getValue(coord - h.last_pos);
      var newPos = obj.getPixelX(obj.value);

      h.style.left = newPos + "px";
    } else {
      if (typeof(event) == "undefined")
        coord = e.clientY;
      else
        coord = event.y;

      obj.value = obj.getValue(coord - h.last_pos);
      var newPos = obj.getPixelY(obj.value);

      h.style.top = newPos + "px";
    }

    if (obj.text)
      obj.text.value = obj.value;

    return false;
  }

  this.endDrag = function() {
    document.onmousemove = null;
    document.onmouseup = null;

    if (typeof(image_preview_onChange) == "function")
      image_preview_onChange();
    if (typeof(this.obj.onSliderChange) == "function")
      this.obj.onSliderChange(this.obj.cname, this.obj.value);
    else if (typeof(slider_onChange) == "function")
      slider_onChange(this.obj.cname, this.obj.value);

    isDragging = false;
  }

  /*
   * Called when changing a value in the text field.
   * To force a move even when disabled, set 'force' to true
   */
  this.moveTo = function(force) {
    var obj = this.obj;

    if (!force && !obj.isEnabled)
        return false;

    var val = obj.boundary(obj.text.value);

    if (val != null && !isNaN(val)) {
      obj.value = val;
      if (obj.text)
        obj.text.value = val;
      obj.refresh();

      if (typeof(image_preview_onChange) == "function")
        image_preview_onChange();
      if (typeof(obj.onSliderChange) == "function")
        obj.onSliderChange(obj.cname, obj.value);
      else if (typeof(slider_onChange) == "function")
        slider_onChange(obj.cname, obj.value);
    }

    return false;
  }

  /*
   * Called when clicking left/right button or when clicking the background
   */
  this.moveX = function(step) {
    var coord;
    var obj = this.obj;

    if (!obj.isEnabled)
      return false;

    var h = obj.handle;
    if (typeof(step) == "number") {
      /* left or right button was clicked */
      obj.value = parseInt(obj.value, 10);
      obj.value = obj.boundary(obj.value + step);
      coord = obj.getPixelX(obj.value);
     } else {
      var dh = ( h ? h.width/2 : 0);
      /* background was clicked.*/
      if (typeof(event) == "undefined" || event == null) {
        coord = step.clientX;
      } else {
        var hScrollbar = getScrollBarState().hScrollbar;
        coord = (!!hScrollbar && obj.barWidth > 1000)? event.offsetX +h.width+dh:event.x;
      }
      obj.value = obj.getValue(coord + document.body.scrollLeft - dh);
      coord = obj.getPixelX(obj.value);
    }

    if (h)
      h.style.left = coord + "px";

    if (obj.text) {
      obj.text.value = obj.value;
    }

    if (obj.moving) {
      obj.timeout = setTimeout(obj.cname + "Slider.obj.moveX(" + step + ")",40);
    } else {
      if (typeof(image_preview_onChange) == "function")
        image_preview_onChange();
      if (typeof(obj.onSliderChange) == "function")
        obj.onSliderChange(obj.cname, obj.value);
      else if (typeof(slider_onChange) == "function")
        slider_onChange(obj.cname, obj.value);
    }

    return false;
  }

  /*
   * Called when clicking up/down button or when clicking the background
   */
  this.moveY = function(step) {
    var coord;
    var obj = this.obj;

    if (!obj.isEnabled)
      return false;

    var h = obj.handle;
    if (typeof(step) == "number") {
      /* up or down button was clicked */
      obj.value = obj.boundary(obj.value + step);
      coord = obj.getPixelY(obj.value);
    } else {
      /* background was clicked.*/
      if (typeof(event) == "undefined" || event == null)
        coord = step.clientY;
      else
        coord = event.y;
      var dh = ( h ? h.width/2 : 0);
      obj.value = obj.getValue(coord + document.body.scrollTop - dh);
      coord = obj.getPixelY(obj.value);
    }

    if (h)
      h.style.top = coord + "px";

    if (obj.text) {
      obj.text.value = obj.value;
    }

    if (obj.moving) {
      obj.timeout = setTimeout(obj.cname + "Slider.obj.moveY(" + step + ")",40);
    } else {
      if (typeof(image_preview_onChange) == "function")
        image_preview_onChange();
      if (typeof(obj.onSliderChange) == "function")
        obj.onSliderChange(obj.cname, obj.value);
      else if (typeof(slider_onChange) == "function")
        slider_onChange(obj.cname, obj.value);
    }

    return false;
  }

  this.stopMoving = function(pixels) {
    var obj = this.obj;
    if (obj.moving) {
      obj.moving = false;
      if (obj.timeout)
        clearTimeout(obj.timeout);
      if (typeof(image_preview_onChange) == "function")
        image_preview_onChange();
      if (typeof(obj.onSliderChange) == "function")
        obj.onSliderChange(obj.cname, obj.value);
      else if (typeof(slider_onChange) == "function")
        slider_onChange(obj.cname, obj.value);
    }
  }

  this.boundary = function(value) {
    var obj = this.obj;
    var v = parseInt(value, 10);
    if (!isNaN(v))
      return Math.min(Math.max(v, obj.min), obj.max);
    else
      return obj.value;
  }

  this.getValue = function(pixel) {
    var obj = this.obj;
    if (obj.isHorizontal)
      return obj.boundary(Math.round((obj.max - obj.min)*(parseInt(pixel, 10) - getRealLeft(obj.bg))/obj.barWidth + obj.min));
    else
      return obj.boundary(Math.round(obj.max - (obj.max - obj.min)*(parseInt(pixel, 10) - getRealTop(obj.bg))/obj.barHeight));
  }

  this.getPixelX = function(value, padding) {
    var obj = this.obj;
    padding = ((typeof(padding) == "number") ? padding: 0);
    var width = obj.barWidth - 2*padding;
    if (obj.max == obj.min)
      return getRealLeft(obj.bg);
    else
      return Math.min(Math.max(Math.round((parseFloat(value) - obj.min) / (obj.max - obj.min) * width), 0), width) + padding + getRealLeft(obj.bg);
  }

  this.getPixelY = function(value, valueHeight) {
    var obj = this.obj;
    valueHeight = ((typeof(valueHeight) == "number") ? valueHeight : 3);
    var padding = Math.floor(valueHeight/2);
    var height = obj.barHeight - valueHeight - valueHeight%2 - 1;
    if (obj.max == obj.min)
      return getRealTop(obj.bg);
    else
      return Math.min(Math.max(Math.round((obj.max - parseFloat(value)) / (obj.max - obj.min) * height), 0), height) + padding + getRealTop(obj.bg);
  }

  this.enable = function() {
    var obj = this.obj;
    obj.isEnabled = true;
  }

  this.disable = function() {
    var obj = this.obj;
    obj.isEnabled = false;
  }

  this.realSetActual = function() {
    var obj = this.obj;
    var o = obj.actualObject;
    var v = obj.actualValue;

    if (o) {
      try {
        if (obj.isHorizontal) {
          o.style.top = (getRealTop(obj.bg) + 1) + "px";
          o.style.left = (obj.getPixelX(v, Math.floor(obj.actualWidth/2) + 1) - Math.ceil(obj.actualWidth/2)) + "px";
          o.style.width = obj.actualWidth + "px";
          o.style.height = (obj.bg.height - 2) + "px";
          o.onmousedown = this.moveX;
        } else {
          o.style.top = obj.getPixelY(v, obj.actualWidth) + "px";
          o.style.left = (getRealLeft(obj.bg) + 1) + "px";
          o.style.width = (obj.bg.width - 2) + "px";
          o.style.height = obj.actualWidth + "px";
          o.onmousedown = this.moveY;
        }

      } catch(e) {}

      if (o.style.top.length    == 0 || o.style.left.length   == 0 ||
          o.style.width.length  == 0 || o.style.height.length == 0) {
        o.style.display = "none";
      }
    }
  }

  this.setActual = function(aValue) {
    var obj = this.obj;
    var o = obj.actualObject;
    var v = obj.boundary(aValue);

    if (isNaN(v))
      return;

    if (!o)
    {
      o = document.createElement("img");
      o.className = "actualSliderPos";
      o.id = "actualSliderPos-" + obj.cname;
      o.src = "/pics/blank.gif";
      o.actualValue = v;
      o.obj = this;
      obj.actualObject = o;
      obj.zIndex = 10;
      document.body.appendChild(o);
    } else {
      o.style.display = "";
    }

    obj.actualValue = v;
    obj.realSetActual();
  }

  this.removeActual = function() {
    var obj = this.obj;
    var o = obj.actualObject;
    if (o)
      o.style.display = "none";
  }

  this.update = function(min, max, value) {
    var obj = this.obj;

    min = parseInt(min, 10);
    max = parseInt(max, 10);
    value = parseInt(value, 10);

    if (isNaN(min) || isNaN(max) || isNaN(value))
      return false;

    if (!!isDragging) {
      return false;
    }

    obj.min = min;
    obj.max = max;
    obj.value = obj.boundary(value);
    if (obj.text)
      obj.text.value = value;
    obj.refresh();
  }

  this.refresh = function() {
    var obj = this.obj;
    var h = obj.handle;

    if (!h)
      return false;

    if (obj.isHorizontal) {
      h.style.left = obj.getPixelX(obj.value) + "px";
      h.style.top = getRealTop(obj.bg) + "px";
    } else {
      h.style.top = obj.getPixelY(obj.value) + "px";
      h.style.left = getRealLeft(obj.bg) + "px";
    }

    obj.realSetActual();
  }

  this.setSliderOnChange = function(method) {
    this.obj.onSliderChange = method;
  }

  /*
   * Save parameters
   */
  this.cname = cname;
  this.min = min;
  this.max = max;
  this.isEnabled = (enabled != false);
  this.isHorizontal = (orientation != "vertical");
  this.onSliderChange = null;

  _handle = document.getElementById(cname + '-handle');
  if (_handle)
  {
    _handle.style.position = "absolute";
    _handle.style.zIndex = 20;
    _handle.onmousedown = this.startDrag;
    _handle.obj = this;
  }
  var _handle_width = ( _handle ? _handle.width : 0 );
  this.handle = _handle;

  try {
    _bg = document.getElementById(cname + '-bg');
    if (_bg.width <= 0)
      this.barWidth = _bg.width - _handle_width; // NON IE
    else
      this.barWidth = _bg.clientWidth - _handle_width; // IE
    if (_bg.height <= 0)
      this.barHeight = _bg.height - _handle_width; // NON IE
    else
      this.barHeight = _bg.clientHeight - _handle_width; // IE
    _bg.obj = this;
    _bg.onmousedown = this.isHorizontal ? this.moveX : this.moveY;
    this.bg = _bg;
  } catch(e) {}

  if (_handle)
  {
    this.handle.style.left = getRealLeft(this.bg) + "px";
    this.handle.style.top = getRealTop(this.bg) + "px";
  }

  /*
   * Left and right button and text field are optional
   */

  if (_left = document.getElementById(cname + '-left')) {
    _left.onmousedown = function () {
      var obj = this.obj;
      if (!obj.isEnabled)
        return false;
      obj.moving = true;
      obj.moveX(-1);
    }
    _left.onmouseup = this.stopMoving;
    _left.onmouseout = this.stopMoving;
    _left.style.cursor = 'pointer';
    _left.obj = this;
  }

  if (_right = document.getElementById(cname + '-right')) {
    _right.onmousedown = function () {
      var obj = this.obj;
      if (!obj.isEnabled)
        return false;
      obj.moving = true;
      obj.moveX(1);
    }
    _right.onmouseup = this.stopMoving;
    _right.onmouseout = this.stopMoving;
    _right.style.cursor = 'pointer';
    _right.obj = this;
  }

  /*
   * Up and down button and text field are optional
   */

  if (_up = document.getElementById(cname + '-up')) {
    _up.onmousedown = function () {
      var obj = this.obj;
      if (!obj.isEnabled)
        return false;
      obj.moving = true;
      obj.moveY(-1);
    }
    _up.onmouseup = this.stopMoving;
    _up.onmouseout = this.stopMoving;
    _up.style.cursor = 'pointer';
    _up.obj = this;
  }

  if (_down = document.getElementById(cname + '-down')) {
    _down.onmousedown = function () {
      var obj = this.obj;
      if (!obj.isEnabled)
        return false;
      obj.moving = true;
      obj.moveY(1);
    }
    _down.onmouseup = this.stopMoving;
    _down.onmouseout = this.stopMoving;
    _down.style.cursor = 'pointer';
    _down.obj = this;
  }

  if (_text = document.getElementById(cname)) {
    _text.moveTo = this.moveTo;
    _text.value = begin;

    _text.onchange = function() {
      var obj = this.obj;
      if (!obj.isEnabled)
        return false;

      if (typeof(validateFunction) != 'function')
        var isValid = validateFunction();
      else
        var isValid = true;
      if (isValid)
        this.moveTo();
      return isValid;
    }

    this.original_onkeypress = _text.onkeypress;
    _text.onkeypress = function(e) {
      var isValid = true;
      var isValid2 = true;

      if (typeof(this.original_onkeypress) == 'function') {
        isValid = this.original_onkeypress(e);
      }

      var isEnterPressed = false;
      if (typeof(event) == "undefined")
        isEnterPressed = e.charCode == 13;
      else
        isEnterPressed = event.keyCode == 13;

      if (isEnterPressed) {
        if (typeof(validateFunction) == 'function') {
          isValid2 = validateFunction();
        }
        this.moveTo();
      }

      return isValid && isValid2;
    }

    _text.obj = this;
    this.text = _text;
  }
  this.moving = false;
  this.value = begin;
  this.obj = this;
  this.timeout = false;

  var o = document.getElementById("actualSliderPos-" + this.cname);
  if (o) {
    this.actualObject = o;
    this.actualValue = o.actualValue;
  } else {
    this.actualObject = null;
    this.actualValue = 0;
  }
  this.actualWidth = 5;

  try {
    if (this.isHorizontal) {
      /*
       * Adjust background if it's offsetted from the left button
       */
      _bg.style.backgroundPosition = '0 ' + (_left?(_left.offsetTop - _bg.offsetTop):0) + 'px';
      /*
       * Initialize position
       */
      if (_handle)
        _handle.style.left = this.getPixelX(begin) + "px";
    } else {
      /*
       * Adjust background if it's offsetted from the top button
       */
      _bg.style.backgroundPosition = '0 ' + (_up?(_up.offsetTop - _bg.offsetTop):0) + 'px';
      /*
       * Initialize position
       */
      if (_handle)
        _handle.style.top = this.getPixelY(begin) + "px";
    }
  } catch(e) {}

  return false;
}
