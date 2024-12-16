// $Id: schemes.cr,v 1.1 2024/10/25 14:47:52 cvsuser Exp $
// base16 - scheme's
//
// See: https://github.com/chriskempson/base16-schemes-source
// Auto-generated Fri Oct 25 22:28:32 2024

void main()
{
	module("base16");
}

// horizon-license
// MIT License
// 
// Copyright (c) 2019 Michaël Ball
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/michael-ball/base16-horizon-scheme

static list
def_horizon_dark()
{
	list scheme = {
		"scheme", "Horizon Dark",
		"author", "Michaël Ball (http://github.com/michael-ball/)",
		"00", 0x1C1E26,
		"01", 0x232530,
		"02", 0x2E303E,
		"03", 0x6F6F70,
		"04", 0x9DA0A2,
		"05", 0xCBCED0,
		"06", 0xDCDFE4,
		"07", 0xE3E6EE,
		"08", 0xE93C58,
		"09", 0xE58D7D,
		"0A", 0xEFB993,
		"0B", 0xEFAF8E,
		"0C", 0x24A8B4,
		"0D", 0xDF5273,
		"0E", 0xB072D1,
		"0F", 0xE4A382,
	};
	return scheme;
}

static list
def_horizon_light()
{
	list scheme = {
		"scheme", "Horizon Light",
		"author", "Michaël Ball (http://github.com/michael-ball/)",
		"00", 0xFDF0ED,
		"01", 0xFADAD1,
		"02", 0xF9CBBE,
		"03", 0xBDB3B1,
		"04", 0x948C8A,
		"05", 0x403C3D,
		"06", 0x302C2D,
		"07", 0x201C1D,
		"08", 0xF7939B,
		"09", 0xF6661E,
		"0A", 0xFBE0D9,
		"0B", 0x94E1B0,
		"0C", 0xDC3318,
		"0D", 0xDA103F,
		"0E", 0x1D8991,
		"0F", 0xE58C92,
	};
	return scheme;
}

static list
def_horizon_terminal_dark()
{
	list scheme = {
		"scheme", "Horizon Dark",
		"author", "Michaël Ball (http://github.com/michael-ball/)",
		"00", 0x1C1E26,
		"01", 0x232530,
		"02", 0x2E303E,
		"03", 0x6F6F70,
		"04", 0x9DA0A2,
		"05", 0xCBCED0,
		"06", 0xDCDFE4,
		"07", 0xE3E6EE,
		"08", 0xE95678,
		"09", 0xFAB795,
		"0A", 0xFAC29A,
		"0B", 0x29D398,
		"0C", 0x59E1E3,
		"0D", 0x26BBD9,
		"0E", 0xEE64AC,
		"0F", 0xF09383,
	};
	return scheme;
}

static list
def_horizon_terminal_light()
{
	list scheme = {
		"scheme", "Horizon Light",
		"author", "Michaël Ball (http://github.com/michael-ball/)",
		"00", 0xFDF0ED,
		"01", 0xFADAD1,
		"02", 0xF9CBBE,
		"03", 0xBDB3B1,
		"04", 0x948C8A,
		"05", 0x403C3D,
		"06", 0x302C2D,
		"07", 0x201C1D,
		"08", 0xE95678,
		"09", 0xF9CEC3,
		"0A", 0xFADAD1,
		"0B", 0x29D398,
		"0C", 0x59E1E3,
		"0D", 0x26BBD9,
		"0E", 0xEE64AC,
		"0F", 0xF9CBBE,
	};
	return scheme;
}

// unikitty-license
// MIT License
// https://github.com/joshwlewis/base16-unikitty

static list
def_unikitty_dark()
{
	list scheme = {
		"scheme", "Unikitty Dark",
		"author", "Josh W Lewis (@joshwlewis)",
		"00", 0x2e2a31,
		"01", 0x4a464d,
		"02", 0x666369,
		"03", 0x838085,
		"04", 0x9f9da2,
		"05", 0xbcbabe,
		"06", 0xd8d7da,
		"07", 0xf5f4f7,
		"08", 0xd8137f,
		"09", 0xd65407,
		"0A", 0xdc8a0e,
		"0B", 0x17ad98,
		"0C", 0x149bda,
		"0D", 0x796af5,
		"0E", 0xbb60ea,
		"0F", 0xc720ca,
	};
	return scheme;
}

static list
def_unikitty_light()
{
	list scheme = {
		"scheme", "Unikitty Light",
		"author", "Josh W Lewis (@joshwlewis)",
		"00", 0xffffff,
		"01", 0xe1e1e2,
		"02", 0xc4c3c5,
		"03", 0xa7a5a8,
		"04", 0x89878b,
		"05", 0x6c696e,
		"06", 0x4f4b51,
		"07", 0x322d34,
		"08", 0xd8137f,
		"09", 0xd65407,
		"0A", 0xdc8a0e,
		"0B", 0x17ad98,
		"0C", 0x149bda,
		"0D", 0x775dff,
		"0E", 0xaa17e6,
		"0F", 0xe013d0,
	};
	return scheme;
}

// gruber-license
// The MIT License (MIT)
// 
// Copyright (c) 2022 Patel, Nimai <nimai.m.patel@gmail.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/nimaipatel/base16-gruber

static list
def_gruber()
{
	list scheme = {
		"scheme", "Gruber",
		"author", "Patel, Nimai <nimai.m.patel@gmail.com>, colors from www.github.com/rexim/gruber-darker-theme",
		"00", 0x181818,
		"01", 0x453d41,
		"02", 0x665c7f,
		"03", 0x9dae93,
		"04", 0xe4e4ef,
		"05", 0xf4f4ff,
		"06", 0xf5f5f5,
		"07", 0xe4e4ef,
		"08", 0xf43841,
		"09", 0xc73c3f,
		"0A", 0xffdd33,
		"0B", 0x73c936,
		"0C", 0x95a99f,
		"0D", 0x96a6c8,
		"0E", 0x9e95c7,
		"0F", 0xcc8c3c,
	};
	return scheme;
}

// dirtysea-license
// MIT License
// https://github.com/tartansandal/base16-dirtysea-scheme

static list
def_dirtysea()
{
	list scheme = {
		"scheme", "dirtysea",
		"author", "Kahlil (Kal) Hodgson",
		"00", 0xe0e0e0,
		"01", 0xd0dad0,
		"02", 0xd0d0d0,
		"03", 0x707070,
		"04", 0x202020,
		"05", 0x000000,
		"06", 0xf8f8f8,
		"07", 0xc4d9c4,
		"08", 0x840000,
		"09", 0x006565,
		"0A", 0x755B00,
		"0B", 0x730073,
		"0C", 0x755B00,
		"0D", 0x007300,
		"0E", 0x000090,
		"0F", 0x755B00,
	};
	return scheme;
}

// material-vivid-license
// MIT License
// 
// Copyright (c) 2018
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/joshyrobot/base16-material-vivid-scheme

static list
def_material_vivid()
{
	list scheme = {
		"scheme", "Material Vivid",
		"author", "joshyrobot",
		"00", 0x202124,
		"01", 0x27292c,
		"02", 0x323639,
		"03", 0x44464d,
		"04", 0x676c71,
		"05", 0x80868b,
		"06", 0x9e9e9e,
		"07", 0xffffff,
		"08", 0xf44336,
		"09", 0xff9800,
		"0A", 0xffeb3b,
		"0B", 0x00e676,
		"0C", 0x00bcd4,
		"0D", 0x2196f3,
		"0E", 0x673ab7,
		"0F", 0x8d6e63,
	};
	return scheme;
}

// one-light-license
// MIT License
// https://github.com/purpleKarrot/base16-one-light-scheme

static list
def_one_light()
{
	list scheme = {
		"scheme", "One Light",
		"author", "Daniel Pfeifer (http://github.com/purpleKarrot)",
		"00", 0xfafafa,
		"01", 0xf0f0f1,
		"02", 0xe5e5e6,
		"03", 0xa0a1a7,
		"04", 0x696c77,
		"05", 0x383a42,
		"06", 0x202227,
		"07", 0x090a0b,
		"08", 0xca1243,
		"09", 0xd75f00,
		"0A", 0xc18401,
		"0B", 0x50a14f,
		"0C", 0x0184bc,
		"0D", 0x4078f2,
		"0E", 0xa626a4,
		"0F", 0x986801,
	};
	return scheme;
}

// brushtrees-license
// MIT License
// 
// Copyright (c) 2017 Abraham White
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/WhiteAbeLincoln/base16-brushtrees-scheme

static list
def_brushtrees_dark()
{
	list scheme = {
		"scheme", "Brush Trees Dark",
		"author", "Abraham White <abelincoln.white@gmail.com>",
		"00", 0x485867,
		"01", 0x5A6D7A,
		"02", 0x6D828E,
		"03", 0x8299A1,
		"04", 0x98AFB5,
		"05", 0xB0C5C8,
		"06", 0xC9DBDC,
		"07", 0xE3EFEF,
		"08", 0xb38686,
		"09", 0xd8bba2,
		"0A", 0xaab386,
		"0B", 0x87b386,
		"0C", 0x86b3b3,
		"0D", 0x868cb3,
		"0E", 0xb386b2,
		"0F", 0xb39f9f,
	};
	return scheme;
}

static list
def_brushtrees()
{
	list scheme = {
		"scheme", "Brush Trees",
		"author", "Abraham White <abelincoln.white@gmail.com>",
		"00", 0xE3EFEF,
		"01", 0xC9DBDC,
		"02", 0xB0C5C8,
		"03", 0x98AFB5,
		"04", 0x8299A1,
		"05", 0x6D828E,
		"06", 0x5A6D7A,
		"07", 0x485867,
		"08", 0xb38686,
		"09", 0xd8bba2,
		"0A", 0xaab386,
		"0B", 0x87b386,
		"0C", 0x86b3b3,
		"0D", 0x868cb3,
		"0E", 0xb386b2,
		"0F", 0xb39f9f,
	};
	return scheme;
}

// framer-license
// MIT License
// https://github.com/jssee/base16-framer-scheme

static list
def_framer()
{
	list scheme = {
		"scheme", "Framer",
		"author", "Framer (Maintained by Jesse Hoyos)",
		"00", 0x181818,
		"01", 0x151515,
		"02", 0x464646,
		"03", 0x747474,
		"04", 0xB9B9B9,
		"05", 0xD0D0D0,
		"06", 0xE8E8E8,
		"07", 0xEEEEEE,
		"08", 0xFD886B,
		"09", 0xFC4769,
		"0A", 0xFECB6E,
		"0B", 0x32CCDC,
		"0C", 0xACDDFD,
		"0D", 0x20BCFC,
		"0E", 0xBA8CFC,
		"0F", 0xB15F4A,
	};
	return scheme;
}

// mellow-license
// MIT License
// 
// Copyright (c) 2017 Timm
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/gidsi/base16-mellow-scheme

static list
def_mellow_purple()
{
	list scheme = {
		"scheme", "Mellow Purple",
		"author", "gidsi",
		"00", 0x1e0528,
		"01", 0x1A092D,
		"02", 0x331354,
		"03", 0x320f55,
		"04", 0x873582,
		"05", 0xffeeff,
		"06", 0xffeeff,
		"07", 0xf8c0ff,
		"08", 0x00d9e9,
		"09", 0xaa00a3,
		"0A", 0x955ae7,
		"0B", 0x05cb0d,
		"0C", 0xb900b1,
		"0D", 0x550068,
		"0E", 0x8991bb,
		"0F", 0x4d6fff,
	};
	return scheme;
}

// vice-license
// MIT License
// https://github.com/Thomashighbaugh/base16-vice-scheme

static list
def_vice_alt()
{
	list scheme = {
		"scheme", "Vice Alt",
		"author", "Thomas Leon Highbaugh",
		"00", 0x1c1c1c,
		"01", 0x282828,
		"02", 0x2c2c2c,
		"03", 0x323232,
		"04", 0x3c3c3c,
		"05", 0x555555,
		"06", 0xb6b6b6,
		"07", 0xd1d1d1,
		"08", 0xff3d81,
		"09", 0xF67544,
		"0A", 0xffff73,
		"0B", 0x44ffdd,
		"0C", 0x00caff,
		"0D", 0x2fb1d4,
		"0E", 0x8265ff,
		"0F", 0xF83D80,
	};
	return scheme;
}

static list
def_vice()
{
	list scheme = {
		"scheme", "Vice Dark",
		"author", "Thomas Leon Highbaugh",
		"00", 0x181818,
		"01", 0x222222,
		"02", 0x323232,
		"03", 0x3f3f3f,
		"04", 0x666666,
		"05", 0x818181,
		"06", 0xc6c6c6,
		"07", 0xe9e9e9,
		"08", 0xff29a8,
		"09", 0x85ffe0,
		"0A", 0xf0ffaa,
		"0B", 0x0badff,
		"0C", 0x8265ff,
		"0D", 0x00eaff,
		"0E", 0x00f6d9,
		"0F", 0xff3d81,
	};
	return scheme;
}

// outrun-license
// MIT License
// 
// Copyright (c) 2017 Hugo Delahousse
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/hugodelahousse/base16-outrun-schemes

static list
def_outrun_dark()
{
	list scheme = {
		"scheme", "Outrun Dark",
		"author", "Hugo Delahousse (http://github.com/hugodelahousse/)",
		"00", 0x00002A,
		"01", 0x20204A,
		"02", 0x30305A,
		"03", 0x50507A,
		"04", 0xB0B0DA,
		"05", 0xD0D0FA,
		"06", 0xE0E0FF,
		"07", 0xF5F5FF,
		"08", 0xFF4242,
		"09", 0xFC8D28,
		"0A", 0xF3E877,
		"0B", 0x59F176,
		"0C", 0x0EF0F0,
		"0D", 0x66B0FF,
		"0E", 0xF10596,
		"0F", 0xF003EF,
	};
	return scheme;
}

// zenburn-license
// MIT License
// https://github.com/elnawe/base16-zenburn-scheme

static list
def_zenburn()
{
	list scheme = {
		"scheme", "Zenburn",
		"author", "elnawe",
		"00", 0x383838,
		"01", 0x404040,
		"02", 0x606060,
		"03", 0x6f6f6f,
		"04", 0x808080,
		"05", 0xdcdccc,
		"06", 0xc0c0c0,
		"07", 0xffffff,
		"08", 0xdca3a3,
		"09", 0xdfaf8f,
		"0A", 0xe0cf9f,
		"0B", 0x5f7f5f,
		"0C", 0x93e0e3,
		"0D", 0x7cb8bb,
		"0E", 0xdc8cc3,
		"0F", 0x000000,
	};
	return scheme;
}

// twilight-license
// MIT License
// https://github.com/hartbit/base16-twilight-scheme

static list
def_twilight()
{
	list scheme = {
		"scheme", "Twilight",
		"author", "David Hart (https://github.com/hartbit)",
		"00", 0x1e1e1e,
		"01", 0x323537,
		"02", 0x464b50,
		"03", 0x5f5a60,
		"04", 0x838184,
		"05", 0xa7a7a7,
		"06", 0xc3c3c3,
		"07", 0xffffff,
		"08", 0xcf6a4c,
		"09", 0xcda869,
		"0A", 0xf9ee98,
		"0B", 0x8f9d6a,
		"0C", 0xafc4db,
		"0D", 0x7587a6,
		"0E", 0x9b859d,
		"0F", 0x9b703f,
	};
	return scheme;
}

// nebula-license
// MIT License
// 
// Copyright (c) 2020 Gabriel Fontes
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/Misterio77/base16-nebula-scheme

static list
def_nebula()
{
	list scheme = {
		"scheme", "Nebula",
		"author", "Gabriel Fontes (https://github.com/Misterio77)",
		"00", 0x22273b,
		"01", 0x414f60,
		"02", 0x5a8380,
		"03", 0x6e6f72,
		"04", 0x87888b,
		"05", 0xa4a6a9,
		"06", 0xc7c9cd,
		"07", 0x8dbdaa,
		"08", 0x777abc,
		"09", 0x94929e,
		"0A", 0x4f9062,
		"0B", 0x6562a8,
		"0C", 0x226f68,
		"0D", 0x4d6bb6,
		"0E", 0x716cae,
		"0F", 0x8c70a7,
	};
	return scheme;
}

// darcula-license
// MIT License
// https://github.com/casonadams/base16-darcula-scheme

static list
def_darcula()
{
	list scheme = {
		"scheme", "Darcula",
		"author", "jetbrains",
		"00", 0x2b2b2b,
		"01", 0x323232,
		"02", 0x323232,
		"03", 0x606366,
		"04", 0xa4a3a3,
		"05", 0xa9b7c6,
		"06", 0xffc66d,
		"07", 0xffffff,
		"08", 0x4eade5,
		"09", 0x689757,
		"0A", 0xbbb529,
		"0B", 0x6a8759,
		"0C", 0x629755,
		"0D", 0x9876aa,
		"0E", 0xcc7832,
		"0F", 0x808080,
	};
	return scheme;
}

// default-license
// MIT License
// https://github.com/chriskempson/base16-default-schemes

static list
def_cupcake()
{
	list scheme = {
		"scheme", "Cupcake",
		"author", "Chris Kempson (http://chriskempson.com)",
		"00", 0xfbf1f2,
		"01", 0xf2f1f4,
		"02", 0xd8d5dd,
		"03", 0xbfb9c6,
		"04", 0xa59daf,
		"05", 0x8b8198,
		"06", 0x72677E,
		"07", 0x585062,
		"08", 0xD57E85,
		"09", 0xEBB790,
		"0A", 0xDCB16C,
		"0B", 0xA3B367,
		"0C", 0x69A9A7,
		"0D", 0x7297B9,
		"0E", 0xBB99B4,
		"0F", 0xBAA58C,
	};
	return scheme;
}

