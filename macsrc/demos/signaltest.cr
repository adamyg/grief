/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: signaltest.cr,v 1.2 2014/10/22 02:34:30 ayoung Exp $
 * Basic signal handling tests
 *
 *
 */

#include "../grief.h"


static void
signal_ctrlc()
{
    message("signal-ctrlc");
}


static void
signal_usr1()
{
    message("signal-usr1");
}


static void
signal_usr2()
{
    message("signal-usr2");
}


void
signalson(void)
{
    register_macro(REG_CTRLC, "::signal_ctrlc");
    register_macro(REG_SIGUSR1, "::signal_usr1");
    register_macro(REG_SIGUSR2, "::signal_usr2");
}


void
signalsoff(void)
{
    unregister_macro(REG_CTRLC, "::signal_ctrlc");
    unregister_macro(REG_SIGUSR1, "::signal_usr1");
    unregister_macro(REG_SIGUSR1, "::signal_usr2");
}
