/* $Id: scrapper.cr,v 1.6 2014/10/27 23:28:26 ayoung Exp $
 * Scrap save and restore.
 *
 *    This macro saves the current scrap and scrap type to the state file
 *    on.  To use it, put the string "-mscrapper" (no quotes) after the
 *    "-mstate" in your GRFLAGS.
 *
 *    For example:
 *       set GRFLAGS=-mstate -mscrapper -msavehist
 *    or
 *       restore: scapper=yes savehist=yes
 */

void
scrapper(void)
{
}


void
_scrapper_save(void)
{
   int scrap_eoln, scrap_type, scrap_buf;
   string buf;

   if (scrap_buf = inq_scrap(scrap_eoln, scrap_type)) {
        end_of_buffer();
        sprintf(buf, "[grief-scrap]\n%d %d\n", scrap_eoln, scrap_type);
        insert(buf);
        paste();
        insert("[grief-scrap-end]\n");
    }
}


void
_scrapper_restore(void)
{
    save_position();
    top_of_buffer();
    if (search_fwd("<\\[grief-scrap\\]>")) {
        int scrap_eoln, scrap_type;
        string line;

        down();
        line = read();
        scrap_eoln = atoi(line);
        scrap_type = atoi(substr(line, index (line, " ")));
        down(); beginning_of_line();
        drop_anchor(scrap_type);
        search_fwd("<\\[grief-scrap-end\\]>");
        if (inq_mark_size() > 0) {
            up(); end_of_line();
            copy();
        } else {
            raise_anchor();
        }
        set_scrap_info(scrap_eoln);
    }
}

/*end*/
