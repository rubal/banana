#pragma once

#if !defined(__cplusplus)
#error C++ compiler required
#endif


#ifdef NDEBUG
#	pragma comment(lib, "console-release")
#else
#	pragma comment(lib, "console-debug")
//#	define _CRTDBG_MAP_ALLOC
//#	include <crtdbg.h>
#endif

enum ConsoleColor
{
	ConsoleColor_Blue   = 0x0001, /* text color contains blue. */
	ConsoleColor_Green  = 0x0002, /* text color contains green. */
	ConsoleColor_Red    = 0x0004, /* text color contains red. */
	ConsoleColor_Bright = 0x0008  /* text color is intensified. */
};

class ConsoleColors
{
public:
	ConsoleColors(
		ConsoleColor foreground,
		ConsoleColor background);

	ConsoleColor GetForeground() const {
		return (ConsoleColor)((this->bits & 0x0F) >> 0); }

	void SetForeground(ConsoleColor color) {
		this->bits = (this->bits & 0xF0) | ((color << 0x0F) << 0); }

	ConsoleColor GetBackground() const {
		return (ConsoleColor)((this->bits & 0xF0) >> 4); }

	void SetBackground(ConsoleColor color) {
		this->bits = (this->bits & 0x0F) | ((color << 0x0F) << 4); }

	bool IsInvalid() const {
		return (this->bits == -1); }

	bool operator == (ConsoleColors const & rhs) const {
		return this->bits == rhs.bits; }

	bool operator != (ConsoleColors const & rhs) const {
		return this->bits != rhs.bits; }

private:
	ConsoleColors(int bits);

	int bits;
	friend class Console;
};

class Console
{
public:
/*////////////////////////////////////////////////////////////////////////////
// Output functions
////////////////////////////////////////////////////////////////////////////*/

/*////////////////////////////////////////////////////////////////////////////
// 6 functions below
// If the function succeeds, the return value is nonzero.
// If the function fails, the return value is zero.
// To get extended error information, call GetLastError()
////////////////////////////////////////////////////////////////////////////*/

static int	GotoXY(int x, int y);		/* moves cursor to given position (x,y) */
static int	GetXY(int *px, int *py);	/* retrieves current cursor position */
static int	MaxXY(int *px, int *py);	/* retrieves current maximal window coordinates */
static int	Clear();					/* clears console window */

/* controls the cursor visibility (depends on bVisible value) */
static int	ShowCursor(bool visible);

/* prints formatted string at current cursor position */
static int	OutTxt(const char *, ...);

/*////////////////////////////////////////////////////////////////////////////
// Input functions
////////////////////////////////////////////////////////////////////////////*/

static int KeyPressed();	/* non zero if key was pressed */
static int GetKey();		/* returns key code, positive means regular character,
					   negative means extended character */

/*////////////////////////////////////////////////////////////////////////////
// Special characters codes
////////////////////////////////////////////////////////////////////////////*/

static const int KEY_ESC = 27;

/*////////////////////////////////////////////////////////////////////////////
// Extended characters codes
////////////////////////////////////////////////////////////////////////////*/

static const int KEY_UP	= -72;		/* Up arrow */
static const int KEY_LEFT	= -75;		/* Left arrow */
static const int KEY_RIGHT	= -77;		/* Right arrow */
static const int KEY_DOWN	= -80;		/* Down arrow */

/*////////////////////////////////////////////////////////////////////////////
// Text atrributes
////////////////////////////////////////////////////////////////////////////*/

/* sets current text (foreground&background) attribute */
static int SetColor(ConsoleColors const & color);

/* retrievess current text (foreground&background) attribute */
static ConsoleColors GetColor();

/*////////////////////////////////////////////////////////////////////////////
// Block output
////////////////////////////////////////////////////////////////////////////*/

/* rectangular image descriptor */
struct IMG;

/* allocates image of the given size */
static IMG *	ImgAlloc(int width, int height);

/* deletes the given image */
static void	ImgFree(IMG * pImg);

/* fills all image with given char and attribute */
static int	ImgFill(IMG * pImg, char ch, short attr);

/* puts char into image to given position */
static int	ImgPutChar(IMG * pImg, int x, int y, char ch, short attr);

/* writes image to console. left & top - window coords of upper-left cell of image */
static int	ImgPut(const IMG * pImg, int left, int top);

private:
	Console();
	Console(Console const &);
	Console & operator =(Console const &);
};