static list
def_default_dark()
{
	list scheme = {
		"scheme", "Default Dark",
		"author", "Chris Kempson (http://chriskempson.com)",
		"00", 0x181818,
		"01", 0x282828,
		"02", 0x383838,
		"03", 0x585858,
		"04", 0xb8b8b8,
		"05", 0xd8d8d8,
		"06", 0xe8e8e8,
		"07", 0xf8f8f8,
		"08", 0xab4642,
		"09", 0xdc9656,
		"0A", 0xf7ca88,
		"0B", 0xa1b56c,
		"0C", 0x86c1b9,
		"0D", 0x7cafc2,
		"0E", 0xba8baf,
		"0F", 0xa16946,
	};
	return scheme;
}

static list
def_default_light()
{
	list scheme = {
		"scheme", "Default Light",
		"author", "Chris Kempson (http://chriskempson.com)",
		"00", 0xf8f8f8,
		"01", 0xe8e8e8,
		"02", 0xd8d8d8,
		"03", 0xb8b8b8,
		"04", 0x585858,
		"05", 0x383838,
		"06", 0x282828,
		"07", 0x181818,
		"08", 0xab4642,
		"09", 0xdc9656,
		"0A", 0xf7ca88,
		"0B", 0xa1b56c,
		"0C", 0x86c1b9,
		"0D", 0x7cafc2,
		"0E", 0xba8baf,
		"0F", 0xa16946,
	};
	return scheme;
}

static list
def_eighties()
{
	list scheme = {
		"scheme", "Eighties",
		"author", "Chris Kempson (http://chriskempson.com)",
		"00", 0x2d2d2d,
		"01", 0x393939,
		"02", 0x515151,
		"03", 0x747369,
		"04", 0xa09f93,
		"05", 0xd3d0c8,
		"06", 0xe8e6df,
		"07", 0xf2f0ec,
		"08", 0xf2777a,
		"09", 0xf99157,
		"0A", 0xffcc66,
		"0B", 0x99cc99,
		"0C", 0x66cccc,
		"0D", 0x6699cc,
		"0E", 0xcc99cc,
		"0F", 0xd27b53,
	};
	return scheme;
}

static list
def_mocha()
{
	list scheme = {
		"scheme", "Mocha",
		"author", "Chris Kempson (http://chriskempson.com)",
		"00", 0x3B3228,
		"01", 0x534636,
		"02", 0x645240,
		"03", 0x7e705a,
		"04", 0xb8afad,
		"05", 0xd0c8c6,
		"06", 0xe9e1dd,
		"07", 0xf5eeeb,
		"08", 0xcb6077,
		"09", 0xd28b71,
		"0A", 0xf4bc87,
		"0B", 0xbeb55b,
		"0C", 0x7bbda4,
		"0D", 0x8ab3b5,
		"0E", 0xa89bb9,
		"0F", 0xbb9584,
	};
	return scheme;
}

static list
def_ocean()
{
	list scheme = {
		"scheme", "Ocean",
		"author", "Chris Kempson (http://chriskempson.com)",
		"00", 0x2b303b,
		"01", 0x343d46,
		"02", 0x4f5b66,
		"03", 0x65737e,
		"04", 0xa7adba,
		"05", 0xc0c5ce,
		"06", 0xdfe1e8,
		"07", 0xeff1f5,
		"08", 0xbf616a,
		"09", 0xd08770,
		"0A", 0xebcb8b,
		"0B", 0xa3be8c,
		"0C", 0x96b5b4,
		"0D", 0x8fa1b3,
		"0E", 0xb48ead,
		"0F", 0xab7967,
	};
	return scheme;
}

// woodland-license
// MIT License
// 
// Copyright (c) 2016 Jay Cornwall
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/jcornwall/base16-woodland-scheme

static list
def_woodland()
{
	list scheme = {
		"scheme", "Woodland",
		"author", "Jay Cornwall (https://jcornwall.com)",
		"00", 0x231e18,
		"01", 0x302b25,
		"02", 0x48413a,
		"03", 0x9d8b70,
		"04", 0xb4a490,
		"05", 0xcabcb1,
		"06", 0xd7c8bc,
		"07", 0xe4d4c8,
		"08", 0xd35c5c,
		"09", 0xca7f32,
		"0A", 0xe0ac16,
		"0B", 0xb7ba53,
		"0C", 0x6eb958,
		"0D", 0x88a4d3,
		"0E", 0xbb90e2,
		"0F", 0xb49368,
	};
	return scheme;
}

// gigavolt-license
// MIT License
// 
// Copyright (c) 2019 Aidan Swope
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/Whillikers/base16-gigavolt-scheme

static list
def_gigavolt()
{
	list scheme = {
		"scheme", "Gigavolt",
		"author", "Aidan Swope (http://github.com/Whillikers)",
		"00", 0x202126,
		"01", 0x2d303d,
		"02", 0x5a576e,
		"03", 0xa1d2e6,
		"04", 0xcad3ff,
		"05", 0xe9e7e1,
		"06", 0xeff0f9,
		"07", 0xf2fbff,
		"08", 0xff661a,
		"09", 0x19f988,
		"0A", 0xffdc2d,
		"0B", 0xf2e6a9,
		"0C", 0xfb6acb,
		"0D", 0x40bfff,
		"0E", 0xae94f9,
		"0F", 0x6187ff,
	};
	return scheme;
}

// edge-license
// MIT License
// 
// Copyright (c) 2019 Calvin Ross
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/cjayross/base16-edge-schemes

static list
def_edge_dark()
{
	list scheme = {
		"scheme", "Edge Dark",
		"author", "cjayross (https://github.com/cjayross)",
		"00", 0x262729,
		"01", 0x88909f,
		"02", 0xb7bec9,
		"03", 0x3e4249,
		"04", 0x73b3e7,
		"05", 0xb7bec9,
		"06", 0xd390e7,
		"07", 0x3e4249,
		"08", 0xe77171,
		"09", 0xe77171,
		"0A", 0xdbb774,
		"0B", 0xa1bf78,
		"0C", 0x5ebaa5,
		"0D", 0x73b3e7,
		"0E", 0xd390e7,
		"0F", 0x5ebaa5,
	};
	return scheme;
}

static list
def_edge_light()
{
	list scheme = {
		"scheme", "Edge Light",
		"author", "cjayross (https://github.com/cjayross)",
		"00", 0xfafafa,
		"01", 0x7c9f4b,
		"02", 0xd69822,
		"03", 0x5e646f,
		"04", 0x6587bf,
		"05", 0x5e646f,
		"06", 0xb870ce,
		"07", 0x5e646f,
		"08", 0xdb7070,
		"09", 0xdb7070,
		"0A", 0xd69822,
		"0B", 0x7c9f4b,
		"0C", 0x509c93,
		"0D", 0x6587bf,
		"0E", 0xb870ce,
		"0F", 0x509c93,
	};
	return scheme;
}

// icy-license
// MIT License
// https://github.com/icyphox/base16-icy-scheme

static list
def_icy()
{
	list scheme = {
		"scheme", "Icy Dark",
		"author", "icyphox (https://icyphox.ga)",
		"00", 0x021012,
		"01", 0x031619,
		"02", 0x041f23,
		"03", 0x052e34,
		"04", 0x064048,
		"05", 0x095b67,
		"06", 0x0c7c8c,
		"07", 0x109cb0,
		"08", 0x16c1d9,
		"09", 0xb3ebf2,
		"0A", 0x80deea,
		"0B", 0x4dd0e1,
		"0C", 0x26c6da,
		"0D", 0x00bcd4,
		"0E", 0x00acc1,
		"0F", 0x0097a7,
	};
	return scheme;
}

// github-license
// MIT License
// https://github.com/Defman21/base16-github-scheme

static list
def_github()
{
	list scheme = {
		"scheme", "Github",
		"author", "Defman21",
		"00", 0xffffff,
		"01", 0xf5f5f5,
		"02", 0xc8c8fa,
		"03", 0x969896,
		"04", 0xe8e8e8,
		"05", 0x333333,
		"06", 0xffffff,
		"07", 0xffffff,
		"08", 0xed6a43,
		"09", 0x0086b3,
		"0A", 0x795da3,
		"0B", 0x183691,
		"0C", 0x183691,
		"0D", 0x795da3,
		"0E", 0xa71d5d,
		"0F", 0x333333,
	};
	return scheme;
}

// onedark-license
// MIT License
// https://github.com/tilal6991/base16-onedark-scheme

static list
def_onedark()
{
	list scheme = {
		"scheme", "OneDark",
		"author", "Lalit Magant (http://github.com/tilal6991)",
		"00", 0x282c34,
		"01", 0x353b45,
		"02", 0x3e4451,
		"03", 0x545862,
		"04", 0x565c64,
		"05", 0xabb2bf,
		"06", 0xb6bdca,
		"07", 0xc8ccd4,
		"08", 0xe06c75,
		"09", 0xd19a66,
		"0A", 0xe5c07b,
		"0B", 0x98c379,
		"0C", 0x56b6c2,
		"0D", 0x61afef,
		"0E", 0xc678dd,
		"0F", 0xbe5046,
	};
	return scheme;
}

// fruit-soda-license
// MIT License
// https://github.com/jozip/base16-fruit-soda-scheme

static list
def_fruit_soda()
{
	list scheme = {
		"scheme", "Fruit Soda",
		"author", "jozip",
		"00", 0xf1ecf1,
		"01", 0xe0dee0,
		"02", 0xd8d5d5,
		"03", 0xb5b4b6,
		"04", 0x979598,
		"05", 0x515151,
		"06", 0x474545,
		"07", 0x2d2c2c,
		"08", 0xfe3e31,
		"09", 0xfe6d08,
		"0A", 0xf7e203,
		"0B", 0x47f74c,
		"0C", 0x0f9cfd,
		"0D", 0x2931df,
		"0E", 0x611fce,
		"0F", 0xb16f40,
	};
	return scheme;
}

// pinky-license
// MIT License
// 
// Copyright (c) 2020 b3nj5m1n
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/b3nj5m1n/base16-pinky-scheme

static list
def_pinky()
{
	list scheme = {
		"scheme", "pinky",
		"author", "Benjamin (https://github.com/b3nj5m1n)",
		"00", 0x171517,
		"01", 0x1b181b,
		"02", 0x1d1b1d,
		"03", 0x383338,
		"04", 0xe7dbdb,
		"05", 0xf5f5f5,
		"06", 0xffffff,
		"07", 0xf7f3f7,
		"08", 0xffa600,
		"09", 0x00ff66,
		"0A", 0x20df6c,
		"0B", 0xff0066,
		"0C", 0x6600ff,
		"0D", 0x00ffff,
		"0E", 0x007fff,
		"0F", 0xdf206c,
	};
	return scheme;
}

// brogrammer-license
// MIT License
// https://github.com/piggyslasher/base16-brogrammer-scheme

static list
def_brogrammer()
{
	list scheme = {
		"scheme", "Brogrammer",
		"author", "Vik Ramanujam (http://github.com/piggyslasher)",
		"00", 0x1f1f1f,
		"01", 0xf81118,
		"02", 0x2dc55e,
		"03", 0xecba0f,
		"04", 0x2a84d2,
		"05", 0x4e5ab7,
		"06", 0x1081d6,
		"07", 0xd6dbe5,
		"08", 0xd6dbe5,
		"09", 0xde352e,
		"0A", 0x1dd361,
		"0B", 0xf3bd09,
		"0C", 0x1081d6,
		"0D", 0x5350b9,
		"0E", 0x0f7ddb,
		"0F", 0xffffff,
	};
	return scheme;
}

// cupertino-license
// MIT License
// https://github.com/Defman21/base16-cupertino

static list
def_cupertino()
{
	list scheme = {
		"scheme", "Cupertino",
		"author", "Defman21",
		"00", 0xffffff,
		"01", 0xc0c0c0,
		"02", 0xc0c0c0,
		"03", 0x808080,
		"04", 0x808080,
		"05", 0x404040,
		"06", 0x404040,
		"07", 0x5e5e5e,
		"08", 0xc41a15,
		"09", 0xeb8500,
		"0A", 0x826b28,
		"0B", 0x007400,
		"0C", 0x318495,
		"0D", 0x0000ff,
		"0E", 0xa90d91,
		"0F", 0x826b28,
	};
	return scheme;
}

// darkmoss-license
// MIT License
// https://github.com/avanzzzi/base16-darkmoss-scheme

static list
def_darkmoss()
{
	list scheme = {
		"scheme", "darkmoss",
		"author", "Gabriel Avanzi (https://github.com/avanzzzi)",
		"00", 0x171e1f,
		"01", 0x252c2d,
		"02", 0x373c3d,
		"03", 0x555e5f,
		"04", 0x818f80,
		"05", 0xc7c7a5,
		"06", 0xe3e3c8,
		"07", 0xe1eaef,
		"08", 0xff4658,
		"09", 0xe6db74,
		"0A", 0xfdb11f,
		"0B", 0x499180,
		"0C", 0x66d9ef,
		"0D", 0x498091,
		"0E", 0x9bc0c8,
		"0F", 0xd27b53,
	};
	return scheme;
}

// danqing-license
// MIT License
// 
// Copyright (c) 2020 Wenhan Zhu (Cosmos)
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/CosmosAtlas/base16-danqing-scheme

static list
def_danqing_light()
{
	list scheme = {
		"scheme", "DanQing Light",
		"author", "Wenhan Zhu (Cosmos) (zhuwenhan950913@gmail.com)",
		"00", 0xfcfefd,
		"01", 0xecf6f2,
		"02", 0xe0f0eF,
		"03", 0xcad8d2,
		"04", 0x9da8a3,
		"05", 0x5a605d,
		"06", 0x434846,
		"07", 0x2d302f,
		"08", 0xF9906F,
		"09", 0xB38A61,
		"0A", 0xF0C239,
		"0B", 0x8AB361,
		"0C", 0x30DFF3,
		"0D", 0xB0A4E3,
		"0E", 0xCCA4E3,
		"0F", 0xCA6924,
	};
	return scheme;
}

static list
def_danqing()
{
	list scheme = {
		"scheme", "DanQing",
		"author", "Wenhan Zhu (Cosmos) (zhuwenhan950913@gmail.com)",
		"00", 0x2d302f,
		"01", 0x434846,
		"02", 0x5a605d,
		"03", 0x9da8a3,
		"04", 0xcad8d2,
		"05", 0xe0f0eF,
		"06", 0xecf6f2,
		"07", 0xfcfefd,
		"08", 0xF9906F,
		"09", 0xB38A61,
		"0A", 0xF0C239,
		"0B", 0x8AB361,
		"0C", 0x30DFF3,
		"0D", 0xB0A4E3,
		"0E", 0xCCA4E3,
		"0F", 0xCA6924,
	};
	return scheme;
}

// hardcore-license
// MIT License
// 
// Copyright (c) 2019 Chris Caller
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/callerc1/base16-hardcore-scheme

static list
def_hardcore()
{
	list scheme = {
		"scheme", "Hardcore",
		"author", "Chris Caller",
		"00", 0x212121,
		"01", 0x303030,
		"02", 0x353535,
		"03", 0x4A4A4A,
		"04", 0x707070,
		"05", 0xcdcdcd,
		"06", 0xe5e5e5,
		"07", 0xffffff,
		"08", 0xf92672,
		"09", 0xfd971f,
		"0A", 0xe6db74,
		"0B", 0xa6e22e,
		"0C", 0x708387,
		"0D", 0x66d9ef,
		"0E", 0x9e6ffe,
		"0F", 0xe8b882,
	};
	return scheme;
}

// sandcastle-license
// MIT License
// 
// Copyright (c) 2019 George Essig
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/gessig/base16-sandcastle-scheme

static list
def_sandcastle()
{
	list scheme = {
		"scheme", "Sandcastle",
		"author", "George Essig (https://github.com/gessig)",
		"00", 0x282c34,
		"01", 0x2c323b,
		"02", 0x3e4451,
		"03", 0x665c54,
		"04", 0x928374,
		"05", 0xa89984,
		"06", 0xd5c4a1,
		"07", 0xfdf4c1,
		"08", 0x83a598,
		"09", 0xa07e3b,
		"0A", 0xa07e3b,
		"0B", 0x528b8b,
		"0C", 0x83a598,
		"0D", 0x83a598,
		"0E", 0xd75f5f,
		"0F", 0xa87322,
	};
	return scheme;
}

// atlas-license
// MIT License
// 
// Copyright (c) 2018 Alex Lende
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/ajlende/base16-atlas-scheme

static list
def_atlas()
{
	list scheme = {
		"scheme", "Atlas",
		"author", "Alex Lende (https://ajlende.com)",
		"00", 0x002635,
		"01", 0x00384d,
		"02", 0x517F8D,
		"03", 0x6C8B91,
		"04", 0x869696,
		"05", 0xa1a19a,
		"06", 0xe6e6dc,
		"07", 0xfafaf8,
		"08", 0xff5a67,
		"09", 0xf08e48,
		"0A", 0xffcc1b,
		"0B", 0x7fc06e,
		"0C", 0x5dd7b9,
		"0D", 0x14747e,
		"0E", 0x9a70a4,
		"0F", 0xc43060,
	};
	return scheme;
}

// synth-midnight-license
// MIT License
// 
// Copyright (c) 2019 Michaël Ball
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/michael-ball/base16-synth-midnight-scheme

static list
def_synth_midnight_dark()
{
	list scheme = {
		"scheme", "Synth Midnight Terminal Dark",
		"author", "Michaël Ball (http://github.com/michael-ball/)",
		"00", 0x050608,
		"01", 0x1a1b1c,
		"02", 0x28292a,
		"03", 0x474849,
		"04", 0xa3a5a6,
		"05", 0xc1c3c4,
		"06", 0xcfd1d2,
		"07", 0xdddfe0,
		"08", 0xb53b50,
		"09", 0xea770d,
		"0A", 0xc9d364,
		"0B", 0x06ea61,
		"0C", 0x42fff9,
		"0D", 0x03aeff,
		"0E", 0xea5ce2,
		"0F", 0xcd6320,
	};
	return scheme;
}

