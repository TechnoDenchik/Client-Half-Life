/*=======================================================================
	  Copyright (c) 2022 TechnoCorporation
=======================================================================*/

#include "common.h"
#include "custom.h"

qboolean CustomDecal_Validate( void *raw, int nFileSize )
{
	rgbdata_t	*test = FS_LoadImage( "#logo.bmp", raw, nFileSize );

	if( test )
	{
		// all's ok, logo is valid
		FS_FreeImage( test );
		return true;
	}

	return false;
}

void COM_ClearCustomizationList( customization_t *pHead, qboolean bCleanDecals )
{
	customization_t	*pCurrent;
	customization_t	*pNext;

	for( pCurrent = pHead->pNext; pCurrent != NULL; pCurrent = pNext )
	{
		pNext = pCurrent->pNext;

		if( pCurrent->bInUse && pCurrent->pBuffer )
			Mem_Free( pCurrent->pBuffer );

		if( pCurrent->bInUse && pCurrent->pInfo )
		{
			if( pCurrent->resource.type == t_decal )
			{
				if( bCleanDecals && CL_Active( ))
					R_DecalRemoveAll( pCurrent->nUserData1 );
			}

			FS_FreeImage( pCurrent->pInfo );
		}
		Mem_Free( pCurrent );
	}

	pHead->pNext = NULL;
}

qboolean COM_CreateCustomization( customization_t *pListHead, resource_t *pResource, int playernumber, int flags, customization_t **pOut, int *nLumps )
{
	qboolean		bError = false;
	int		checksize = 0;
	customization_t	*pCust;

	if( pOut ) *pOut = NULL;

	pCust = Z_Calloc( sizeof( customization_t ));
	pCust->resource = *pResource;

	if( pResource->nDownloadSize <= 0 )
		goto CustomizationError;

	pCust->bInUse = true;

	if( FBitSet( flags, FCUST_FROMHPAK ))
	{
		if( !HPAK_GetDataPointer( CUSTOM_RES_PATH, pResource, (byte **)&pCust->pBuffer, NULL ))
			bError = true;
	}
	else
	{

		pCust->pBuffer = FS_LoadFile( pResource->szFileName, &checksize, true );
		if( checksize != pCust->resource.nDownloadSize )
			bError = true;
	}

	if( bError )
		goto CustomizationError;

	if( FBitSet( pCust->resource.ucFlags, RES_CUSTOM ) && pCust->resource.type == t_decal )
	{
		pCust->resource.playernum = playernumber;

		if( CustomDecal_Validate( pCust->pBuffer, pResource->nDownloadSize ))
		{
			if( !FBitSet( flags, FCUST_IGNOREINIT ))
			{
				if( pResource->nDownloadSize >= (1 * 1024) && pResource->nDownloadSize <= ( 16 * 1024 ))
				{
					pCust->bTranslated = true;
					pCust->nUserData1 = 0;
					pCust->nUserData2 = 1;

					if( !FBitSet( flags, FCUST_WIPEDATA ))
						pCust->pInfo = FS_LoadImage( "#logo.bmp", pCust->pBuffer, pCust->resource.nDownloadSize );
					else pCust->pInfo = NULL;
					if( nLumps ) *nLumps = 1;
				}
			}
		}
	}

	if( pOut ) *pOut = pCust;
	pCust->pNext = pListHead->pNext;
	pListHead->pNext = pCust;

	return true;

CustomizationError:
	if( pCust->pBuffer )
		Mem_Free( pCust->pBuffer );

	if( pCust->pInfo )
		Mem_Free( pCust->pInfo );
	Mem_Free( pCust );

	return false;
}

int COM_SizeofResourceList( resource_t *pList, resourceinfo_t *ri )
{
	int		nSize = 0;
	resource_t	*p;

	memset( ri, 0, sizeof( *ri ));

	for( p = pList->pNext; p != pList; p = p->pNext )
	{
		nSize += p->nDownloadSize;

		if( p->type == t_model && p->nIndex == 1 )
			ri->info[t_world].size += p->nDownloadSize;
		else ri->info[p->type].size += p->nDownloadSize;
	}

	return nSize;
}