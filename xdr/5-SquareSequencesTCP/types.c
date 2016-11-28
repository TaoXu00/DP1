/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "types.h"

bool_t
xdr_xdrhyper (XDR *xdrs, xdrhyper *objp)
{
	register int32_t *buf;

	 if (!xdr_quad_t (xdrs, objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_xdrhypersequence (XDR *xdrs, xdrhypersequence *objp)
{
	register int32_t *buf;

	 if (!xdr_array (xdrs, (char **)&objp->xdrhypersequence_val, (u_int *) &objp->xdrhypersequence_len, ~0,
		sizeof (quad_t), (xdrproc_t) xdr_quad_t))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_Response (XDR *xdrs, Response *objp)
{
	register int32_t *buf;

	 if (!xdr_xdrhypersequence (xdrs, &objp->request))
		 return FALSE;
	 if (!xdr_xdrhypersequence (xdrs, &objp->response))
		 return FALSE;
	return TRUE;
}