static list
def_synth_midnight_light()
{
	list scheme = {
		"scheme", "Synth Midnight Terminal Light",
		"author", "Michaël Ball (http://github.com/michael-ball/)",
		"00", 0xdddfe0,
		"01", 0xcfd1d2,
		"02", 0xc1c3c4,
		"03", 0xa3a5a6,
		"04", 0x474849,
		"05", 0x28292a,
		"06", 0x1a1b1c,
		"07", 0x050608,
		"08", 0xb53b50,
		"09", 0xea770d,
		"0A", 0xc9d364,
		"0B", 0x06ea61,
		"0C", 0x42fff9,
		"0D", 0x03aeff,
		"0E", 0xea5ce2,
		"0F", 0xcd6320,
	};
	return scheme;
}

// apprentice-license
// MIT License
// https://github.com/casonadams/base16-apprentice-scheme

static list
def_apprentice()
{
	list scheme = {
		"scheme", "Apprentice",
		"author", "romainl",
		"00", 0x262626,
		"01", 0xAF5F5F,
		"02", 0x5F875F,
		"03", 0x87875F,
		"04", 0x5F87AF,
		"05", 0x5F5F87,
		"06", 0x5F8787,
		"07", 0x6C6C6C,
		"08", 0x444444,
		"09", 0xFF8700,
		"0A", 0x87AF87,
		"0B", 0xFFFFAF,
		"0C", 0x87AFD7,
		"0D", 0x8787AF,
		"0E", 0x5FAFAF,
		"0F", 0xBCBCBC,
	};
	return scheme;
}

// circus-license
// # License
// 
// Copyright 2024 Stephan Boyer and Esther Wang
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// https://github.com/stepchowfun/base16-circus-scheme

static list
def_circus()
{
	list scheme = {
		"scheme", "Circus",
		"author", "Stephan Boyer (https://github.com/stepchowfun) and Esther Wang (https://github.com/ewang12)",
		"00", 0x191919,
		"01", 0x202020,
		"02", 0x303030,
		"03", 0x5f5a60,
		"04", 0x505050,
		"05", 0xa7a7a7,
		"06", 0x808080,
		"07", 0xffffff,
		"08", 0xdc657d,
		"09", 0x4bb1a7,
		"0A", 0xc3ba63,
		"0B", 0x84b97c,
		"0C", 0x4bb1a7,
		"0D", 0x639ee4,
		"0E", 0xb888e2,
		"0F", 0xb888e2,
	};
	return scheme;
}

// solarflare-license
// MIT License
// 
// Copyright (c) 2018 Michael Nussbaum
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/mnussbaum/base16-solarflare-scheme

static list
def_solarflare_light()
{
	list scheme = {
		"scheme", "Solar Flare Light",
		"author", "Chuck Harmston (https://chuck.harmston.ch)",
		"00", 0xF5F7FA,
		"01", 0xE8E9ED,
		"02", 0xA6AFB8,
		"03", 0x85939E,
		"04", 0x667581,
		"05", 0x586875,
		"06", 0x222E38,
		"07", 0x18262F,
		"08", 0xEF5253,
		"09", 0xE66B2B,
		"0A", 0xE4B51C,
		"0B", 0x7CC844,
		"0C", 0x52CBB0,
		"0D", 0x33B5E1,
		"0E", 0xA363D5,
		"0F", 0xD73C9A,
	};
	return scheme;
}

static list
def_solarflare()
{
	list scheme = {
		"scheme", "Solar Flare",
		"author", "Chuck Harmston (https://chuck.harmston.ch)",
		"00", 0x18262F,
		"01", 0x222E38,
		"02", 0x586875,
		"03", 0x667581,
		"04", 0x85939E,
		"05", 0xA6AFB8,
		"06", 0xE8E9ED,
		"07", 0xF5F7FA,
		"08", 0xEF5253,
		"09", 0xE66B2B,
		"0A", 0xE4B51C,
		"0B", 0x7CC844,
		"0C", 0x52CBB0,
		"0D", 0x33B5E1,
		"0E", 0xA363D5,
		"0F", 0xD73C9A,
	};
	return scheme;
}

// eva-license
// MIT License
// 
// Copyright (c) 2020 Jakapat Kannika
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/kjakapat/base16-eva-scheme

static list
def_eva_dim()
{
	list scheme = {
		"scheme", "Eva Dim",
		"author", "kjakapat (https://github.com/kjakapat)",
		"00", 0x2a3b4d,
		"01", 0x3d566f,
		"02", 0x4b6988,
		"03", 0x55799c,
		"04", 0x7e90a3,
		"05", 0x9fa2a6,
		"06", 0xd6d7d9,
		"07", 0xffffff,
		"08", 0xc4676c,
		"09", 0xff9966,
		"0A", 0xcfd05d,
		"0B", 0x5de561,
		"0C", 0x4b8f77,
		"0D", 0x1ae1dc,
		"0E", 0x9c6cd3,
		"0F", 0xbb64a9,
	};
	return scheme;
}

static list
def_eva()
{
	list scheme = {
		"scheme", "Eva",
		"author", "kjakapat (https://github.com/kjakapat)",
		"00", 0x2a3b4d,
		"01", 0x3d566f,
		"02", 0x4b6988,
		"03", 0x55799c,
		"04", 0x7e90a3,
		"05", 0x9fa2a6,
		"06", 0xd6d7d9,
		"07", 0xffffff,
		"08", 0xc4676c,
		"09", 0xff9966,
		"0A", 0xffff66,
		"0B", 0x66ff66,
		"0C", 0x4b8f77,
		"0D", 0x15f4ee,
		"0E", 0x9c6cd3,
		"0F", 0xbb64a9,
	};
	return scheme;
}

// snazzy-license
// MIT License
// 
// Copyright (c) 2018-present Chawye Hsu
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/h404bi/base16-snazzy-scheme

static list
def_snazzy()
{
	list scheme = {
		"scheme", "Snazzy",
		"author", "Chawye Hsu (https://github.com/chawyehsu) based on Hyper Snazzy Theme (https://github.com/sindresorhus/hyper-snazzy)",
		"00", 0x282a36,
		"01", 0x34353e,
		"02", 0x43454f,
		"03", 0x78787e,
		"04", 0xa5a5a9,
		"05", 0xe2e4e5,
		"06", 0xeff0eb,
		"07", 0xf1f1f0,
		"08", 0xff5c57,
		"09", 0xff9f43,
		"0A", 0xf3f99d,
		"0B", 0x5af78e,
		"0C", 0x9aedfe,
		"0D", 0x57c7ff,
		"0E", 0xff6ac1,
		"0F", 0xb2643c,
	};
	return scheme;
}

// rebecca-license
// MIT License
// https://github.com/vic/base16-rebecca

static list
def_rebecca()
{
	list scheme = {
		"scheme", "Rebecca",
		"author", "Victor Borja (http://github.com/vic) based on Rebecca Theme (http://github.com/vic/rebecca-theme)",
		"00", 0x292a44,
		"01", 0x663399,
		"02", 0x383a62,
		"03", 0x666699,
		"04", 0xa0a0c5,
		"05", 0xf1eff8,
		"06", 0xccccff,
		"07", 0x53495d,
		"08", 0xa0a0c5,
		"09", 0xefe4a1,
		"0A", 0xae81ff,
		"0B", 0x6dfedf,
		"0C", 0x8eaee0,
		"0D", 0x2de0a7,
		"0E", 0x7aa5ff,
		"0F", 0xff79c6,
	};
	return scheme;
}

// pasque-license
// MIT License
// 
// Copyright (c) 2020 Gabriel Fontes
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/Misterio77/base16-pasque-scheme

static list
def_pasque()
{
	list scheme = {
		"scheme", "Pasque",
		"author", "Gabriel Fontes (https://github.com/Misterio77)",
		"00", 0x271C3A,
		"01", 0x100323,
		"02", 0x3E2D5C,
		"03", 0x5D5766,
		"04", 0xBEBCBF,
		"05", 0xDEDCDF,
		"06", 0xEDEAEF,
		"07", 0xBBAADD,
		"08", 0xA92258,
		"09", 0x918889,
		"0A", 0x804ead,
		"0B", 0xC6914B,
		"0C", 0x7263AA,
		"0D", 0x8E7DC6,
		"0E", 0x953B9D,
		"0F", 0x59325C,
	};
	return scheme;
}

// equilibrium-license
// MIT License
// 
// Copyright (c) 2020 Carlo Abelli
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/carloabelli/base16-equilibrium-scheme

static list
def_equilibrium_dark()
{
	list scheme = {
		"scheme", "Equilibrium Dark",
		"author", "Carlo Abelli",
		"00", 0x0c1118,
		"01", 0x181c22,
		"02", 0x22262d,
		"03", 0x7b776e,
		"04", 0x949088,
		"05", 0xafaba2,
		"06", 0xcac6bd,
		"07", 0xe7e2d9,
		"08", 0xf04339,
		"09", 0xdf5923,
		"0A", 0xbb8801,
		"0B", 0x7f8b00,
		"0C", 0x00948b,
		"0D", 0x008dd1,
		"0E", 0x6a7fd2,
		"0F", 0xe3488e,
	};
	return scheme;
}

static list
def_equilibrium_gray_dark()
{
	list scheme = {
		"scheme", "Equilibrium Gray Dark",
		"author", "Carlo Abelli",
		"00", 0x111111,
		"01", 0x1b1b1b,
		"02", 0x262626,
		"03", 0x777777,
		"04", 0x919191,
		"05", 0xababab,
		"06", 0xc6c6c6,
		"07", 0xe2e2e2,
		"08", 0xf04339,
		"09", 0xdf5923,
		"0A", 0xbb8801,
		"0B", 0x7f8b00,
		"0C", 0x00948b,
		"0D", 0x008dd1,
		"0E", 0x6a7fd2,
		"0F", 0xe3488e,
	};
	return scheme;
}

static list
def_equilibrium_gray_light()
{
	list scheme = {
		"scheme", "Equilibrium Gray Light",
		"author", "Carlo Abelli",
		"00", 0xf1f1f1,
		"01", 0xe2e2e2,
		"02", 0xd4d4d4,
		"03", 0x777777,
		"04", 0x5e5e5e,
		"05", 0x474747,
		"06", 0x303030,
		"07", 0x1b1b1b,
		"08", 0xd02023,
		"09", 0xbf3e05,
		"0A", 0x9d6f00,
		"0B", 0x637200,
		"0C", 0x007a72,
		"0D", 0x0073b5,
		"0E", 0x4e66b6,
		"0F", 0xc42775,
	};
	return scheme;
}

static list
def_equilibrium_light()
{
	list scheme = {
		"scheme", "Equilibrium Light",
		"author", "Carlo Abelli",
		"00", 0xf5f0e7,
		"01", 0xe7e2d9,
		"02", 0xd8d4cb,
		"03", 0x73777f,
		"04", 0x5a5f66,
		"05", 0x43474e,
		"06", 0x2c3138,
		"07", 0x181c22,
		"08", 0xd02023,
		"09", 0xbf3e05,
		"0A", 0x9d6f00,
		"0B", 0x637200,
		"0C", 0x007a72,
		"0D", 0x0073b5,
		"0E", 0x4e66b6,
		"0F", 0xc42775,
	};
	return scheme;
}

// atelier-license
// The MIT License (MIT)
// 
// Copyright (c) 2016 Bram de Haan
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/atelierbram/base16-atelier-schemes

static list
def_atelier_cave_light()
{
	list scheme = {
		"scheme", "Atelier Cave Light",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0xefecf4,
		"01", 0xe2dfe7,
		"02", 0x8b8792,
		"03", 0x7e7887,
		"04", 0x655f6d,
		"05", 0x585260,
		"06", 0x26232a,
		"07", 0x19171c,
		"08", 0xbe4678,
		"09", 0xaa573c,
		"0A", 0xa06e3b,
		"0B", 0x2a9292,
		"0C", 0x398bc6,
		"0D", 0x576ddb,
		"0E", 0x955ae7,
		"0F", 0xbf40bf,
	};
	return scheme;
}

static list
def_atelier_cave()
{
	list scheme = {
		"scheme", "Atelier Cave",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0x19171c,
		"01", 0x26232a,
		"02", 0x585260,
		"03", 0x655f6d,
		"04", 0x7e7887,
		"05", 0x8b8792,
		"06", 0xe2dfe7,
		"07", 0xefecf4,
		"08", 0xbe4678,
		"09", 0xaa573c,
		"0A", 0xa06e3b,
		"0B", 0x2a9292,
		"0C", 0x398bc6,
		"0D", 0x576ddb,
		"0E", 0x955ae7,
		"0F", 0xbf40bf,
	};
	return scheme;
}

static list
def_atelier_dune_light()
{
	list scheme = {
		"scheme", "Atelier Dune Light",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0xfefbec,
		"01", 0xe8e4cf,
		"02", 0xa6a28c,
		"03", 0x999580,
		"04", 0x7d7a68,
		"05", 0x6e6b5e,
		"06", 0x292824,
		"07", 0x20201d,
		"08", 0xd73737,
		"09", 0xb65611,
		"0A", 0xae9513,
		"0B", 0x60ac39,
		"0C", 0x1fad83,
		"0D", 0x6684e1,
		"0E", 0xb854d4,
		"0F", 0xd43552,
	};
	return scheme;
}

static list
def_atelier_dune()
{
	list scheme = {
		"scheme", "Atelier Dune",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0x20201d,
		"01", 0x292824,
		"02", 0x6e6b5e,
		"03", 0x7d7a68,
		"04", 0x999580,
		"05", 0xa6a28c,
		"06", 0xe8e4cf,
		"07", 0xfefbec,
		"08", 0xd73737,
		"09", 0xb65611,
		"0A", 0xae9513,
		"0B", 0x60ac39,
		"0C", 0x1fad83,
		"0D", 0x6684e1,
		"0E", 0xb854d4,
		"0F", 0xd43552,
	};
	return scheme;
}

static list
def_atelier_estuary_light()
{
	list scheme = {
		"scheme", "Atelier Estuary Light",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0xf4f3ec,
		"01", 0xe7e6df,
		"02", 0x929181,
		"03", 0x878573,
		"04", 0x6c6b5a,
		"05", 0x5f5e4e,
		"06", 0x302f27,
		"07", 0x22221b,
		"08", 0xba6236,
		"09", 0xae7313,
		"0A", 0xa5980d,
		"0B", 0x7d9726,
		"0C", 0x5b9d48,
		"0D", 0x36a166,
		"0E", 0x5f9182,
		"0F", 0x9d6c7c,
	};
	return scheme;
}

static list
def_atelier_estuary()
{
	list scheme = {
		"scheme", "Atelier Estuary",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0x22221b,
		"01", 0x302f27,
		"02", 0x5f5e4e,
		"03", 0x6c6b5a,
		"04", 0x878573,
		"05", 0x929181,
		"06", 0xe7e6df,
		"07", 0xf4f3ec,
		"08", 0xba6236,
		"09", 0xae7313,
		"0A", 0xa5980d,
		"0B", 0x7d9726,
		"0C", 0x5b9d48,
		"0D", 0x36a166,
		"0E", 0x5f9182,
		"0F", 0x9d6c7c,
	};
	return scheme;
}

static list
def_atelier_forest_light()
{
	list scheme = {
		"scheme", "Atelier Forest Light",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0xf1efee,
		"01", 0xe6e2e0,
		"02", 0xa8a19f,
		"03", 0x9c9491,
		"04", 0x766e6b,
		"05", 0x68615e,
		"06", 0x2c2421,
		"07", 0x1b1918,
		"08", 0xf22c40,
		"09", 0xdf5320,
		"0A", 0xc38418,
		"0B", 0x7b9726,
		"0C", 0x3d97b8,
		"0D", 0x407ee7,
		"0E", 0x6666ea,
		"0F", 0xc33ff3,
	};
	return scheme;
}

static list
def_atelier_forest()
{
	list scheme = {
		"scheme", "Atelier Forest",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0x1b1918,
		"01", 0x2c2421,
		"02", 0x68615e,
		"03", 0x766e6b,
		"04", 0x9c9491,
		"05", 0xa8a19f,
		"06", 0xe6e2e0,
		"07", 0xf1efee,
		"08", 0xf22c40,
		"09", 0xdf5320,
		"0A", 0xc38418,
		"0B", 0x7b9726,
		"0C", 0x3d97b8,
		"0D", 0x407ee7,
		"0E", 0x6666ea,
		"0F", 0xc33ff3,
	};
	return scheme;
}

static list
def_atelier_heath_light()
{
	list scheme = {
		"scheme", "Atelier Heath Light",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0xf7f3f7,
		"01", 0xd8cad8,
		"02", 0xab9bab,
		"03", 0x9e8f9e,
		"04", 0x776977,
		"05", 0x695d69,
		"06", 0x292329,
		"07", 0x1b181b,
		"08", 0xca402b,
		"09", 0xa65926,
		"0A", 0xbb8a35,
		"0B", 0x918b3b,
		"0C", 0x159393,
		"0D", 0x516aec,
		"0E", 0x7b59c0,
		"0F", 0xcc33cc,
	};
	return scheme;
}

static list
def_atelier_heath()
{
	list scheme = {
		"scheme", "Atelier Heath",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0x1b181b,
		"01", 0x292329,
		"02", 0x695d69,
		"03", 0x776977,
		"04", 0x9e8f9e,
		"05", 0xab9bab,
		"06", 0xd8cad8,
		"07", 0xf7f3f7,
		"08", 0xca402b,
		"09", 0xa65926,
		"0A", 0xbb8a35,
		"0B", 0x918b3b,
		"0C", 0x159393,
		"0D", 0x516aec,
		"0E", 0x7b59c0,
		"0F", 0xcc33cc,
	};
	return scheme;
}

