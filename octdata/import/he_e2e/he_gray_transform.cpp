
#include <cstdint>

#include "he_gray_transform.h"


namespace OctData
{
	HeGrayTransformXml::HeGrayTransformXml()
	: lutXML(new uint8_t[1 << sizeof(uint16_t)*8])
	{
		uint8_t* lutXmlIt = lutXML;
		for(int i = 0; i < (1 << sizeof(uint16_t)*8); ++i)
		{
			*lutXmlIt = getXmlValue(i);
			++lutXmlIt;
		}
	}


	HeGrayTransformVol::HeGrayTransformVol()
	: lutVol(new uint8_t[1 << sizeof(uint16_t)*8])
	{
		uint8_t* lutVolIt = lutVol;
		for(int i = 0; i < (1 << sizeof(uint16_t)*8); ++i)
		{
			*lutVolIt = getVolValue(i);
			++lutVolIt;
		}
	}


	uint8_t HeGrayTransformXml::getXmlValue(uint16_t val)
	{
		if(val < 45852)
			return 0;
		if(val < 46906)
			return 1;
		if(val < 47492)
			return 2;
		if(val < 47994)
			return 3;
		if(val < 48320)
			return 4;
		if(val < 48588)
			return 5;
		if(val < 48865)
			return 6;
		if(val < 49151)
			return 7;
		if(val < 49300)
			return 8;
		if(val < 49452)
			return 9;
		if(val < 49609)
			return 10;
		if(val < 49772)
			return 11;
		if(val < 49941)
			return 12;
		if(val < 50114)
			return 13;
		if(val < 50235)
			return 14;
		if(val < 50328)
			return 15;
		if(val < 50423)
			return 16;
		if(val < 50522)
			return 17;
		if(val < 50624)
			return 18;
		if(val < 50730)
			return 19;
		if(val < 50839)
			return 20;
		if(val < 50951)
			return 21;
		if(val < 51068)
			return 22;
		if(val < 51189)
			return 23;
		if(val < 51257)
			return 24;
		if(val < 51321)
			return 25;
		if(val < 51386)
			return 26;
		if(val < 51455)
			return 27;
		if(val < 51526)
			return 28;
		if(val < 51599)
			return 29;
		if(val < 51674)
			return 30;
		if(val < 51752)
			return 31;
		if(val < 51832)
			return 32;
		if(val < 51917)
			return 33;
		if(val < 52002)
			return 34;
		if(val < 52092)
			return 35;
		if(val < 52182)
			return 36;
		if(val < 52251)
			return 37;
		if(val < 52299)
			return 38;
		if(val < 52351)
			return 39;
		if(val < 52403)
			return 40;
		if(val < 52457)
			return 41;
		if(val < 52512)
			return 42;
		if(val < 52570)
			return 43;
		if(val < 52629)
			return 44;
		if(val < 52691)
			return 45;
		if(val < 52754)
			return 46;
		if(val < 52819)
			return 47;
		if(val < 52887)
			return 48;
		if(val < 52957)
			return 49;
		if(val < 53030)
			return 50;
		if(val < 53104)
			return 51;
		if(val < 53181)
			return 52;
		if(val < 53254)
			return 53;
		if(val < 53296)
			return 54;
		if(val < 53339)
			return 55;
		if(val < 53382)
			return 56;
		if(val < 53428)
			return 57;
		if(val < 53474)
			return 58;
		if(val < 53523)
			return 59;
		if(val < 53572)
			return 60;
		if(val < 53625)
			return 61;
		if(val < 53678)
			return 62;
		if(val < 53733)
			return 63;
		if(val < 53790)
			return 64;
		if(val < 53848)
			return 65;
		if(val < 53909)
			return 66;
		if(val < 53973)
			return 67;
		if(val < 54037)
			return 68;
		if(val < 54104)
			return 69;
		if(val < 54173)
			return 70;
		if(val < 54246)
			return 71;
		if(val < 54295)
			return 72;
		if(val < 54334)
			return 73;
		if(val < 54373)
			return 74;
		if(val < 54414)
			return 75;
		if(val < 54456)
			return 76;
		if(val < 54499)
			return 77;
		if(val < 54544)
			return 78;
		if(val < 54591)
			return 79;
		if(val < 54639)
			return 80;
		if(val < 54687)
			return 81;
		if(val < 54739)
			return 82;
		if(val < 54792)
			return 83;
		if(val < 54846)
			return 84;
		if(val < 54902)
			return 85;
		if(val < 54961)
			return 86;
		if(val < 55021)
			return 87;
		if(val < 55083)
			return 88;
		if(val < 55147)
			return 89;
		if(val < 55214)
			return 90;
		if(val < 55281)
			return 91;
		if(val < 55324)
			return 92;
		if(val < 55361)
			return 93;
		if(val < 55398)
			return 94;
		if(val < 55438)
			return 95;
		if(val < 55478)
			return 96;
		if(val < 55520)
			return 97;
		if(val < 55562)
			return 98;
		if(val < 55607)
			return 99;
		if(val < 55652)
			return 100;
		if(val < 55700)
			return 101;
		if(val < 55749)
			return 102;
		if(val < 55800)
			return 103;
		if(val < 55852)
			return 104;
		if(val < 55906)
			return 105;
		if(val < 55961)
			return 106;
		if(val < 56019)
			return 107;
		if(val < 56078)
			return 108;
		if(val < 56140)
			return 109;
		if(val < 56203)
			return 110;
		if(val < 56268)
			return 111;
		if(val < 56329)
			return 112;
		if(val < 56363)
			return 113;
		if(val < 56400)
			return 114;
		if(val < 56437)
			return 115;
		if(val < 56475)
			return 116;
		if(val < 56516)
			return 117;
		if(val < 56556)
			return 118;
		if(val < 56599)
			return 119;
		if(val < 56643)
			return 120;
		if(val < 56688)
			return 121;
		if(val < 56735)
			return 122;
		if(val < 56783)
			return 123;
		if(val < 56834)
			return 124;
		if(val < 56885)
			return 125;
		if(val < 56938)
			return 126;
		if(val < 56994)
			return 127;
		if(val < 57050)
			return 128;
		if(val < 57109)
			return 129;
		if(val < 57170)
			return 130;
		if(val < 57234)
			return 131;
		if(val < 57297)
			return 132;
		if(val < 57355)
			return 133;
		if(val < 57389)
			return 134;
		if(val < 57425)
			return 135;
		if(val < 57462)
			return 136;
		if(val < 57500)
			return 137;
		if(val < 57540)
			return 138;
		if(val < 57580)
			return 139;
		if(val < 57622)
			return 140;
		if(val < 57666)
			return 141;
		if(val < 57711)
			return 142;
		if(val < 57757)
			return 143;
		if(val < 57805)
			return 144;
		if(val < 57854)
			return 145;
		if(val < 57905)
			return 146;
		if(val < 57958)
			return 147;
		if(val < 58013)
			return 148;
		if(val < 58069)
			return 149;
		if(val < 58128)
			return 150;
		if(val < 58187)
			return 151;
		if(val < 58249)
			return 152;
		if(val < 58313)
			return 153;
		if(val < 58374)
			return 154;
		if(val < 58408)
			return 155;
		if(val < 58443)
			return 156;
		if(val < 58480)
			return 157;
		if(val < 58518)
			return 158;
		if(val < 58557)
			return 159;
		if(val < 58597)
			return 160;
		if(val < 58639)
			return 161;
		if(val < 58682)
			return 162;
		if(val < 58726)
			return 163;
		if(val < 58772)
			return 164;
		if(val < 58820)
			return 165;
		if(val < 58868)
			return 166;
		if(val < 58919)
			return 167;
		if(val < 58971)
			return 168;
		if(val < 59025)
			return 169;
		if(val < 59081)
			return 170;
		if(val < 59138)
			return 171;
		if(val < 59197)
			return 172;
		if(val < 59260)
			return 173;
		if(val < 59324)
			return 174;
		if(val < 59387)
			return 175;
		if(val < 59424)
			return 176;
		if(val < 59459)
			return 177;
		if(val < 59495)
			return 178;
		if(val < 59533)
			return 179;
		if(val < 59571)
			return 180;
		if(val < 59611)
			return 181;
		if(val < 59653)
			return 182;
		if(val < 59695)
			return 183;
		if(val < 59738)
			return 184;
		if(val < 59784)
			return 185;
		if(val < 59831)
			return 186;
		if(val < 59879)
			return 187;
		if(val < 59929)
			return 188;
		if(val < 59981)
			return 189;
		if(val < 60034)
			return 190;
		if(val < 60089)
			return 191;
		if(val < 60147)
			return 192;
		if(val < 60205)
			return 193;
		if(val < 60267)
			return 194;
		if(val < 60329)
			return 195;
		if(val < 60393)
			return 196;
		if(val < 60438)
			return 197;
		if(val < 60473)
			return 198;
		if(val < 60509)
			return 199;
		if(val < 60546)
			return 200;
		if(val < 60584)
			return 201;
		if(val < 60624)
			return 202;
		if(val < 60664)
			return 203;
		if(val < 60706)
			return 204;
		if(val < 60751)
			return 205;
		if(val < 60794)
			return 206;
		if(val < 60841)
			return 207;
		if(val < 60889)
			return 208;
		if(val < 60938)
			return 209;
		if(val < 60989)
			return 210;
		if(val < 61042)
			return 211;
		if(val < 61096)
			return 212;
		if(val < 61155)
			return 213;
		if(val < 61212)
			return 214;
		if(val < 61271)
			return 215;
		if(val < 61334)
			return 216;
		if(val < 61397)
			return 217;
		if(val < 61452)
			return 218;
		if(val < 61486)
			return 219;
		if(val < 61521)
			return 220;
		if(val < 61558)
			return 221;
		if(val < 61596)
			return 222;
		if(val < 61638)
			return 223;
		if(val < 61675)
			return 224;
		if(val < 61717)
			return 225;
		if(val < 61759)
			return 226;
		if(val < 61804)
			return 227;
		if(val < 61850)
			return 228;
		if(val < 61897)
			return 229;
		if(val < 61946)
			return 230;
		if(val < 61998)
			return 231;
		if(val < 62049)
			return 232;
		if(val < 62104)
			return 233;
		if(val < 62159)
			return 234;
		if(val < 62219)
			return 235;
		if(val < 62276)
			return 236;
		if(val < 62341)
			return 237;
		if(val < 62401)
			return 238;
		if(val < 62465)
			return 239;
		if(val < 62499)
			return 240;
		if(val < 62535)
			return 241;
		if(val < 62573)
			return 242;
		if(val < 62608)
			return 243;
		if(val < 62646)
			return 244;
		if(val < 62690)
			return 245;
		if(val < 62734)
			return 246;
		if(val < 62770)
			return 247;
		if(val < 62818)
			return 248;
		if(val < 62859)
			return 249;
		if(val < 62915)
			return 250;
		if(val < 62960)
			return 251;
		if(val < 63011)
			return 252;
		if(val < 63060)
			return 253;
		if(val < 63117)
			return 254;
		return 255;

	};

}
