
/*	$Id: frontierdefs.h 1789 2008-09-14 20:19:40Z andreradke $    */

/******************************************************************************

    UserLand Frontier(tm) -- High performance Web content management,
    object database, system-level and Internet scripting environment,
    including source code editing and debugging.

    Copyright (C) 1992-2004 UserLand Software, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/

/*
	2004-10-26 aradke: New global header file, to be included from frontier.h and frontier.r.
*/

#ifndef __FRONTIERDEFS_H__
#define __FRONTIERDEFS_H__


	#define flcomponent 1
	#ifdef __powerc
		#define noextended 1
	#elif defined(__GNUC__)
		#define noextended 1
	#else
		#define noextended 0
	#endif


#undef MEMTRACKER		/* define as 1 to enable tracking of memory allocations */
#undef DATABASE_DEBUG	/* define as 1 to enable database debugging and logging code */


#undef flruntime
#define fldebug 1
#define flnewfeatures 1
#define version42orgreater 1
#define version5orgreater 1
#define isFrontier 1
#undef fliowa
#define threadverbs 1
#define oplanglists 1
#define flregexpverbs 1
#define PASCALSTRINGVERSION 1
#define SPEED 1
#undef fltracklocaladdresses		/*2004-12-08 aradke: disable [buggy] code for tracking deleted local addresses*/


#ifdef MACVERSION
	#define macBirdRuntime	1
	#undef appRunsCards			/*for Applet Toolkit, Iowa Runtime is baked in*/
	#define iowaRuntimeInApp	/*iowa code knows it's in an app*/
	#define iowaRuntime			/*iowa code knows it's not compiling in Card Editor*/
	#define cmdPeriodKillsCard
	#define IOAinsideApp		/*all the IOA's are baked into the app*/
	#undef coderesource			/*we're not running inside a code resource*/
#endif
#define Rez true


#endif /*__FRONTIERDEFS_H__*/