static list
def_atelier_lakeside_light()
{
	list scheme = {
		"scheme", "Atelier Lakeside Light",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0xebf8ff,
		"01", 0xc1e4f6,
		"02", 0x7ea2b4,
		"03", 0x7195a8,
		"04", 0x5a7b8c,
		"05", 0x516d7b,
		"06", 0x1f292e,
		"07", 0x161b1d,
		"08", 0xd22d72,
		"09", 0x935c25,
		"0A", 0x8a8a0f,
		"0B", 0x568c3b,
		"0C", 0x2d8f6f,
		"0D", 0x257fad,
		"0E", 0x6b6bb8,
		"0F", 0xb72dd2,
	};
	return scheme;
}

static list
def_atelier_lakeside()
{
	list scheme = {
		"scheme", "Atelier Lakeside",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0x161b1d,
		"01", 0x1f292e,
		"02", 0x516d7b,
		"03", 0x5a7b8c,
		"04", 0x7195a8,
		"05", 0x7ea2b4,
		"06", 0xc1e4f6,
		"07", 0xebf8ff,
		"08", 0xd22d72,
		"09", 0x935c25,
		"0A", 0x8a8a0f,
		"0B", 0x568c3b,
		"0C", 0x2d8f6f,
		"0D", 0x257fad,
		"0E", 0x6b6bb8,
		"0F", 0xb72dd2,
	};
	return scheme;
}

static list
def_atelier_plateau_light()
{
	list scheme = {
		"scheme", "Atelier Plateau Light",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0xf4ecec,
		"01", 0xe7dfdf,
		"02", 0x8a8585,
		"03", 0x7e7777,
		"04", 0x655d5d,
		"05", 0x585050,
		"06", 0x292424,
		"07", 0x1b1818,
		"08", 0xca4949,
		"09", 0xb45a3c,
		"0A", 0xa06e3b,
		"0B", 0x4b8b8b,
		"0C", 0x5485b6,
		"0D", 0x7272ca,
		"0E", 0x8464c4,
		"0F", 0xbd5187,
	};
	return scheme;
}

static list
def_atelier_plateau()
{
	list scheme = {
		"scheme", "Atelier Plateau",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0x1b1818,
		"01", 0x292424,
		"02", 0x585050,
		"03", 0x655d5d,
		"04", 0x7e7777,
		"05", 0x8a8585,
		"06", 0xe7dfdf,
		"07", 0xf4ecec,
		"08", 0xca4949,
		"09", 0xb45a3c,
		"0A", 0xa06e3b,
		"0B", 0x4b8b8b,
		"0C", 0x5485b6,
		"0D", 0x7272ca,
		"0E", 0x8464c4,
		"0F", 0xbd5187,
	};
	return scheme;
}

static list
def_atelier_savanna_light()
{
	list scheme = {
		"scheme", "Atelier Savanna Light",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0xecf4ee,
		"01", 0xdfe7e2,
		"02", 0x87928a,
		"03", 0x78877d,
		"04", 0x5f6d64,
		"05", 0x526057,
		"06", 0x232a25,
		"07", 0x171c19,
		"08", 0xb16139,
		"09", 0x9f713c,
		"0A", 0xa07e3b,
		"0B", 0x489963,
		"0C", 0x1c9aa0,
		"0D", 0x478c90,
		"0E", 0x55859b,
		"0F", 0x867469,
	};
	return scheme;
}

static list
def_atelier_savanna()
{
	list scheme = {
		"scheme", "Atelier Savanna",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0x171c19,
		"01", 0x232a25,
		"02", 0x526057,
		"03", 0x5f6d64,
		"04", 0x78877d,
		"05", 0x87928a,
		"06", 0xdfe7e2,
		"07", 0xecf4ee,
		"08", 0xb16139,
		"09", 0x9f713c,
		"0A", 0xa07e3b,
		"0B", 0x489963,
		"0C", 0x1c9aa0,
		"0D", 0x478c90,
		"0E", 0x55859b,
		"0F", 0x867469,
	};
	return scheme;
}

static list
def_atelier_seaside_light()
{
	list scheme = {
		"scheme", "Atelier Seaside Light",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0xf4fbf4,
		"01", 0xcfe8cf,
		"02", 0x8ca68c,
		"03", 0x809980,
		"04", 0x687d68,
		"05", 0x5e6e5e,
		"06", 0x242924,
		"07", 0x131513,
		"08", 0xe6193c,
		"09", 0x87711d,
		"0A", 0x98981b,
		"0B", 0x29a329,
		"0C", 0x1999b3,
		"0D", 0x3d62f5,
		"0E", 0xad2bee,
		"0F", 0xe619c3,
	};
	return scheme;
}

static list
def_atelier_seaside()
{
	list scheme = {
		"scheme", "Atelier Seaside",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0x131513,
		"01", 0x242924,
		"02", 0x5e6e5e,
		"03", 0x687d68,
		"04", 0x809980,
		"05", 0x8ca68c,
		"06", 0xcfe8cf,
		"07", 0xf4fbf4,
		"08", 0xe6193c,
		"09", 0x87711d,
		"0A", 0x98981b,
		"0B", 0x29a329,
		"0C", 0x1999b3,
		"0D", 0x3d62f5,
		"0E", 0xad2bee,
		"0F", 0xe619c3,
	};
	return scheme;
}

static list
def_atelier_sulphurpool_light()
{
	list scheme = {
		"scheme", "Atelier Sulphurpool Light",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0xf5f7ff,
		"01", 0xdfe2f1,
		"02", 0x979db4,
		"03", 0x898ea4,
		"04", 0x6b7394,
		"05", 0x5e6687,
		"06", 0x293256,
		"07", 0x202746,
		"08", 0xc94922,
		"09", 0xc76b29,
		"0A", 0xc08b30,
		"0B", 0xac9739,
		"0C", 0x22a2c9,
		"0D", 0x3d8fd1,
		"0E", 0x6679cc,
		"0F", 0x9c637a,
	};
	return scheme;
}

static list
def_atelier_sulphurpool()
{
	list scheme = {
		"scheme", "Atelier Sulphurpool",
		"author", "Bram de Haan (http://atelierbramdehaan.nl)",
		"00", 0x202746,
		"01", 0x293256,
		"02", 0x5e6687,
		"03", 0x6b7394,
		"04", 0x898ea4,
		"05", 0x979db4,
		"06", 0xdfe2f1,
		"07", 0xf5f7ff,
		"08", 0xc94922,
		"09", 0xc76b29,
		"0A", 0xc08b30,
		"0B", 0xac9739,
		"0C", 0x22a2c9,
		"0D", 0x3d8fd1,
		"0E", 0x6679cc,
		"0F", 0x9c637a,
	};
	return scheme;
}

// summerfruit-license
// MIT License
// 
// Copyright (c) 2016 Christopher Corley
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/cscorley/base16-summerfruit-scheme

static list
def_summerfruit_dark()
{
	list scheme = {
		"scheme", "Summerfruit Dark",
		"author", "Christopher Corley (http://christop.club/)",
		"00", 0x151515,
		"01", 0x202020,
		"02", 0x303030,
		"03", 0x505050,
		"04", 0xB0B0B0,
		"05", 0xD0D0D0,
		"06", 0xE0E0E0,
		"07", 0xFFFFFF,
		"08", 0xFF0086,
		"09", 0xFD8900,
		"0A", 0xABA800,
		"0B", 0x00C918,
		"0C", 0x1FAAAA,
		"0D", 0x3777E6,
		"0E", 0xAD00A1,
		"0F", 0xCC6633,
	};
	return scheme;
}

static list
def_summerfruit_light()
{
	list scheme = {
		"scheme", "Summerfruit Light",
		"author", "Christopher Corley (http://christop.club/)",
		"00", 0xFFFFFF,
		"01", 0xE0E0E0,
		"02", 0xD0D0D0,
		"03", 0xB0B0B0,
		"04", 0x000000,
		"05", 0x101010,
		"06", 0x151515,
		"07", 0x202020,
		"08", 0xFF0086,
		"09", 0xFD8900,
		"0A", 0xABA800,
		"0B", 0x00C918,
		"0C", 0x1FAAAA,
		"0D", 0x3777E6,
		"0E", 0xAD00A1,
		"0F", 0xCC6633,
	};
	return scheme;
}

// sakura-license
// MIT License
// 
// Copyright (c) 2020 Gabriel Fontes
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/Misterio77/base16-sakura-scheme

static list
def_sakura()
{
	list scheme = {
		"scheme", "Sakura",
		"author", "Misterio77 (http://github.com/Misterio77)",
		"00", 0xfeedf3,
		"01", 0xf8e2e7,
		"02", 0xe0ccd1,
		"03", 0x755f64,
		"04", 0x665055,
		"05", 0x564448,
		"06", 0x42383a,
		"07", 0x33292b,
		"08", 0xdf2d52,
		"09", 0xf6661e,
		"0A", 0xc29461,
		"0B", 0x2e916d,
		"0C", 0x1d8991,
		"0D", 0x006e93,
		"0E", 0x5e2180,
		"0F", 0xba0d35,
	};
	return scheme;
}

// materia-license
// MIT License
// https://github.com/Defman21/base16-materia

static list
def_materia()
{
	list scheme = {
		"scheme", "Materia",
		"author", "Defman21",
		"00", 0x263238,
		"01", 0x2C393F,
		"02", 0x37474F,
		"03", 0x707880,
		"04", 0xC9CCD3,
		"05", 0xCDD3DE,
		"06", 0xD5DBE5,
		"07", 0xFFFFFF,
		"08", 0xEC5F67,
		"09", 0xEA9560,
		"0A", 0xFFCC00,
		"0B", 0x8BD649,
		"0C", 0x80CBC4,
		"0D", 0x89DDFF,
		"0E", 0x82AAFF,
		"0F", 0xEC5F67,
	};
	return scheme;
}

// tender-license
// MIT License
// 
// Copyright (c) 2020 Daniel Nakhimovich
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/DanManN/base16-tender-scheme

static list
def_tender()
{
	list scheme = {
		"scheme", "tender",
		"author", "Jacobo Tabernero (https://github/com/jacoborus/tender.vim)",
		"00", 0x282828,
		"01", 0x383838,
		"02", 0x484848,
		"03", 0x4c4c4c,
		"04", 0xb8b8b8,
		"05", 0xeeeeee,
		"06", 0xe8e8e8,
		"07", 0xfeffff,
		"08", 0xf43753,
		"09", 0xdc9656,
		"0A", 0xffc24b,
		"0B", 0xc9d05c,
		"0C", 0x73cef4,
		"0D", 0xb3deef,
		"0E", 0xd3b987,
		"0F", 0xa16946,
	};
	return scheme;
}

// materialtheme-license
// MIT License
// 
// Copyright (c) 2017 Nathan Peterson
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/ntpeters/base16-materialtheme-scheme

static list
def_material_darker()
{
	list scheme = {
		"scheme", "Material Darker",
		"author", "Nate Peterson",
		"00", 0x212121,
		"01", 0x303030,
		"02", 0x353535,
		"03", 0x4A4A4A,
		"04", 0xB2CCD6,
		"05", 0xEEFFFF,
		"06", 0xEEFFFF,
		"07", 0xFFFFFF,
		"08", 0xF07178,
		"09", 0xF78C6C,
		"0A", 0xFFCB6B,
		"0B", 0xC3E88D,
		"0C", 0x89DDFF,
		"0D", 0x82AAFF,
		"0E", 0xC792EA,
		"0F", 0xFF5370,
	};
	return scheme;
}

static list
def_material_lighter()
{
	list scheme = {
		"scheme", "Material Lighter",
		"author", "Nate Peterson",
		"00", 0xFAFAFA,
		"01", 0xE7EAEC,
		"02", 0xCCEAE7,
		"03", 0xCCD7DA,
		"04", 0x8796B0,
		"05", 0x80CBC4,
		"06", 0x80CBC4,
		"07", 0xFFFFFF,
		"08", 0xFF5370,
		"09", 0xF76D47,
		"0A", 0xFFB62C,
		"0B", 0x91B859,
		"0C", 0x39ADB5,
		"0D", 0x6182B8,
		"0E", 0x7C4DFF,
		"0F", 0xE53935,
	};
	return scheme;
}

static list
def_material_palenight()
{
	list scheme = {
		"scheme", "Material Palenight",
		"author", "Nate Peterson",
		"00", 0x292D3E,
		"01", 0x444267,
		"02", 0x32374D,
		"03", 0x676E95,
		"04", 0x8796B0,
		"05", 0x959DCB,
		"06", 0x959DCB,
		"07", 0xFFFFFF,
		"08", 0xF07178,
		"09", 0xF78C6C,
		"0A", 0xFFCB6B,
		"0B", 0xC3E88D,
		"0C", 0x89DDFF,
		"0D", 0x82AAFF,
		"0E", 0xC792EA,
		"0F", 0xFF5370,
	};
	return scheme;
}

static list
def_material()
{
	list scheme = {
		"scheme", "Material",
		"author", "Nate Peterson",
		"00", 0x263238,
		"01", 0x2E3C43,
		"02", 0x314549,
		"03", 0x546E7A,
		"04", 0xB2CCD6,
		"05", 0xEEFFFF,
		"06", 0xEEFFFF,
		"07", 0xFFFFFF,
		"08", 0xF07178,
		"09", 0xF78C6C,
		"0A", 0xFFCB6B,
		"0B", 0xC3E88D,
		"0C", 0x89DDFF,
		"0D", 0x82AAFF,
		"0E", 0xC792EA,
		"0F", 0xFF5370,
	};
	return scheme;
}

// kimber-license
// Base16-kimber-scheme is released under the MIT License:
// 
// Copyright © 2020 [Kenny Nguyen](https://github.com/akhsiM)
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// https://github.com/akhsiM/base16-kimber-scheme

static list
def_kimber()
{
	list scheme = {
		"scheme", "Kimber",
		"author", "Mishka Nguyen (https://github.com/akhsiM)",
		"00", 0x222222,
		"01", 0x313131,
		"02", 0x555D55,
		"03", 0x644646,
		"04", 0x5A5A5A,
		"05", 0xDEDEE7,
		"06", 0xC3C3B4,
		"07", 0xFFFFE6,
		"08", 0xC88C8C,
		"09", 0x476C88,
		"0A", 0xD8B56D,
		"0B", 0x99C899,
		"0C", 0x78B4B4,
		"0D", 0x537C9C,
		"0E", 0x86CACD,
		"0F", 0x704F4F,
	};
	return scheme;
}

// espresso-license
// MIT License
// 
// Copyright (c) 2019 Alex Mirrington
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// This license covers the base16-espresso-scheme repository.
// 
// The copyright/license for each individual theme belongs to the author of that theme, as follows:
// 
// The contents of espresso.yaml were adapted from the iTerm2 Espresso theme, located here: https://github.com/mbadolato/iTerm2-Color-Schemes, which was in turn ported from the original MacOS Terminal.app theme. The greyscale colours in the scheme were pulled from Chris Kempson's Tomorrow themes, located here: https://github.com/chriskempson/base16-tomorrow-scheme.
// 
// The colours in decaf.yaml were created by Alex Mirrington, inspired by espresso.yaml, and the greyscale colours were pulled from Chris Kempson's Tomorrow theme, located here: https://github.com/chriskempson/base16-tomorrow-scheme.
// https://github.com/alexmirrington/base16-espresso-scheme

static list
def_decaf()
{
	list scheme = {
		"scheme", "Decaf",
		"author", "Alex Mirrington (https://github.com/alexmirrington)",
		"00", 0x2d2d2d,
		"01", 0x393939,
		"02", 0x515151,
		"03", 0x777777,
		"04", 0xb4b7b4,
		"05", 0xcccccc,
		"06", 0xe0e0e0,
		"07", 0xffffff,
		"08", 0xff7f7b,
		"09", 0xffbf70,
		"0A", 0xffd67c,
		"0B", 0xbeda78,
		"0C", 0xbed6ff,
		"0D", 0x90bee1,
		"0E", 0xefb3f7,
		"0F", 0xff93b3,
	};
	return scheme;
}

static list
def_espresso()
{
	list scheme = {
		"scheme", "Espresso",
		"author", "Unknown. Maintained by Alex Mirrington (https://github.com/alexmirrington)",
		"00", 0x2d2d2d,
		"01", 0x393939,
		"02", 0x515151,
		"03", 0x777777,
		"04", 0xb4b7b4,
		"05", 0xcccccc,
		"06", 0xe0e0e0,
		"07", 0xffffff,
		"08", 0xd25252,
		"09", 0xf9a959,
		"0A", 0xffc66d,
		"0B", 0xa5c261,
		"0C", 0xbed6ff,
		"0D", 0x6c99bb,
		"0E", 0xd197d9,
		"0F", 0xf97394,
	};
	return scheme;
}

// nova-license
// MIT License
// 
// Copyright (c) 2019 George Essig
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/gessig/base16-nova-scheme

static list
def_nova()
{
	list scheme = {
		"scheme", "Nova",
		"author", "George Essig (https://github.com/gessig), Trevor D. Miller (https://trevordmiller.com)",
		"00", 0x3C4C55,
		"01", 0x556873,
		"02", 0x6A7D89,
		"03", 0x899BA6,
		"04", 0x899BA6,
		"05", 0xC5D4DD,
		"06", 0x899BA6,
		"07", 0x556873,
		"08", 0x83AFE5,
		"09", 0x7FC1CA,
		"0A", 0xA8CE93,
		"0B", 0x7FC1CA,
		"0C", 0xF2C38F,
		"0D", 0x83AFE5,
		"0E", 0x9A93E1,
		"0F", 0xF2C38F,
	};
	return scheme;
}

// vulcan-license
// MIT License
// https://github.com/andreyvpng/base16-vulcan-scheme

static list
def_vulcan()
{
	list scheme = {
		"scheme", "vulcan",
		"author", "Andrey Varfolomeev",
		"00", 0x041523,
		"01", 0x122339,
		"02", 0x003552,
		"03", 0x7a5759,
		"04", 0x6b6977,
		"05", 0x5b778c,
		"06", 0x333238,
		"07", 0x214d68,
		"08", 0x818591,
		"09", 0x9198a3,
		"0A", 0xadb4b9,
		"0B", 0x977d7c,
		"0C", 0x977d7c,
		"0D", 0x977d7c,
		"0E", 0x9198a3,
		"0F", 0x977d7c,
	};
	return scheme;
}

