
/*	$Id: memory.c 1259 2006-04-13 04:56:46Z sethdill $    */

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

#include "frontier.h"
#include "standard.h"
#include "error.h"
#include "memory.h"
#include "ops.h"
#include "shellhooks.h"
#include "strings.h"
#include "byteorder.h"	/* 2006-04-08 aradke: endianness conversion macros */


static Handle getnewhandle (long ctbytes) {
	
	return NewHandle (ctbytes);
}

boolean newhandle (long ctbytes, Handle *h) {
	
	*h = getnewhandle(ctbytes);
	return *h != nil;
}

static boolean resizehandle (Handle hresize, long size) {
	
	SetHandleSize (hresize, size);
	return (MemError () == noErr);
}

void lockhandle (Handle h) {
	
	HLock (h);
	} /*lockhandle*/


void unlockhandle (Handle h) {
	
	HUnlock (h);
	} /*unlockhandle*/


boolean validhandle (Handle h) {
	
	if (h == nil)
		return (true);
	
	if (GetHandleSize (h) < 0) /*negative length never valid*/
		return (false);
	
	return (MemError () == noErr);
	} /*validhandle*/


boolean newemptyhandle (Handle *h) {
	
	return (newhandle ((long) 0, h));
	} /*newemptyhandle*/


void disposehandle (Handle h) {
	
	if (h != nil) {
		
	#ifdef MACVERSION

		DisposeHandle (h);
		//Code change by Timothy Paustian Friday, May 19, 2000 1:18:56 PM
		//disposed of handles should always be set to nil
		h = nil;
	#endif		
		}
	} /*disposehandle*/


long gethandlesize (Handle h) {
	
	if (h == nil)
		return (0L);
	
	return (GetHandleSize (h));
	} /*gethandlesize*/


boolean sethandlesize (Handle h, long size) {
	
	if (h == nil) /*defensive driving*/
		return (false);
	
	if (!resizehandle (h, size)) {
		
		memoryerror ();
		
		return (false);
		}
	
	return (true);
	} /*sethandlesize*/
	
	
boolean minhandlesize (Handle h, long size) {
	
	if (gethandlesize (h) >= size) /*already big enough*/
		return (true);
		
	return (sethandlesize (h, size)); /*try to make it larger*/
	} /*minhandlesize*/


void moveleft (ptrvoid psource, ptrvoid pdest, long length) {
	
	/*
	do a mass memory move with the left edge leading.  good for closing
	up a gap in a buffer, among other things...
	
	2002-11-12 AR: It is dangerous to use CopyMemory on Windows here. The
	current Win32 SDK headers map this to RtlCopyMemory which in turn is
	mapped to memcpy. According to the ANSI-C standard, the memcpy function
	does not have to deal properly with situations where the source and
	destination regions overlap, as may very well be the case here.
	
	Apparently, this still worked all right with the implementation of
	memcpy from the Micrsoft C runtime library, but it did not with
	the Metrowerks Standard Library (MSL).
	
	Just calling memmove on all platforms looks to be the best solution
	both in terms of performance and future maintainability.
	*/
	
	memmove (pdest, psource, length);
	} /*moveleft*/


void moveright (ptrvoid psource, ptrvoid pdest, long length) {
	
	/*
	do a mass memory move with the right edge leading.  good for opening
	up a gap in a buffer, among other things...
	
	2002-11-12 AR: see note with same date in moveleft above
	*/
	
	memmove (pdest, psource, length);
	} /*moveright*/
	

void clearbytes (ptrvoid pclear, long ctclear) {
	
	/*
	fill memory with 0's.
	*/
	
	memset (pclear, 0, ctclear);
	} /*clearbytes*/
	

void clearhandle (Handle hclear) {
	
	/*
	fill a handle's data with 0's.
	*/
	
	register Handle h = hclear;
	register long size;
	
	size = gethandlesize (h);
	
	memset (*h, 0, size);
	} /*clearhandle*/
	

void movefromhandle (Handle h, ptrvoid pdest, long length) {
	
	if (h != nil) {
		
		moveleft (*h, pdest, length);
	}
} /*movefromhandle*/


boolean newclearhandle (long size, Handle *hreturned) {
	
	register Handle h;
	register long ctbytes;
	
	ctbytes = size; /*copy into a register*/
	
	h = getnewhandle (ctbytes);
	
	if (h == nil) {
		
		*hreturned = nil;
		
		memoryerror ();
		
		return (false);
		}
		
	clearhandle (h);
	
	*hreturned = h;
	
	return (true);
	} /*newclearhandle*/
	

boolean newfilledhandle (ptrvoid pdata, long size, Handle *hreturned) {
	
	register Handle h;
	register long ctbytes;
	
	ctbytes = size; 
	
	h = getnewhandle (ctbytes);
	
	if (h == nil) {
		
		*hreturned = nil;
		
		memoryerror ();
		
		return (false);
		}
	
	moveleft (pdata, *h, ctbytes);
		
	*hreturned = h;
	
	return (true);
	} /*newfilledhandle*/


