#ifndef __OSG4WEB_WINDOWWIN32__
#define __OSG4WEB_WINDOWWIN32__ 1


#define WGL_DRAW_TO_WINDOW_ARB			0x2001
#define WGL_SUPPORT_OPENGL_ARB			0x2010
#define WGL_ACCELERATION_ARB			0x2003
#define WGL_FULL_ACCELERATION_ARB		0x2027
#define WGL_PIXEL_TYPE_ARB				0x2013
#define WGL_TYPE_RGBA_ARB				0x202B
#define WGL_COLOR_BITS_ARB				0x2014
#define WGL_RED_BITS_ARB				0x2015
#define WGL_GREEN_BITS_ARB				0x2017
#define WGL_BLUE_BITS_ARB				0x2019
#define WGL_DEPTH_BITS_ARB				0x2022
#define WGL_DOUBLE_BUFFER_ARB			0x2011

#include <windows.h>
#include <gl/gl.h>

#include <map>
#include <iostream>

/***********************************************************************
 *
 * OSG WindowWin32 Class
 *
 ***********************************************************************/
class WindowWin32
{
public:
	//Costruttore/Distruttore
	WindowWin32();
	~WindowWin32();

	static LRESULT CALLBACK WindowWin32Proc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	
	virtual LRESULT handleNativeWindowWin32Event(HWND hWnd, UINT eventmsg, WPARAM wParam, LPARAM lParam) { return (0L); };

protected:
	class Win32Stack
	{
	public:
		Win32Stack(){ };
		~Win32Stack();

		// Map Win32 window handles to WindowWin32 instance
	    typedef std::pair< HWND, WindowWin32* >  WindowHandleEntry;
	    typedef std::map<  HWND, WindowWin32* >  WindowHandles;

		void registerWindow( HWND hwnd, WindowWin32* window );
		void unregisterWindow( HWND hwnd );
		WindowWin32* getWindowWin32For( HWND hwnd );

		unsigned int getActiveWindowSize(){ return _activeWindows.size(); };

	private:
		
		WindowHandles _activeWindows;
	};

	//Funzione di inizializzazione della finestra
	bool initializeWindow(HWND handle);
	//Funzione di distruzione di finestra
	void destroyWindow();

	//Selezione del pixel format ereditato
	bool setPixelFormat();

	//Richiesta Make Current per il contesto
	bool makeCurrentImplementation();
	//Rilascia il contesto
	bool releaseContextImplementation();
	//Swap Buffer
	void swapBuffersImplementation();

	//Registrazione delle procedure di finestra (WindProc)
	bool registerWindowProcedure();
	bool unregisterWindowProcedure();

	//Get current active window stack
	static Win32Stack* getWindowInterface(bool erase = false);

	void refreshWindowDimensions();

	bool isCurrent() { return _current; }

	void logError( const std::string& msg );
	void logError( const std::string& msg, unsigned int errorCode );

	int getWindowsX() { return _windowX; }
	int getWindowsY() { return _windowY; }
	int getWindowsWidth() { return _windowWidth; }
	int getWindowsHeight() { return _windowHeight; }
private:
	std::string _windowNameWithCursor, _windowNameWithoutCursor;

	bool _windowClassesRegistered;

	bool _initialized;
	bool _realized;
	bool _current;

	HWND _hwnd;
	HDC _hdc;
	HGLRC _hglrc;
	HCURSOR _currentCursor;
	WNDPROC _windowProcedure;

	int _windowX;
	int _windowY;
	int _windowWidth;
	int _windowHeight;
};


#endif //__OSG4WEB_WINDOWWIN32__




