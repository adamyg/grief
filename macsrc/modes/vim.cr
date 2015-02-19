/* -*- mode: cr; indent-width: 4; -*-
/* $Id: vim.cr,v 1.5 2014/10/22 02:34:36 ayoung Exp $
 * vim scripting lnaguage -- basic support
 *
 *
 */

#include "../grief.h"

#define MODENAME "VIM"

static list commands = {                        // commands, abbrev and full
    "abc-lear", "argdo", "argu-ment", "bel-owright", "bN-ext", "breakd-el", "b-uffer", "caddb-uffer", "cb-uffer", "cex-pr", "cg-etfile",
    "checkt-ime", "cnew-er", "col-der", "con-tinue", "cq-uit", "delc-ommand", "diffoff", "diffu-pdate", "dr-op", "echom-sg", "em-enu",
    "endt-ry", "exi-t", "fina-lly", "fix-del", "foldd-oopen", "go-to", "hid-e", "ij-ump", "isp-lit", "k", "laddb-uffer", "la-st",
    "lch-dir", "lex-pr", "lgete-xpr", "l-ist", "lmak-e", "lN-ext", "loc-kmarks", "lpf-ile", "lt-ag", "lv-imgrep", "ma-rk", "mk-exrc",
    "mkv-imrc", "mz-scheme", "new", "noh-lsearch", "on-ly", "ped-it", "popu", "prev-ious", "prof-ile", "pta-g", "ptn-ext", "pts-elect",
    "py-thon", "r-ead", "redr-aw", "ret-ab", "rightb-elow", "rundo", "san-dbox", "sbf-irst", "sbN-ext", "scripte-ncoding", "setg-lobal",
    "sh-ell", "sla-st", "sme", "sni-ff", "sor-t", "spelli-nfo", "sp-lit", "startg-replace", "st-op", "sunme", "syncbind", "tabd-o",
    "tabl-ast", "tabN-ext", "tabs", "tcld-o", "th-row", "tm-enu", "tp-revious", "tu", "undoj-oin", "uns-ilent", "vert-ical", "vi-sual",
    "wa-ll", "winp-os", "wp-revious", "ws-verb", "xa-ll", "xmenu", "xnoremenu", "abo-veleft", "arge-dit", "as-cii", "bf-irst",
    "bo-tright", "breakl-ist", "buffers", "cad-dexpr", "cc", "cf-ile", "c-hange", "cla-st", "cn-ext", "colo-rscheme", "cope-n",
    "cr-ewind", "d-elete", "diffpatch", "dig-raphs", "ds-earch", "echon", "emenu*", "endw-hile", "f-ile", "fin-d", "fo-ld", "foldo-pen",
    "gr-ep", "his-tory", "il-ist", "iuna-bbrev", "keepalt", "lad-dexpr", "later", "lcl-ose", "lf-ile", "lg-etfile", "ll", "lmapc-lear",
    "lnf-ile", "lockv-ar", "lp-revious", "lua", "lvimgrepa-dd", "marks", "mks-ession", "mod-e", "nbc-lose", "n-ext", "nu-mber", "o-pen",
    "pe-rl", "popu-p", "p-rint", "promptf-ind", "ptf-irst", "ptN-ext", "pu-t", "qa-ll", "rec-over", "redraws-tatus", "retu-rn", "rub-y",
    "ru-ntime", "sa-rgument", "sbl-ast", "sbp-revious", "scrip-tnames", "setl-ocal", "sign", "sl-eep", "smenu", "sno-magic", "so-urce",
    "spellr-epall", "spr-evious", "star-tinsert", "stopi-nsert", "sunmenu", "t", "tabe-dit", "tabm-ove", "tabo-nly", "ta-g", "tclf-ile",
    "tj-ump", "tn-ext", "tr-ewind", "tu-nmenu", "undol-ist", "up-date", "vie-w", "vmapc-lear", "wh-ile", "win-size", "wq", "wundo",
    "x-it", "XMLent", "xunme", "al-l", "argg-lobal", "bad-d", "bl-ast", "bp-revious", "br-ewind", "bun-load", "caddf-ile", "ccl-ose",
    "cfir-st", "changes", "cl-ist", "cN-ext", "comc-lear", "co-py", "cuna-bbrev", "delf-unction", "diffpu-t", "di-splay", "dsp-lit",
    "e-dit", "endfo-r", "ene-w", "files", "fini-sh", "foldc-lose", "for", "grepa-dd", "iabc-lear", "imapc-lear", "j-oin", "keepj-umps",
    "laddf-ile", "lb-uffer", "le-ft", "lfir-st", "lgr-ep", "lla-st", "lnew-er", "lNf-ile", "lol-der", "lr-ewind", "luado", "lw-indow",
    "mat-ch", "mksp-ell", "m-ove", "nb-key", "N-ext", "ol-dfiles", "opt-ions", "perld-o", "pp-op", "P-rint", "promptr-epl", "ptj-ump",
    "ptp-revious", "pw-d", "q-uit", "redi-r", "reg-isters", "rew-ind", "rubyd-o", "rv-iminfo", "sav-eas", "sbm-odified", "sbr-ewind",
    "se-t", "sf-ind", "sil-ent", "sm-agic", "sn-ext", "snoreme", "spelld-ump", "spellu-ndo", "sre-wind", "startr-eplace", "sts-elect",
    "sus-pend", "tab", "tabf-ind", "tabnew", "tabp-revious", "tags", "te-aroff", "tl-ast", "tN-ext", "try", "una-bbreviate", "unh-ide",
    "verb-ose", "vim-grep", "vne-w", "winc-md", "wn-ext", "wqa-ll", "wv-iminfo", "xmapc-lear", "XMLns", "xunmenu", "arga-dd", "argl-ocal",
    "ba-ll", "bm-odified", "brea-k", "bro-wse", "bw-ipeout", "cal-l", "cd", "cgetb-uffer", "chd-ir", "clo-se", "cnf-ile", "comp-iler",
    "cpf-ile", "cw-indow", "delm-arks", "diffsplit", "dj-ump", "earlier", "el-se", "endf-unction", "ex", "filetype", "fir-st",
    "folddoc-losed", "fu-nction", "ha-rdcopy", "if", "is-earch", "ju-mps", "kee-pmarks", "lan-guage", "lc-d", "lefta-bove", "lgetb-uffer",
    "lgrepa-dd", "lli-st", "lne-xt", "lo-adview", "lop-en", "ls", "luafile", "mak-e", "menut-ranslate", "mkvie-w", "mzf-ile", "nbs-tart",
    "nmapc-lear", "omapc-lear", "pc-lose", "po-p", "pre-serve", "profd-el", "ps-earch", "ptl-ast", "ptr-ewind", "pyf-ile", "quita-ll",
    "red-o", "res-ize", "ri-ght", "rubyf-ile", "sal-l", "sba-ll", "sbn-ext", "sb-uffer", "setf-iletype", "sfir-st", "sim-alt", "sm-ap",
    "sN-ext", "snoremenu", "spe-llgood", "spellw-rong", "sta-g", "stj-ump", "sun-hide", "sv-iew", "tabc-lose", "tabfir-st", "tabn-ext",
    "tabr-ewind", "tc-l", "tf-irst", "tm", "to-pleft", "ts-elect", "u-ndo", "unlo-ckvar", "ve-rsion", "vimgrepa-dd", "vs-plit", "windo",
    "wN-ext", "w-rite", "X", "xme", "xnoreme", "y-ank", "argd-elete", "ar-gs", "bd-elete", "bn-ext", "breaka-dd", "bufdo", "cabc-lear",
    "cat-ch", "ce-nter", "cgete-xpr", "che-ckpath", "cmapc-lear", "cNf-ile", "conf-irm", "cp-revious", "debugg-reedy", "diffg-et",
    "diffthis", "dl-ist", "echoe-rr","elsei-f","en-dif" };


