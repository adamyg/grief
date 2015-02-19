/* -*- mode: cr; indent-width: 4; -*-
 * $Id: masm.cr,v 1.10 2014/10/27 23:28:33 ayoung Exp $
 * GRIEF macros to support MASM (x96 assembler) Programming
 *
 *
 */

#include "../grief.h"

/*
  Keywords:
    AH          AL          AX          BL
    BP          BX          CH          CL
    CS          DB          DD          DH
    DI          DL          DS          DW
    DX          EP          ES          GS
    SI          SS          BH
    CR0         CR1         CR2         CR3
    CX          DR1         DR2         DR3
    DR4         DR5         DR6         DR7
    EAX         EBP         EBX         ECX
    EDI         EDX         EQU         ESI
    ESP         FAR         PTR

    BYTE        ENDM        ENDP        ENDS
    HUGE        NAME        NEAR        PAGE
    PROC        WORD

    ALIGN       DWORD       ERROR       EXTRN
    LABEL       MACRO       STRUC       TITLE
    UNION       USE16       USE32

    ASSUME      COMMON      EXPORT      OFFSET
    PUBLIC      STRUCT

    COMMENT     NOTHING     SEGMENT     TYPEDEF

    READONLY

  Keywords (type1):
    AAA         AAD         AAM         AAS
    ADC         ADD         AND         ARPL
    BOUND       BSF         BSR         BSWAP
    BT          BTC         BTR         BTS
    CALL        CBW         CDQ         CLC
    CLD         CLI         CLTS        CMC
    CMP         CMPS        CMPSB       CMPSD
    CMPSW       CMPXCHG     CMPXCHG8B   CPUID
    CWD         CWDE        DAA         DAS
    DEC         DIV         ENTER       F2XM1
    FABS        FADD        FADDP       FBLD
    FBSTP       FCHS        FCLEX       FCOM
    FCOMP       FCOMPP      FCOS        FDECSTP
    FDIV        FDIVP       FDIVR       FDIVRP
    FFREE       FIADD       FICOM       FICOMP
    FIDIV       FIDIVR      FILD        FIMUL
    FINCSTP     FINIT       FIST        FISTP
    FISUB       FISUBR      FLD         FLD1
    FLDCW       FLDENV      FLDL2E      FLDL2T
    FLDLG2      FLDLN2      FLDPI       FLDZ
    FMUL        FMULP       FNCLEX      FNINIT
    FNOP        FNSAVE      FNSTCW      FNSTENV
    FNSTSW      FPATAN      FPREM       FPREM1
    FPTAN       FRNDINT     FRSTOR      FSAVE
    FSCALE      FSIN        FSINCOS     FSQRT
    FST         FSTCW       FSTENV      FSTP
    FSTSW       FSUB        FSUBP       FSUBR
    FSUBRP      FTST        FUCOM       FUCOMP
    FUCOMPP     FWAIT       FXAM        FXCH
    FXTRACT     FYL2X       FYL2XP1     HLT
    IDIV        IMUL        IN          INC
    INS         INSB        INSD        INSW
    INT         INTO        INVD        INVLPG
    IRET        IRETD       IRETDF      IRETF
    JCXZ        JECXZ       JMP         LAHF
    LAR         LDS         LEA         LEAVE
    LES         LFS         LGDT        LGS
    LIDT        LLDT        LMSW        LOCK
    LODS        LODSB       LODSD       LODSW
    LOOP        LOOPE       LOOPNE      LOOPNZ
    LOOPZ       LSL         LSS         LTR
    MOV         MOVS        MOVSB       MOVSD
    MOVSW       MOVSX       MOVZX       MUL
    NEG         NOP         NOT         OR
    OUT         OUTS        OUTSB       OUTSD
    OUTSW       POP         POPA        POPAD
    POPF        POPFD       PUSH        PUSHA
    PUSHAD      PUSHF       PUSHFD      RCL
    RCR         RDMSR       REP         REPE
    REPNE       RETF        RETN        ROL
    ROR         RSM         SAHF        SAL
    SAR         SBB         SCAS        SCASB
    SCASD       SCASW       SGDT        SHL
    SHLD        SHR         SHRD        SIDT
    SLDT        SMSW        STC         STD
    STI         STOS        STOSB       STOSD
    STOSW       STR         SUB         TEST
    VERR        VERW        WAIT        WBINVD
    WRMSR       XADD        XCHG        XLAT
    XLATB       XOR
*/

#define MODENAME "MASM"