boolean copyhandle (Handle horig, Handle *hcopy) {
	
	register Handle h;
	register long ct;
	
	if (horig == nil) /*easy to copy a nil handle*/
		h = nil;
	
	else {
		
		ct = gethandlesize (horig);
		
		h = getnewhandle (ct);
		
		if (h == nil) {
			
			memoryerror ();
			
			return (false);
			}
		
		moveleft (*horig, *h, ct);
		}
	
	*hcopy = h;
	
	return (true);
	} /*copyhandle*/


short comparehandles (Handle h1, Handle h2) { //, boolean flunicase) {
	
	/*
	return -1 if h1 is less than h2, or +1 if h1 is greater than h2.
	
	return zero if the two byte streams are equal.
	
	9/21/92 dmb: make sure we don't dereference nil handles.
	*/
	
	long len1, len2;
	register long ct;
	
	len1 = gethandlesize (h1);
	
	len2 = gethandlesize (h2);
	
	ct = min (len1, len2);
	
	if (ct > 0) { /*both handles are non-nil; safe to dereference*/
		
		register ptrbyte p1 = (ptrbyte) *h1;
		register ptrbyte p2 = (ptrbyte) *h2;
		
		for (; --ct >= 0; ++p1, ++p2)
		
		//	if (flunicase? toupper (*p1) != toupper (*p2) : *p1 != *p2) { /*unequal chars*/
		
			if (*p1 != *p2) { /*unequal chars*/
				
				register char n = (char) (*p1 - *p2); /*change into signed value*/
				
				return (sgn (n));
				}
		}
	
	return (sgn (len1 - len2));
	} /*comparehandles*/


boolean equalhandles (Handle h1, Handle h2) {
	
	/*
	we could check for equal handle sizes first, we don't expect this 
	to be a time-critical routine
	*/
	
	return (comparehandles (h1, h2) == 0);
	} /*equalhandles*/


long searchhandle (Handle hsearch, Handle hpattern, long ixstart, long ixlimit) {
	
	/*
	the beginning of something bigger.  first version -- search for the 
	pattern in the byte stream, return the offset of the first occurrence of
	the pattern.  -1 if not found.
	
	8/12/92 dmb: make sure ixstart is in range
	
	8/14/92 dmb: added ixlimit parameter
	
	2004-11-13 aradke: accept nil handles for hsearch and hpattern as valid input.
	this fixes a crashing bug when accessing an empty result received from filemaker
	via apple events. [thanks to Karsten Wolf for the bug report and investigation]
	*/
	
	register ptrbyte psearch;
	register ptrbyte ppattern;
	register long ixstring;
	register long i;
	register long lensearch;
	register long lenpattern;
	register byte chfirst;
	
	if ((hsearch == nil) || (hpattern == nil))
		return (-1L);

	lensearch = gethandlesize (hsearch);
	
	lenpattern = gethandlesize (hpattern);
	
	lensearch = min (ixlimit, lensearch) - lenpattern + 1;
	
	ixstring = ixstart;
	
	if ((lensearch <= 0) || (lenpattern == 0) || (ixstring >= lensearch))
		return (-1L);

	psearch = (ptrbyte) *hsearch;
	
	ppattern = (ptrbyte) *hpattern;

	chfirst = ppattern [0];
	
	while (true) {
		
		if (psearch [ixstring] == chfirst) { /*matched at least first character in string*/
			
			for (i = 1; i < lenpattern; ++i) {
				
				if (psearch [ixstring + i] != ppattern [i]) 
					goto L1;
				} /*for*/
			
			return (ixstring); /*loop terminated, full match*/
			}
		
		L1: /*advance to next character in string*/
		
		if (++ixstring == lensearch) /*reached end of string, not found*/
			return (-1L);
		} /*while*/
	} /*searchhandle*/


long searchhandleunicase (Handle hsearch, Handle hpattern, long ixstart, long ixlimit) {
	
	/*
	the beginning of something bigger.  first version -- search for the 
	pattern in the byte stream, return the offset of the first occurrence of
	the pattern.  -1 if not found.
	
	8/12/92 dmb: make sure ixstart is in range
	
	8/14/92 dmb: added ixlimit parameter
	
	2004-11-13 aradke: accept nil handles for hsearch and hpattern as valid input.
	this fixes a crashing bug when accessing an empty result received from filemaker
	via apple events. [thanks to Karsten Wolf for the bug report and investigation]
	*/
	
	register ptrbyte psearch;
	register ptrbyte ppattern;
	register long ixstring;
	register long i;
	register long lensearch;
	register long lenpattern;
	register byte chfirst;
	
	if ((hsearch == nil) || (hpattern == nil))
		return (-1L);

	lensearch = gethandlesize (hsearch);
	
	lenpattern = gethandlesize (hpattern);

	lensearch = min (ixlimit, lensearch) - lenpattern + 1;
	
	ixstring = ixstart;
	
	if ((lensearch <= 0) || (lenpattern == 0) || (ixstring >= lensearch))
		return (-1L);
	
	psearch = (ptrbyte) *hsearch;
	
	ppattern = (ptrbyte) *hpattern;

	chfirst = getlower (ppattern [0]);

	while (true) {
		
		if (getlower (psearch [ixstring]) == chfirst) { /*matched at least first character in string*/
			
			for (i = 1; i < lenpattern; ++i) {
				
				if (getlower (psearch [ixstring + i]) != tolower(ppattern [i])) 
					goto L1;
				} /*for*/
			
			return (ixstring); /*loop terminated, full match*/
			}
		
		L1: /*advance to next character in string*/
		
		if (++ixstring == lensearch) /*reached end of string, not found*/
			return (-1L);
		} /*while*/
	} /*searchhandleunicase*/


