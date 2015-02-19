# -*- mode: sh; -*-
# $Id: wc.profile,v 1.4 2014/11/27 19:17:12 ayoung Exp $
# makedepend Open Watcom C/C++ profile
#
__WATCOMC__=1900
WIN32_LEAN_AND_MEAN=1
WIN32=0x601
_WIN32=0x601
_M_IX86
-Y@INCLUDE

# aliases, example
#   pragma include_alias(<d3d9types.h>,<d3d9type.h>)
#   see windows SDK
#
-Ad3d9types.h               =d3d9type.h
-Adevpropdef.h              =devprdef.h
-Adriverspecs.h             =drvspecs.h
-Aknownfolders.h            =knownfld.h
-Aobjectarray.h             =objarray.h
-Apropkeydef.h              =propkdef.h
-Aschemadef.h               =schemdef.h
-Asdkddkver.h               =sdkddver.h
-Aspecstrings.h             =specstr.h
-Astructuredquery.h         =strquery.h
-Astructuredquerycondition.h=strqcond.h
-Atcpestats.h               =tcpestat.h
-Awlantypes.h               =wlantype.h
-Awinsdkver.h               =wsdkver.h
-Aexception                 =exceptio


