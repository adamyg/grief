/* -*- indent-width: 4; encoding: utf-8; -*- */
/* $Id: spellinfo.cr,v 1.4 2014/10/27 23:28:28 ayoung Exp $
 * Spell checker information.
 *
 *
 */

#include "grief.h"

static void             spellinfo_cb(int ident, string name, int p1, int p2);

//  Common miss-spellings, build auto-correct functionality
//
//      See:
//          Wikipedia lists
//          Wikitravel lists
//          Oxford dictionaries (100 top)
//
//      Example:
//          abotu       --> about
//

//  Common language codes
//
//      <language_<country>
//
//      IEFT/RFC 1766/RFC 3006/RFC 4647/RFC 5646 -
//
//          ISO639      - Language codes.
//          ISO3166     - Country codes.
//
//      www.iana.org/protocols
//      www.iana.org/assignments/language-subtab-regtistry
//          Language
//          Region
//      www.w3.org/International/articles/bcp47
//
//      CLDR data.
//
//  Language Tag:
//      sl-Latn-IT-rozaj-1994-r-foovia-x-mine
//
//      ^ ISO 639-1/2
//         ^ ISO 15924 script codes
//              ^ ISO 3166
//                 ^ Registered variants
//                            ^ Extensions (none at present)
//                                     ^ Private use
//
//  Examples:
//      de_DE   de          Deutsch
//      en_US   en          English
//      en_AU   en_AU       Australian English
//      en_CA   en_CA       Canadian English
//      en_GB   en_GB       British English
//      es_ES   es          Español
//      fr_FR   fr          Français
//      it_IT   it          Italiano
//      nl_NL   nl          Nederlands
//      pt_PT   pt          Português
//      pt_BR   pt_BR       Português do Brasil
//      sv_SE   sv          Svenska
//      hu_HU   hu          Magyar (MySpellX)
//
void
spellinfo()
{
    list dictionaries = spell_control(SPELL_DICTIONARIES);
    int dialog =
        dialog_create(make_list(
            DLGA_TITLE,                         "Spell Info",
            DLGA_CALLBACK,                      "::spellinfo_cb",
            DLGC_CONTAINER,
                DLGA_ATTACH_TOP,
                DLGA_ALIGN_NE,
                DLGC_LABEL,
                    DLGA_ALIGN_E,
                    DLGA_NAME,                  "description",
                    DLGA_VALUE,                 spell_control(SPELL_DESCRIPTION),
                    DLGA_ROWS,                  1,
                    DLGA_COLS,                  48,
                    DLGA_ALLOW_FILLX,
            DLGC_END,
            DLGC_LIST_BOX,
                DLGA_LABEL,                     "Dictionaries",
                DLGA_ROWS,                      10,
                DLGA_COLS,                      30,
                DLGA_LBELEMENTS,                dictionaries,
                DLGA_ALLOW_FILLX,
            DLGC_CONTAINER,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,                 "&Done",
                    DLGA_NAME,                  "done",
                    DLGA_ALLOW_FILLX,
                    DLGA_DEFAULT_BUTTON,
            DLGC_END
            ));

    dialog_run(dialog);
    dialog_delete(dialog);
}


static void
spellinfo_cb(int ident, string name, int p1, int p2)
{
    UNUSED(ident, p2);
    switch (p1) {
    case DLGE_INIT:
        break;
    case DLGE_BUTTON:
        switch(name) {
        case "done":
            dialog_exit();
            break;
        case "help":
            execute_macro("explain spellinfo");
            break;
        }
        break;
    }
}

/*eof*/
