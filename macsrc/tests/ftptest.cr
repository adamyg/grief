//
// ftp interface test
//

#include "../grief.h"

#if !defined(PROTOCOL_FTP)
#define PROTOCOL_FTP    0
#define PROTOCOL_HTTP   1
#define PROTOCOL_HTTPS  2
#define PROTOCOL_FILE   3
#endif

static void ftpx(void);
static void httpx(void);
static void httpsx(void);
static void urlx(void);


void
ftptest(void)
{
debug(1);
   ftpx();
// httpx();
// httpsx();
// urlx();
debug(0);
}


static void
ftpx(void)
{
   int size, mtime, atime, mode, idx = 1;
   string file;
   list files;

   int fd = ftp_create(PROTOCOL_FTP);
// ftp_connect(fd, "ftp.freebsd.org/pub/FreeBSD", 0, NULL, NULL);
   ftp_connect(fd, "netwinsite.com/pub/", 0, NULL, NULL);         // MLSD support
   ftp_directory(fd, NULL, files);
// ftp_mkdir(fd, "test-mkdir");
// ftp_rename(fd, "test-rename-from", "test-rename-to");
// ftp_remove(fd, "test-remove");
// ftp_remove(fd, "test-remove/");
   while (list_each(files, file) >= 0) {
      ftp_stat(fd, file, size, mtime, atime, mode);
//    if (size > 0) {
//       ftp_get_file(fd, file, format("ftptest.ftpx%d", idx++));
//    }
   }
   ftp_close(fd);
}


static void
httpx(void)
{
   int size, mtime, atime, mode, idx = 1;
   string file;
   list files;

   int fd = ftp_create(PROTOCOL_HTTP);
   ftp_connect(fd, "bitsavers.informatik.uni-stuttgart.de/bits/", 0);
   ftp_directory(fd, NULL, files);
   while (list_each(files, file) >= 0) {
      ftp_stat(fd, file, size, mtime, atime, mode);
      if (size > 0) {
         ftp_get_file(fd, file, format("ftptest.httpx%d", idx++));
      }
   }
   ftp_close(fd);
}


static void
httpsx(void)
{
   int size, mtime, atime, mode, idx = 1;
   string file;
   list files;

   int fd = ftp_create(PROTOCOL_HTTPS);
   ftp_connect(fd, "ftp.ipv6.uni-leipzig.de/pub/ftp.netbsd.org/pub/NetBSD/", 0);
   ftp_directory(fd, NULL, files);
   while (list_each(files, file) >= 0) {
      ftp_stat(fd, file, size, mtime, atime, mode);
      if (size > 0) {
         ftp_get_file(fd, file, format("ftptest.httpsx%d", idx++));
      }
   }
   ftp_close(fd);
}


static void
urlx(void)
{
   int size, mtime, atime, mode, idx = 1;
   string file;
   list files;

   int fd = ftp_create();
   ftp_connect(fd, "http://bitsavers.informatik.uni-stuttgart.de/bits");
   ftp_directory(fd, NULL, files);
   while (list_each(files, file) >= 0) {
      ftp_stat(fd, file, size, mtime, atime, mode);
      if (size > 0) {
         ftp_get_file(fd, file, format("ftptest.urlx%d", idx++));
      }
   }
   ftp_close(fd);
}
/*end*/
