/* -*- mode: c; tabs: 2; -*- */
/* $Id: pi.cr,v 1.4 2014/10/22 02:34:29 ayoung Exp $
 * Evaluate PI to an arbitrary number.
 *
 *
 */

#include "../grief.h"

void
pi()
{
  string str;
  int a = 10000, b, c, d, e, g;
  int t, num, i;
  list f;

  get_parm(1, num, "PI Iterations: ");
  if (num == 0) num = 100;

  c = (num * 7) / 2;
  c -= c % 14;

  for (i = 0; i < 4 * c + 4; i++)
    f[i] = 0;

  while (b != c)
    f[b++] = a / 5;

  edit_file(0, "pi.txt");
  clear_buffer();

  t = time();
  sprintf(str, "PI to %d digits\n", num); insert(str);
  while ((g = c * 2) != 0) {
    d = 0;
    for (b = c; ; d *= b) {
      d += f[b] * a;
      f[b] = d % --g;
      d /= g--;
      if (--b == 0)
        break;
    }
    c -= 14;
    sprintf(str, "%04d", e + d / a); insert(str);
    refresh();
    e = d % a;
  }
  beginning_of_line();
  delete_char();
  insert("3.");
  end_of_line();
  insert("\n");
  sprintf(str, "Time: %d seconds\n", time() - t); insert(str);
  beginning_of_line();
}
