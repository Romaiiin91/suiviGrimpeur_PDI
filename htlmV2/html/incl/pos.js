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
  if (document.body) {
    box.w = document.body.clientWidth;
    box.h = document.body.clientHeight;
  }
  else if (document.documentElement && !isNaN(document.documentElement.clientWidth)) { // Explorer 6 Strict Mode
    box.w = document.documentElement.clientWidth;
    box.h = document.documentElement.clientHeight;
  }
  if (!isNaN(document.body.scrollLeft)) {  // all other Explorers
    if (document.body.scrollLeft > 0)
      box.x = document.body.scrollLeft;
    if (document.body.scrollTop > 0)
      box.y = document.body.scrollTop;
  }
  if (!isNaN(self.pageYOffset)) { // all except Explorer
    if (self.pageXOffset > 0)
      box.x = self.pageXOffset;
    if (self.pageYOffset > 0)
      box.y = self.pageYOffset;
  }
  if (document.documentElement && !isNaN(document.documentElement.scrollLeft)) { // Explorer 6 Strict
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
  var box = new Box(0,0,0,0);
  box.w=obj.offsetWidth;
  box.h=obj.offsetHeight;
  if (obj.offsetParent) {
    while (obj.offsetParent) {
      box.x += obj.offsetLeft;
      box.y += obj.offsetTop;
      obj = obj.offsetParent;
    }
  }
  else if (!isNaN(obj.x)) {
    box.x += obj.x;
    box.y += obj.y;
  }
//  alert("obj "+obj+" pos: "+box.x+","+box.y);
  return box;
}

function getMousePosition(e)
{
  // www.quirksmode.org
  var box = new Box(0,0,0,0);
  if (!isNaN(e.pageX) && !isNaN(e.pageY)) {
    box.x = e.pageX;
    box.y = e.pageY;
  }
  else if (!isNaN(e.clientX) && !isNaN(e.clientY)) {
    var bound = getBounds();
    box.x = (e.clientX + bound.x);
    box.y = (e.clientY + bound.y);
  }
  //alert("Mouse pos: "+box.x+","+box.y);
  return box;
}

