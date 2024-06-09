
	Source: https://www.libarchive.org/

	** Multi-format archive and compression library **

	The source distribution includes the libarchive library, the bsdtar and bsdcpio command-line programs, full test suite, and documentation:

		Stable release: libarchive-3.3.3.tar.gz libarchive-3.3.3.zip (since September 8, 2018)
		Current Development Sources: Tar.gz of github master branch Zip of github master branch
		Legacy releases 

	The libarchive library features:

		Support for a variety of archive and compression formats.
		Robust automatic format detection, including archive/compression combinations such as tar.gz.
		Zero-copy internal architecture for high performance.
		Streaming architecture eliminates all limits on size of archive, limits on entry sizes depend on particular formats.
		Carefully factored code to minimize bloat when programs are statically linked.
		Growing test suite to verify correctness of new ports.
		Works on most POSIX-like systems (including FreeBSD, Linux, Solaris, etc.)
		Supports Windows, including Cygwin, MinGW, and Visual Studio.

	The bsdtar and bsdcpio command-line utilities are feature- and performance-competitive with other tar and cpio implementations:

		Reads a variety of formats, including tar, pax, cpio, zip, xar, lha, ar, cab, mtree, rar, and ISO images.
		Writes tar, pax, cpio, zip, xar, ar, ISO, mtree, and shar archives.
		Automatically handles archives compressed with gzip, bzip2, lzip, xz, lzma, or compress.
		Unique format conversion feature.

	This site includes:

		Build Instructions
		Examples of using the library
		Formats supported by the library
		Manpages included in the distribution
		Libarchive Users
		Release Notes
		Ideas for people who want to contribute

	License

		New BSD License c

	-----------------------------------------------------------------------------------------------------------------------

        o libarchive 3.3.3

		patch

			archive_windows.h   

				+	#if HAVE_DIRENT_H
				+	# include <dirent.h>
				+	#else
					# include <direct.h>
				+	#endif

				foreach (definition)

				+	#else
				+	#if _S_IFIFO != 0010000
				+	#error _S_IFIFO redefinition error ...
				+	#endif
				+	#endif

			archive_write_set_format_warc.c

				-	/*len*/sizeof(warcinfo) - 1U,
				+	/*len*/sizeof(warcinfo) - 1U

				-	/*len*/0,
				-	/*len*/0

		new

			archive_cryptor.c (replaced archive_crypto.c)
			archive_digest.c
			archive_disk_acl_darwin.c
			archive_disk_acl_freebsd.c
			archive_disk_acl_linux.c
			archive_disk_acl_sunos.c
			archive_hmac.c
			archive_pack_dev.c
			archive_read_add_passphrase.c
			archive_read_extract2.c
			archive_read_support_filter_lz4.c
			archive_read_support_filter_zstd.c
			archive_read_support_format_warc.c
			archive_version_details.c
			archive_write_add_filter_lz4.c
			archive_write_add_filter_zstd.c
			archive_write_set_format_filter_by_ext.c
			archive_write_set_format_raw.c
			archive_write_set_format_warc.c
			archive_write_set_passphrase.c
			xxhash.c

   
