/* -*- mode: cr; indent-width: 4; -*- */

static list
specs[] = {
    { "Normal", 231,  16, NULL, "ffffff", "000000", NULL },
    { "Visual", 240, 253, NULL, "585858", "dadada", NULL }
    };

void
listtest2(void)
{
    list spec;

    while (list_each(specs, spec) >= 0) {
        string val;

        val = format(" hi ctermfg=%d ctermbg=%d cterm=%s guifg=#%s guibg=#%s gui=%s", 
                spec[0], spec[1], spec[2], spec[3], spec[4], spec[5]);
    }
}
