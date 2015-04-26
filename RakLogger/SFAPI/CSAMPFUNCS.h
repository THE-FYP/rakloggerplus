#pragma once

#if defined( SF_EXPORTS )
#define API _declspec( dllexport )
typedef CPlugin						*PPLUGIN;
typedef CPluginInfo					*PPLUGININFO;
#else
#define API _declspec( dllimport )
#pragma comment( lib, "SAMPFUNCS" )
#pragma comment( lib, "d3dx9" )
#include <string>
#include <d3d9.h>
#include <d3dx9.h>
typedef void						*PPLUGIN, *PPLUGININFO;
#endif

typedef signed char					int8_t;
typedef unsigned char				uint8_t;
typedef signed short				int16_t;
typedef unsigned short				uint16_t;
typedef signed int					int32_t;
typedef unsigned int				uint32_t;
typedef signed long long			int64_t;
typedef unsigned long long			uint64_t;
typedef const signed char			cchar;
typedef const unsigned char			cuchar;
typedef const signed short			csort;
typedef const unsigned short		cushort;
typedef const signed int			cint;
typedef const unsigned int			cuint;
typedef const signed long long		clong;
typedef const unsigned long long	culong;
template <typename T>
struct rect
{
	T _left;
	T _top;
	T _right;
	T _bottom;

	rect( T left, T top, T right, T bottom ) : _left( left ), _top( top ), _right( right ), _bottom( bottom ) { }
	rect( RECT rect ) : rect( rect.left, rect.top, rect.right, rect.bottom ) { }
	rect( void ) : rect( 0, 0, 0, 0 ) { }
};
typedef rect<float> frect;
typedef void ( __stdcall *CommandProc ) ( std::string params );

#include "SFAPI\CRakNet.h"
#include "SFAPI\CRender.h"
#include "SFAPI\CSAMP.h"
#include "SFAPI\CCLEO.h"
#include "SFAPI\CGame.h"

class API							CSAMPFUNCS
{
public:
	PPLUGIN pPlugin;

#if defined( SF_CRAKNET )
	CRakNet			*getRakNet( void );
#endif
#if defined( SF_CSAMP )
	CSAMP			*getSAMP( void );
#endif
#if defined( SF_CRENDER )
	CRender			*getRender( void );
#endif
#if defined( SF_CGAME )
	CGame			*getGame( void );
#endif
#if defined( SF_CCLEO )
	CCLEO			*getCLEO( void );
#endif
	void			Log( std::string Text );
	void			LogFile( std::string Text );
	void			LogConsole( std::string Text );
	bool			initPlugin( void (CALLBACK *PluginThread) (void), HMODULE hModule );
	unsigned int	getAPIVersion( void );
	unsigned int	getSFVersion( void );
	void			registerConsoleCommand( std::string cmd, CommandProc callback );
	void			unregisterConsoleCommand( std::string cmd );
	void			execConsoleCommand( std::string command );
};