// black-metal-license
// Copyright 2020 Andrea Schiavini
// 
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// https://github.com/metalelf0/base16-black-metal-scheme

static list
def_black_metal_bathory()
{
	list scheme = {
		"scheme", "Black Metal (Bathory)",
		"author", "metalelf0 (https://github.com/metalelf0)",
		"00", 0x000000,
		"01", 0x121212,
		"02", 0x222222,
		"03", 0x333333,
		"04", 0x999999,
		"05", 0xc1c1c1,
		"06", 0x999999,
		"07", 0xc1c1c1,
		"08", 0x5f8787,
		"09", 0xaaaaaa,
		"0A", 0xe78a53,
		"0B", 0xfbcb97,
		"0C", 0xaaaaaa,
		"0D", 0x888888,
		"0E", 0x999999,
		"0F", 0x444444,
	};
	return scheme;
}

static list
def_black_metal_burzum()
{
	list scheme = {
		"scheme", "Black Metal (Burzum)",
		"author", "metalelf0 (https://github.com/metalelf0)",
		"00", 0x000000,
		"01", 0x121212,
		"02", 0x222222,
		"03", 0x333333,
		"04", 0x999999,
		"05", 0xc1c1c1,
		"06", 0x999999,
		"07", 0xc1c1c1,
		"08", 0x5f8787,
		"09", 0xaaaaaa,
		"0A", 0x99bbaa,
		"0B", 0xddeecc,
		"0C", 0xaaaaaa,
		"0D", 0x888888,
		"0E", 0x999999,
		"0F", 0x444444,
	};
	return scheme;
}

static list
def_black_metal_dark_funeral()
{
	list scheme = {
		"scheme", "Black Metal (Dark Funeral)",
		"author", "metalelf0 (https://github.com/metalelf0)",
		"00", 0x000000,
		"01", 0x121212,
		"02", 0x222222,
		"03", 0x333333,
		"04", 0x999999,
		"05", 0xc1c1c1,
		"06", 0x999999,
		"07", 0xc1c1c1,
		"08", 0x5f8787,
		"09", 0xaaaaaa,
		"0A", 0x5f81a5,
		"0B", 0xd0dfee,
		"0C", 0xaaaaaa,
		"0D", 0x888888,
		"0E", 0x999999,
		"0F", 0x444444,
	};
	return scheme;
}

static list
def_black_metal_gorgoroth()
{
	list scheme = {
		"scheme", "Black Metal (Gorgoroth)",
		"author", "metalelf0 (https://github.com/metalelf0)",
		"00", 0x000000,
		"01", 0x121212,
		"02", 0x222222,
		"03", 0x333333,
		"04", 0x999999,
		"05", 0xc1c1c1,
		"06", 0x999999,
		"07", 0xc1c1c1,
		"08", 0x5f8787,
		"09", 0xaaaaaa,
		"0A", 0x8c7f70,
		"0B", 0x9b8d7f,
		"0C", 0xaaaaaa,
		"0D", 0x888888,
		"0E", 0x999999,
		"0F", 0x444444,
	};
	return scheme;
}

static list
def_black_metal_immortal()
{
	list scheme = {
		"scheme", "Black Metal (Immortal)",
		"author", "metalelf0 (https://github.com/metalelf0)",
		"00", 0x000000,
		"01", 0x121212,
		"02", 0x222222,
		"03", 0x333333,
		"04", 0x999999,
		"05", 0xc1c1c1,
		"06", 0x999999,
		"07", 0xc1c1c1,
		"08", 0x5f8787,
		"09", 0xaaaaaa,
		"0A", 0x556677,
		"0B", 0x7799bb,
		"0C", 0xaaaaaa,
		"0D", 0x888888,
		"0E", 0x999999,
		"0F", 0x444444,
	};
	return scheme;
}

static list
def_black_metal_khold()
{
	list scheme = {
		"scheme", "Black Metal (Khold)",
		"author", "metalelf0 (https://github.com/metalelf0)",
		"00", 0x000000,
		"01", 0x121212,
		"02", 0x222222,
		"03", 0x333333,
		"04", 0x999999,
		"05", 0xc1c1c1,
		"06", 0x999999,
		"07", 0xc1c1c1,
		"08", 0x5f8787,
		"09", 0xaaaaaa,
		"0A", 0x974b46,
		"0B", 0xeceee3,
		"0C", 0xaaaaaa,
		"0D", 0x888888,
		"0E", 0x999999,
		"0F", 0x444444,
	};
	return scheme;
}

static list
def_black_metal_marduk()
{
	list scheme = {
		"scheme", "Black Metal (Marduk)",
		"author", "metalelf0 (https://github.com/metalelf0)",
		"00", 0x000000,
		"01", 0x121212,
		"02", 0x222222,
		"03", 0x333333,
		"04", 0x999999,
		"05", 0xc1c1c1,
		"06", 0x999999,
		"07", 0xc1c1c1,
		"08", 0x5f8787,
		"09", 0xaaaaaa,
		"0A", 0x626b67,
		"0B", 0xa5aaa7,
		"0C", 0xaaaaaa,
		"0D", 0x888888,
		"0E", 0x999999,
		"0F", 0x444444,
	};
	return scheme;
}

static list
def_black_metal_mayhem()
{
	list scheme = {
		"scheme", "Black Metal (Mayhem)",
		"author", "metalelf0 (https://github.com/metalelf0)",
		"00", 0x000000,
		"01", 0x121212,
		"02", 0x222222,
		"03", 0x333333,
		"04", 0x999999,
		"05", 0xc1c1c1,
		"06", 0x999999,
		"07", 0xc1c1c1,
		"08", 0x5f8787,
		"09", 0xaaaaaa,
		"0A", 0xeecc6c,
		"0B", 0xf3ecd4,
		"0C", 0xaaaaaa,
		"0D", 0x888888,
		"0E", 0x999999,
		"0F", 0x444444,
	};
	return scheme;
}

static list
def_black_metal_nile()
{
	list scheme = {
		"scheme", "Black Metal (Nile)",
		"author", "metalelf0 (https://github.com/metalelf0)",
		"00", 0x000000,
		"01", 0x121212,
		"02", 0x222222,
		"03", 0x333333,
		"04", 0x999999,
		"05", 0xc1c1c1,
		"06", 0x999999,
		"07", 0xc1c1c1,
		"08", 0x5f8787,
		"09", 0xaaaaaa,
		"0A", 0x777755,
		"0B", 0xaa9988,
		"0C", 0xaaaaaa,
		"0D", 0x888888,
		"0E", 0x999999,
		"0F", 0x444444,
	};
	return scheme;
}

static list
def_black_metal_venom()
{
	list scheme = {
		"scheme", "Black Metal (Venom)",
		"author", "metalelf0 (https://github.com/metalelf0)",
		"00", 0x000000,
		"01", 0x121212,
		"02", 0x222222,
		"03", 0x333333,
		"04", 0x999999,
		"05", 0xc1c1c1,
		"06", 0x999999,
		"07", 0xc1c1c1,
		"08", 0x5f8787,
		"09", 0xaaaaaa,
		"0A", 0x79241f,
		"0B", 0xf8f7f2,
		"0C", 0xaaaaaa,
		"0D", 0x888888,
		"0E", 0x999999,
		"0F", 0x444444,
	};
	return scheme;
}

static list
def_black_metal()
{
	list scheme = {
		"scheme", "Black Metal",
		"author", "metalelf0 (https://github.com/metalelf0)",
		"00", 0x000000,
		"01", 0x121212,
		"02", 0x222222,
		"03", 0x333333,
		"04", 0x999999,
		"05", 0xc1c1c1,
		"06", 0x999999,
		"07", 0xc1c1c1,
		"08", 0x5f8787,
		"09", 0xaaaaaa,
		"0A", 0xa06666,
		"0B", 0xdd9999,
		"0C", 0xaaaaaa,
		"0D", 0x888888,
		"0E", 0x999999,
		"0F", 0x444444,
	};
	return scheme;
}

// heetch-license
// MIT License
// 
// Copyright (c) 2018 Geoffrey J. Teale
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/tealeg/base16-heetch-scheme

static list
def_heetch_light()
{
	list scheme = {
		"scheme", "Heetch Light",
		"author", "Geoffrey Teale (tealeg@gmail.com)",
		"00", 0xfeffff,
		"01", 0x392551,
		"02", 0x7b6d8b,
		"03", 0x9c92a8,
		"04", 0xddd6e5,
		"05", 0x5a496e,
		"06", 0x470546,
		"07", 0x190134,
		"08", 0x27d9d5,
		"09", 0xbdb6c5,
		"0A", 0x5ba2b6,
		"0B", 0xf80059,
		"0C", 0xc33678,
		"0D", 0x47f9f5,
		"0E", 0xbd0152,
		"0F", 0xdedae2,
	};
	return scheme;
}

static list
def_heetch()
{
	list scheme = {
		"scheme", "Heetch Dark",
		"author", "Geoffrey Teale (tealeg@gmail.com)",
		"00", 0x190134,
		"01", 0x392551,
		"02", 0x5A496E,
		"03", 0x7B6D8B,
		"04", 0x9C92A8,
		"05", 0xBDB6C5,
		"06", 0xDEDAE2,
		"07", 0xFEFFFF,
		"08", 0x27D9D5,
		"09", 0x5BA2B6,
		"0A", 0x8F6C97,
		"0B", 0xC33678,
		"0C", 0xF80059,
		"0D", 0xBD0152,
		"0E", 0x82034C,
		"0F", 0x470546,
	};
	return scheme;
}

// rose-pine-license
// MIT License
// 
// Copyright (c) 2021 Emilia Dunfelt
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/edunfelt/base16-rose-pine-scheme

static list
def_rose_pine_dawn()
{
	list scheme = {
		"scheme", "Rosé Pine Dawn",
		"author", "Emilia Dunfelt <edun@dunfelt.se>",
		"00", 0xfaf4ed,
		"01", 0xfffaf3,
		"02", 0xf2e9de,
		"03", 0x9893a5,
		"04", 0x797593,
		"05", 0x575279,
		"06", 0x575279,
		"07", 0xcecacd,
		"08", 0xb4637a,
		"09", 0xea9d34,
		"0A", 0xd7827e,
		"0B", 0x286983,
		"0C", 0x56949f,
		"0D", 0x907aa9,
		"0E", 0xea9d34,
		"0F", 0xcecacd,
	};
	return scheme;
}

static list
def_rose_pine_moon()
{
	list scheme = {
		"scheme", "Rosé Pine Moon",
		"author", "Emilia Dunfelt <edun@dunfelt.se>",
		"00", 0x232136,
		"01", 0x2a273f,
		"02", 0x393552,
		"03", 0x6e6a86,
		"04", 0x908caa,
		"05", 0xe0def4,
		"06", 0xe0def4,
		"07", 0x56526e,
		"08", 0xeb6f92,
		"09", 0xf6c177,
		"0A", 0xea9a97,
		"0B", 0x3e8fb0,
		"0C", 0x9ccfd8,
		"0D", 0xc4a7e7,
		"0E", 0xf6c177,
		"0F", 0x56526e,
	};
	return scheme;
}

static list
def_rose_pine()
{
	list scheme = {
		"scheme", "Rosé Pine",
		"author", "Emilia Dunfelt <edun@dunfelt.se>",
		"00", 0x191724,
		"01", 0x1f1d2e,
		"02", 0x26233a,
		"03", 0x6e6a86,
		"04", 0x908caa,
		"05", 0xe0def4,
		"06", 0xe0def4,
		"07", 0x524f67,
		"08", 0xeb6f92,
		"09", 0xf6c177,
		"0A", 0xebbcba,
		"0B", 0x31748f,
		"0C", 0x9ccfd8,
		"0D", 0xc4a7e7,
		"0E", 0xf6c177,
		"0F", 0x524f67,
	};
	return scheme;
}

// windows-license
// MIT License
// 
// Copyright (c) 2021 C-Fergus
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/C-Fergus/base16-windows-scheme

static list
def_windows_10_light()
{
	list scheme = {
		"scheme", "Windows 10 Light",
		"author", "Fergus Collins (https://github.com/C-Fergus)",
		"00", 0xf2f2f2,
		"01", 0xe5e5e5,
		"02", 0xd9d9d9,
		"03", 0xcccccc,
		"04", 0xababab,
		"05", 0x767676,
		"06", 0x414141,
		"07", 0x0c0c0c,
		"08", 0xc50f1f,
		"09", 0xf9f1a5,
		"0A", 0xc19c00,
		"0B", 0x13a10e,
		"0C", 0x3a96dd,
		"0D", 0x0037da,
		"0E", 0x881798,
		"0F", 0x16c60c,
	};
	return scheme;
}

static list
def_windows_10()
{
	list scheme = {
		"scheme", "Windows 10",
		"author", "Fergus Collins (https://github.com/C-Fergus)",
		"00", 0x0c0c0c,
		"01", 0x2f2f2f,
		"02", 0x535353,
		"03", 0x767676,
		"04", 0xb9b9b9,
		"05", 0xcccccc,
		"06", 0xdfdfdf,
		"07", 0xf2f2f2,
		"08", 0xe74856,
		"09", 0xc19c00,
		"0A", 0xf9f1a5,
		"0B", 0x16c60c,
		"0C", 0x61d6d6,
		"0D", 0x3b78ff,
		"0E", 0xb4009e,
		"0F", 0x13a10e,
	};
	return scheme;
}

static list
def_windows_95_light()
{
	list scheme = {
		"scheme", "Windows 95 Light",
		"author", "Fergus Collins (https://github.com/C-Fergus)",
		"00", 0xfcfcfc,
		"01", 0xe0e0e0,
		"02", 0xc4c4c4,
		"03", 0xa8a8a8,
		"04", 0x7e7e7e,
		"05", 0x545454,
		"06", 0x2a2a2a,
		"07", 0x000000,
		"08", 0xa80000,
		"09", 0xfcfc54,
		"0A", 0xa85400,
		"0B", 0x00a800,
		"0C", 0x00a8a8,
		"0D", 0x0000a8,
		"0E", 0xa800a8,
		"0F", 0x54fc54,
	};
	return scheme;
}

static list
def_windows_95()
{
	list scheme = {
		"scheme", "Windows 95",
		"author", "Fergus Collins (https://github.com/C-Fergus)",
		"00", 0x000000,
		"01", 0x1C1C1C,
		"02", 0x383838,
		"03", 0x545454,
		"04", 0x7e7e7e,
		"05", 0xa8a8a8,
		"06", 0xd2d2d2,
		"07", 0xfcfcfc,
		"08", 0xfc5454,
		"09", 0xa85400,
		"0A", 0xfcfc54,
		"0B", 0x54fc54,
		"0C", 0x54fcfc,
		"0D", 0x5454fc,
		"0E", 0xfc54fc,
		"0F", 0x00a800,
	};
	return scheme;
}

static list
def_windows_highcontrast_light()
{
	list scheme = {
		"scheme", "Windows High Contrast Light",
		"author", "Fergus Collins (https://github.com/C-Fergus)",
		"00", 0xfcfcfc,
		"01", 0xe8e8e8,
		"02", 0xd4d4d4,
		"03", 0xc0c0c0,
		"04", 0x7e7e7e,
		"05", 0x545454,
		"06", 0x2a2a2a,
		"07", 0x000000,
		"08", 0x800000,
		"09", 0xfcfc54,
		"0A", 0x808000,
		"0B", 0x008000,
		"0C", 0x008080,
		"0D", 0x000080,
		"0E", 0x800080,
		"0F", 0x54fc54,
	};
	return scheme;
}

static list
def_windows_highcontrast()
{
	list scheme = {
		"scheme", "Windows High Contrast",
		"author", "Fergus Collins (https://github.com/C-Fergus)",
		"00", 0x000000,
		"01", 0x1C1C1C,
		"02", 0x383838,
		"03", 0x545454,
		"04", 0xa2a2a2,
		"05", 0xc0c0c0,
		"06", 0xdedede,
		"07", 0xfcfcfc,
		"08", 0xfc5454,
		"09", 0x808000,
		"0A", 0xfcfc54,
		"0B", 0x54fc54,
		"0C", 0x54fcfc,
		"0D", 0x5454fc,
		"0E", 0xfc54fc,
		"0F", 0x008000,
	};
	return scheme;
}

static list
def_windows_nt_light()
{
	list scheme = {
		"scheme", "Windows NT Light",
		"author", "Fergus Collins (https://github.com/C-Fergus)",
		"00", 0xffffff,
		"01", 0xeaeaea,
		"02", 0xd5d5d5,
		"03", 0xc0c0c0,
		"04", 0xa0a0a0,
		"05", 0x808080,
		"06", 0x404040,
		"07", 0x000000,
		"08", 0x800000,
		"09", 0xffff00,
		"0A", 0x808000,
		"0B", 0x008000,
		"0C", 0x008080,
		"0D", 0x000080,
		"0E", 0x800080,
		"0F", 0x00ff00,
	};
	return scheme;
}

static list
def_windows_nt()
{
	list scheme = {
		"scheme", "Windows NT",
		"author", "Fergus Collins (https://github.com/C-Fergus)",
		"00", 0x000000,
		"01", 0x2a2a2a,
		"02", 0x555555,
		"03", 0x808080,
		"04", 0xa1a1a1,
		"05", 0xc0c0c0,
		"06", 0xe0e0e0,
		"07", 0xffffff,
		"08", 0xff0000,
		"09", 0x808000,
		"0A", 0xffff00,
		"0B", 0x00ff00,
		"0C", 0x00ffff,
		"0D", 0x0000ff,
		"0E", 0xff00ff,
		"0F", 0x008000,
	};
	return scheme;
}

// codeschool-license
// MIT License
// 
// Copyright (c) 2018 Brett
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/blockloop/base16-codeschool-scheme

