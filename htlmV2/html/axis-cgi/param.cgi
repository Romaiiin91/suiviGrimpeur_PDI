ELF����o����O>��{��bm�O>����~����;�Dm0@���@�a��`���A��� @��nﰰ��n���J���@�� �O�"�/>����O�:��0�oF
"�R�-���"`�=���-F����+0���-�	����=B�!��6���oʥ�a���6���o�����t�O�<������`�*O�.������`�j�a�������`���� 4�o�X�����
�Aҁ�p�C�O�.a�c��� ��TY۰���H�����q�����r�������a��|&�����`��`�`�/�������|���\��o����=��v���. ���0���`���z���L���=*�Rg�\���m���.���c�j��>���c��`�O�.�c�O�_h�o�0�o�h���H���`���t����&O.�`�O�_�o�O�_��@�O� �m�
��@�O� 0���-T���B��=��R�vg�(�o����0P�����#�i�� �d���b�`��������jV��N��M0�c���f��
۰�
����������������ݔb�=<�����&*��&&�b���O�8
h��<parameterDefinitions
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