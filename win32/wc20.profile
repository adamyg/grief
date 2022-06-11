# -*- mode: sh; -*-
# $Id: wc20.profile,v 1.1 2022/06/02 12:42:28 cvsuser Exp $
# makedepend Open Watcom C/C++ 2.0 profile
#
__WATCOMC__=1300
#  __WATCOMC__ = 1290      /* 1.9 */
#  __WATCOMC__ = 1300      /* 2.0 */

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
-Asdkddkver.h               =sdkddkve.h
-Aspecstrings.h             =specstr.h
-Astructuredquery.h         =strquery.h
-Astructuredquerycondition.h=strqcond.h
-Atcpestats.h               =tcpestat.h
-Awlantypes.h               =wlantype.h
-Awinsdkver.h               =wsdkver.h
-Aexception                 =exceptio

# C++ aliases
# see:  _preincl.h
-Aalgorithm                 =algorith
-Acinttypes                 =cinttype
-Acondition_variable        =conditio
-Aexception                 =exceptio
-Aexception.h               =exceptio.h
-Aforward_list              =forward_
-Afunctional                =function
-Ainitializer_list          =initiali
-Asemaphore.h               =semaphor.h
-Astdexcept                 =stdexcep
-Astdexcept.h               =stdexcep.h
-Astreambuf                 =streambu
-Astreambuf.h               =streambu.h
-Astrstream                 =strstrea
-Astrstream.h               =strstrea.h
-Asystem_error              =system_e
-Atypeindex                 =typeinde
-Atype_traits               =type_tra
-Aunordered_map             =unorderm
-Aunordered_set             =unorders

-Astdnoreturn.h             =stdnoret.h