static list
def_codeschool()
{
	list scheme = {
		"scheme", "Codeschool",
		"author", "blockloop",
		"00", 0x232c31,
		"01", 0x1c3657,
		"02", 0x2a343a,
		"03", 0x3f4944,
		"04", 0x84898c,
		"05", 0x9ea7a6,
		"06", 0xa7cfa3,
		"07", 0xb5d8f6,
		"08", 0x2a5491,
		"09", 0x43820d,
		"0A", 0xa03b1e,
		"0B", 0x237986,
		"0C", 0xb02f30,
		"0D", 0x484d79,
		"0E", 0xc59820,
		"0F", 0xc98344,
	};
	return scheme;
}

// porple-license
// MIT License
// 
// Copyright (c) 2017 Niek den Breeje
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/AuditeMarlow/base16-porple-scheme

static list
def_porple()
{
	list scheme = {
		"scheme", "Porple",
		"author", "Niek den Breeje (https://github.com/AuditeMarlow)",
		"00", 0x292c36,
		"01", 0x333344,
		"02", 0x474160,
		"03", 0x65568a,
		"04", 0xb8b8b8,
		"05", 0xd8d8d8,
		"06", 0xe8e8e8,
		"07", 0xf8f8f8,
		"08", 0xf84547,
		"09", 0xd28e5d,
		"0A", 0xefa16b,
		"0B", 0x95c76f,
		"0C", 0x64878f,
		"0D", 0x8485ce,
		"0E", 0xb74989,
		"0F", 0x986841,
	};
	return scheme;
}

// helios-license
// MIT License
// 
// Copyright (c) 2019 Alex Meyer
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/reyemxela/base16-helios-scheme

static list
def_helios()
{
	list scheme = {
		"scheme", "Helios",
		"author", "Alex Meyer (https://github.com/reyemxela)",
		"00", 0x1d2021,
		"01", 0x383c3e,
		"02", 0x53585b,
		"03", 0x6f7579,
		"04", 0xcdcdcd,
		"05", 0xd5d5d5,
		"06", 0xdddddd,
		"07", 0xe5e5e5,
		"08", 0xd72638,
		"09", 0xeb8413,
		"0A", 0xf19d1a,
		"0B", 0x88b92d,
		"0C", 0x1ba595,
		"0D", 0x1e8bac,
		"0E", 0xbe4264,
		"0F", 0xc85e0d,
	};
	return scheme;
}

// unclaimed-license
// MIT License
// https://github.com/chriskempson/base16-unclaimed-schemes

static list
def_3024()
{
	list scheme = {
		"scheme", "3024",
		"author", "Jan T. Sott (http://github.com/idleberg)",
		"00", 0x090300,
		"01", 0x3a3432,
		"02", 0x4a4543,
		"03", 0x5c5855,
		"04", 0x807d7c,
		"05", 0xa5a2a2,
		"06", 0xd6d5d4,
		"07", 0xf7f7f7,
		"08", 0xdb2d20,
		"09", 0xe8bbd0,
		"0A", 0xfded02,
		"0B", 0x01a252,
		"0C", 0xb5e4f4,
		"0D", 0x01a0e4,
		"0E", 0xa16a94,
		"0F", 0xcdab53,
	};
	return scheme;
}

static list
def_apathy()
{
	list scheme = {
		"scheme", "Apathy",
		"author", "Jannik Siebert (https://github.com/janniks)",
		"00", 0x031A16,
		"01", 0x0B342D,
		"02", 0x184E45,
		"03", 0x2B685E,
		"04", 0x5F9C92,
		"05", 0x81B5AC,
		"06", 0xA7CEC8,
		"07", 0xD2E7E4,
		"08", 0x3E9688,
		"09", 0x3E7996,
		"0A", 0x3E4C96,
		"0B", 0x883E96,
		"0C", 0x963E4C,
		"0D", 0x96883E,
		"0E", 0x4C963E,
		"0F", 0x3E965B,
	};
	return scheme;
}

static list
def_ashes()
{
	list scheme = {
		"scheme", "Ashes",
		"author", "Jannik Siebert (https://github.com/janniks)",
		"00", 0x1C2023,
		"01", 0x393F45,
		"02", 0x565E65,
		"03", 0x747C84,
		"04", 0xADB3BA,
		"05", 0xC7CCD1,
		"06", 0xDFE2E5,
		"07", 0xF3F4F5,
		"08", 0xC7AE95,
		"09", 0xC7C795,
		"0A", 0xAEC795,
		"0B", 0x95C7AE,
		"0C", 0x95AEC7,
		"0D", 0xAE95C7,
		"0E", 0xC795AE,
		"0F", 0xC79595,
	};
	return scheme;
}

static list
def_bespin()
{
	list scheme = {
		"scheme", "Bespin",
		"author", "Jan T. Sott",
		"00", 0x28211c,
		"01", 0x36312e,
		"02", 0x5e5d5c,
		"03", 0x666666,
		"04", 0x797977,
		"05", 0x8a8986,
		"06", 0x9d9b97,
		"07", 0xbaae9e,
		"08", 0xcf6a4c,
		"09", 0xcf7d34,
		"0A", 0xf9ee98,
		"0B", 0x54be0d,
		"0C", 0xafc4db,
		"0D", 0x5ea6ea,
		"0E", 0x9b859d,
		"0F", 0x937121,
	};
	return scheme;
}

static list
def_brewer()
{
	list scheme = {
		"scheme", "Brewer",
		"author", "Timothée Poisot (http://github.com/tpoisot)",
		"00", 0x0c0d0e,
		"01", 0x2e2f30,
		"02", 0x515253,
		"03", 0x737475,
		"04", 0x959697,
		"05", 0xb7b8b9,
		"06", 0xdadbdc,
		"07", 0xfcfdfe,
		"08", 0xe31a1c,
		"09", 0xe6550d,
		"0A", 0xdca060,
		"0B", 0x31a354,
		"0C", 0x80b1d3,
		"0D", 0x3182bd,
		"0E", 0x756bb1,
		"0F", 0xb15928,
	};
	return scheme;
}

static list
def_bright()
{
	list scheme = {
		"scheme", "Bright",
		"author", "Chris Kempson (http://chriskempson.com)",
		"00", 0x000000,
		"01", 0x303030,
		"02", 0x505050,
		"03", 0xb0b0b0,
		"04", 0xd0d0d0,
		"05", 0xe0e0e0,
		"06", 0xf5f5f5,
		"07", 0xffffff,
		"08", 0xfb0120,
		"09", 0xfc6d24,
		"0A", 0xfda331,
		"0B", 0xa1c659,
		"0C", 0x76c7b7,
		"0D", 0x6fb3d2,
		"0E", 0xd381c3,
		"0F", 0xbe643c,
	};
	return scheme;
}

static list
def_chalk()
{
	list scheme = {
		"scheme", "Chalk",
		"author", "Chris Kempson (http://chriskempson.com)",
		"00", 0x151515,
		"01", 0x202020,
		"02", 0x303030,
		"03", 0x505050,
		"04", 0xb0b0b0,
		"05", 0xd0d0d0,
		"06", 0xe0e0e0,
		"07", 0xf5f5f5,
		"08", 0xfb9fb1,
		"09", 0xeda987,
		"0A", 0xddb26f,
		"0B", 0xacc267,
		"0C", 0x12cfc0,
		"0D", 0x6fc2ef,
		"0E", 0xe1a3ee,
		"0F", 0xdeaf8f,
	};
	return scheme;
}

static list
def_darktooth()
{
	list scheme = {
		"scheme", "Darktooth",
		"author", "Jason Milkins (https://github.com/jasonm23)",
		"00", 0x1D2021,
		"01", 0x32302F,
		"02", 0x504945,
		"03", 0x665C54,
		"04", 0x928374,
		"05", 0xA89984,
		"06", 0xD5C4A1,
		"07", 0xFDF4C1,
		"08", 0xFB543F,
		"09", 0xFE8625,
		"0A", 0xFAC03B,
		"0B", 0x95C085,
		"0C", 0x8BA59B,
		"0D", 0x0D6678,
		"0E", 0x8F4673,
		"0F", 0xA87322,
	};
	return scheme;
}

static list
def_embers()
{
	list scheme = {
		"scheme", "Embers",
		"author", "Jannik Siebert (https://github.com/janniks)",
		"00", 0x16130F,
		"01", 0x2C2620,
		"02", 0x433B32,
		"03", 0x5A5047,
		"04", 0x8A8075,
		"05", 0xA39A90,
		"06", 0xBEB6AE,
		"07", 0xDBD6D1,
		"08", 0x826D57,
		"09", 0x828257,
		"0A", 0x6D8257,
		"0B", 0x57826D,
		"0C", 0x576D82,
		"0D", 0x6D5782,
		"0E", 0x82576D,
		"0F", 0x825757,
	};
	return scheme;
}

static list
def_flat()
{
	list scheme = {
		"scheme", "Flat",
		"author", "Chris Kempson (http://chriskempson.com)",
		"00", 0x2C3E50,
		"01", 0x34495E,
		"02", 0x7F8C8D,
		"03", 0x95A5A6,
		"04", 0xBDC3C7,
		"05", 0xe0e0e0,
		"06", 0xf5f5f5,
		"07", 0xECF0F1,
		"08", 0xE74C3C,
		"09", 0xE67E22,
		"0A", 0xF1C40F,
		"0B", 0x2ECC71,
		"0C", 0x1ABC9C,
		"0D", 0x3498DB,
		"0E", 0x9B59B6,
		"0F", 0xbe643c,
	};
	return scheme;
}

static list
def_google_dark()
{
	list scheme = {
		"scheme", "Google Dark",
		"author", "Seth Wright (http://sethawright.com)",
		"00", 0x1d1f21,
		"01", 0x282a2e,
		"02", 0x373b41,
		"03", 0x969896,
		"04", 0xb4b7b4,
		"05", 0xc5c8c6,
		"06", 0xe0e0e0,
		"07", 0xffffff,
		"08", 0xCC342B,
		"09", 0xF96A38,
		"0A", 0xFBA922,
		"0B", 0x198844,
		"0C", 0x3971ED,
		"0D", 0x3971ED,
		"0E", 0xA36AC7,
		"0F", 0x3971ED,
	};
	return scheme;
}

static list
def_google_light()
{
	list scheme = {
		"scheme", "Google Light",
		"author", "Seth Wright (http://sethawright.com)",
		"00", 0xffffff,
		"01", 0xe0e0e0,
		"02", 0xc5c8c6,
		"03", 0xb4b7b4,
		"04", 0x969896,
		"05", 0x373b41,
		"06", 0x282a2e,
		"07", 0x1d1f21,
		"08", 0xCC342B,
		"09", 0xF96A38,
		"0A", 0xFBA922,
		"0B", 0x198844,
		"0C", 0x3971ED,
		"0D", 0x3971ED,
		"0E", 0xA36AC7,
		"0F", 0x3971ED,
	};
	return scheme;
}

static list
def_grayscale_dark()
{
	list scheme = {
		"scheme", "Grayscale Dark",
		"author", "Alexandre Gavioli (https://github.com/Alexx2/)",
		"00", 0x101010,
		"01", 0x252525,
		"02", 0x464646,
		"03", 0x525252,
		"04", 0xababab,
		"05", 0xb9b9b9,
		"06", 0xe3e3e3,
		"07", 0xf7f7f7,
		"08", 0x7c7c7c,
		"09", 0x999999,
		"0A", 0xa0a0a0,
		"0B", 0x8e8e8e,
		"0C", 0x868686,
		"0D", 0x686868,
		"0E", 0x747474,
		"0F", 0x5e5e5e,
	};
	return scheme;
}

static list
def_grayscale_light()
{
	list scheme = {
		"scheme", "Grayscale Light",
		"author", "Alexandre Gavioli (https://github.com/Alexx2/)",
		"00", 0xf7f7f7,
		"01", 0xe3e3e3,
		"02", 0xb9b9b9,
		"03", 0xababab,
		"04", 0x525252,
		"05", 0x464646,
		"06", 0x252525,
		"07", 0x101010,
		"08", 0x7c7c7c,
		"09", 0x999999,
		"0A", 0xa0a0a0,
		"0B", 0x8e8e8e,
		"0C", 0x868686,
		"0D", 0x686868,
		"0E", 0x747474,
		"0F", 0x5e5e5e,
	};
	return scheme;
}

static list
def_greenscreen()
{
	list scheme = {
		"scheme", "Green Screen",
		"author", "Chris Kempson (http://chriskempson.com)",
		"00", 0x001100,
		"01", 0x003300,
		"02", 0x005500,
		"03", 0x007700,
		"04", 0x009900,
		"05", 0x00bb00,
		"06", 0x00dd00,
		"07", 0x00ff00,
		"08", 0x007700,
		"09", 0x009900,
		"0A", 0x007700,
		"0B", 0x00bb00,
		"0C", 0x005500,
		"0D", 0x009900,
		"0E", 0x00bb00,
		"0F", 0x005500,
	};
	return scheme;
}

static list
def_harmonic_dark()
{
	list scheme = {
		"scheme", "Harmonic16 Dark",
		"author", "Jannik Siebert (https://github.com/janniks)",
		"00", 0x0b1c2c,
		"01", 0x223b54,
		"02", 0x405c79,
		"03", 0x627e99,
		"04", 0xaabcce,
		"05", 0xcbd6e2,
		"06", 0xe5ebf1,
		"07", 0xf7f9fb,
		"08", 0xbf8b56,
		"09", 0xbfbf56,
		"0A", 0x8bbf56,
		"0B", 0x56bf8b,
		"0C", 0x568bbf,
		"0D", 0x8b56bf,
		"0E", 0xbf568b,
		"0F", 0xbf5656,
	};
	return scheme;
}

static list
def_harmonic_light()
{
	list scheme = {
		"scheme", "Harmonic16 Light",
		"author", "Jannik Siebert (https://github.com/janniks)",
		"00", 0xf7f9fb,
		"01", 0xe5ebf1,
		"02", 0xcbd6e2,
		"03", 0xaabcce,
		"04", 0x627e99,
		"05", 0x405c79,
		"06", 0x223b54,
		"07", 0x0b1c2c,
		"08", 0xbf8b56,
		"09", 0xbfbf56,
		"0A", 0x8bbf56,
		"0B", 0x56bf8b,
		"0C", 0x568bbf,
		"0D", 0x8b56bf,
		"0E", 0xbf568b,
		"0F", 0xbf5656,
	};
	return scheme;
}

static list
def_hopscotch()
{
	list scheme = {
		"scheme", "Hopscotch",
		"author", "Jan T. Sott",
		"00", 0x322931,
		"01", 0x433b42,
		"02", 0x5c545b,
		"03", 0x797379,
		"04", 0x989498,
		"05", 0xb9b5b8,
		"06", 0xd5d3d5,
		"07", 0xffffff,
		"08", 0xdd464c,
		"09", 0xfd8b19,
		"0A", 0xfdcc59,
		"0B", 0x8fc13e,
		"0C", 0x149b93,
		"0D", 0x1290bf,
		"0E", 0xc85e7c,
		"0F", 0xb33508,
	};
	return scheme;
}

static list
def_irblack()
{
	list scheme = {
		"scheme", "IR Black",
		"author", "Timothée Poisot (http://timotheepoisot.fr)",
		"00", 0x000000,
		"01", 0x242422,
		"02", 0x484844,
		"03", 0x6c6c66,
		"04", 0x918f88,
		"05", 0xb5b3aa,
		"06", 0xd9d7cc,
		"07", 0xfdfbee,
		"08", 0xff6c60,
		"09", 0xe9c062,
		"0A", 0xffffb6,
		"0B", 0xa8ff60,
		"0C", 0xc6c5fe,
		"0D", 0x96cbfe,
		"0E", 0xff73fd,
		"0F", 0xb18a3d,
	};
	return scheme;
}

static list
def_isotope()
{
	list scheme = {
		"scheme", "Isotope",
		"author", "Jan T. Sott",
		"00", 0x000000,
		"01", 0x404040,
		"02", 0x606060,
		"03", 0x808080,
		"04", 0xc0c0c0,
		"05", 0xd0d0d0,
		"06", 0xe0e0e0,
		"07", 0xffffff,
		"08", 0xff0000,
		"09", 0xff9900,
		"0A", 0xff0099,
		"0B", 0x33ff00,
		"0C", 0x00ffff,
		"0D", 0x0066ff,
		"0E", 0xcc00ff,
		"0F", 0x3300ff,
	};
	return scheme;
}

static list
def_macintosh()
{
	list scheme = {
		"scheme", "Macintosh",
		"author", "Rebecca Bettencourt (http://www.kreativekorp.com)",
		"00", 0x000000,
		"01", 0x404040,
		"02", 0x404040,
		"03", 0x808080,
		"04", 0x808080,
		"05", 0xc0c0c0,
		"06", 0xc0c0c0,
		"07", 0xffffff,
		"08", 0xdd0907,
		"09", 0xff6403,
		"0A", 0xfbf305,
		"0B", 0x1fb714,
		"0C", 0x02abea,
		"0D", 0x0000d3,
		"0E", 0x4700a5,
		"0F", 0x90713a,
	};
	return scheme;
}

static list
def_marrakesh()
{
	list scheme = {
		"scheme", "Marrakesh",
		"author", "Alexandre Gavioli (http://github.com/Alexx2/)",
		"00", 0x201602,
		"01", 0x302e00,
		"02", 0x5f5b17,
		"03", 0x6c6823,
		"04", 0x86813b,
		"05", 0x948e48,
		"06", 0xccc37a,
		"07", 0xfaf0a5,
		"08", 0xc35359,
		"09", 0xb36144,
		"0A", 0xa88339,
		"0B", 0x18974e,
		"0C", 0x75a738,
		"0D", 0x477ca1,
		"0E", 0x8868b3,
		"0F", 0xb3588e,
	};
	return scheme;
}