static list sets = {                            // set <value>
    "acd", "ambiwidth", "arabicshape", "autowriteall", "backupdir", "bdlay", "binary", "breakat", "bufhidden", "cd", "ci", "cinw", "co",
    "commentstring", "confirm", "cpoptions", "cscopetag", "csto", "cwh", "dg", "dip", "eadirection", "ek", "equalprg", "ex", "fdi", "fen",
    "fileencodings", "flp", "foldexpr", "foldnestmax", "fp", "gfm", "grepformat", "guifontwide", "helpheight", "highlight", "hlg", "im",
    "imi", "incsearch", "infercase", "isk", "keymap", "langmenu", "linespace", "loadplugins", "macatsui", "maxcombine", "mef", "mls",
    "modelines", "mousehide", "mp", "nu", "omnifunc", "paragraphs", "penc", "pm", "printdevice", "printoptions", "quoteescape",
    "restorescreen", "rnu", "rulerformat", "scr", "sect", "sft", "shellredir", "shm", "showmode", "sj", "smd", "spell", "splitbelow",
    "ssl", "stl", "sw", "sxq", "tabpagemax", "tags", "tbis", "terse", "thesaurus", "titleold", "toolbariconsize", "tsr", "ttyfast", "tx",
    "undofile", "ut", "verbosefile", "virtualedit", "wb", "wfw", "wildcharm", "winaltkeys", "winminwidth", "wmnu", "write", "ai", "ambw",
    "ari", "aw", "backupext", "beval", "biosk", "brk", "buflisted", "cdpath", "cin", "cinwords", "cocu", "compatible", "consk", "cpt",
    "cscopetagorder", "csverb", "debug", "dict", "dir", "eb", "enc", "errorbells", "expandtab", "fdl", "fenc", "fileformat", "fml",
    "foldignore", "foldopen", "fs", "gfn", "grepprg", "guiheadroom", "helplang", "history", "hls", "imactivatekey", "iminsert", "inde",
    "insertmode", "iskeyword", "keymodel", "laststatus", "lisp", "lpl", "magic", "maxfuncdepth", "menuitems", "mm", "modifiable",
    "mousem", "mps", "number", "opendevice", "paste", "pex", "pmbcs", "printencoding", "prompt", "rdt", "revins", "ro", "runtimepath",
    "scroll", "sections", "sh", "shellslash", "shortmess", "showtabline", "slm", "sn", "spellcapcheck", "splitright", "ssop", "stmp",
    "swapfile", "syn", "tabstop", "tagstack", "tbs", "textauto", "tildeop", "titlestring", "top", "ttimeout", "ttym", "uc", "undolevels",
    "vb", "vfile", "visualbell", "wc", "wh", "wildignore", "window", "winwidth", "wmw", "writeany", "akm", "anti", "arshape", "awa",
    "backupskip", "bex", "bioskey", "browsedir", "buftype", "cedit", "cindent", "clipboard", "cole", "complete", "conskey", "crb",
    "cscopeverbose", "cuc", "deco", "dictionary", "directory", "ed", "encoding", "errorfile", "exrc", "fdls", "fencs", "fileformats",
    "fmr", "foldlevel", "foldtext", "fsync", "gfs", "gtl", "guioptions", "hf", "hk", "hlsearch", "imak", "ims", "indentexpr", "is", "isp",
    "keywordprg", "lazyredraw", "lispwords", "ls", "makeef", "maxmapdepth", "mfd", "mmd", "modified", "mousemodel", "msm", "numberwidth",
    "operatorfunc", "pastetoggle", "pexpr", "pmbfn", "printexpr", "pt", "readonly", "ri", "rs", "sb", "scrollbind", "secure", "shcf",
    "shelltemp", "shortname", "shq", "sm", "so", "spellfile", "spr", "st", "sts", "swapsync", "synmaxcol", "tag", "tal", "tenc",
    "textmode", "timeout", "tl", "tpm", "ttimeoutlen", "ttymouse", "udf", "undoreload", "vbs", "vi", "vop", "wcm", "whichwrap",
    "wildmenu", "winfixheight", "wiv", "wop", "writebackup", "al", "antialias", "autochdir", "background", "balloondelay", "bexpr", "bk",
    "bs", "casemap", "cf", "cink", "cmdheight", "colorcolumn", "completefunc", "copyindent", "cryptmethod", "cspc", "cul", "def", "diff",
    "display", "edcompatible", "endofline", "errorformat", "fcl", "fdm", "fex", "filetype", "fo", "foldlevelstart", "formatexpr", "ft",
    "gfw", "gtt", "guipty", "hh", "hkmap", "ic", "imc", "imsearch", "indentkeys", "isf", "isprint", "km", "lbr", "list", "lsp", "makeprg",
    "maxmem", "mh", "mmp", "more", "mouses", "mzq", "nuw", "opfunc", "patchexpr", "pfn", "popt", "printfont", "pumheight", "redrawtime",
    "rightleft", "rtp", "sbo", "scrolljump", "sel", "shell", "shelltype", "showbreak", "si", "smartcase", "softtabstop", "spelllang",
    "sps", "sta", "su", "swb", "syntax", "tagbsearch", "tb", "term", "textwidth", "timeoutlen", "tm", "tr", "ttm", "ttyscroll", "udir",
    "updatecount", "vdir", "viewdir", "wa", "wd", "wi", "wildmode", "winfixwidth", "wiw", "wrap", "writedelay", "aleph", "ar",
    "autoindent", "backspace", "ballooneval", "bg", "bkc", "bsdir", "cb", "cfu", "cinkeys", "cmdwinheight", "columns", "completeopt",
    "cot", "cscopepathcomp", "csprg", "cursorbind", "define", "diffexpr", "dy", "ef", "eol", "esckeys", "fcs", "fdn", "ff", "fillchars",
    "foldclose", "foldmarker", "formatlistpat", "gcr", "ghr", "guicursor", "guitablabel", "hi", "hkmapp", "icon", "imcmdline", "inc",
    "indk", "isfname", "joinspaces", "kmp", "lcs", "listchars", "lw", "mat", "maxmempattern", "mis", "mmt", "mouse", "mouseshape",
    "mzquantum", "odev", "osfiletype", "patchmode", "ph", "preserveindent", "printheader", "pvh", "relativenumber", "rightleftcmd", "ru",
    "sbr", "scrolloff", "selection", "shellcmdflag", "shellxquote", "showcmd", "sidescroll", "smartindent", "sol", "spellsuggest", "sr",
    "stal", "sua", "swf", "ta", "taglength", "tbi", "termbidi", "tf", "title", "to", "ts", "tty", "ttytype", "ul", "updatetime", "ve",
    "viewoptions", "wak", "weirdinvert", "wig", "wildoptions", "winheight", "wm", "wrapmargin", "ws", "allowrevins", "arab", "autoread",
    "backup", "balloonexpr", "bh", "bl", "bsk", "cc", "ch", "cino", "cmp", "com", "concealcursor", "cp", "cscopeprg", "csqf",
    "cursorcolumn", "delcombine", "diffopt", "ea", "efm", "ep", "et", "fdc", "fdo", "ffs", "fk", "foldcolumn", "foldmethod",
    "formatoptions", "gd", "go", "guifont", "guitabtooltip", "hid", "hkp", "iconstring", "imd", "include", "inex", "isi", "js", "kp",
    "linebreak", "lm", "lz", "matchpairs", "maxmemtot", "mkspellmem", "mod", "mousef", "mouset", "nf", "oft", "pa", "path", "pheader",
    "previewheight", "printmbcharset", "pvw", "remap", "rl", "ruf", "sc", "scrollopt", "selectmode", "shellpipe", "shiftround",
    "showfulltag", "sidescrolloff", "smarttab", "sp", "spf", "srr", "startofline", "suffixes", "switchbuf", "tabline", "tagrelative",
    "tbidi", "termencoding", "tgst", "titlelen", "toolbar", "tsl", "ttybuiltin", "tw", "undodir", "ur", "verbose", "viminfo", "warn",
    "wfh", "wildchar", "wim", "winminheight", "wmh", "wrapscan", "ww", "altkeymap", "arabic", "autowrite", "backupcopy", "bdir", "bin",
    "bomb", "bt", "ccv", "charconvert", "cinoptions", "cms", "comments", "conceallevel", "cpo", "cscopequickfix", "cst", "cursorline",
    "dex", "digraph", "ead", "ei", "equalalways", "eventignore", "fde", "fdt", "fileencoding", "fkmap", "foldenable", "foldminlines",
    "formatprg", "gdefault", "gp", "guifontset", "helpfile", "hidden", "hl", "ignorecase", "imdisable", "includeexpr", "inf", "isident",
    "key", "langmap", "lines", "lmap", "ma", "matchtime", "mco", "ml", "modeline", "mousefocus", "mousetime", "nrformats", "ofu", "para",
    "pdev", "pi", "previewwindow", "printmbfont", "qe", "report", "rlc", "ruler", "scb", "scs", "sessionoptions", "shellquote",
    "shiftwidth", "showmatch", "siso", "smc","spc","spl","ss","statusline","suffixesadd","sws" };


