/* -*- mode: cr; tabs: 4; -*- */
/* $Id: grief_tail.h,v 1.25 2024/12/05 18:48:25 cvsuser Exp $
 * Common GRIEF macro definitions -- tail.
 * Utilised by the makeinc.pl script.
 *
 *
 */

/*--export--*/

/*
 * Name mangling, compat for older macros
 */
#define PF_WAIT                 PF_WAITING
#define BF_READONLY             BF_RDONLY
#define EDIT_CR                 EDIT_STRIP_CR


/*
 *  Manifest constants returned by inq_mode
 */
#define MODE_OVERTYPE           0
#define MODE_INSERT             1


/*
 *  Flags for select_buffer() macro.
 */
#define SEL_NORMAL              0x0000          /* Text is not centered */
#define SEL_CENTER              0x0001          /* Text is centered */
#define SEL_TOP_OF_WINDOW       0x0002          /* Make indicated line top of window */


/*
 *  File Mode Bits
 */
extern const int                S_IFMT;
extern const int                S_IFDIR;
extern const int                S_IFCHR;
extern const int                S_IFIFO;
extern const int                S_IFREG;
extern const int                S_IFLNK;
extern const int                S_IFSOCK;

extern const int                S_IRUSR;
extern const int                S_IWUSR;
extern const int                S_IXUSR;
extern const int                S_IRGRP;
extern const int                S_IWGRP;
extern const int                S_IXGRP;
extern const int                S_IROTH;
extern const int                S_IWOTH;
extern const int                S_IXOTH;

extern const int                S_ISUID;
extern const int                S_ISGID;
extern const int                S_ISVTX;

/*
 *  File open flags
 */
extern const int                O_CREAT;
extern const int                O_EXCL;
extern const int                O_RDONLY;
extern const int                O_RDWR;
extern const int                O_TRUNC;
extern const int                O_WRONLY;
extern const int                O_BINARY;

/*
 *  access
 */
extern const int                F_OK;
extern const int                R_OK;
extern const int                W_OK;
extern const int                X_OK;

/*
 *  seek
 */
extern const int                SEEK_SET;
extern const int                SEEK_CUR;
extern const int                SEEK_END;


/*
 *  System values
 */
extern const int                current_buffer;
extern const int                current_window;
extern const int                current_line;
extern const int                current_col;

/*
 *  errno
 *
 *      TODO - publish base error number manifest constants
 */
extern int                      errno;


/*
 *  Simple debug
 */
#if defined(MACRO_DEBUG)
#define DBG(x)                  debug_pause(x)
#else
#define DBG(x)
#endif


/*
 *  Following definitions are used to support the
 *  BRIEF macro names for compatability.
 */
#define inq_environment(s)      getenv(s)


/*
 *  System type (OS/2, Win32, MACOSX, DOS and UNIX)
 *
 *      TODO - change DELIM, SLASH and DIRSEP to const system values
 */
extern const string             CRISP_OPSYS;

extern string                   CRISP_DELIM;
extern string                   CRISP_SLASH;
extern string                   CRISP_DIRSEP;

/*
 *  Profile and configuration names.
 */
extern const string             GRRC;
extern const string             GRRC_FILE;

extern const string             GRRESTORE_FILE;
extern const string             GRSTATE_FILE;
extern const string             GRSTATE_DB;
extern const string             GRINIT_FILE;
extern const string             GRLOG_FILE;

extern const string             GREXTENSION;
extern const string             GRPROGNAME;

extern const int                GRVERSIONMAJOR;
extern const int                GRVERSIONMINOR;

extern const string             C_TERM_CHR;

/*
 *  Advanced save/restore functionality
 */
#define save_excursion()        save_position()
#define restore_excursion()     restore_position(2)


/*
 *  Prototypes
 */
#if defined(__PROTOTYPES__)
                                /*grief.cr*/
extern string                   inq_grinit(void);
extern string                   grinit_query(string section, string key);
extern int                      grinit_update(string section, string key, string value);
extern void                     grinit_onload(void);
extern void                     grinit_onexit(void);