boolean sethandlecontents (ptrvoid pdata, long ctset, Handle hset) {
	
	if (!sethandlesize (hset, ctset))
		return (false);
	
	moveleft (pdata, *hset, ctset);
	
	return (true);
	} /*sethandlecontents*/


void texttostring (ptrvoid ptext, long ctchars, bigstring bs) {
	
	register long len = ctchars;
	
	if (len > lenbigstring)
		len = lenbigstring;
	
	setstringlength (bs, len);
	
	moveleft (ptext, &bs [1], len);
	} /*texttostring*/


void texthandletostring (Handle htext, bigstring bs) {
	
	/*
	5.1.2 dmb: handle nil htext
	*/

	if (htext == nil)
		setemptystring (bs);
	else
		texttostring (*htext, gethandlesize (htext), bs);
	} /*texthandletostring*/


boolean newtexthandle (const bigstring bs, Handle *htext) {
	
	/*
	create a new handle to hold the text of the string.
	
	if the string is "\pABC" -- you get a handle of size 3.
	*/
	
	register long len = stringlength (bs);
	register Handle h;
	
	h = getnewhandle (len);
	
	if (h == nil) {
		
		memoryerror ();
		
		return (false);
		}
	
	if (len > 0)
		moveleft ((ptrstring) stringbaseaddress (bs), *h, len);
	
	*htext = h; /*pass handle back to caller*/
	
	return (true);
	} /*newtexthandle*/


boolean insertinhandle (Handle hgrow, long ix, ptrvoid pdata, long ctgrow) {
	
	/*
	make the handle big enough to hold the new data, and insert the new 
	data into the newly enlarged handle.
	
	8.1.97 dmb: if pdata is nil, initialize new data to zeros
	*/
	
	unsigned long origsize;
	
	if (ctgrow == 0)
		return (true);
	
	origsize = gethandlesize (hgrow);
	
	if (!sethandlesize (hgrow, origsize + ctgrow))
		return (false);
	
	if (ix < (long) origsize)
		moveright (*hgrow + ix, *hgrow + ix + ctgrow, origsize - ix);
	
	if (pdata == nil)
		clearbytes (*hgrow + ix, ctgrow);
	else
		moveleft (pdata, *hgrow + ix, ctgrow);
	
	return (true);
	} /*insertinhandle*/


boolean inserttextinhandle (Handle hgrow, long ix, bigstring bs) {
	
	return (insertinhandle (hgrow, ix, stringbaseaddress (bs), stringlength (bs)));
	} /*inserttextinhandle*/


boolean inserthandleinhandle (Handle hinsert, Handle h, unsigned long ix) {
	
	unsigned long insertlen = gethandlesize (hinsert);
	unsigned long ct = gethandlesize (h);
	unsigned long newlen = ct + insertlen;
	
	if (hinsert == nil) /*nothing to do*/
		return (true);
		
	if (!sethandlesize (h, newlen))
		return (false);
	
	moveright (&(*h) [ix], &(*h) [ix + insertlen], ct - ix);
	
	moveleft (*hinsert, &(*h) [ix], insertlen);
	
	return (true);
	} /*inserthandleinhandle*/
	
	
boolean pullfromhandle (Handle h, long ix, long ct, ptrvoid pdata) {
	
	/*
	8/14/91 dmb: accept nil for pdata to remove bytes without retaining them
	*/
	
	register long newsize;
	
	if (ct <= 0)
		return (true);
	
	newsize = gethandlesize (h) - ct;
	
	if ((ix < 0) || (ix > newsize))
		return (false);
	
	if (pdata != nil)
		moveleft (*h + ix, pdata, ct); /*copy out of the handle*/
	
	moveleft (*h + ix + ct, *h + ix, newsize - ix);
	
	return (sethandlesize (h, newsize)); /*should never fail -- getting smaller*/
	} /*pullfromhandle*/


boolean enlargehandle (Handle hgrow, long ctgrow, ptrvoid newdata) {
	
	/*
	make the handle big enough to hold the new data, and move the new data in
	at the end of the newly enlarged handle.
	
	8.1.97 dmb: if newdata is nil, initialize new data to zeros
	*/
	
	return (insertinhandle (hgrow, gethandlesize (hgrow), newdata, ctgrow));
	
	/*
	register Handle h = hgrow;
	register long ct = ctgrow;
	register ptrbyte p = newdata;
	register long origsize;
	
	if (hgrow == nil)
		Debugger ();
	
	if (ct == 0)
		return (true);
	
	origsize = gethandlesize (h);
	
	if (!sethandlesize (h, origsize + ct))
		return (false);
	
	moveleft (p, *h + origsize, ct);
	
	return (true);
	*/
	
	} /*enlargehandle*/