static list nosets = {                          // turn-off setting variants
    "noacd", "noallowrevins", "noantialias", "noarabic", "noarshape", "noautoread", "noaw", "noballooneval", "nobinary", "nobk",
    "nobuflisted", "nocin", "noconfirm", "nocopyindent", "nocscopetag", "nocsverb", "nocursorbind", "nodeco", "nodiff", "noeb", "noek",
    "noeol", "noerrorbells", "noet", "noexpandtab", "nofen", "nofkmap", "nogd", "noguipty", "nohidden", "nohkmap", "nohkp", "nohlsearch",
    "noicon", "noim", "noimcmdline", "noimdisable", "noinf", "noinsertmode", "nojoinspaces", "nolazyredraw", "nolinebreak", "nolist",
    "nolpl", "noma", "nomagic", "noml", "nomodeline", "nomodified", "nomousef", "nomousehide", "nonumber", "noopendevice", "nopi",
    "nopreviewwindow", "nopvw", "norelativenumber", "norestorescreen", "nori", "norl", "noro", "noru", "nosb", "noscb", "noscs", "nosft",
    "noshelltemp", "noshortname", "noshowfulltag", "noshowmode", "nosm", "nosmartindent", "nosmd", "nosol", "nosplitbelow", "nospr",
    "nossl", "nostartofline", "noswapfile", "nota", "notagrelative", "notbi", "notbs", "noterse", "notextmode", "notgst", "notimeout",
    "noto", "notr", "nottybuiltin", "notx", "novisualbell", "nowarn", "noweirdinvert", "nowfw", "nowinfixheight", "nowiv", "nowrap",
    "nowrite", "nowritebackup", "noai", "noaltkeymap", "noar", "noarabicshape", "noautochdir", "noautowrite", "noawa", "nobeval",
    "nobiosk", "nobl", "nocf", "nocindent", "noconsk", "nocp", "nocscopeverbose", "nocuc", "nocursorcolumn", "nodelcombine", "nodigraph",
    "noed", "noendofline", "noequalalways", "noesckeys", "noex", "noexrc", "nofk", "nofoldenable", "nogdefault", "nohid", "nohk",
    "nohkmapp", "nohls", "noic", "noignorecase", "noimc", "noimd", "noincsearch", "noinfercase", "nois", "nojs", "nolbr", "nolisp",
    "noloadplugins", "nolz", "nomacatsui", "nomh", "nomod", "nomodifiable", "nomore", "nomousefocus", "nonu", "noodev", "nopaste",
    "nopreserveindent", "noprompt", "noreadonly", "noremap", "norevins", "norightleft", "nornu", "nors", "noruler", "nosc",
    "noscrollbind", "nosecure", "noshellslash", "noshiftround", "noshowcmd", "noshowmatch", "nosi", "nosmartcase", "nosmarttab", "nosn",
    "nospell", "nosplitright", "nosr", "nosta", "nostmp", "noswf", "notagbsearch", "notagstack", "notbidi", "notermbidi", "notextauto",
    "notf", "notildeop", "notitle", "notop", "nottimeout", "nottyfast", "novb", "nowa", "nowb", "nowfh", "nowildmenu", "nowinfixwidth",
    "nowmnu", "nowrapscan", "nowriteany", "nows", "noakm", "noanti", "noarab", "noari", "noautoindent", "noautowriteall", "nobackup",
    "nobin", "nobioskey", "nobomb", "noci", "nocompatible", "noconskey", "nocrb", "nocst", "nocul", "nocursorline", "nodg", "noea",
    "noedcompatible" };