extern void                     load_indent(void);
extern void                     load_compile(void);
extern void                     load_package(void);
extern void                     clear_buffer(void);

                                /*modeline.cr*/
extern void                     mode(~string);
extern void                     modeline(void);

                                /*colors.cr*/
extern void                     coloriser(~ string);
extern int                      colorscheme(~ string scheme, ...);
extern string                   inq_coloriser(void);

                                /*colorsvim.cr*/
extern int                      vim_colorscheme(string label, int colors, ~string base, list spec, int asgui);

                                /*command.cr*/
extern string                   fixslash(string str);
extern int                      perform_command(string cmd, ~string header, ~int, ~int);

                                /*select.cr*/
extern void                     select_editable(void);
extern int                      select_list(string title, string message_string, int step,
                                    declare l, ~int flags, ~declare help_var, ~list do_list, ~int, ~string);
extern int                      select_slim_list(string title, string message_string,
                                    declare l, int flags, ~declare help_var, ~string do_list, ~int step);
extern int                      select_buffer(int buf, int win, ~int flags, ~declare, ~list do_list,
                                    ~declare help_list, ~int start_line, ~int keep_window);

extern void                     sel_down(void);
extern void                     sel_end(void);
extern void                     sel_enter(void);
extern void                     sel_esc(void);
extern void                     sel_exit(~ int retval);
extern void                     sel_home(void);
extern void                     sel_pgdn(void);
extern void                     sel_pgup(void);
extern void                     sel_up(void);
extern void                     sel_warp(void);

extern void                     buffer_list(~int shortmode, ~int sysbuffers);

extern string                   select_file(string wild_card, ~string title, int dirs);
extern list                     select_files(string wild_card, ~string title, int dirs);

extern int                      sized_window(int lines, int width, ~string msg, ~int, ~int);

extern list                     field_list(string title, list result, list arg, ~int, int escnull = 0);

                                /*extra.cr*/
extern string                   buftype_description(int type);

                                /*debug.cr*/
extern void                     trace(~string);
extern void                     vars(~int);
extern void                     bvars(~int);
extern int                      inq_nest_level(void);

extern void                     __dbg_init(void);
extern void                     __dbg_trace__(~int, ~string, ~string);

                                /*feature.cr*/
extern int                      select_feature(list lst, int width, ~int dohelp);

                                /*ff.cr*/
extern string                   ff(~string, ~string, ~int);
extern string                   dir(~string, ~string);
extern string                   tree(~string);
extern void                     treecd(~string);
extern string                   bs(~string, ...);
extern string                   ts(~string, ~string, ...);

                                /*scrblank.cr*/
extern void                     scrblank(string arg);
extern void                     scr__blank(void);
extern void                     screen_blank(void);

                                /*restore.cr*/
extern void                     save_state(void);

                                /*autosave.cr*/
extern void                     autosave_disable(string filename);
extern int                      autosave_state(void);

                                /*search.cr*/
extern int                      search_hilite(int match_len);
extern void                     search_options(void);
extern void                     translate__fwd(void);
extern void                     translate__back(void);
extern void                     translate_again(void);
extern void                     search__fwd(void);
extern void                     search__back(void);
extern void                     search_next(void);
extern void                     search_prev(void);
extern void                     i_search(void);

                                /*setcolor.cr*/
extern void                     setcolor(~ string mode);

                                /*objects.cr*/
extern void                     objects(string function, ~declare arg1);
extern void                     shift_right(void);
extern void                     shiftr(void);
extern void                     shift_left(~declare);
extern void                     shiftl(~declare);

extern void                     default_shift_right(void);
extern void                     default_shift_left(void);
extern void                     default_delete_word_right(void);
extern void                     default_delete_word_left(void);
extern int                      default_word_left(void);
extern void                     default_word_right(void);

extern void                     delete_word(~declare);
extern int                      word_left(string pat);
extern void                     word_right(void);

                                /*options.cr*/