static list
def_monokai()
{
	list scheme = {
		"scheme", "Monokai",
		"author", "Wimer Hazenberg (http://www.monokai.nl)",
		"00", 0x272822,
		"01", 0x383830,
		"02", 0x49483e,
		"03", 0x75715e,
		"04", 0xa59f85,
		"05", 0xf8f8f2,
		"06", 0xf5f4f1,
		"07", 0xf9f8f5,
		"08", 0xf92672,
		"09", 0xfd971f,
		"0A", 0xf4bf75,
		"0B", 0xa6e22e,
		"0C", 0xa1efe4,
		"0D", 0x66d9ef,
		"0E", 0xae81ff,
		"0F", 0xcc6633,
	};
	return scheme;
}

static list
def_oceanicnext()
{
	list scheme = {
		"scheme", "OceanicNext",
		"author", "https://github.com/voronianski/oceanic-next-color-scheme",
		"00", 0x1B2B34,
		"01", 0x343D46,
		"02", 0x4F5B66,
		"03", 0x65737E,
		"04", 0xA7ADBA,
		"05", 0xC0C5CE,
		"06", 0xCDD3DE,
		"07", 0xD8DEE9,
		"08", 0xEC5f67,
		"09", 0xF99157,
		"0A", 0xFAC863,
		"0B", 0x99C794,
		"0C", 0x5FB3B3,
		"0D", 0x6699CC,
		"0E", 0xC594C5,
		"0F", 0xAB7967,
	};
	return scheme;
}

static list
def_paraiso()
{
	list scheme = {
		"scheme", "Paraiso",
		"author", "Jan T. Sott",
		"00", 0x2f1e2e,
		"01", 0x41323f,
		"02", 0x4f424c,
		"03", 0x776e71,
		"04", 0x8d8687,
		"05", 0xa39e9b,
		"06", 0xb9b6b0,
		"07", 0xe7e9db,
		"08", 0xef6155,
		"09", 0xf99b15,
		"0A", 0xfec418,
		"0B", 0x48b685,
		"0C", 0x5bc4bf,
		"0D", 0x06b6ef,
		"0E", 0x815ba4,
		"0F", 0xe96ba8,
	};
	return scheme;
}

static list
def_phd()
{
	list scheme = {
		"scheme", "PhD",
		"author", "Hennig Hasemann (http://leetless.de/vim.html)",
		"00", 0x061229,
		"01", 0x2a3448,
		"02", 0x4d5666,
		"03", 0x717885,
		"04", 0x9a99a3,
		"05", 0xb8bbc2,
		"06", 0xdbdde0,
		"07", 0xffffff,
		"08", 0xd07346,
		"09", 0xf0a000,
		"0A", 0xfbd461,
		"0B", 0x99bf52,
		"0C", 0x72b9bf,
		"0D", 0x5299bf,
		"0E", 0x9989cc,
		"0F", 0xb08060,
	};
	return scheme;
}

static list
def_pico()
{
	list scheme = {
		"scheme", "Pico",
		"author", "PICO-8 (http://www.lexaloffle.com/pico-8.php)",
		"00", 0x000000,
		"01", 0x1d2b53,
		"02", 0x7e2553,
		"03", 0x008751,
		"04", 0xab5236,
		"05", 0x5f574f,
		"06", 0xc2c3c7,
		"07", 0xfff1e8,
		"08", 0xff004d,
		"09", 0xffa300,
		"0A", 0xfff024,
		"0B", 0x00e756,
		"0C", 0x29adff,
		"0D", 0x83769c,
		"0E", 0xff77a8,
		"0F", 0xffccaa,
	};
	return scheme;
}

static list
def_pop()
{
	list scheme = {
		"scheme", "Pop",
		"author", "Chris Kempson (http://chriskempson.com)",
		"00", 0x000000,
		"01", 0x202020,
		"02", 0x303030,
		"03", 0x505050,
		"04", 0xb0b0b0,
		"05", 0xd0d0d0,
		"06", 0xe0e0e0,
		"07", 0xffffff,
		"08", 0xeb008a,
		"09", 0xf29333,
		"0A", 0xf8ca12,
		"0B", 0x37b349,
		"0C", 0x00aabb,
		"0D", 0x0e5a94,
		"0E", 0xb31e8d,
		"0F", 0x7a2d00,
	};
	return scheme;
}

static list
def_railscasts()
{
	list scheme = {
		"scheme", "Railscasts",
		"author", "Ryan Bates (http://railscasts.com)",
		"00", 0x2b2b2b,
		"01", 0x272935,
		"02", 0x3a4055,
		"03", 0x5a647e,
		"04", 0xd4cfc9,
		"05", 0xe6e1dc,
		"06", 0xf4f1ed,
		"07", 0xf9f7f3,
		"08", 0xda4939,
		"09", 0xcc7833,
		"0A", 0xffc66d,
		"0B", 0xa5c261,
		"0C", 0x519f50,
		"0D", 0x6d9cbe,
		"0E", 0xb6b3eb,
		"0F", 0xbc9458,
	};
	return scheme;
}

static list
def_seti()
{
	list scheme = {
		"scheme", "Seti UI",
		"00", 0x151718,
		"01", 0x282a2b,
		"02", 0x3B758C,
		"03", 0x41535B,
		"04", 0x43a5d5,
		"05", 0xd6d6d6,
		"06", 0xeeeeee,
		"07", 0xffffff,
		"08", 0xCd3f45,
		"09", 0xdb7b55,
		"0A", 0xe6cd69,
		"0B", 0x9fca56,
		"0C", 0x55dbbe,
		"0D", 0x55b5db,
		"0E", 0xa074c4,
		"0F", 0x8a553f,
	};
	return scheme;
}

static list
def_shapeshifter()
{
	list scheme = {
		"scheme", "Shapeshifter",
		"author", "Tyler Benziger (http://tybenz.com)",
		"00", 0xf9f9f9,
		"01", 0xe0e0e0,
		"02", 0xababab,
		"03", 0x555555,
		"04", 0x343434,
		"05", 0x102015,
		"06", 0x040404,
		"07", 0x000000,
		"08", 0xe92f2f,
		"09", 0xe09448,
		"0A", 0xdddd13,
		"0B", 0x0ed839,
		"0C", 0x23edda,
		"0D", 0x3b48e3,
		"0E", 0xf996e2,
		"0F", 0x69542d,
	};
	return scheme;
}

static list
def_spacemacs()
{
	list scheme = {
		"scheme", "Spacemacs",
		"author", "Nasser Alshammari (https://github.com/nashamri/spacemacs-theme)",
		"00", 0x1f2022,
		"01", 0x282828,
		"02", 0x444155,
		"03", 0x585858,
		"04", 0xb8b8b8,
		"05", 0xa3a3a3,
		"06", 0xe8e8e8,
		"07", 0xf8f8f8,
		"08", 0xf2241f,
		"09", 0xffa500,
		"0A", 0xb1951d,
		"0B", 0x67b11d,
		"0C", 0x2d9574,
		"0D", 0x4f97d7,
		"0E", 0xa31db1,
		"0F", 0xb03060,
	};
	return scheme;
}

static list
def_tube()
{
	list scheme = {
		"scheme", "London Tube",
		"author", "Jan T. Sott",
		"00", 0x231f20,
		"01", 0x1c3f95,
		"02", 0x5a5758,
		"03", 0x737171,
		"04", 0x959ca1,
		"05", 0xd9d8d8,
		"06", 0xe7e7e8,
		"07", 0xffffff,
		"08", 0xee2e24,
		"09", 0xf386a1,
		"0A", 0xffd204,
		"0B", 0x00853e,
		"0C", 0x85cebc,
		"0D", 0x009ddc,
		"0E", 0x98005d,
		"0F", 0xb06110,
	};
	return scheme;
}

// gruvbox-license
// MIT License
// 
// Copyright (c) 2016 Dawid Kurek
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/dawikur/base16-gruvbox-scheme

static list
def_gruvbox_dark_hard()
{
	list scheme = {
		"scheme", "Gruvbox dark, hard",
		"author", "Dawid Kurek (dawikur@gmail.com), morhetz (https://github.com/morhetz/gruvbox)",
		"00", 0x1d2021,
		"01", 0x3c3836,
		"02", 0x504945,
		"03", 0x665c54,
		"04", 0xbdae93,
		"05", 0xd5c4a1,
		"06", 0xebdbb2,
		"07", 0xfbf1c7,
		"08", 0xfb4934,
		"09", 0xfe8019,
		"0A", 0xfabd2f,
		"0B", 0xb8bb26,
		"0C", 0x8ec07c,
		"0D", 0x83a598,
		"0E", 0xd3869b,
		"0F", 0xd65d0e,
	};
	return scheme;
}

static list
def_gruvbox_dark_medium()
{
	list scheme = {
		"scheme", "Gruvbox dark, medium",
		"author", "Dawid Kurek (dawikur@gmail.com), morhetz (https://github.com/morhetz/gruvbox)",
		"00", 0x282828,
		"01", 0x3c3836,
		"02", 0x504945,
		"03", 0x665c54,
		"04", 0xbdae93,
		"05", 0xd5c4a1,
		"06", 0xebdbb2,
		"07", 0xfbf1c7,
		"08", 0xfb4934,
		"09", 0xfe8019,
		"0A", 0xfabd2f,
		"0B", 0xb8bb26,
		"0C", 0x8ec07c,
		"0D", 0x83a598,
		"0E", 0xd3869b,
		"0F", 0xd65d0e,
	};
	return scheme;
}

static list
def_gruvbox_dark_pale()
{
	list scheme = {
		"scheme", "Gruvbox dark, pale",
		"author", "Dawid Kurek (dawikur@gmail.com), morhetz (https://github.com/morhetz/gruvbox)",
		"00", 0x262626,
		"01", 0x3a3a3a,
		"02", 0x4e4e4e,
		"03", 0x8a8a8a,
		"04", 0x949494,
		"05", 0xdab997,
		"06", 0xd5c4a1,
		"07", 0xebdbb2,
		"08", 0xd75f5f,
		"09", 0xff8700,
		"0A", 0xffaf00,
		"0B", 0xafaf00,
		"0C", 0x85ad85,
		"0D", 0x83adad,
		"0E", 0xd485ad,
		"0F", 0xd65d0e,
	};
	return scheme;
}

static list
def_gruvbox_dark_soft()
{
	list scheme = {
		"scheme", "Gruvbox dark, soft",
		"author", "Dawid Kurek (dawikur@gmail.com), morhetz (https://github.com/morhetz/gruvbox)",
		"00", 0x32302f,
		"01", 0x3c3836,
		"02", 0x504945,
		"03", 0x665c54,
		"04", 0xbdae93,
		"05", 0xd5c4a1,
		"06", 0xebdbb2,
		"07", 0xfbf1c7,
		"08", 0xfb4934,
		"09", 0xfe8019,
		"0A", 0xfabd2f,
		"0B", 0xb8bb26,
		"0C", 0x8ec07c,
		"0D", 0x83a598,
		"0E", 0xd3869b,
		"0F", 0xd65d0e,
	};
	return scheme;
}

static list
def_gruvbox_light_hard()
{
	list scheme = {
		"scheme", "Gruvbox light, hard",
		"author", "Dawid Kurek (dawikur@gmail.com), morhetz (https://github.com/morhetz/gruvbox)",
		"00", 0xf9f5d7,
		"01", 0xebdbb2,
		"02", 0xd5c4a1,
		"03", 0xbdae93,
		"04", 0x665c54,
		"05", 0x504945,
		"06", 0x3c3836,
		"07", 0x282828,
		"08", 0x9d0006,
		"09", 0xaf3a03,
		"0A", 0xb57614,
		"0B", 0x79740e,
		"0C", 0x427b58,
		"0D", 0x076678,
		"0E", 0x8f3f71,
		"0F", 0xd65d0e,
	};
	return scheme;
}

static list
def_gruvbox_light_medium()
{
	list scheme = {
		"scheme", "Gruvbox light, medium",
		"author", "Dawid Kurek (dawikur@gmail.com), morhetz (https://github.com/morhetz/gruvbox)",
		"00", 0xfbf1c7,
		"01", 0xebdbb2,
		"02", 0xd5c4a1,
		"03", 0xbdae93,
		"04", 0x665c54,
		"05", 0x504945,
		"06", 0x3c3836,
		"07", 0x282828,
		"08", 0x9d0006,
		"09", 0xaf3a03,
		"0A", 0xb57614,
		"0B", 0x79740e,
		"0C", 0x427b58,
		"0D", 0x076678,
		"0E", 0x8f3f71,
		"0F", 0xd65d0e,
	};
	return scheme;
}

static list
def_gruvbox_light_soft()
{
	list scheme = {
		"scheme", "Gruvbox light, soft",
		"author", "Dawid Kurek (dawikur@gmail.com), morhetz (https://github.com/morhetz/gruvbox)",
		"00", 0xf2e5bc,
		"01", 0xebdbb2,
		"02", 0xd5c4a1,
		"03", 0xbdae93,
		"04", 0x665c54,
		"05", 0x504945,
		"06", 0x3c3836,
		"07", 0x282828,
		"08", 0x9d0006,
		"09", 0xaf3a03,
		"0A", 0xb57614,
		"0B", 0x79740e,
		"0C", 0x427b58,
		"0D", 0x076678,
		"0E", 0x8f3f71,
		"0F", 0xd65d0e,
	};
	return scheme;
}

// silk-license
// MIT License
// 
// Copyright (c) 2020 Gabriel Fontes
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/misterio77/base16-silk-scheme

static list
def_silk_dark()
{
	list scheme = {
		"scheme", "Silk Dark",
		"author", "Gabriel Fontes (https://github.com/Misterio77)",
		"00", 0x0e3c46,
		"01", 0x1D494E,
		"02", 0x2A5054,
		"03", 0x587073,
		"04", 0x9DC8CD,
		"05", 0xC7DBDD,
		"06", 0xCBF2F7,
		"07", 0xD2FAFF,
		"08", 0xfb6953,
		"09", 0xfcab74,
		"0A", 0xfce380,
		"0B", 0x73d8ad,
		"0C", 0x3fb2b9,
		"0D", 0x46bddd,
		"0E", 0x756b8a,
		"0F", 0x9b647b,
	};
	return scheme;
}

static list
def_silk_light()
{
	list scheme = {
		"scheme", "Silk Light",
		"author", "Gabriel Fontes (https://github.com/Misterio77)",
		"00", 0xE9F1EF,
		"01", 0xCCD4D3,
		"02", 0x90B7B6,
		"03", 0x5C787B,
		"04", 0x4B5B5F,
		"05", 0x385156,
		"06", 0x0e3c46,
		"07", 0xD2FAFF,
		"08", 0xCF432E,
		"09", 0xD27F46,
		"0A", 0xCFAD25,
		"0B", 0x6CA38C,
		"0C", 0x329CA2,
		"0D", 0x39AAC9,
		"0E", 0x6E6582,
		"0F", 0x865369,
	};
	return scheme;
}

// nord-license
// MIT License
// https://github.com/spejamchr/base16-nord-scheme

static list
def_nord()
{
	list scheme = {
		"scheme", "Nord",
		"author", "arcticicestudio",
		"00", 0x2E3440,
		"01", 0x3B4252,
		"02", 0x434C5E,
		"03", 0x4C566A,
		"04", 0xD8DEE9,
		"05", 0xE5E9F0,
		"06", 0xECEFF4,
		"07", 0x8FBCBB,
		"08", 0xBF616A,
		"09", 0xD08770,
		"0A", 0xEBCB8B,
		"0B", 0xA3BE8C,
		"0C", 0x88C0D0,
		"0D", 0x81A1C1,
		"0E", 0xB48EAD,
		"0F", 0x5E81AC,
	};
	return scheme;
}

// mexico-light-license
// MIT License
// https://github.com/drzel/base16-mexico-light-scheme

static list
def_mexico_light()
{
	list scheme = {
		"scheme", "Mexico Light",
		"author", "Sheldon Johnson",
		"00", 0xf8f8f8,
		"01", 0xe8e8e8,
		"02", 0xd8d8d8,
		"03", 0xb8b8b8,
		"04", 0x585858,
		"05", 0x383838,
		"06", 0x282828,
		"07", 0x181818,
		"08", 0xab4642,
		"09", 0xdc9656,
		"0A", 0xf79a0e,
		"0B", 0x538947,
		"0C", 0x4b8093,
		"0D", 0x7cafc2,
		"0E", 0x96609e,
		"0F", 0xa16946,
	};
	return scheme;
}

// tango-license
// ISC License
// 
// Copyright (c) 2020, Thomas Jost
// 
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
// 
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// https://github.com/Schnouki/base16-tango-scheme

static list
def_tango()
{
	list scheme = {
		"scheme", "Tango",
		"author", "@Schnouki, based on the Tango Desktop Project",
		"00", 0x2e3436,
		"01", 0x8ae234,
		"02", 0xfce94f,
		"03", 0x555753,
		"04", 0x729fcf,
		"05", 0xd3d7cf,
		"06", 0xad7fa8,
		"07", 0xeeeeec,
		"08", 0xcc0000,
		"09", 0xef2929,
		"0A", 0xc4a000,
		"0B", 0x4e9a06,
		"0C", 0x06989a,
		"0D", 0x3465a4,
		"0E", 0x75507b,
		"0F", 0x34e2e2,
	};
	return scheme;
}

// classic-license
// MIT License
// 
// Copyright (c) 2017 Jason Heeris
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/detly/base16-classic-scheme

static list
def_classic_dark()
{
	list scheme = {
		"scheme", "Classic Dark",
		"author", "Jason Heeris (http://heeris.id.au)",
		"00", 0x151515,
		"01", 0x202020,
		"02", 0x303030,
		"03", 0x505050,
		"04", 0xB0B0B0,
		"05", 0xD0D0D0,
		"06", 0xE0E0E0,
		"07", 0xF5F5F5,
		"08", 0xAC4142,
		"09", 0xD28445,
		"0A", 0xF4BF75,
		"0B", 0x90A959,
		"0C", 0x75B5AA,
		"0D", 0x6A9FB5,
		"0E", 0xAA759F,
		"0F", 0x8F5536,
	};
	return scheme;
}

