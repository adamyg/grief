#ifndef UPGETOPT_H_INCLUDED
#define UPGETOPT_H_INCLUDED
// $Id: upgetopt.h,v 1.2 2015/01/05 23:44:26 ayoung Exp $
//
//  getopt() implementation
//

namespace Updater {

extern int          optind,                     /* index into parent argv vector */
                    optopt;                     /* character checked for validity */
extern const char  *optarg;                     /* argument associated with option */

extern int          Getopt(int nargc, char **nargv, const char *ostr);

} //namespace Updater

#endif  /*UPGETOPT_H_INCLUDED*/