extern void                     options(void);
extern void                     echo_line_options(void);
extern void                     tab_options(void);
extern void                     indent_options(void);

                                /*region.cr*/
extern void                     block_upper_case(void);
extern void                     block_lower_case(void);
extern void                     block_delete(void);

                                /*history.cr*/
extern int                      _inq_history(void);
extern void                     _prompt_begin(void);
extern void                     _prompt_end(void);
extern void                     prompt_help(void);

extern string                   completion(string word, string prompt);
extern string                   compl_buffer(int what);
extern string                   compl_paste(void);
extern string                   compl_cmd(~string file);
extern string                   compl_readfile(~string file);
extern string                   compl_editfile(~string file);
extern string                   compl_bookmark(~string str);
extern string                   compl_cd(~string cmd);
extern string                   compl_history(~string str);

                                /*help.cr*/
extern void                     help(void);
extern int                      explain(~string,...);
extern void                     cshelp(string dir, string topic);
extern string                   help_resolve(string filename);
extern void                     help_display(string file, string title, declare section);
extern string                   help_window(int type, int buf, int lines, int width,
                                        int initial_line, ~int level, ~string msg);

                                /*indent.cr*/
extern void                     _slide_in();
extern void                     _slide_out();

                                /*menu.cr*/
extern void                     menu(void);
extern void                     menubar(void);
extern void                     menuon(void);
extern void                     menuoff(void);

                                /*mouse.cr*/
extern void                     mouse_enable(void);
extern void                     mouse_disable(void);
extern void                     mouse_buttons_enable(void);

                                /*misc.cr*/
extern void                     _indent(void);
extern string                   search_path(string path, string filename);
extern string                   add_to_path(string path, string name);
extern void                     delete_curr_buffer(void);
extern void                     edit_next_buffer(void);
extern void                     edit_previous_buffer(void);
extern void                     previous_edited_buffer(void);
extern void                     previous_alpha_buffer(void);
extern void                     redo(void);
extern void                     insert_tab(void);
extern void                     insert_backtab(void);
extern void                     previous_tab(void);
extern void                     tab_to_col(int col);
extern void                     display_file_name(void);
extern void                     repeat(void);
extern void                     home(void);
extern void                     end(void);
extern void                     quote(void);
extern string                   sub(string r, string s, string t);
extern string                   gsub(string r, string s, string t);
extern void                     join_line(void);
extern void                     delete_character(void);
extern void                     force_input(string str);

                                /*keys.cr*/
extern void                     _back_tab(void);
extern void                     _open_line(void);

                                /*set.cr*/
extern void                     set(~ string, ~string);
extern void                     setenv(~string);
extern string                   inq_env(~string, ~int);

                                /*shell,cr*/
extern int                      create_shell(string shell_path, string buffer_name, ...);
extern void                     sh_char_mode(void);
extern void                     sh_next_cmd(void);
extern void                     sh_line_mode(void);

                                /*tabs.cr*/
extern void                     show_tabs(void);
extern string                   detab_str(string line);
extern string                   entab_str(string line);

                                /*telnet.cr*/
extern void                     telnet(~string);
extern void                     rlogin(~string);
extern void                     ftp(~string);
extern void                     ncftp(~string);
extern string                   get_host_entry(void);

                                /*text.cr*/
extern void                     wc(void);
extern void                     grep(~string, ~string);
extern void                     fgrep(~string, ~string);
extern void                     egrep(~string, ~string);

                                /*view.cr*/
extern void                     view(string arg);
extern void                     literal(void);

                                /*wp.cr*/
extern int                      autoindent(~string arg);

                                /*window.cr*/
extern void                     goto_left_edge(void);
extern void                     goto_right_edge(void);
extern void                     set_top_of_window(void);
extern void                     set_bottom_of_window(void);
extern void                     set_center_of_window(void);

                                /*zoom.cr*/
extern void                     zoom(void);
extern void                     unzoom(void);
#endif

/*end*/
#endif /*MACSRC_GRIEF_H_INCLUDED*/
/*--end--*/


