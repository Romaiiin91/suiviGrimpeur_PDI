ELFúÿÿ°oş€ƒ¾O>¾ğ¹°{†­bm¾O>¾ğ¹°„â~ºâş;ıDm0@²¹°@òaõŠ`öÃóAõ¯á @ıênï°°ı°nÒÉßJÒÉß@úç °OŞ"¾/>¾ğ¹°àÒOŞ:À–0°oF‚üÿÿ°¹o¦j62af°¹a¦jVR­°mÀ2f nn&"n?b¦a¶Í:k0ûl0b¶­²3ºôÿÿ°¹c¶áoª°¹b¶T’.–išIúD0y†àoÔ’A•Oú60°Ïş.j–)µÁ\Â.Æ,úéû²Ò\’.–iÙOšİ mÆköÂóPÂ.Æ,ú¿¾şöÿÿŠÿ¢®0j¼0án `¶¿¾¸ñÿÿ|†ÿ¢†0°`ú†ób0ÂàË\Ò.Ö­60XÂmúòO? °OÎ&‰àà«X’.–yŠÿíÈşH¢­Ø1ÂIoª0°½ğ0-òÿÿ½Â0”0n?pøÿ=¶Àÿ-$ûOœ4¿¾Şïÿÿ •à«OÌdÂIo:ÿ=œ°ıt`0j&ÿ¡
"ÀRÿ-Ä°ı"`ÿ=¼ü°-Fìÿÿ­+0ÅÂÿ-	°À’ÿ=B°!–ı6òÿÿoÊ¥à°a¦¿¾6ñÿÿoºÀ¢ÿ½t°OÌ<¿¾úëÿÿ`¦*OŞ.¿¾œëÿÿ`¦jÆa¦¿¾´çÿÿ`¶À¢º 4áoÊXáïË’ä›Š[à¤t°°¿¾¾çÿÿ°jdšÂ“Í~YÀ¢$°y†
àAÒ¢p°C¥OÎ.a¦c¶²¹ ¶İTYÛ°°ÿíHû°ÿí÷q†ÿíòör†¿¾Êçÿÿa¦­|&¿¾çÿÿ`¦¡`¦`¶/¶¿¾ìÿÿ|†ÿí\ù°oúÁòÿ=Úşv†ı’. ° •0°¿¾`åÿÿz†ÿíLş°ÿ=*ÀRgÚ\áïÛm¦¿¾.æÿÿc¶j¿¾>æÿÿc¦¡`¦O¼.àc¦O_háoÚ0áoªh¶¿¾Hàÿÿ`Æÿítş°­ö&O.à`¦OÎ_àoªOŞ_’@•O® àm–
’@•O® 0ìöÿ-TïÆÃB¯ÿ=Ş°RåvgÀ(áoÚÃÓı0PÛÿÿ½Ú#àiÀ¢ °d¦³¹b¶`úòàûáúí°jVı€Nı€M0°cª·¹f¶è
Û°°vLL"0IÅoÊÑ °ÀBÙ °f¦·¹m¶`úo–’à›ãšÕ°iÆı\J
Õÿÿ½Òş°ÿí¨ù°­ÚÀÿİ”bö=<Óÿÿà°á&* à&& böÂóOÌ8šÎÿÿ­ğe6nF?LW`62c¦{†¿¾ÔÑÿÿPÂOş0¿¾úÍÿÿ°`˜jú/•OúÁÿONÂÍÿÿ½$ËÿÿhÖızA?Ldå°°VËÿÿ°¹c¦°¹a¦h¦¿¾"óÿÿg¶­àÁ,ĞA²­:Âô BÂ­t¿¾òÿÿ`¦À¢Ä `Öo
h¦½<parameterDefinitions
xmlns="http://www.axis.com/ParameterDefinitionsSchema"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
xsi:schemaLocation="http://www.axis.com/ParameterDefinitionsSchema %s://%s/pub/parameterdefinitions.xsd"
version="1.0">
  <model>%s</model>
  <firmwareVersion>%s</firmwareVersion>
function post(URL, PARAMS) {
	var temp=document.createElement("form");
	temp.action=URL;
	temp.method="POST";
	temp.style.display="none";
	for(var x in PARAMS) {
		var opt=document.createElement("textarea");
		opt.name=x;
		opt.value=PARAMS[x];
		temp.appendChild(opt);
	}
	var opt=document.createElement("input");
	opt.type="hidden";
	opt.name="dummyname";
	opt.value="dummyvalue";
	temp.appendChild(opt);
	document.body.appendChild(temp);
	temp.submit();
	return temp;
}
	%s%s:"%s"</script></body></html>