boolean mungehandle (Handle hmunge, long ixmunge, long ctmunge, ptrvoid pinsert, long ctinsert) {
	
	/*
	modeled after the Mac Toolbox utility "Munger", minus the search 
	capability, plus some error handling...
	
	overwrite ctmunge bytes in the handle hmunge, starting at offset ixmunge, 
	with the bytes indicated by pinsert, ctinsert.  resize the handle as 
	required, shifting any bytes to the right
	*/
	
	register long sizediff = ctinsert - ctmunge;
	register byte *pdata = pinsert;
	
	switch (sgn (sizediff)) { /*insert or delete unmatched bytes*/
		
		case +1: /*growing*/
			if (!insertinhandle (hmunge, ixmunge + ctmunge, pdata + ctmunge, sizediff))
				return (false);
			
			break;
		
		case -1: /*shrinking*/
			pullfromhandle (hmunge, ixmunge + ctinsert, -sizediff, nil);
			
			break;
		
		default: /*no change*/
			break;
		}
	
	moveleft (pdata, *hmunge + ixmunge, min (ctinsert, ctmunge));
	
	return (true);
	} /*mungehandle*/


boolean pushcharhandle (char ch, Handle htext) {

	/*
	2006-03-10 aradke: grow handle by one and push char onto end
	*/

	unsigned long size;
	
	size = gethandlesize (htext);
	
	if (!sethandlesize (htext, size + 1))
		return (false);
	
	(*htext)[size] = ch;

	return (true);
	} /*pushcharhandle*/


boolean pushstringhandle (const bigstring bs, Handle htext) {
	
	/*
	htext is a handle created with newtexthandle.
	
	increase the size of the handle so we can push the pascal string bs 
	at the end of the handle (including length byte).
	*/
	
	return (enlargehandle (htext, (long) stringsize (bs), (ptrvoid) bs));
	} /*pushstringhandle*/


boolean pushtexthandle (const bigstring bs, Handle htext) {
	
	/*
	htext is a handle created with newtexthandle.
	
	increase the size of the handle so we can push the text of bs at 
	the end of the handle (not including length byte).
	*/
	
	return (enlargehandle (htext, (long) stringlength (bs), (ptrvoid) stringbaseaddress (bs)));
	} /*pushtexthandle*/


boolean newheapstring (const bigstring bs, hdlstring *hstring) {
	
	/*
	3/28/97 dmb: rewrote x-plat; don't use NewString toolbox
	*/

	return (newfilledhandle ((void *) bs, stringsize (bs), (Handle *)hstring));
	} /*newheapstring*/


boolean setheapstring (const bigstring bs, hdlstring hstring) {
	
	return (sethandlecontents ((ptrvoid) bs, stringlength (bs) + 1, (Handle) hstring));
	} /*setheapstring*/


/*
boolean concatheapstrings (hdlstring *h1, hdlstring *h2, hdlstring *hreturned) {

	/%
	given two heap strings, return a heap string that's the result of 
	concatenating the two strings.
	
	return false if the resulting string would be too long, or if there
	was a memory allocation error.
	%/
	
	bigstring bs1, bs2;
	bigstring bs;
	register long len1, len2;
	register long len;
	register hdlstring hstring;
	
	copyheapstring (*h1, bs1);
	
	copyheapstring (*h2, bs2);
	
	len1 = stringlength(bs1);
	
	len2 = stringlength(bs2);
	
	len = len1 + len2;
	
	if (len > lenbigstring)
		return (false);
		
	setstringlength(bs, len);
	
	moveleft (&bs1 [1], &bs [1], len1);
	
	moveleft (&bs2 [1], &bs [1 + len1], len2);
		
	return (newheapstring (bs, hreturned));
	} /%concatheapstrings%/
*/


boolean pushhandle (Handle hsource, Handle hdest) {
	
	/*
	add the content of the source handle at the end of the destination handle.
	
	5.0.2b10 dmb: don't lock source handle during dest resize; don't
	use enlargehandle
	*/
	
	unsigned long ctsource, ctdest;
	
	if (hsource == nil)
		return (true);
	
	ctsource = gethandlesize (hsource);
	
	ctdest = gethandlesize (hdest);
	
	if (!sethandlesize (hdest, ctsource + ctdest))
		return (false);
	
	moveleft (*hsource, *hdest + ctdest, ctsource);
	
	return (true);
	} /*pushhandle*/
	

boolean loadfromhandle (Handle hload, long *ixload, long ctload, ptrvoid pdata) {
	
	/*
	copy the next ctload bytes from hload into pdata and increment the index.
	
	return false if there aren't enough bytes.
	
	start ixload at 0.
	*/
	
	register Handle h = hload;
	register ptrbyte p = pdata;
	register long ct = ctload;
	register long ix = *ixload;
	register long size;
	
	size = gethandlesize (h);
	
	if (ix < 0) /*bad index*/
		return (false);
	
	if ((ix + ct) > size) /*asked for more bytes than there are*/
		return (false); 
		
	moveleft (*h + ix, p, ct); /*copy out of the handle*/
	
	*ixload = ix + ct; /*increment the index into the handle*/
	
	return (true);
	} /*loadfromhandle*/


