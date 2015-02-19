/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: vfstest.cr,v 1.3 2014/10/22 02:34:30 ayoung Exp $
 * Virtual File System -- basic test mode.
 *
 *
 */
 
#include "../grief.h"

void
vfs_mount_test1(void)
{
   vfs_mount("/vfstest/unknown.tar", 0, "unknown");
}


void
vfs_mount_test2(void)
{
   vfs_mount("/vfstest/unknown.tar", 0, "tar");
}


void
vfs_mount_test3(void)
{
    if (0 == vfs_mount("/vfstest/vfstest.tar", 0, "tar")) {
        file_glob("/vfstest/*");
        file_glob("/vfstest/vfstest.tar/*");
    }
}


void
vfs_mount_test4(void)
{
    string ftparguments, ftpmount = "/vfstest/localhost";

    if (ftest("d", ftpmount) != 1) {
        mkdir(ftpmount);
    }

    sprintf(ftparguments, "//ftp:%s@%s:21%s", inq_username(), inq_hostname(), inq_home());

    if (0 == vfs_mount(ftpmount, 0, ftparguments)) {
        file_glob(ftpmount);
    }

    vfs_mount(ftpmount, 0, "//ftp:user,password@localhost:9999/path");
    vfs_mount(ftpmount, 0, "//ftp:user,\"password\"@localhost:9999/path");
    vfs_mount(ftpmount, 0, "//ftp:user,'password'@localhost:9999/path");
    vfs_mount(ftpmount, 0, "//ftp:user,password@localhost:9999/path");
    vfs_mount(ftpmount, 0, "//ftp:user:password@localhost:999/path");


    vfs_mount(ftpmount, 0, "//ftp:'specialuser'@localhost:9990/path");
    vfs_mount(ftpmount, 0, "//ftp:@localhost:9990/path");
    vfs_mount(ftpmount, 0, "//ftp::9990/path");
    vfs_mount(ftpmount, 0, "//ftp::/path");

    vfs_mount(ftpmount, 0, "//ftp:@localhost:9990");
    vfs_mount(ftpmount, 0, "//ftp:user@localhost");

    vfs_mount(ftpmount, 0, "//ftp:user:9990/path");
    vfs_mount(ftpmount, 0, "//ftp:@localhost:9990/path");
    vfs_mount(ftpmount, 0, "//ftp:@:9990/path");
}

void
vfs_mount_test5(void)
{
    list mounts = inq_vfs_mounts();

    if (is_list(mounts)) {
        /*
         *  dump mount list
         */
        int idx;

        for (idx = 0; idx < length_of_list(mounts); ++idx) {
            list mount = mounts[idx];

            dprintf("%d] mount:%s, prefix:%s, flags:0x%0x\n", idx, mount[0], mount[1], mount[2]);
        }
    }
}


void
vfs_mount_tests(void)
{
   vfs_mount_test1();
   vfs_mount_test2();
   vfs_mount_test3();
   vfs_mount_test4();
   vfs_mount_test5();
}


void
main(void)
{
}

/*end*/
