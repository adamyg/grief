
  libchartable

        Character code interface library, provides character encoding/decoding
        primitives where the local system resources are not available/suitable; like
        for example win32.

        General iconv() streams, core UTF functionality and character encoding
        functionality.

        It is in part a basic implementation of iconv(), yet with limited conversion
        features other then when a unicode mapping table has been defined; not intended
        to replace but to be used hand-in-hand with the system supplied iconv()
        implementation; speciality in-line character encode/decode conversions whereas
        iconv() is stream/block orientated.

