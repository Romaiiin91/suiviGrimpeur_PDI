String.prototype.trim = function() {
  return this.replace(/^\s+|\s+$/g,"");
}
String.prototype.ltrim = function() {
  return this.replace(/^\s+/,"");
}
String.prototype.rtrim = function() {
  return this.replace(/\s+$/,"");
}
String.prototype.crop = function(cropLength, cropMark) {
  if (typeof(cropMark) != "string")
    cropMark = "";

  if (this.length > cropLength && this.length > cropMark.length)
    return this.substr(0, cropLength - cropMark.length) + cropMark;
  else
    return this;
}