void
main()
{
    create_syntax(MODENAME);

    /*
     *  Operators etc
     */
    syntax_token(SYNT_COMMENT,      ";");
    syntax_token(SYNT_QUOTE,        "\\");
    syntax_token(SYNT_PREPROCESSOR, ".");
    syntax_token(SYNT_CHARACTER,    "\'");
    syntax_token(SYNT_STRING,       "\"");
    syntax_token(SYNT_BRACKET,      "([{", ")]}");
    syntax_token(SYNT_OPERATOR,     "-%+/&*=<>|!~^");
    syntax_token(SYNT_DELIMITER,    ",;.?:");
    syntax_token(SYNT_WORD,         "0-9a-zA-Z_");
    syntax_token(SYNT_NUMERIC,      "-+0-9a-fA-F.xXL");

    /*
     *  Options
     */
    set_syntax_flags(SYNF_CASEINSENSITIVE);

    /*
     *  keywords
     */
    define_keywords(0, "AHALAX" +
                       "BHBLBP" +
                       "BXCHCL" +
                       "CSCXDB" +
                       "DDDHDIDLDSDWDX" +
                       "EPES" +
                       "GS" +
                       "SISS", 2);
    define_keywords(0, "CR0CR1CR2CR3" +
                       "DR1DR2DR3DR4DR5DR6DR7" +
                       "EAXEBPEBXECXEDIEDXEQUESIESP" +
                       "FARPTR", 3);
    define_keywords(0, "BYTE" +
                       "ENDMENDPENDS" +
                       "HUGE" +
                       "NAMENEAR" +
                       "PAGEPROC" +
                       "WORD", 4);
    define_keywords(0, "ALIGN" +
                       "DWORD" +
                       "ERROREXTRN" +
                       "LABEL" +
                       "MACRO" +
                       "STRUC" +
                       "TITLE" +
                       "UNIONUSE16USE32", 5);
    define_keywords(0, "ASSUME" +
                       "COMMON" +
                       "EXPORT" +
                       "OFFSET" +
                       "PUBLIC" +
                       "STRUCT", 6);
    define_keywords(0, "COMMENT" +
                       "NOTHING" +
                       "SEGMENT" +
                       "TYPEDEF", 7);
    define_keywords(0, "READONLY", 8);

    /* Type 1 keywords */
    define_keywords(1, "BTINOR", 2);
    define_keywords(1, "AAAAADAAMAASADCADDAND" +
                       "BSFBSRBTCBTRBTS" +
                       "CBWCDQCLCCLDCLICMCCMPCWD" +
                       "DAADASDECDIV" +
                       "FLDFSTHLT" +
                       "INCINSINT" +
                       "JMP" +
                       "LARLDSLEALESLFSLGSLSLLSSLTR" +
                       "MOVMUL" +
                       "NEGNOPNOT" +
                       "OUT" +
                       "POP" +
                       "RCLRCRREPROLRORRSM" +
                       "SALSARSBBSHLSHRSTCSTDSTISTRSUB" +
                       "XOR", 3);
    define_keywords(1, "ARPL" +
                       "CALLCLTSCMPSCWDE" +
                       "FABSFADDFBLDFCHSFCOMFCOSFDIVFILDFISTFLD1FLDZ" +
                       "FMULFNOPFSINFSTPFSUBFTSTFXAMFXCH" +
                       "IDIVIMULINSBINSDINSWINTOINVDIRET" +
                       "JCXZ" +
                       "LAHFLGDTLIDTLLDTLMSWLOCKLODSLOOP" +
                       "MOVS" +
                       "OUTS" +
                       "POPAPOPFPUSH" +
                       "REPERETFRETN" +
                       "SAHFSCASSGDTSHLDSHRDSIDTSLDTSMSWSTOS" +
                       "TEST" +
                       "VERRVERW" +
                       "WAIT" +
                       "XADDXCHGXLAT", 4);
    define_keywords(1, "BOUNDBSWAP" +
                       "CMPSBCMPSDCMPSWCPUID" +
                       "ENTER" +
                       "F2XM1FADDPFBSTPFCLEXFCOMPFDIVPFDIVRFFREEFIADD" +
                       "FICOMFIDIVFIMULFINITFISTPFISUBFLDCWFLDPIFMULP" +
                       "FPREMFPTANFSAVEFSQRTFSTCWFSTSWFSUBPFSUBRFUCOM" +
                       "FWAITFYL2X" +
                       "IRETDIRETF" +
                       "JECXZ" +
                       "LEAVELODSBLODSDLODSWLOOPELOOPZ" +
                       "MOVSBMOVSDMOVSWMOVSXMOVZX" +
                       "OUTSBOUTSDOUTSW" +
                       "POPADPOPFDPUSHAPUSHF" +
                       "RDMSRREPNE" +
                       "SCASBSCASDSCASWSTOSBSTOSDSTOSW" +
                       "WRMSR" +
                       "XLATB", 5);
    define_keywords(1, "FCOMPPFDIVRPFICOMPFIDIVRFISUBRFLDENVFLDL2E" +
                       "FLDL2TFLDLG2FLDLN2FNCLEXFNINITFNSAVEFNSTCW"+
                       "FNSTSWFPATANFPREM1FRSTORFSCALEFSTENVFSUBRP"+
                       "FUCOMP"+
                       "INVLPGIRETDFLOOPNE"+
                       "LOOPNZ"+
                       "PUSHADPUSHFD"+
                       "WBINVD", 6);
    define_keywords(1, "CMPXCHG" +
                       "FDECSTPFINCSTPFNSTENVFRNDINTFSINCOS" +
                       "FUCOMPPFXTRACTFYL2XP1", 7);
    define_keywords(1, "CMPXCHG8B", 9);
}


/*
 *  Modeline support
 */
string
_masm_mode()
{
    return "masm";                              /* return package extension */
}


/*
 *  package support
 */
string
_masm_highlight_first()
{
    attach_syntax(MODENAME);
    return "";
}

/*end*/