static list
def_classic_light()
{
	list scheme = {
		"scheme", "Classic Light",
		"author", "Jason Heeris (http://heeris.id.au)",
		"00", 0xF5F5F5,
		"01", 0xE0E0E0,
		"02", 0xD0D0D0,
		"03", 0xB0B0B0,
		"04", 0x505050,
		"05", 0x303030,
		"06", 0x202020,
		"07", 0x151515,
		"08", 0xAC4142,
		"09", 0xD28445,
		"0A", 0xF4BF75,
		"0B", 0x90A959,
		"0C", 0x75B5AA,
		"0D", 0x6A9FB5,
		"0E", 0xAA759F,
		"0F", 0x8F5536,
	};
	return scheme;
}

// colors-license
// MIT License
// https://github.com/hakatashi/base16-colors-scheme

static list
def_colors()
{
	list scheme = {
		"scheme", "Colors",
		"author", "mrmrs (http://clrs.cc)",
		"00", 0x111111,
		"01", 0x333333,
		"02", 0x555555,
		"03", 0x777777,
		"04", 0x999999,
		"05", 0xbbbbbb,
		"06", 0xdddddd,
		"07", 0xffffff,
		"08", 0xff4136,
		"09", 0xff851b,
		"0A", 0xffdc00,
		"0B", 0x2ecc40,
		"0C", 0x7fdbff,
		"0D", 0x0074d9,
		"0E", 0xb10dc9,
		"0F", 0x85144b,
	};
	return scheme;
}

// sagelight-license
// MIT License
// 
// Copyright (c) 2020 Carter Veldhuizen
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// https://github.com/cveldy/base16-sagelight-scheme

static list
def_sagelight()
{
	list scheme = {
		"scheme", "Sagelight",
		"author", "Carter Veldhuizen",
		"00", 0xf8f8f8,
		"01", 0xe8e8e8,
		"02", 0xd8d8d8,
		"03", 0xb8b8b8,
		"04", 0x585858,
		"05", 0x383838,
		"06", 0x282828,
		"07", 0x181818,
		"08", 0xfa8480,
		"09", 0xffaa61,
		"0A", 0xffdc61,
		"0B", 0xa0d2c8,
		"0C", 0xa2d6f5,
		"0D", 0xa0a7d2,
		"0E", 0xc8a0d2,
		"0F", 0xd2b2a0,
	};
	return scheme;
}

// papercolor-license
// MIT License
// 
// Copyright (c) 2018 Jon Leopard
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/jonleopard/base16-papercolor-scheme

static list
def_papercolor_dark()
{
	list scheme = {
		"scheme", "PaperColor Dark",
		"author", "Jon Leopard (http://github.com/jonleopard) based on PaperColor Theme (https://github.com/NLKNguyen/papercolor-theme)",
		"00", 0x1c1c1c,
		"01", 0xaf005f,
		"02", 0x5faf00,
		"03", 0xd7af5f,
		"04", 0x5fafd7,
		"05", 0x808080,
		"06", 0xd7875f,
		"07", 0xd0d0d0,
		"08", 0x585858,
		"09", 0x5faf5f,
		"0A", 0xafd700,
		"0B", 0xaf87d7,
		"0C", 0xffaf00,
		"0D", 0xff5faf,
		"0E", 0x00afaf,
		"0F", 0x5f8787,
	};
	return scheme;
}

static list
def_papercolor_light()
{
	list scheme = {
		"scheme", "PaperColor Light",
		"author", "Jon Leopard (http://github.com/jonleopard) based on PaperColor Theme (https://github.com/NLKNguyen/papercolor-theme)",
		"00", 0xeeeeee,
		"01", 0xaf0000,
		"02", 0x008700,
		"03", 0x5f8700,
		"04", 0x0087af,
		"05", 0x444444,
		"06", 0x005f87,
		"07", 0x878787,
		"08", 0xbcbcbc,
		"09", 0xd70000,
		"0A", 0xd70087,
		"0B", 0x8700af,
		"0C", 0xd75f00,
		"0D", 0xd75f00,
		"0E", 0x005faf,
		"0F", 0x005f87,
	};
	return scheme;
}

// shades-of-purple-license
// MIT License
// 
// Copyright (c) 2020 Iolar Demartini Junior
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/ahmadawais/base16-shades-of-purple

static list
def_shades_of_purple()
{
	list scheme = {
		"scheme", "Shades of Purple",
		"author", "Iolar Demartini Junior (http://github.com/demartini) based on Shades of Purple Theme (https://github.com/ahmadawais/shades-of-purple-vscode).",
		"00", 0x1e1e3f,
		"01", 0x43d426,
		"02", 0xf1d000,
		"03", 0x808080,
		"04", 0x6871ff,
		"05", 0xc7c7c7,
		"06", 0xff77ff,
		"07", 0xffffff,
		"08", 0xd90429,
		"09", 0xf92a1c,
		"0A", 0xffe700,
		"0B", 0x3ad900,
		"0C", 0x00c5c7,
		"0D", 0x6943ff,
		"0E", 0xff2c70,
		"0F", 0x79e8fb,
	};
	return scheme;
}

// dracula-license
// The MIT License (MIT)
// 
// Copyright (c) 2016 Dracula Theme
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/dracula/base16-dracula-scheme

static list
def_dracula()
{
	list scheme = {
		"scheme", "Dracula",
		"author", "Mike Barkmin (http://github.com/mikebarkmin) based on Dracula Theme (http://github.com/dracula)",
		"00", 0x282936,
		"01", 0x3a3c4e,
		"02", 0x4d4f68,
		"03", 0x626483,
		"04", 0x62d6e8,
		"05", 0xe9e9f4,
		"06", 0xf1f2f8,
		"07", 0xf7f7fb,
		"08", 0xea51b2,
		"09", 0xb45bcf,
		"0A", 0x00f769,
		"0B", 0xebff87,
		"0C", 0xa1efe4,
		"0D", 0x62d6e8,
		"0E", 0xb45bcf,
		"0F", 0x00f769,
	};
	return scheme;
}

// humanoid-license
// MIT License
// 
// Copyright (c) 2020 humanoids
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/humanoid-colors/base16-humanoid-schemes

static list
def_humanoid_dark()
{
	list scheme = {
		"scheme", "Humanoid dark",
		"author", "Thomas (tasmo) Friese",
		"00", 0x232629,
		"01", 0x333b3d,
		"02", 0x484e54,
		"03", 0x60615d,
		"04", 0xc0c0bd,
		"05", 0xf8f8f2,
		"06", 0xfcfcf6,
		"07", 0xfcfcfc,
		"08", 0xf11235,
		"09", 0xff9505,
		"0A", 0xffb627,
		"0B", 0x02d849,
		"0C", 0x0dd9d6,
		"0D", 0x00a6fb,
		"0E", 0xf15ee3,
		"0F", 0xb27701,
	};
	return scheme;
}

static list
def_humanoid_light()
{
	list scheme = {
		"scheme", "Humanoid light",
		"author", "Thomas (tasmo) Friese",
		"00", 0xf8f8f2,
		"01", 0xefefe9,
		"02", 0xdeded8,
		"03", 0xc0c0bd,
		"04", 0x60615d,
		"05", 0x232629,
		"06", 0x2f3337,
		"07", 0x070708,
		"08", 0xb0151a,
		"09", 0xff3d00,
		"0A", 0xffb627,
		"0B", 0x388e3c,
		"0C", 0x008e8e,
		"0D", 0x0082c9,
		"0E", 0x700f98,
		"0F", 0xb27701,
	};
	return scheme;
}

// summercamp-license
// MIT License
// 
// Copyright (c) 2019 Zoe FiriH
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// https://github.com/zoefiri/base16-summercamp

static list
def_summercamp()
{
	list scheme = {
		"scheme", "summercamp",
		"author", "zoe firi (zoefiri.github.io)",
		"00", 0x1c1810,
		"01", 0x2a261c,
		"02", 0x3a3527,
		"03", 0x504b38,
		"04", 0x5f5b45,
		"05", 0x736e55,
		"06", 0xbab696,
		"07", 0xf8f5de,
		"08", 0xe35142,
		"09", 0xfba11b,
		"0A", 0xf2ff27,
		"0B", 0x5ceb5a,
		"0C", 0x5aebbc,
		"0D", 0x489bf0,
		"0E", 0xFF8080,
		"0F", 0xF69BE7,
	};
	return scheme;
}

// xcode-dusk-license
// MIT License
// https://github.com/gonsie/base16-xcode-dusk-scheme

static list
def_xcode_dusk()
{
	list scheme = {
		"scheme", "XCode Dusk",
		"author", "Elsa Gonsiorowski (https://github.com/gonsie)",
		"00", 0x282B35,
		"01", 0x3D4048,
		"02", 0x53555D,
		"03", 0x686A71,
		"04", 0x7E8086,
		"05", 0x939599,
		"06", 0xA9AAAE,
		"07", 0xBEBFC2,
		"08", 0xB21889,
		"09", 0x786DC5,
		"0A", 0x438288,
		"0B", 0xDF0002,
		"0C", 0x00A0BE,
		"0D", 0x790EAD,
		"0E", 0xB21889,
		"0F", 0xC77C48,
	};
	return scheme;
}

// darkviolet-license
// MIT License
// https://github.com/ruler501/base16-darkviolet-scheme

static list
def_darkviolet()
{
	list scheme = {
		"scheme", "Dark Violet",
		"author", "ruler501 (https://github.com/ruler501/base16-darkviolet)",
		"00", 0x000000,
		"01", 0x231a40,
		"02", 0x432d59,
		"03", 0x593380,
		"04", 0x00ff00,
		"05", 0xb08ae6,
		"06", 0x9045e6,
		"07", 0xa366ff,
		"08", 0xa82ee6,
		"09", 0xbb66cc,
		"0A", 0xf29df2,
		"0B", 0x4595e6,
		"0C", 0x40dfff,
		"0D", 0x4136d9,
		"0E", 0x7e5ce6,
		"0F", 0xa886bf,
	};
	return scheme;
}

//
//  base16_schemes ---
//      Return the scheme list.
//
list
base16_schemes()
{
    list schemes = {
        "3024", "3024",
        "Apathy", "apathy",
        "Apprentice", "apprentice",
        "Ashes", "ashes",
        "Atelier Cave", "atelier_cave",
        "Atelier Cave Light", "atelier_cave_light",
        "Atelier Dune", "atelier_dune",
        "Atelier Dune Light", "atelier_dune_light",
        "Atelier Estuary", "atelier_estuary",
        "Atelier Estuary Light", "atelier_estuary_light",
        "Atelier Forest", "atelier_forest",
        "Atelier Forest Light", "atelier_forest_light",
        "Atelier Heath", "atelier_heath",
        "Atelier Heath Light", "atelier_heath_light",
        "Atelier Lakeside", "atelier_lakeside",
        "Atelier Lakeside Light", "atelier_lakeside_light",
        "Atelier Plateau", "atelier_plateau",
        "Atelier Plateau Light", "atelier_plateau_light",
        "Atelier Savanna", "atelier_savanna",
        "Atelier Savanna Light", "atelier_savanna_light",
        "Atelier Seaside", "atelier_seaside",
        "Atelier Seaside Light", "atelier_seaside_light",
        "Atelier Sulphurpool", "atelier_sulphurpool",
        "Atelier Sulphurpool Light", "atelier_sulphurpool_light",
        "Atlas", "atlas",
        "Bespin", "bespin",
        "Black Metal", "black_metal",
        "Black Metal (Bathory)", "black_metal_bathory",
        "Black Metal (Burzum)", "black_metal_burzum",
        "Black Metal (Dark Funeral)", "black_metal_dark_funeral",
        "Black Metal (Gorgoroth)", "black_metal_gorgoroth",
        "Black Metal (Immortal)", "black_metal_immortal",
        "Black Metal (Khold)", "black_metal_khold",
        "Black Metal (Marduk)", "black_metal_marduk",
        "Black Metal (Mayhem)", "black_metal_mayhem",
        "Black Metal (Nile)", "black_metal_nile",
        "Black Metal (Venom)", "black_metal_venom",
        "Brewer", "brewer",
        "Bright", "bright",
        "Brogrammer", "brogrammer",
        "Brush Trees", "brushtrees",
        "Brush Trees Dark", "brushtrees_dark",
        "Chalk", "chalk",
        "Circus", "circus",
        "Classic Dark", "classic_dark",
        "Classic Light", "classic_light",
        "Codeschool", "codeschool",
        "Colors", "colors",
        "Cupcake", "cupcake",
        "Cupertino", "cupertino",
        "DanQing", "danqing",
        "DanQing Light", "danqing_light",
        "Darcula", "darcula",
        "Dark Violet", "darkviolet",
        "darkmoss", "darkmoss",
        "Darktooth", "darktooth",
        "Decaf", "decaf",
        "Default Dark", "default_dark",
        "Default Light", "default_light",
        "dirtysea", "dirtysea",
        "Dracula", "dracula",
        "Edge Dark", "edge_dark",
        "Edge Light", "edge_light",
        "Eighties", "eighties",
        "Embers", "embers",
        "Equilibrium Dark", "equilibrium_dark",
        "Equilibrium Gray Dark", "equilibrium_gray_dark",
        "Equilibrium Gray Light", "equilibrium_gray_light",
        "Equilibrium Light", "equilibrium_light",
        "Espresso", "espresso",
        "Eva", "eva",
        "Eva Dim", "eva_dim",
        "Flat", "flat",
        "Framer", "framer",
        "Fruit Soda", "fruit_soda",
        "Gigavolt", "gigavolt",
        "Github", "github",
        "Google Dark", "google_dark",
        "Google Light", "google_light",
        "Grayscale Dark", "grayscale_dark",
        "Grayscale Light", "grayscale_light",
        "Green Screen", "greenscreen",
        "Gruber", "gruber",
        "Gruvbox dark, hard", "gruvbox_dark_hard",
        "Gruvbox dark, medium", "gruvbox_dark_medium",
        "Gruvbox dark, pale", "gruvbox_dark_pale",
        "Gruvbox dark, soft", "gruvbox_dark_soft",
        "Gruvbox light, hard", "gruvbox_light_hard",
        "Gruvbox light, medium", "gruvbox_light_medium",
        "Gruvbox light, soft", "gruvbox_light_soft",
        "Hardcore", "hardcore",
        "Harmonic16 Dark", "harmonic_dark",
        "Harmonic16 Light", "harmonic_light",
        "Heetch Dark", "heetch",
        "Heetch Light", "heetch_light",
        "Helios", "helios",
        "Hopscotch", "hopscotch",
        "Horizon Dark", "horizon_terminal_dark",
        "Horizon Light", "horizon_terminal_light",
        "Humanoid dark", "humanoid_dark",
        "Humanoid light", "humanoid_light",
        "Icy Dark", "icy",
        "IR Black", "irblack",
        "Isotope", "isotope",
        "Kimber", "kimber",
        "London Tube", "tube",
        "Macintosh", "macintosh",
        "Marrakesh", "marrakesh",
        "Materia", "materia",
        "Material", "material",
        "Material Darker", "material_darker",
        "Material Lighter", "material_lighter",
        "Material Palenight", "material_palenight",
        "Material Vivid", "material_vivid",
        "Mellow Purple", "mellow_purple",
        "Mexico Light", "mexico_light",
        "Mocha", "mocha",
        "Monokai", "monokai",
        "Nebula", "nebula",
        "Nord", "nord",
        "Nova", "nova",
        "Ocean", "ocean",
        "OceanicNext", "oceanicnext",
        "One Light", "one_light",
        "OneDark", "onedark",
        "Outrun Dark", "outrun_dark",
        "PaperColor Dark", "papercolor_dark",
        "PaperColor Light", "papercolor_light",
        "Paraiso", "paraiso",
        "Pasque", "pasque",
        "PhD", "phd",
        "Pico", "pico",
        "pinky", "pinky",
        "Pop", "pop",
        "Porple", "porple",
        "Railscasts", "railscasts",
        "Rebecca", "rebecca",
        "Rosé Pine", "rose_pine",
        "Rosé Pine Dawn", "rose_pine_dawn",
        "Rosé Pine Moon", "rose_pine_moon",
        "Sagelight", "sagelight",
        "Sakura", "sakura",
        "Sandcastle", "sandcastle",
        "Seti UI", "seti",
        "Shades of Purple", "shades_of_purple",
        "Shapeshifter", "shapeshifter",
        "Silk Dark", "silk_dark",
        "Silk Light", "silk_light",
        "Snazzy", "snazzy",
        "Solar Flare", "solarflare",
        "Solar Flare Light", "solarflare_light",
        "Spacemacs", "spacemacs",
        "summercamp", "summercamp",
        "Summerfruit Dark", "summerfruit_dark",
        "Summerfruit Light", "summerfruit_light",
        "Synth Midnight Terminal Dark", "synth_midnight_dark",
        "Synth Midnight Terminal Light", "synth_midnight_light",
        "Tango", "tango",
        "tender", "tender",
        "Twilight", "twilight",
        "Unikitty Dark", "unikitty_dark",
        "Unikitty Light", "unikitty_light",
        "Vice Alt", "vice_alt",
        "Vice Dark", "vice",
        "vulcan", "vulcan",
        "Windows 10", "windows_10",
        "Windows 10 Light", "windows_10_light",
        "Windows 95", "windows_95",
        "Windows 95 Light", "windows_95_light",
        "Windows High Contrast", "windows_highcontrast",
        "Windows High Contrast Light", "windows_highcontrast_light",
        "Windows NT", "windows_nt",
        "Windows NT Light", "windows_nt_light",
        "Woodland", "woodland",
        "XCode Dusk", "xcode_dusk",
        "Zenburn", "zenburn",
    };
    return schemes;
}

//end
