#include <edidentifier.h>
__CIDENT_RCSID(gr_tagsex_c,"$Id: tagsex.c,v 1.13 2014/10/22 02:33:22 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*$Id: tagsex.c,v 1.13 2014/10/22 02:33:22 ayoung Exp $
 *
 *
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

/*
 *  INCLUDE FILES
 */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>                          /* to declare off_t */

#include "tagsex.h"

#if defined(DEBUG)
#define DD(x)       trace_log x
#else
#define DD(x)
#endif

/*
 *  MACROS
 */
#define TAB '\t'

/*
 *  DATA DECLARATIONS
 */
typedef struct {
    size_t size;
    char *buffer;
} vstring;

/* Information about current tag file */
typedef struct sTagFile {
    short initialized;                          /* has the file been opened and this structure initialized? */
    short format;                               /* format of tag file */
    exsortType sortMethod;                      /* how is the tag file sorted? */
    FILE* fp;                                   /* pointer to file structure */
    off_t pos;                                  /* file position of first character of `line' */
    off_t size;                                 /* size of tag file in seekable positions */
    vstring line;                               /* last line read */
    vstring name;                               /* name of tag in last line read */

    /* defines tag search state */
    struct {
            off_t pos;                          /* file position of last match for tag */
            const char *name;                   /* name of tag last searched for */
            size_t nameLength;                  /* length of name for partial matches */
            short partial;                      /* peforming partial match */
            short ignorecase;                   /* ignoring case */
    } search;

    /* miscellaneous extension fields */
    struct {
        unsigned short max;                     /* number of entries in `list' */
        extagExtensionField *list;              /* list of key value pairs */
    } fields;

    /* buffers to be freed at close */
    struct {
        char *author;                           /* name of program author */
        char *name;                             /* name of program */
        char *url;                              /* URL of distribution */
        char *version;                          /* program version */
    } program;
} tagFile;

/*
 *  DATA DEFINITIONS
 */
static const char *const EmptyString = "";
static const char *const PseudoTagPrefix = "!_";

/*
 *  FUNCTION DEFINITIONS
 */

/*
 * Compare two strings, ignoring case.
 * Return 0 for match, < 0 for smaller, > 0 for bigger
 * Make sure case is folded to uppercase in comparison (like for 'sort -f')
 * This makes a difference when one of the chars lies between upper and lower
 * ie. one of the chars [ \ ] ^ _ ` for ascii. (The '_' in particular !)
 */
static int
struppercmp (const char *s1, const char *s2)
{
    int result;

    do {
        result = toupper(*((unsigned char *)s1)) - toupper(*((unsigned char *)s2));
    } while (0 == result && *s1++ && *s2++);
    return result;
}


static int
strnuppercmp (const char *s1, const char *s2, size_t n)
{
    int result;

    do {
        result = toupper(*((unsigned char *)s1)) - toupper(*((unsigned char *)s2));
    } while (0 == result && --n > 0 && *s1++ && *s2++);
    return result;
}


static int
growString (vstring *s)
{
    int result = 0;
    size_t newLength;
    char *newLine;

    if (0 == s->size) {
        newLength = 128;
        newLine   = (char *) malloc (newLength);
        *newLine  = '\0';
    } else {
        newLength = 2 * s->size;
        newLine   = (char *) realloc (s->buffer, newLength);
    }

    if (NULL == newLine) {
        perror ("string too large");
    } else {
        s->buffer = newLine;
        s->size = newLength;
        result = 1;
    }
    return result;
}

/* Copy name of tag out of tag line */
static void
copyName (tagFile *const file)
{
    size_t length;
    const char *end = strchr (file->line.buffer, '\t');

    if (end == NULL)
    {
        end = strchr (file->line.buffer, '\n');
        if (end == NULL)
            end = strchr (file->line.buffer, '\r');
    }

    if (end != NULL)
        length = end - file->line.buffer;
    else
        length = strlen (file->line.buffer);
    while (length >= file->name.size)
        growString (&file->name);
    strncpy (file->name.buffer, file->line.buffer, length);
    file->name.buffer [length] = '\0';
}


static int
readTagLineRaw (tagFile *const file)
{
    int result = 1;
    int reReadLine;

    /*  If reading the line places any character other than a null or a
     *  newline at the last character position in the buffer (one less than
     *  the buffer size), then we must resize the buffer and reattempt to read
     *  the line.
     */
    do {
        char *const pLastChar = file->line.buffer + file->line.size - 2;
        char *line;

        file->pos = ftell (file->fp);
        reReadLine = 0;
        *pLastChar = '\0';
        line = fgets (file->line.buffer, (int) file->line.size, file->fp);
        if (line == NULL)
        {
            /* read error */
            result = 0;
        }
        else if (*pLastChar != '\0'  &&
                    *pLastChar != '\n'  &&  *pLastChar != '\r')
        {
            /*  buffer overflow */
            growString (&file->line);
            fseek (file->fp, file->pos, SEEK_SET);
            reReadLine = 1;
        }
        else
        {
            size_t i = strlen (file->line.buffer);
            while (i > 0  &&
                   (file->line.buffer [i - 1] == '\n' || file->line.buffer [i - 1] == '\r')) {
                file->line.buffer [i - 1] = '\0';
                --i;
            }
        }
    } while (reReadLine && result);
    if (result)
        copyName (file);
    return result;
}


static int
readTagLine (tagFile *const file)
{
    int result;
    do {
        result = readTagLineRaw (file);
    } while (result && *file->name.buffer == '\0');
    return result;
}


static tagResult
growFields (tagFile *const file)
{
    tagResult result = TagFailure;
    unsigned short newCount = 2 * file->fields.max;
    extagExtensionField *newFields = (extagExtensionField*)
            realloc (file->fields.list, newCount * sizeof (extagExtensionField));

    if (newFields == NULL)
        perror ("too many extension fields");
    else
    {
        file->fields.list = newFields;
        file->fields.max = newCount;
        result = TagSuccess;
    }
    return result;
}


static void
parseExtensionFields (
    tagFile *const file, extagEntry *const entry, char *const string)
{
    char *p = string;

    while (p != NULL  &&  *p != '\0') {
        while (*p == TAB)
            *p++ = '\0';

        if (*p != '\0') {
            char *colon, *field = p;

            p = strchr (p, TAB);
            if (p != NULL)
                *p++ = '\0';
            colon = strchr (field, ':');
            if (colon == NULL)
                entry->kind = field;
            else
            {
                const char *key = field;
                const char *value = colon + 1;
                *colon = '\0';
                if (strcmp (key, "kind") == 0)
                    entry->kind = value;
                else if (strcmp (key, "file") == 0)
                    entry->fileScope = 1;
                else if (strcmp (key, "line") == 0)
                    entry->address.lineNumber = atol (value);
                else
                {
                    if (entry->fields.count == file->fields.max)
                        growFields (file);
                    file->fields.list [entry->fields.count].key = key;
                    file->fields.list [entry->fields.count].value = value;
                    ++entry->fields.count;
                }
            }
        }
    }
}


static void
parseTagLine (tagFile *file, extagEntry *const entry)
{
    int i;
    char *p = file->line.buffer;
    char *tab = strchr (p, TAB);
    int fieldsPresent = 0;

    entry->fields.list = NULL;
    entry->fields.count = 0;
    entry->kind = NULL;
    entry->fileScope = 0;

    entry->name = p;
    if (tab != NULL) {
        *tab = '\0';
        p = tab + 1;
        entry->file = p;
        tab = strchr (p, TAB);
        if (tab != NULL)
        {
            *tab = '\0';
            p = tab + 1;
            if (*p == '/'  ||  *p == '?')
            {
                /* parse pattern */
                int delimiter = *(unsigned char*) p;
                entry->address.lineNumber = 0;
                entry->address.pattern = p;
                do
                {
                    p = strchr (p + 1, delimiter);
                } while (p != NULL  &&  *(p - 1) == '\\');
                if (p == NULL)
                {
                    /* invalid pattern */
                }
                else
                    ++p;
            }
            else if (isdigit ((int) *(unsigned char*) p))
            {
                /* parse line number */
                entry->address.pattern = p;
                entry->address.lineNumber = atol (p);
                while (isdigit ((int) *(unsigned char*) p))
                    ++p;
            }
            else
            {
                /* invalid pattern */
            }
            fieldsPresent = (strncmp (p, ";\"", 2) == 0);
            *p = '\0';
            if (fieldsPresent)
                parseExtensionFields (file, entry, p + 2);
        }
    }
    if (entry->fields.count > 0)
        entry->fields.list = file->fields.list;
    for (i = entry->fields.count  ;  i < file->fields.max  ;  ++i)
    {
        file->fields.list [i].key = NULL;
        file->fields.list [i].value = NULL;
    }
}


static char *
duplicate (const char *str)
{
    char *result = NULL;
    if (str != NULL) {
        result = (char*) malloc (strlen (str) + 1);
        if (result)
            strcpy (result, str);
    }
    return result;
}


static void
readPseudoTags (tagFile *const file, extagFileInfo *const info)
{
    fpos_t startOfLine;
    const size_t prefixLength = strlen (PseudoTagPrefix);

    file->format            = 0;
    file->sortMethod        = EXTAG_UNSORTED;
    file->program.author    = NULL;
    file->program.name      = NULL;
    file->program.url       = NULL;
    file->program.version   = NULL;

    for (;;)
    {
        fgetpos (file->fp, &startOfLine);
        if (! readTagLine (file))
            break;

        if (strncmp (file->line.buffer, PseudoTagPrefix, prefixLength) != 0)
            break;

        else
        {
            extagEntry entry;
            const char *key, *value;

            parseTagLine (file, &entry);
            key = entry.name + prefixLength;
            value = entry.file;
            if (strcmp (key, "TAG_FILE_SORTED") == 0)
                file->sortMethod = (exsortType) atoi (value);
            else if (strcmp (key, "TAG_FILE_FORMAT") == 0)
                file->format = (short)atoi (value);
            else if (strcmp (key, "TAG_PROGRAM_AUTHOR") == 0)
                file->program.author = duplicate (value);
            else if (strcmp (key, "TAG_PROGRAM_NAME") == 0)
                file->program.name = duplicate (value);
            else if (strcmp (key, "TAG_PROGRAM_URL") == 0)
                file->program.url = duplicate (value);
            else if (strcmp (key, "TAG_PROGRAM_VERSION") == 0)
                file->program.version = duplicate (value);
        }
    }
    fsetpos (file->fp, &startOfLine);

    if (info != NULL)
    {
        info->file.format     = file->format;
        info->file.sort       = file->sortMethod;
        info->program.author  = file->program.author;
        info->program.name    = file->program.name;
        info->program.url     = file->program.url;
        info->program.version = file->program.version;
    }

    DD (("tagsex: (D) header: format(%d), sort(%d), author(%s) version(%s/%s)\n",
        file->format, file->sortMethod,
        file->program.author  ? file->program.author : "n.a",
        file->program.name    ? file->program.name : "n.a",
        file->program.version ? file->program.version : "n.a" ));
}


static void
gotoFirstLogicalTag (tagFile *const file)
{
    fpos_t startOfLine;
    const size_t prefixLength = strlen (PseudoTagPrefix);

    rewind (file->fp);
    for (;;)
    {
        fgetpos (file->fp, &startOfLine);
        if (! readTagLine (file))
            break;
        if (strncmp (file->line.buffer, PseudoTagPrefix, prefixLength) != 0)
            break;
    }
    fsetpos (file->fp, &startOfLine);
}


static tagFile *
initialize(const char *const filePath, extagFileInfo *const info)
{
    tagFile *result = (tagFile*) malloc (sizeof (tagFile));

    if (result != NULL) {
        memset (result, 0, sizeof (tagFile));
        growString (&result->line);
        growString (&result->name);
        result->fields.max = 20;
        result->fields.list = (extagExtensionField*) malloc (
                                    result->fields.max * sizeof (extagExtensionField));

#if defined(DOSISH)         /* binary-mode */
        result->fp = fopen(filePath, "rb");
#else
        result->fp = fopen(filePath, "r");
#endif
        if (result->fp == NULL) {
            DD (("tagsex: (D) open error '%s' : %s\n", filePath, strerror(errno)));
            free (result);
            result = NULL;
            info->status.error_number = errno;

        } else {
            DD (("tagsex: (D) opened (%s)\n", filePath));
            fseek (result->fp, 0, SEEK_END);
            result->size = ftell (result->fp);
            rewind (result->fp);
            readPseudoTags (result, info);
            info->status.opened = 1;
            result->initialized = 1;
        }
    }
    return result;
}


static void
terminate(tagFile *const file)
{
    fclose(file->fp);

    free(file->line.buffer);
    free(file->name.buffer);
    free(file->fields.list);

    if (file->program.author != NULL)
        free(file->program.author);
    if (file->program.name != NULL)
        free(file->program.name);
    if (file->program.url != NULL)
        free(file->program.url);
    if (file->program.version != NULL)
        free(file->program.version);

    memset(file, 0, sizeof (tagFile));

    free(file);
}


static tagResult
readNext(tagFile *const file, extagEntry *const entry)
{
    tagResult result = TagFailure;

    if (file == NULL  ||  ! file->initialized)
        result = TagFailure;
    else if (! readTagLine (file))
        result = TagFailure;
    else
    {
        if (entry != NULL)
            parseTagLine (file, entry);
        result = TagSuccess;
    }
    return result;
}


static const char *
readFieldValue (
    const extagEntry *const entry, const char *const key)
{
    const char *result = NULL;
    int i;

    if (strcmp (key, "kind") == 0)
        result = entry->kind;
    else if (strcmp (key, "file") == 0)
        result = EmptyString;
    else for (i = 0  ;  i < entry->fields.count  &&  result == NULL  ;  ++i)
        if (strcmp (entry->fields.list [i].key, key) == 0)
            result = entry->fields.list [i].value;
    return result;
}


static int
readTagLineSeek (tagFile *const file, const off_t pos)
{
    int result = 0;

    if (fseek (file->fp, pos, SEEK_SET) == 0) {
        result = readTagLine (file);            /* read probable partial line */
        if (pos > 0  &&  result)
            result = readTagLine (file);        /* read complete line */
    }
    return result;
}


static int
nameComparison (tagFile *const file)
{
    int result;

    if (file->search.ignorecase) {
        if (file->search.partial)
            result = strnuppercmp (file->search.name, file->name.buffer, file->search.nameLength);
        else
            result = struppercmp (file->search.name, file->name.buffer);

    } else {
        if (file->search.partial)
            result = strncmp (file->search.name, file->name.buffer, file->search.nameLength);
        else
            result = strcmp (file->search.name, file->name.buffer);
    }

    DD(("tagsex: (D)    %s : %d\n", file->name.buffer, result));
    return result;
}


static void
findFirstNonMatchBefore (tagFile *const file)
{
#define JUMP_BACK 512
    int more_lines, comp;
    off_t start = file->pos;
    off_t pos = start;

    do {
        if (pos < (off_t) JUMP_BACK)
            pos = 0;
        else
            pos = pos - JUMP_BACK;
        more_lines = readTagLineSeek (file, pos);
        comp = nameComparison (file);
    } while (more_lines  &&  comp == 0  &&  pos > 0  &&  pos < start);
}


static tagResult
findFirstMatchBefore (tagFile *const file)
{
    tagResult result = TagFailure;
    int more_lines;

    off_t start = file->pos;
    findFirstNonMatchBefore (file);
    do {
        more_lines = readTagLine (file);
        if (nameComparison (file) == 0)
            result = TagSuccess;
    } while (more_lines  &&  result != TagSuccess  &&  file->pos < start);
    return result;
}


static tagResult
findBinary (tagFile *const file)
{
    tagResult result = TagFailure;
    off_t lower_limit = 0;
    off_t upper_limit = file->size;
    off_t last_pos = 0;
    off_t pos = upper_limit / 2;

    while (result != TagSuccess) {
        if (! readTagLineSeek (file, pos))
        {
            /* in case we fell off end of file */
            result = findFirstMatchBefore (file);
            break;
        }
        else if (pos == last_pos)
        {
            /* prevent infinite loop if we backed up to beginning of file */
            break;
        }
        else
        {
            const int comp = nameComparison (file);
            last_pos = pos;
            if (comp < 0)
            {
                upper_limit = pos;
                pos = lower_limit + ((upper_limit - lower_limit) / 2);
            }
            else if (comp > 0)
            {
                lower_limit = pos;
                pos = lower_limit + ((upper_limit - lower_limit) / 2);
            }
            else if (pos == 0)
                result = TagSuccess;
            else
                result = findFirstMatchBefore (file);
        }
    }
    return result;
}


static tagResult
findSequential (tagFile *const file)
{
    tagResult result = TagFailure;
    if (file->initialized)
    {
        while (result == TagFailure  &&  readTagLine (file))
        {
            if (nameComparison (file) == 0)
                result = TagSuccess;
        }
    }
    return result;
}


static tagResult
find (tagFile *const file, extagEntry *const entry, const char *const name, const int options)
{
    tagResult result = TagFailure;

    file->search.name = name;
    file->search.nameLength = strlen (name);
    file->search.partial = (short)((options & TAG_FPARTIALMATCH) != 0);
    file->search.ignorecase = (short)((options & TAG_FIGNORECASE) != 0);
    DD(("tagsex: (D) name(%s/%d) partial(%d), ignorecase(%d)\n",
            file->search.name, file->search.nameLength,
            file->search.partial, file->search.ignorecase));

    fseek (file->fp, 0, SEEK_END);
    file->size = ftell (file->fp);
    rewind (file->fp);
    if ((file->sortMethod == EXTAG_SORTED      && !file->search.ignorecase) ||
        (file->sortMethod == EXTAG_FOLDSORTED  &&  file->search.ignorecase))
    {
        DD(("tagsex: (D) performing binary search\n"));
        result = findBinary (file);
    }
    else
    {
        DD(("tagsex: (D) performing sequential search\n"));
        result = findSequential (file);
    }

    if (result != TagSuccess)
    {
        DD(("tagsex: (D) not found\n"));
        file->search.pos = file->size;
    }
    else
    {
        DD(("tagsex: (D) match found\n"));
        file->search.pos = file->pos;
        if (entry != NULL)
            parseTagLine (file, entry);
    }
    return result;
}


static tagResult
findNext (tagFile *const file, extagEntry *const entry)
{
    tagResult result = TagFailure;

    if ((file->sortMethod == EXTAG_SORTED      && !file->search.ignorecase) ||
        (file->sortMethod == EXTAG_FOLDSORTED  &&  file->search.ignorecase)) {
        result = extagsNext (file, entry);
        if (result == TagSuccess  && nameComparison (file) != 0)
            result = TagFailure;

    } else {
        result = findSequential (file);
        if (result == TagSuccess  &&  entry != NULL)
            parseTagLine (file, entry);
    }
    return result;
}


/*
 *  EXTERNAL INTERFACE
 */
void *
extagsOpen (const char *const filePath, extagFileInfo *const info)
{
    return (void *)initialize (filePath, info);
}


tagResult
extagsSetSortType (void *const vfile, const exsortType type)
{
    tagFile *const file = vfile;
    tagResult result = TagFailure;

    if (file != NULL && file->initialized) {
        file->sortMethod = type;
        result = TagSuccess;
    }
    return result;
}


tagResult
extagsFirst (void *const vfile, extagEntry *const entry)
{
    tagFile *const file = vfile;
    tagResult result = TagFailure;

    if (file != NULL  &&  file->initialized)
    {
        gotoFirstLogicalTag (file);
        result = readNext (file, entry);
    }
    return result;
}


tagResult
extagsNext (void *const vfile, extagEntry *const entry)
{
    tagFile *const file = vfile;
    tagResult result = TagFailure;

    if (file != NULL  &&  file->initialized) {
        result = readNext (file, entry);
    }
    return result;
}


const char *
extagsField (const extagEntry *const entry, const char *const key)
{
    const char *result = NULL;

    if (entry != NULL) {
        result = readFieldValue (entry, key);
    }
    return result;
}


tagResult
extagsFind (void *const vfile, extagEntry *const entry, const char *const name, const int options)
{
    tagFile *const file = vfile;
    tagResult result = TagFailure;

    if (file != NULL  &&  file->initialized) {
        result = find (file, entry, name, options);
    }
    return result;
}


tagResult
extagsFindNext (void *const vfile, extagEntry *const entry)
{
    tagFile *const file = vfile;
    tagResult result = TagFailure;

    if (file != NULL && file->initialized) {
        result = findNext (file, entry);
    }
    return result;
}


tagResult
extagsClose (void *const vfile)
{
    tagFile *const file = vfile;
    tagResult result = TagFailure;

    if (file != NULL && file->initialized) {
        terminate (file);
        result = TagSuccess;
    }
    return result;
}

/* vi:set tabstop=8 shiftwidth=4: */
