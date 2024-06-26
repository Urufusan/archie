/*
 * Copyright (c) 1989, 1990, 1991 by the University of Washington
 * Copyright (c) 1993 by the University of Southern California
 *
 * For copying and distribution information, please see the files
 * <uw-copyright.h> and <usc-copyr.h>
 */

#include <uw-copyright.h>
#include <pfs.h>

/*
 * vl_comp - compare the names of two virtual links
 *
 *           VL_COMP compares the names of two links.  It returns
 *           0 if they are equal, negative if vl1 < vl2, and positive if
 *           vl1 > vl2.
 *
 *    ARGS:  vl1,vl2 - Virtual links to be compared
 * 
 * RETURNS:  0 if equal, + is vl1 > vl2, - if vl1 < vl2
 *
 *   NOTES:  Order of significance is as follows.  Existence,
 *           name.  If names do not exist, then hosttype, host,
 *           native filenametype, native filename.  The only time
 *           the name will not exist if if the link is a union link.
 */
int
vl_comp(VLINK vl1,VLINK vl2)
{	int	retval;

	if(vl1->name && !vl2->name) return(1);
	if(!vl1->name && vl2->name) return(-1);
	if(vl1->name && vl2->name && (*(vl1->name) || *(vl2->name)))
	    return(strcmp(vl1->name,vl2->name));

	retval = strcmp(vl1->hosttype,vl2->hosttype);
	if(!retval) retval = strcmp(vl1->host,vl2->host);
	if(!retval) retval = strcmp(vl1->hsonametype,vl2->hsonametype);
	if(!retval) retval = strcmp(vl1->hsoname,vl2->hsoname);
	return(retval);
    }

/*
 * vl_equal - compare the values of two virtual links
 *
 *           VL_EQUAL compares the values of two links.  It returns
 *           1 if all important fields are the same, and 0 otherwise.
 *
 *    ARGS:  vl1,vl2 - Virtual links to be compared
 * 
 * RETURNS:  1 if equal, 0 if not equal
 *
 */
int
vl_equal(vl1,vl2)
    VLINK	vl1;
    VLINK	vl2;
    {

	if(strcmp(vl1->name,vl2->name) != 0) return(0);
	if(strcmp(vl1->target,vl2->target) != 0) return(0);
	if(strcmp(vl1->hosttype,vl2->hosttype) != 0) return(0);
	if(strcmp(vl1->host,vl2->host) != 0) return(0);
	if(strcmp(vl1->hsonametype,vl2->hsonametype) != 0) return(0);
	if(strcmp(vl1->hsoname,vl2->hsoname) != 0) return(0);
	if(vl1->version != vl2->version) return(0);
	if(vl1->f_magic_no != vl2->f_magic_no) return(0);
	/* See if links types are compatible */
	if((vl1->linktype == 'I' ? 'L' : vl1->linktype) != 
	   (vl2->linktype == 'I' ? 'L' : vl2->linktype)) return(0);

	return(1);
    }

