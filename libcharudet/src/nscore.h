#ifndef nscore_h_included
#define nscore_h_included

typedef bool            PRBool;

typedef int             PRInt32;
typedef unsigned int    PRUint32;
typedef short           PRInt16;
typedef unsigned short  PRUint16;
typedef char            PRInt8;
typedef unsigned char   PRUint8;

#define PR_FALSE        false
#define PR_TRUE         true

#define nsnull          0
#define PR_Malloc       malloc

enum nsresult {
    NS_OK,
    NS_ERROR_OUT_OF_MEMORY
};

#endif  /*nscore_h_included*/
