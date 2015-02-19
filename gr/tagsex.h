#ifndef GR_TAGSEX_H_INCLUDED
#define GR_TAGSEX_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_tagsex_h,"$Id: tagsex.h,v 1.7 2014/10/22 02:33:22 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
*/

#ifdef __cplusplus
extern "C" {
#endif

/*
*  MACROS
*/

/* Options for tagsSetSortType() */
typedef enum {
    EXTAG_UNSORTED, EXTAG_SORTED, EXTAG_FOLDSORTED
} exsortType;

/* Options for tagsFind() */
#ifndef TAG_FPARTIALMATCH
#define TAG_FPARTIALMATCH   0x01
#define TAG_FIGNORECASE     0x02
#endif

/*
*  DATA DECLARATIONS
*/

typedef enum { TagFailure = 0, TagSuccess = 1 } tagResult;

/* This structure contains information about the tag file. */
typedef struct {

	struct {
		    /* was the tag file successfully opened? */
		int opened;

		    /* errno value when 'opened' is false */
		int error_number;
	} status;

	    /* information about the structure of the tag file */
	struct {
		    /* format of tag file (1 = original, 2 = extended) */
		short format;

		    /* how is the tag file sorted? */
		exsortType sort;
	} file;


	    /* information about the program which created this tag file */
	struct {
		    /* name of author of generating program (may be null) */
		const char *author;

		    /* name of program (may be null) */
		const char *name;

		    /* URL of distribution (may be null) */
		const char *url;

		    /* program version (may be null) */
		const char *version;
	} program;

} extagFileInfo;

/* This structure contains information about an extension field for a tag.
 * These exist at the end of the tag in the form "key:value").
 */
typedef struct {

	    /* the key of the extension field */
	const char *key;

	    /* the value of the extension field (may be an empty string) */
	const char *value;

} extagExtensionField;

/* This structure contains information about a specific tag. */
typedef struct {

	    /* name of tag */
	const char *name;

	    /* path of source file containing definition of tag */
	const char *file;

	    /* address for locating tag in source file */
	struct {
		    /* pattern for locating source line
		     * (may be NULL if not present) */
		const char *pattern;

		    /* line number in source file of tag definition
		     * (may be zero if not known) */
		unsigned long lineNumber;
	} address;

	    /* kind of tag (may by name, character, or NULL if not known) */
	const char *kind;

	    /* is tag of file-limited scope? */
	short fileScope;

	    /* miscellaneous extension fields */
	struct {
		    /* number of entries in `list' */
		unsigned short count;

		    /* list of key value pairs */
		extagExtensionField *list;
	} fields;

} extagEntry;


/*
*  FUNCTION PROTOTYPES
*/

/*
*  This function must be called before calling other functions in this
*  library. It is passed the path to the tag file to read and a (possibly
*  null) pointer to a structure which, if not null, will be populated with
*  information about the tag file. If successful, the function will return a
*  handle which must be supplied to other calls to read information from the
*  tag file, and info.status.opened will be set to true. If unsuccessful,
*  info.status.opened will be set to false and info.status.error_number will
*  be set to the errno value representing the system error preventing the tag
*  file from being successfully opened.
*/
extern void *               extagsOpen(const char *const filePath, extagFileInfo *const info);

/*
*  This function allows the client to override the normal automatic detection
*  of how a tag file is sorted. Permissible values for `type' are
*  TAG_UNSORTED, TAG_SORTED, TAG_FOLDSORTED. Tag files in the new extended
*  format contain a key indicating whether or not they are sorted. However,
*  tag files in the original format do not contain such a key even when
*  sorted, preventing this library from taking advantage of fast binary
*  lookups. If the client knows that such an unmarked tag file is indeed
*  sorted (or not), it can override the automatic detection. Note that
*  incorrect lookup results will result if a tag file is marked as sorted when
*  it actually is not. The function will return TagSuccess if called on an
*  open tag file or TagFailure if not.
*/
extern tagResult            extagsSetSortType(void *const file, const exsortType type);

/*
*  Reads the first tag in the file, if any. It is passed the handle to an
*  opened tag file and a (possibly null) pointer to a structure which, if not
*  null, will be populated with information about the first tag file entry.
*  The function will return TagSuccess another tag entry is found, or
*  TagFailure if not (i.e. it reached end of file).
*/
extern tagResult            extagsFirst(void *const file, extagEntry *const entry);

/*
*  Step to the next tag in the file, if any. It is passed the handle to an
*  opened tag file and a (possibly null) pointer to a structure which, if not
*  null, will be populated with information about the next tag file entry. The
*  function will return TagSuccess another tag entry is found, or TagFailure
*  if not (i.e. it reached end of file). It will always read the first tag in
*  the file immediately after calling extags_Open().
*/
extern tagResult            extagsNext(void *const file, extagEntry *const entry);

/*
*  Retrieve the value associated with the extension field for a specified key.
*  It is passed a pointer to a structure already populated with values by a
*  previous call to extags_Next(), extags_Find(), or extags_FindNext(), and a string
*  containing the key of the desired extension field. If no such field of the
*  specified key exists, the function will return null.
*/
extern const char *         extagsField(const extagEntry *const entry, const char *const key);

/*
*  Find the first tag matching `name'. The structure pointed to by `entry'
*  will be populated with information about the tag file entry. If a tag file
*  is sorted using the C locale, a binary search algorithm is used to search
*  the tag file, resulting in very fast tag lookups, even in huge tag files.
*  Various options controlling the matches can be combined by bit-wise or-ing
*  certain values together. The available values are:
*
*   TAG_FPARTIALMATCH (0x01)
*       Tags whose leading characters match `name' will qualify, otherwise
*       Only tags whose full lengths match `name' will qualify.
*
*   TAG_FIGNORECASE (0x02)
*       Matching will be performed in a case-insenstive manner. Note that
*       this disables binary searches of the tag file,
*
*       Otherwise, matching will be performed in a case-senstive manner.
*       Note that this enables binary searches of the tag file.
*
*  The function will return TagSuccess if a tag matching the name is found, or
*  TagFailure if not.
*/
extern tagResult            extagsFind(void *const file, extagEntry *const entry, const char *const name, const int options);

/*
*  Find the next tag matching the name and options supplied to the most recent
*  call to extags_Find() for the same tag file. The structure pointed to by
*  `entry' will be populated with information about the tag file entry. The
*  function will return TagSuccess if another tag matching the name is found,
*  or TagFailure if not.
*/
extern tagResult            extagsFindNext(void *const file, extagEntry *const entry);

/*
*  Call extags_Terminate() at completion of reading the tag file, which will
*  close the file and free any internal memory allocated. The function will
*  return TagFailure is no file is currently open, TagSuccess otherwise.
*/
extern tagResult            extagsClose(void *const file);

#ifdef __cplusplus
};
#endif

/* vi:set tabstop=8 shiftwidth=4: */
#endif /*GR_TAGSEX_H_INCLUDED*/