boolean loadfromhandletohandle (Handle hload, long *ixload, long ctload, boolean fltemp, Handle *hnew) {
	
	/*
	load from the source handle, creating a new handle to hold the loaded stuff.
	
	6/8/91 dmb: fixed memory leak if loadfromhandle fails
	
	11/27/91 dmb: reject ctload < 0
	
	2.1b3 dmb: added fltemp parameter to determine whether caller is going 
	to dispose the result soon.
	*/
	
	Handle h;
	
	if (ctload < 0)
		return (false);
	
	h = getnewhandle (ctload);
	
	if (h == nil) {
		
		memoryerror ();
		
		return (false);
		}
	
	if (!loadfromhandle (hload, ixload, ctload, *h)) {
		
		disposehandle (h);
		
		return (false);
		}
	
	*hnew = h;
	
	return (true);
	} /*loadfromhandletohandle*/


boolean loadhandleremains (long ix, Handle hsource, Handle *hdest) {

	/*
	load all the bytes following ix from the source handle into the dest handle.
	
	2.1b3 dmb: callers of this routine are (almost) always fine with 
	temporary memory, so let's use it.
	*/
	
	return (loadfromhandletohandle (hsource, &ix, gethandlesize (hsource) - ix, true, hdest));
	} /*loadhandleremains*/

	
boolean pushlongondiskhandle (long x, Handle hpush) {
	
	memtodisklong (x);
	
	return (enlargehandle (hpush, sizeof (long), &x));
	} /*pushlongtodiskhandle*/


boolean loadlongfromdiskhandle (Handle hload, long *ixload, long *x) {
	
	if (!loadfromhandle (hload, ixload, sizeof (long), x))
		return (false);
	
	disktomemlong (*x);
	
	return (true);
	} /*loadlongfromdiskhandle*/


boolean popfromhandle (Handle hpop, long ctpop, ptrvoid pdata) {
	
	register Handle h = hpop;
	register long ixpop = gethandlesize (h) - ctpop;
	
	if (ixpop < 0)
		return (false);
	
	if (pdata != nil)
		moveleft (*h + ixpop, pdata, ctpop); /*copy out of the handle*/
	
	return (sethandlesize (h, ixpop)); /*should never fail -- getting smaller*/
	} /*popfromhandle*/


boolean copyhandlecontents (Handle hsource, Handle hdest) {
	
	/*
	copy the contents of hsource into hdest's space.  if hdest isn't large
	enough to receive hsource's content, we enlarge hdest.  if this fails 
	we return false.
	
	7/5/91 dmb: we now implement a more strict copy; the destination handle 
	is always sized to match the original.
	*/
	
	register long ctsource;
	
	ctsource = gethandlesize (hsource);
	
	if (!sethandlesize (hdest, ctsource)) /*heap allocation failed*/
		return (false);
	
	moveleft (*hsource, *hdest, ctsource);
	
	return (true);
	} /*copyhandlecontents*/
	

boolean concathandles (Handle h1, Handle h2, Handle *hmerged) {
	
	/*
	create a new handle which is the concatenation of two handles.
	*/
	
	register Handle h;
	long sizefirsthandle;
	long sizesecondhandle;
	register ptrbyte p;
	
	*hmerged = nil; /*default return value*/
	
	sizefirsthandle = gethandlesize (h1);
	
	sizesecondhandle = gethandlesize (h2);
	
	h = getnewhandle (sizefirsthandle + sizesecondhandle);
	
	if (h == nil) {
		
		memoryerror ();
		
		return (false);
		}
	
	p = (ptrbyte) *h;
		
	moveleft (*h1, p, sizefirsthandle);
	
	p += sizefirsthandle;
	
	moveleft (*h2, p, sizesecondhandle);
	
	*hmerged = h;
	
	return (true);
	} /*concathandles*/
	
	