static list invsets = {                         // invertible variants
    "invacd", "invallowrevins", "invantialias", "invarabic", "invarshape", "invautoread", "invaw", "invballooneval", "invbinary", "invbk",
    "invbuflisted", "invcin", "invconfirm", "invcopyindent", "invcscopetag", "invcsverb", "invcursorbind", "invdeco", "invdiff", "inveb",
    "invek", "inveol", "inverrorbells", "invet", "invexpandtab", "invfen", "invfkmap", "invgd", "invguipty", "invhidden", "invhkmap",
    "invhkp", "invhlsearch", "invicon", "invim", "invimcmdline", "invimdisable", "invinf", "invinsertmode", "invjoinspaces",
    "invlazyredraw", "invlinebreak", "invlist", "invlpl", "invma", "invmagic", "invml", "invmodeline", "invmodified", "invmousef",
    "invmousehide", "invnumber", "invopendevice", "invpi", "invpreviewwindow", "invpvw", "invrelativenumber", "invrestorescreen", "invri",
    "invrl", "invro", "invru", "invsb", "invscb", "invscs", "invsft", "invshelltemp", "invshortname", "invshowfulltag", "invshowmode",
    "invsm", "invsmartindent", "invsmd", "invsol", "invsplitbelow", "invspr", "invssl", "invstartofline", "invswapfile", "invta",
    "invtagrelative", "invtbi", "invtbs", "invterse", "invtextmode", "invtgst", "invtimeout", "invto", "invtr", "invttybuiltin", "invtx",
    "invvisualbell", "invwarn", "invweirdinvert", "invwfw", "invwinfixheight", "invwiv", "invwrap", "invwrite", "invwritebackup", "invai",
    "invaltkeymap", "invar", "invarabicshape", "invautochdir", "invautowrite", "invawa", "invbeval", "invbiosk", "invbl", "invcf",
    "invcindent", "invconsk", "invcp", "invcscopeverbose", "invcuc", "invcursorcolumn", "invdelcombine", "invdigraph", "inved",
    "invendofline", "invequalalways", "invesckeys", "invex", "invexrc", "invfk", "invfoldenable", "invgdefault", "invhid", "invhk",
    "invhkmapp", "invhls", "invic", "invignorecase", "invimc", "invimd", "invincsearch", "invinfercase", "invis", "invjs", "invlbr",
    "invlisp", "invloadplugins", "invlz", "invmacatsui", "invmh", "invmod", "invmodifiable", "invmore", "invmousefocus", "invnu",
    "invodev", "invpaste", "invpreserveindent", "invprompt", "invreadonly", "invremap", "invrevins", "invrightleft", "invrnu", "invrs",
    "invruler", "invsc", "invscrollbind", "invsecure", "invshellslash", "invshiftround", "invshowcmd", "invshowmatch", "invsi",
    "invsmartcase", "invsmarttab", "invsn", "invspell", "invsplitright", "invsr", "invsta", "invstmp", "invswf", "invtagbsearch",
    "invtagstack", "invtbidi", "invtermbidi", "invtextauto", "invtf", "invtildeop", "invtitle", "invtop", "invttimeout", "invttyfast",
    "invvb", "invwa", "invwb", "invwfh", "invwildmenu", "invwinfixwidth", "invwmnu", "invwrapscan", "invwriteany", "invws", "invakm",
    "invanti", "invarab", "invari", "invautoindent", "invautowriteall", "invbackup", "invbin", "invbioskey", "invbomb", "invci",
    "invcompatible", "invconskey", "invcrb", "invcst", "invcul", "invcursorline", "invdg", "invea", "invedcompatible" };


