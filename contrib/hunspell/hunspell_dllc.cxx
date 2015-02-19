/* -*- mode: c; indent-width: 4; -*- */
/* ==no-notice==
 * hunspelldll cdecl interface
 * This file is part of the GRIEF Editor and is placed in the public domain.
 *
 * Externals:
 *  _hunspell_initialize
 *  _hunspell_initialize_key
 *  _hunspell_uninitialize
 *  _hunspell_spell
 *  _hunspell_suggest
 *  _hunspell_free_list
 *  _hunspell_get_dic_encoding
 *  _hunspell_add
 *  _hunspell_add_with_affix
 * 
 */

#include "hunspell.hxx"
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>

extern "C" {

#define DLLLINKAGE      __declspec(dllexport)
#define DLLENTRY        __cdecl


DLLLINKAGE void * DLLENTRY
hunspell_initialize(char *aff_file, char *dict_file)
{
    Hunspell * pMS = new Hunspell(aff_file, dict_file);
    return pMS;
}


DLLLINKAGE void * DLLENTRY
hunspell_initialize_key(char *aff_file, char *dict_file, char * key)
{
    Hunspell * pMS = new Hunspell(aff_file, dict_file, key);
    return pMS;
}


DLLLINKAGE void DLLENTRY
hunspell_uninitialize(Hunspell *pMS)
{
    delete pMS;
}


DLLLINKAGE int DLLENTRY
hunspell_spell(Hunspell *pMS, char *word)
{
    return pMS->spell(word);
}


DLLLINKAGE int DLLENTRY
hunspell_suggest(Hunspell *pMS, char *word, char ***slst)
{
    return pMS->suggest(slst, word);
}


DLLLINKAGE void DLLENTRY
hunspell_free_list(Hunspell *pMS, char ***slst, int len)
{
    pMS->free_list(slst, len);
}


DLLLINKAGE char * DLLENTRY
hunspell_get_dic_encoding(Hunspell *pMS)
{
    return pMS->get_dic_encoding();
}


DLLLINKAGE int DLLENTRY
hunspell_add(Hunspell *pMS, char *word)
{
    return pMS->add(word);
}


DLLLINKAGE int DLLENTRY
hunspell_add_with_affix(Hunspell *pMS, char *word, char *modelword)
{
    return pMS->add_with_affix(word, modelword);
}

}   //extern