boolean mergehandles (Handle h1, Handle h2, Handle *hmerged) {
	
	/*
	create a new handle which is the concatenation of two handles.  the first
	four bytes of the new handle store the size of the first handle so the merged
	handle can be easily unpacked.
	
	6/8/90 DW: modified so it could deal with nil handles.
	
	10/7/91 dmb: try to merge result into the larger of the original handles, so 
	that our memory overhead can be almost cut in half.
	
	2.1b3 dmb: in the unusual case the we allocated a new handle, go ahead 
	and use temporary memory if available. this might not always be ideal, but 
	more often than not it will be best -- we're likely Saving, and tossing 
	the merged handle soon.
	*/
	
	register Handle h;
	long sizefirsthandle;
	long sizesecondhandle;
	long sizemergedhandle;
	long storesizefirsthandle;
	register ptrbyte p;
	
	*hmerged = nil; /*default return value*/
	
	sizefirsthandle = gethandlesize (h1);
	storesizefirsthandle = disklong (sizefirsthandle);
	
	sizesecondhandle = gethandlesize (h2);
	
	sizemergedhandle = sizeof (long) + sizefirsthandle + sizesecondhandle;
	
	if (sizefirsthandle > sizesecondhandle) { /*try using h1 for result*/
		
		if (resizehandle (h1, sizemergedhandle)) {
			
			p = (ptrbyte) *h1;
			
			moveright (p, p + sizeof (long), sizefirsthandle);
			
			moveleft (&storesizefirsthandle, p, sizeof (long));
			
			if (h2 != nil)
				moveleft (*h2, p + sizeof (long) + sizefirsthandle, sizesecondhandle);
			
			*hmerged = h1;
			
			disposehandle (h2);
			
			return (true);
			}
		}
	
	else if (h2 != nil) { /*try using h2 for result*/
		
		if (resizehandle (h2, sizemergedhandle)) {
			
			p = (ptrbyte) *h2;
			
			moveright (p, p + sizeof (long) + sizefirsthandle, sizesecondhandle);
			
			moveleft (&storesizefirsthandle, p, sizeof (long));
			
			if (h1 != nil)
				moveleft (*h1, p + sizeof (long), sizefirsthandle);
			
			*hmerged = h2;
			
			disposehandle (h1);
			
			return (true);
			}
		}
	
	/*resizing didn't work; try it the old way, using a newly-allocated handle*/
	
	h = getnewhandle (sizemergedhandle);
	
	if (h == nil) {
		
		memoryerror ();
		
		disposehandle (h1);
		
		disposehandle (h2);
		
		return (false);
		}
	
	p = (ptrbyte) *h;
	
	moveleft (&storesizefirsthandle, p, sizeof (long));
	
	p += sizeof (long);
	
	if (h1 != nil)
		moveleft (*h1, p, sizefirsthandle);
	
	if (h2 != nil)
		moveleft (*h2, p + sizefirsthandle, sizesecondhandle);
	
	*hmerged = h;
	
	disposehandle (h1);
	
	disposehandle (h2);
	
	return (true);
	} /*mergehandles*/


boolean unmergehandles (Handle hmerged, Handle *hfirst, Handle *hsecond) {
	
	/*
	split up a handle created by mergehandle.
	
	3/12/91 dmb: rewrote so that we own hmerged; caller no longer diposes it.  
	this allows us to reuse the handle to greatly reduce memory requirements.  
	we could further optimize by reusing hmerged for the larger handle instead 
	of always hsecond.
	
	also, avoid locking handles when we're trying to allocate potentially 
	large chunks of memory.
	
	2.1b3 dmb: newly-created handle is always ok as temporary memory
	*/
	
	register Handle h1 = nil, h2 = nil;
	register Handle h = hmerged;
	long ix;
	long sizefirsthandle, sizesecondhandle;
	
	*hfirst = *hsecond = nil; /*default return values*/
	
	moveleft (*h, &sizefirsthandle, sizeof (long));

	disktomemlong (sizefirsthandle);
	
	ix = sizeof (long);
	
	if (sizefirsthandle == 0)
		h1 = nil;
		
	else {
		h1 = getnewhandle (sizefirsthandle);
		
		if (h1 == nil) 
			goto error;
		
		moveleft (*h + ix, *h1, sizefirsthandle);
		}
	
	ix += sizefirsthandle;
	
	sizesecondhandle = gethandlesize (h) - sizefirsthandle - sizeof (long);
	
	if (sizesecondhandle == 0) {
		
		h2 = nil;
		
		disposehandle (h);
		}
	else {
		
		h2 = h; /*second handle can safely re-use merged handle*/
		
		moveleft (*h2 + ix, *h2, sizesecondhandle);
		
		sethandlesize (h2, sizesecondhandle);
		}
	
	*hfirst = h1; /*return handles to caller*/
	
	*hsecond = h2;
	
	return (true);
		
	error:
	
	disposehandle (h);
	
	disposehandle (h1);
		
	disposehandle (h2);
	
	memoryerror ();
	
	return (false);
	} /*unmergehandles*/
	
	
boolean newintarray (short ct, hdlintarray *harray) {
	
	Handle h;
	
	if (!newclearhandle (sizeof (short) * ct, &h))
		return (false);
		
	*harray = (hdlintarray) h;
	
	return (true);
	} /*newintarray*/
	
boolean setintarray (hdlintarray harray, short ix, short val) {
	
	/*
	assign into a cell in a variable-size array of integers, 0-based index.
	
	return false if the array needed to be extended but there isn't enough
	memory to do it.
	*/
	
	register hdlintarray h = harray;

	if (!minhandlesize ((Handle) h, (ix + 1) * sizeof (short)))
		return (false);
	
	(*harray) [ix] = val;
	
	return (true);
	} /*setintarray*/
	
	
boolean getintarray (hdlintarray harray, short ix, short *val) {
	
	*val = (*harray) [ix];
	
	return (true);
	} /*getintarray*/
	
	
