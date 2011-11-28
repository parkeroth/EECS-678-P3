// $Id: nerrno.h,v 1.3 2009/09/22 14:17:36 mjantz Exp $
//
// This file contains the definitions of the various values of the nerrno
// variable, and also provides the "extern" declaration of the nerrno 
// variable. 
//
// Authors: Matt Peters, Doug McClendon, Shyam Pather

#ifdef USER_PROGRAM

#ifndef __NERRNO_H
#define __NERRNO_H

#define ENOENT 2
#define EBADF  9
#define ECHILD 10
#define EAGAIN 11
#define ENOMEM 12
#define EMFILE 24

#endif

#endif
