/*=======================================================================
	  Copyright (c) 2022 TechnoCorporation
=======================================================================*/

#include "common.h"
#include <mathlib.h>
#include "gl_local.h"

//-----------------------------------------------------------------------------
// Gamma conversion support
//-----------------------------------------------------------------------------
static byte	texgammatable[256];		// palette is sent through this to convert to screen gamma
static byte	lightgammatable[256];
static int	lineargammatable[1024];
static int	screengammatable[1024];

void BuildGammaTable( float lightgamma, float brightness )
{
	int	i, inf;
	float	f, g, g1, g3;

	lightgamma = bound( 1.8f, lightgamma, 3.0f );
	brightness = bound( 0.0f, brightness, 10.0f );

	if( brightness <= 0.0f )
		g3 = 0.125f;
	else if( brightness > 1.0f )
		g3 = 0.05f;
	else g3 = 0.125f - (brightness * brightness) * 0.075f;

	g = 1.0f / lightgamma;
	g1 = GAMMA * g; 

	for( i = 0; i < 256; i++ )
	{
		f = pow( i / 255.f, GAMMA );

		// scale up
		if( brightness > 1.0f )
			f = f * brightness;

		// shift up
		if( f <= g3 ) f = (f / g3) * 0.125f;
		else f = 0.125f + ((f - g3) / (1.0f - g3)) * 0.875f;

		// convert linear space to desired gamma space
		inf = (int)( 255.0 * pow( f, g ));

		lightgammatable[i] = bound( 0, inf, 255 );
	}

	for( i = 0; i < 256; i++ )
	{
		f = 255.0 * pow(( float )i / 255.0f, TEXGAMMA );
		inf = (int)(f + 0.5f);
		texgammatable[i] = bound( 0, inf, 255 );
	}

	for( i = 0; i < 1024; i++ )
	{
		// convert from screen gamma space to linear space
		lineargammatable[i] = 1023 * pow( i / 1023.0, g1 );

		// convert from linear gamma space to screen space
		screengammatable[i] = 1023 * pow( i / 1023.0, 1.0 / g1 );
	}
}

byte LightToTexGamma( byte b )
{
	return lightgammatable[b];
}

byte TextureToGamma( byte b )
{
	return texgammatable[b];
}