static list hilite = {
    "case", "ccomment", "clear", "cluster", "contained", "contains", "end", "ignore", "keepend", "keyword", "linecont", "link", "match",
    "matchgroup", "nextgroup", "oneline", "region", "skip", "skipwhite", "start", "transparent" };


void
main(void)
{
    /*
     *  Build vim lexier
     */
    create_syntax(MODENAME);

    // comments
    syntax_rule("\\\"[^\\\"]+$", "spell,todo:comment");

    // keywords/identifiers
    syntax_rule("[A-Za-z_][A-Za-z_0-9#@*&%]*", "keyword,word:normal");

    // numeric constants (hex, oct, dec and errors)
    syntax_rule("#[0-9A-Fa-f]+", "number");
    syntax_rule("[0-9]+", "number");
    syntax_rule("0[8-9][0-9]*", "alert");

    // floating point
    syntax_rule("[0-9]+\\.[0-9]*([Ee][-+]?[0-9]*)?[fFlL]?", "float");
    syntax_rule("[0-9]+[Ee][-+]?[0-9]*[fFlL]?", "float");
    syntax_rule("\\.[0-9]+([Ee][-+]?[0-9]*)?[fFlL]?", "alert");

    // strings
    syntax_rule("\"(\\\\.|[^\\\\\"])*\"", "string");

    // delimiters/operators
    syntax_rule("[()\\-\\,{},;.?:]", "delimiter");
    syntax_rule("[-%+/&*=<>|!~^]+", "operator");

    syntax_build(__COMPILETIME__);

    /*
     *  Keywords ...
     */
    int dflag = debug(0, FALSE);                // disable debug
    string command;
    int abbv;

    while (list_each(commands, command) >= 0) { // commands
        if ((abbv = index(command, '-')) > 0) {
            //
            //  abbv-name plus long-name
            //
            //      "wh-ile"
            //          ==> wh
            //          ==> while
            //
            define_keywords(SYNK_PRIMARY, substr(command, 1, abbv-1));
            define_keywords(SYNK_PRIMARY, substr(command, 1, abbv-1) + substr(command, abbv+1));
        } else {
            //
            //  simple form
            //
            define_keywords(SYNK_PRIMARY, command);
        }
    }

    define_keywords(SYNK_PRIMARY, sets);
    define_keywords(SYNK_PRIMARY, nosets);
    define_keywords(SYNK_PRIMARY, invsets);

    define_keywords(SYNK_FUNCTION,              // functions
        "abs,append,argv,atan2,bufexists,bufname,byte2line,ceil,cindent,complete,confirm,cosh,cursor,"+
        "did_filetype,empty,eventhandler,exp,extend,filewritable,findfile,fmod,foldclosed,foldtext,"+
        "function,getbufline,getcharmod,getcmdtype,getfperm,getftype,getmatches,getqflist,gettabvar,"+
        "getwinposy,globpath,haslocaldir,histdel,hlexists,iconv,input,inputrestore,insert,items,len,"+
        "line,localtime,map,match,matchdelete,matchstr,min,mode,nextnonblank,pathshorten,prevnonblank,"+
        "pumvisible,readfile,reltimestr,remote_foreground,remote_read,remove,repeat,reverse,search,"+
        "searchpair,searchpos,serverlist,setcmdpos,setloclist,setpos,setreg,settabwinvar,shellescape,"+
        "sin,sort,spellbadword,split,str2float,strchars,strftime,string,strpart,strtrans,submatch,"+
        "synconcealed,synIDattr,synstack,tabpagebuflist,tabpagewinnr,taglist,tanh,tolower,tr,type,"+
        "undotree,virtcol,winbufnr,winheight,winnr,winrestview,winwidth,acos,argc,asin,browse,"+
        "buflisted,bufnr,byteidx,changenr,clearmatches,complete_add,copy,count,deepcopy,diff_filler,"+
        "escape,executable,expand,feedkeys,filter,float2nr,fnameescape,foldclosedend,foldtextresult,"+
        "garbagecollect,getbufvar,getcmdline,getcwd,getfsize,getline,getpid,getreg,gettabwinvar,"+
        "getwinvar,has,hasmapto,histget,hlID,indent,inputdialog,inputsave,isdirectory,join,libcall,"+
        "line2byte,log,maparg,matchadd,matchend,max,mkdir,mzeval,nr2char,pow,printf,range,reltime,"+
        "remote_expr,remote_peek,remote_send,rename,resolve,round,searchdecl,searchpairpos,"+
        "server2client,setbufvar,setline,setmatches,setqflist,settabvar,setwinvar,simplify,sinh,"+
        "soundfold,spellsuggest,sqrt,str2nr,strdisplaywidth,stridx,strlen,strridx,strwidth,substitute,"+
        "synID,synIDtrans,system,tabpagenr,tagfiles,tan,tempname,toupper,trunc,undofile,values,"+
        "visualmode,wincol,winline,winrestcmd,winsaveview,writefile,add,argidx,atan,browsedir,"+
        "bufloaded,bufwinnr,call,char2nr,col,complete_check,cos,cscope_connection,delete,diff_hlID,"+
        "eval,exists,expr8,filereadable,finddir,floor,fnamemodify,foldlevel,foreground,get,getchar,"+
        "getcmdpos,getfontname,getftime,getloclist,getpos,getregtype,getwinposx,glob,has_key,histadd,"+
        "histnr,hostname,index,inputlist,inputsecret,islocked,keys,libcallnr,lispindent,log10,"+
        "mapcheck,matcharg,matchlist");

    define_keywords(SYNK_CONSTANT,              // termcap identifiers
        "t_AB,t_al,t_bc,t_ce,t_cl,t_Co,t_cs,t_Cs,t_CS,t_CV,t_da,t_db,t_dl,t_DL,t_EI,t_F1,t_F2,t_F3,"+
        "t_F4,t_F5,t_F6,t_F7,t_F8,t_F9,t_fs,t_IE,t_IS,t_k1,t_K1,t_k2,t_k3,t_K3,t_k4,t_K4,t_k5,t_K5,"+
        "t_k6,t_K6,t_k7,t_K7,t_k8,t_K8,t_k9,t_K9,t_KA,t_kb,t_kB,t_KB,t_KC,t_kd,t_kD,t_KD,t_ke,t_KE,"+
        "t_KF,t_KG,t_kh,t_KH,t_kI,t_KI,t_KJ,t_KK,t_kl,t_KL,t_kN,t_kP,t_kr,t_ks,t_ku,t_le,t_mb,t_md,"+
        "t_me,t_mr,t_ms,t_nd,t_op,t_RI,t_RV,t_Sb,t_se,t_Sf,t_SI,t_so,t_sr,t_te,t_ti,t_ts,t_ue,t_us,"+
        "t_ut,t_vb,t_ve,t_vi,t_vs,t_WP,t_WS,t_xs,t_ZH,t_ZR,t_AF,t_AL,t_cd,t_Ce,t_cm,t_%1,t_#2,t_#4,"+
        "t_@7,t_*7,t_&8,t_%i,t_k");

    define_keywords(SYNK_CONSTANT,              // syntax groups
        "Boolean,Character,Comment,Condtional,Constant,Cursor,Debug,Define,Delimiter,DiffAdd,"+
        "DiffChange,DiffDelete,DiffText,Directory,Error,ErrorMsg,Exception,Float,FoldColumn,Folded,"+
        "Function,Identifier,Include,IncSearch,Keyword,Label,LineNr,Macro,ModeMsg,MoreMsg,NonText,"+
        "Normal,Number,Operator,PreCondit,PreProc,Question,Repeat,Search,SpecialChar,SpecialComment,"+
        "Special,SpecialKey,Statement,StatusLine,StatusLineNC,StorageClass,String,Structure,Tag,"+
        "Title,Todo,Typedef,Type,VertSplit,Visual,VisualNOS,WarningMsg,WildMenua");

#if (XXX)
    define_keywords(SYNK_CONSTANT,              // colors and attributes
        none ...
        black ...
        );
#endif

    while (list_each(hilite, command) >= 0) {
        define_keywords(SYNK_PRIMARY, command);
    }

    define_keywords(SYNK_TODO,                  // todo markups
        "COMBAK,XXX,TODO,FIXME,DEPRECATED,MAGIC,HACK");

    debug(dflag, FALSE);                        // restore debug.
}


/*
 * Modeline/package support
 */
string
_vim_mode(void)
{
    return "vim";
}


string
_vim_highlight_first(void)
{
    attach_syntax(MODENAME);
    return "";
}