void fillintarray (hdlintarray harray, short val) {
	
	register hdlintarray h = harray;
	register short ct;
	/*
	register short i;
	*/
	
	ct = (short) gethandlesize ((Handle) h) / sizeof (short);

	while (--ct	>= 0)
		(*h) [ct] = val;

	/*
	lockhandle ((Handle) h);
	
	for (i = 0; i < ct; i++)
		(*h) [i] = x;

	unlockhandle ((Handle) h);
	*/

	} /*fillintarray*/


void openhandlestream (Handle h, handlestream *s) {
	
	(*s).data = h;
	(*s).pos = 0;
	(*s).eof = (*s).size = gethandlesize (h);
	} /*openhandlestream*/


boolean growhandlestream (handlestream *s, long ct) {
	
	/*
	make the stream large enough to accomodate ct bytes of data

	6.1d1 AR: Fixed a bug: (*s).sizeneeded was not set correctly
	for newly created handles.
	*/
	
	#define blocksize 1024
	long sizeneeded = quantumize (ct, blocksize);
	
	if ((*s).data == nil) {
		
		if (!newhandle (sizeneeded, &(*s).data))
			return (false);
		
		(*s).size = sizeneeded;
		}
	else {
		if ((*s).size < sizeneeded) {
			
			if (!sethandlesize ((*s).data, sizeneeded))
				return (false);
			
			(*s).size = sizeneeded;
			}
		}
	
	return (true);
	} /*growhandlestream*/


boolean writehandlestream (handlestream *s, void *p, long ct) {
	
	if (!growhandlestream (s, (*s).pos + ct))
		return (false);
	
	moveleft (p, *(*s).data + (*s).pos, ct);
	
	(*s).pos += ct;
	
	(*s).eof = max ((*s).eof, (*s).pos);
	
	return (true);
	} /*writehandlestream*/


boolean writehandlestreamlong (handlestream *s, long x) {

	bigstring bs;

	numbertostring (x, bs);
	
	return (writehandlestream (s, stringbaseaddress (bs), stringlength (bs)));
	} /*writehandlestreamstring*/

boolean writehandlestreamchar (handlestream *s, byte ch) {
	
	return (writehandlestream (s, &ch, sizeof (ch)));
	} /*writehandlestreamchar*/


boolean writehandlestreamstring (handlestream *s, bigstring bs) {
	
	return (writehandlestream (s, stringbaseaddress (bs), stringlength (bs)));
	} /*writehandlestreamstring*/


boolean writehandlestreamhandle (handlestream *s, Handle h) {
	
	long ct = gethandlesize (h);
	
	if (ct == 0)
		return (true);
	
	if (!growhandlestream (s, (*s).pos + ct)) // grow first, so we don't have to lock handle
		return (false);
	
	return (writehandlestream (s, *h, ct));
	} /*writehandlestreamhandle*/


boolean writehandlestreamhandlepart (handlestream *s, Handle h, long ix, long len) {
	
	long ct = gethandlesize (h);
	
	if (len <= 0 || ix < 0 || ix >= ct)
		return (true);
	
	if (ix + len > ct)
		len = ct - ix;
	
	if (!growhandlestream (s, (*s).pos + len)) // grow first, so we don't have to lock handle
		return (false);
	
	return (writehandlestream (s, &((*h)[ix]), len));
	} /*writehandlestreamhandlepart*/


boolean writehandlestreamhandleindent (handlestream *s, Handle h, long i) {

	while (i > 0) { /*indent*/

		if (!writehandlestreamchar (s, '\t'))
			return (false);
		
		i--;
		} /*while*/

	if (!writehandlestreamhandle (s, h))
		return (false);

	return (writehandlestreamchar (s, '\r'));
	} /*writehandlestreamhandleindent*/


boolean writehandlestreamstringindent (handlestream *s, bigstring bs, long i) {

	while (i > 0) { /*indent*/

		if (!writehandlestreamchar (s, '\t'))
			return (false);
		
		i--;
		} /*while*/

	if (!writehandlestreamstring (s, bs))
		return (false);

	return (writehandlestreamchar (s, '\r'));
	}/*writehandlestreamstringindent*/


boolean mergehandlestreamdata (handlestream *s, long ctreplace, ptrvoid pdata, long ctmerge) {
	
	/*
	"munge" the stream data.
	
	unlike the write calls, we preserve data following the current position.
	we push out the eof accordingly.
	*/
	
	long ctgrow = ctmerge - ctreplace;
	char *p;
	
	if (!growhandlestream (s, (*s).eof + ctgrow))
		return (false);
	
	p = *(*s).data + (*s).pos;
	
	// these directions will be wrong is ctmerge < ctreplace, but they're the same
	moveright (p + ctreplace, p + ctmerge, (*s).eof - (*s).pos - ctreplace);
	
	moveleft (pdata, p, ctmerge);
	
	(*s).pos += ctmerge;
	
	(*s).eof += ctgrow;
	
	return (true);
	} /*mergehandlestreamdata*/


