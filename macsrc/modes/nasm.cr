/* -*- mode: cr; indent-width: 4; -*-
 * $Id: nasm.cr,v 1.12 2024/07/30 16:29:20 cvsuser Exp $
 * GNU assembler (gas) mode
 *
 *
 */

#include "../grief.h"

#define MODENAME "NASM"

void
main()
{
    create_syntax(MODENAME);

    /*
     *  Standard syntax engine
     */
    syntax_token(SYNT_COMMENT,      ";");
    syntax_token(SYNT_QUOTE,        "\\");
    syntax_token(SYNT_PREPROCESSOR, ".");
    syntax_token(SYNT_CHARACTER,    "\'");
    syntax_token(SYNT_STRING,       "\"");
    syntax_token(SYNT_BRACKET,      "([{", ")]}");
    syntax_token(SYNT_OPERATOR,     "-%+/&*=<>|!~^");
    syntax_token(SYNT_DELIMITER,    ",;.?:");
    syntax_token(SYNT_KEYWORD,      "a-zA-Z_", "0-9a-zA-Z_");
    syntax_token(SYNT_NUMERIC,      "-+0-9a-fA-F.xXL");

    /*
     *  Options/
     *      case insensitive keywords
     */
    set_syntax_flags(SYNF_CASEINSENSITIVE);

    /*
     *  DFA syntax engine
     */
    syntax_rule("^[ \t]*#", "quick:preprocessor");
    syntax_rule("^[ \t]*\\%{?[^%\\$\\+\\-0-9]", "quick:preprocessor");
    syntax_rule("^%$", "preprocessor");
    syntax_rule("%([%\\$]?-?[0-9A-Za-z_\\.\\?\\$~@]+|{[^}]*}?)", "preprocessor");

    syntax_rule(";.*$", "spell,todo,quick:comment");

    syntax_rule("[A-Za-z_\\.\\?][A-Za-z0-9_\\.\\?\\$#@~]*", "keyword:normal");
    syntax_rule("$([A-Za-z_\\.\\?][A-Za-z0-9_\\.\\?\\$#@~]*)?", "normal");
    syntax_rule("[0-9]+(\\.[0-9]*)?([Ee][\\+\\-]?[0-9]*)?",  "number");

    syntax_rule("[0-9]+[QqBb]", "number");
    syntax_rule("(0x|\\$[0-9A-Fa-f])[0-9A-Fa-f]*", "number");
    syntax_rule("[0-9A-Fa-f]+[Hh]", "number");

    syntax_rule("\"[^\"]*\"", "string");
    syntax_rule("\"[^\"]*$", "string");
    syntax_rule("'[^']*'", "string");
    syntax_rule("'[^']*$", "string");

    syntax_rule("[\\(\\)\\[\\],:]*", "delimiter");
    syntax_rule("[\\|\\^&<>\\+\\-\\*/%~]*", "operator");

    syntax_rule("[ \t]*", "normal");
    syntax_rule(".", "normal");

    syntax_build(__COMPILETIME__);              /* build and auto-cache */

    /*
     *  Keywords
     */
    define_keywords(0,
        "ahalaxbhblbpbtbxchclcscxdbdddhdidldqdsdtdwdxes"+
        "fsgsinjajbjcjejgjljojpjsjzorsispssto", 2);
    define_keywords(0,
        "a16a32aaaaadaamaasadcaddandbsfbsrbtcbtrbtscbw"+
        "cdqclccldclicmccmpcr0cr2cr3cr4cwddaadasdecdiv"+
        "dr0dr1dr2dr3dr6dr7eaxebpebxecxediedxequesiesp"+
        "farfldfsthltincintjaejbejgejlejmpjnajnbjncjne"+
        "jngjnljnojnpjnsjnzjpejpolarldslealeslfslgslsl"+
        "lssltrmm0mm1mm2mm3mm4mm5mm6mm7movmulnegnopnot"+
        "o16o32outpopporrclrcrrepretrolrorrsmsalsarsbb"+
        "segshlshrsmist0st1st2st3st4st5st6st7stcstdsti"+
        "strsubtr3tr4tr5tr6tr7wrtxor", 3);
    define_keywords(0,
        "arplbytecallcltscwdeemmsfabsfaddfbldfchsfcom"+
        "fcosfdivfenifildfistfld1fldzfmulfnopfsinfstp"+
        "fsubftstfxamfxchibtsidivimulinsbinsdinswint1"+
        "int3intoinvdiretjcxzjnaejnbejngejnlelahflgdt"+
        "lidtlldtlmswlocklongloopmovdmovqnearpandpopa"+
        "popfpushpxorreperepzresbresdresqrestreswretf"+
        "retnsahfsalcsetasetbsetcsetesetgsetlsetosetp"+
        "setssetzsgdtshldshrdsidtsldtsmswtestumovverr"+
        "verwwaitwordxaddxbtsxchg", 4);
    define_keywords(0,
        "boundbswapcmovacmovbcmovccmovecmovgcmovlcmovo"+
        "cmovpcmovscmovzcmpsbcmpsdcmpswcpuiddwordenter"+
        "f2xm1faddpfbstpfclexfcomifcompfdisifdivpfdivr"+
        "ffreefiaddficomfidivfimulfinitfistpfisubfldcw"+
        "fldpifmulpfpremfptanfsavefsqrtfstcwfstswfsubp"+
        "fsubrfucomfyl2xicebpint01iretdiretwjecxzleave"+
        "lodsblodsdlodswloopeloopzmovsbmovsdmovswmovsx"+
        "movzxoutsboutsdoutswpaddbpadddpaddwpandnpopad"+
        "popawpopfdpopfwpslldpsllqpsllwpsradpsrawpsrld"+
        "psrlqpsrlwpsubbpsubdpsubwpushapushfqwordrdmsr"+
        "rdpmcrdtscrepnerepnzscasbscasdscaswsetaesetbe"+
        "setgesetlesetnasetnbsetncsetnesetngsetnlsetno"+
        "setnpsetnssetnzsetpesetposhortstosbstosdstosw"+
        "timestwordwrmsrxlatb", 5);
    define_keywords(0,
        "cmovaecmovbecmovgecmovlecmovnacmovnbcmovnc"+
        "cmovnecmovngcmovnlcmovnocmovnpcmovnscmovnz"+
        "cmovpecmovpofcmovbfcmovefcmovufcomipfcompp"+
        "fdivrpficompfidivrfisubrfldenvfldl2efldl2t"+
        "fldlg2fldln2fpatanfprem1frstorfscalefsetpm"+
        "fstenvfsubrpfucomifucompincbininvlpgloopne"+
        "loopnzpaddsbpaddswpmulhwpmullwpsubsbpsubsw"+
        "pushadpushawpushfdpushfwsetnaesetnbesetnge"+
        "setnlewbinvd", 6);
    define_keywords(0,
        "cmovnaecmovnbecmovngecmovnlecmpxchgfcmovbe"+
        "fcmovnbfcmovnefcmovnufdecstpfincstpfrndint"+
        "fsincosfucomipfucomppfxtractfyl2xp1loadall"+
        "paddusbpadduswpcmpeqbpcmpeqdpcmpeqwpcmpgtb"+
        "pcmpgtdpcmpgtwpmaddwdpsubusbpsubusw", 7);
    define_keywords(0,
        "fcmovnbepackssdwpacksswbpackuswb", 8);
    define_keywords(0,
        "cmpxchg8bpunpckhbwpunpckhdqpunpckhwdpunpcklbw"+
        "punpckldqpunpcklwd", 9);
    define_keywords(0,
        "cmpxchg486loadall286", 10);

    define_keywords(1, "org", 3);
    define_keywords(1, "bitsiend", 4);
    define_keywords(1, "aligngroupstruc", 5);
    define_keywords(1, "alignbcommonexternglobalistruc", 6);
    define_keywords(1, "sectionsegmentlibrary", 7);
    define_keywords(1, "absoluteendstruc", 8);
    define_keywords(1, "uppercase", 9);
}


/*
 *  Modeline
 */
string
_nasm_mode()
{
    return "nasm";                              /* return package extension */
}


/*
 *  package support
 */
string
_nasm_highlight_first()
{
    attach_syntax(MODENAME);                    /* attach colorizer */
    return "";
}

/*end*/

