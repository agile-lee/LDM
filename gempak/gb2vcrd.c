#include "config.h"
#include "log.h"

#include "gb2def.h"
#include "proto_gemlib.h"

void gb2_vcrd ( char *wmolvltbl, char *lcllvltbl, Gribmsg *cmsg, 
                int *levels, int *vcord, char *unit, int *iret )
/************************************************************************
 * gb2_vcrd								*
 *									*
 * This routine computes the vertical coordinate level/layer values     *
 * from the GRIB2 PDS.	Also, the GEMPAK vertical coordinate type is    *
 * obtained from the appropriate GRIB2 vertical coordinate table.       *
 *									*
 * If either wmolvltbl or lcllvltbl are NULL, the default tables are    *
 * read.                                                                *
 *									*
 * gb2_vcrd ( wmolvltbl, lcllvltbl, cmsg, levels, vcord, iret )		*
 *									*
 * Input parameters:							*
 *      *wmolvltbl      char            WMO vertical coordinate table   *
 *      *lcllvltbl      char            Local vertical coordinate table *
 *	*cmsg  	    struct Gribmsg      GRIB2  message structure        *
 *									*
 * Output parameters:							*
 *      levels[2]       int             lower/upper level/layer values  *
 *	*vcord		int		GEMPAK vertical goordinate type *
 *	*iret		int		return code			*
 *                                        -27 = Unrecognized PDT number *
 **									*
 * Log:									*
 * S. Gilbert/NCEP      11/04                                           *
 * S. Gilbert/NCEP      10/05           Use new routines to read tables *
 ***********************************************************************/
{
    int     ier, lvl1, lvl2, iver, lclver, ilen;
    G2level g2lev;
    G2lvls  *g2lvltbl;
    char    vparm[12];
    float   rlevel[2];
    double  sfact;
    const char* filename;

/*---------------------------------------------------------------------*/

    *iret = 0;

    /* 
     *  Get Level info from vertical coordinate table(s).
     */
    iver=cmsg->gfld->idsect[2];
    lclver=cmsg->gfld->idsect[3];
    lvl1=cmsg->gfld->ipdtmpl[9];
    lvl2=cmsg->gfld->ipdtmpl[12];

    /*
     * According to the GRIB2 documentation,
     * <http://www.wmo.int/pages/prog/www/WMOCodes/Guides/GRIB/GRIB2_062006.pdf>,
     * all the following conditions are true for a GRIB2 message that uses the
     * GRIB Master Table, which is maintained by the WMO Secretariat:
     *   - Master Table version number isn't missing (255)
     *   - Discipline number isn't reserved for local use (192-254)
     *   - Category number isn't reserved for local use (192-254)
     *   - Parameter number isn't reserved for local use (192-254)
     *   - Product definition template number isn't reserved for local use
     *     (32768-65534)
     *   - Local Table version number is zero
     *
     * We've seen many GRIB2 messages from NCEP that violate these conditions.
     *
     * Steve Emmerson 2018-08-30
     */
    if ((iver != 255) && (
         (lclver == 0  || lclver == 255) &&  // Allow missing value
         (lvl1 < 192   || lvl1 == 255  ) &&  // Allow missing value
         (lvl2 < 192   || lvl2 == 255  ))) { // Allow missing value
       /* 
        *  Get WMO vertical coordinate table.
        */
        gb2_gtwmolvltbl( wmolvltbl, iver, &g2lvltbl, &filename, &ier);
    }
    else {
       /* 
        *  Get local vertical coordinate table.
        */
        gb2_gtlcllvltbl( lcllvltbl, cmsg->origcntr, lclver, &g2lvltbl,
                &filename, &ier);
    }
    if ( ier != 0 ) {
        log_add("Couldn't get vertical coordinate table: iver=%d, lvl1=%d, "
                    "lvl2=%d, center=%.*s, lclver=%d",
                iver, lvl1, lvl2, (int)sizeof(cmsg->origcntr), cmsg->origcntr,
                lclver);
        *iret=ier;
        return;
    }
   /*
    *  Get Level into from vertical coordinate table.
    */
    gb2_sklvl( lvl1, lvl2, g2lvltbl, &g2lev, &ier);
    if ( ier != 0 ) {
        log_add("Couldn't get vertical coordinate info: iver=%d, "
                "lvl1=%d, lvl2=%d, center=%.*s, lclver=%d, file=%s",
                iver, lvl1, lvl2, (int)sizeof(cmsg->origcntr),
                cmsg->origcntr, lclver, filename);
        *iret=ier;
        return;
    }

    /* 
     *  Calculate GEMPAK vertical coordinate type
     */
    lv_cord ( g2lev.abbrev, vparm, vcord, &ier, 4, 12);
    if ( ier != 0 ) {
        *iret=-7;
        return;
    }

    /*cst_lstr ( g2lev.unit, &ilen, &ier );*/
    cst_rmbl ( g2lev.unit, unit, &ilen, &ier );
    if ( ( strcmp(unit,"Pa") == 0 ) && ( g2lev.scale == -2 ) )
       sprintf ( unit, "hPa" );
    /*else
       strncat( unit, g2lev.unit, ilen);*/

    /* 
     *  Extract vertical coordinate levels from GRIB2 PDS
     */
    gb2_vlev ( cmsg->gfld, rlevel, &ier );
    if ( ier != 0 ) {
        *iret=ier;
        return;
    }

    /* 
     *  Scale level values by scale factor found in vertical 
     *  coordinate table.
     */
    sfact=pow((double)10.0,g2lev.scale);
    levels[0] = (int)rint( rlevel[0] * sfact );
    if  ( rlevel[1] == -1.0 )  {
        levels[1] = -1;
    }
    else {
        levels[1] = (int)rint( rlevel[1] * sfact );
    }

    if ( lvl1 == 1 ) {          /*  Surface  */
        levels[0] = 0;
        levels[1] = -1;
    }


}