boolean mergehandlestreamhandle (handlestream *s, long ctreplace, Handle hmerge) {
	
	/*
	"munge" the stream data, cosuming h.
	
	unlike the write calls, we preserve data following the current position.
	we push out the eof accordingly.
	*/
	
	long ctmerge = gethandlesize (hmerge);
	long ctgrow = ctmerge - ctreplace;
	
	if (!growhandlestream (s, (*s).eof + ctgrow)) { // grow first, so we don't have to lock handle
		
		disposehandle (hmerge);
		
		return (false);
		}
	
	mergehandlestreamdata (s, ctreplace, *hmerge, ctmerge);

	disposehandle (hmerge);
	
	return (true);
	} /*mergehandlestreamhandle*/


boolean mergehandlestreamstring (handlestream *s, long ctreplace, bigstring bsmerge) {
	
	return (mergehandlestreamdata (s, ctreplace, stringbaseaddress (bsmerge), stringlength (bsmerge)));
	} /*mergehandlestreamstring*/


boolean readhandlestream (handlestream *s, void *p, long ct) {
	
	if (ct > (*s).eof - (*s).pos)
		return (false);
	
	moveleft (*(*s).data + (*s).pos, p, ct);
	
	(*s).pos += ct;
	
	return (true);
	} /*readhandlestream*/


boolean pullfromhandlestream (handlestream *s, long ctpull, void *pdata) {

	/*
	pull bytes out of the stream, shrinking the data. if a pointer is 
	provided, copy the data to it.
	*/
	
	char *p;
	
	p = *(*s).data + (*s).pos;
	
	if (pdata != nil)
		moveleft (p, pdata, ctpull);
	
	moveleft (p + ctpull, p, (*s).eof - (*s).pos - ctpull);
	
	(*s).eof -= ctpull;
	
	return (true);
	} /*pullfromhandlestream*/


long skiphandlestreamchars (handlestream *s, byte chskip) {
	
	/*
	return the number of characters skipped
	*/
	
	char *p = *(*s).data;
	long ctskipped = 0;
	
	while ((*s).pos < (*s).eof) {
		
		if (p [(*s).pos] != chskip)
			break;
		
		++ctskipped;
		
		++(*s).pos;
		}
	
	return (ctskipped);
	} /*skiphandlestreamchars*/


boolean skiphandlestreamwhitespace (handlestream *s) {
	
	char *p = *(*s).data;
	
	while (isspace (p [(*s).pos])) {
		
		if (++(*s).pos >= (*s).eof)
			return (false);
		}
	
	return (true);
	} /*skiphandlestreamwhitespace*/


long seekhandlestreamchar (handlestream *s, byte chseek)  {
	
	/*
	return the number of characters skipped
	*/
	
	char *p = *(*s).data;
	long ctskipped = 0;
	
	while ((*s).pos < (*s).eof) {
		
		if (p [(*s).pos] == chseek)
			break;
		
		++ctskipped;
		
		++(*s).pos;
		}
	
	return (ctskipped);
	} /*skiphandlestreamchars*/


boolean readhandlestreamfieldtohandle (handlestream *s, byte chdelimiter, Handle *hreturned) {
	
	long startpos = (*s).pos;
	char *p = *(*s).data;
	boolean fl;
	
	while (p [(*s).pos] != chdelimiter) {
		
		if (++(*s).pos >= (*s).eof)
			break;
		}
	
	fl = loadfromhandletohandle ((*s).data, &startpos, (*s).pos - startpos, false, hreturned);
	
	++(*s).pos; //skip delimiter
	
	return (fl);
	} /*readhandlestreamfieldtohandle*/


boolean readhandlestreamfield (handlestream *s, byte chdelimiter, bigstring bsfield) {
	
	long startpos = (*s).pos;
	char *p = *(*s).data;
	
	while (p [(*s).pos] != chdelimiter) {
		
		if (++(*s).pos >= (*s).eof)
			break;
		}
	
	texttostring (p + startpos, (*s).pos - startpos, bsfield);
	
	++(*s).pos; //skip delimiter
	
	return (!isemptystring (bsfield));
	} /*readhandlestreamfield*/


byte gethandlestreamcharacter (handlestream *s, long pos) {
	
	return ((*(*s).data) [pos]);
	} /*gethandlestreamcharacter*/


byte nexthandlestreamcharacter (handlestream *s) {
	
	return (gethandlestreamcharacter (s, (*s).pos));
	} /*nexthandlestreamcharacter*/


byte lasthandlestreamcharacter (handlestream *s) {
	
	return (gethandlestreamcharacter (s, (*s).eof - 1));
	} /*lasthandlestreamcharacter*/


boolean athandlestreameof (handlestream *s) {
	
	return ((*s).pos >= (*s).eof);
	} /*athandlestreameof*/


Handle closehandlestream (handlestream *s) {
	
	sethandlesize ((*s).data, (*s).eof);
	
	(*s).size = (*s).eof;
	
	return ((*s).data);
	} /*closehandlestream*/


void disposehandlestream (handlestream *s) {
	
	disposehandle ((*s).data);
	
	clearbytes (s, sizeof (*s));
	} /*disposehandlestream*/






