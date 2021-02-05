//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Base Layer Bitmap implementation
//---------------------------------------------------------------------------
#ifndef LayerBitmapImplH
#define LayerBitmapImplH

#include "tvpfontstruc.h"
#include "ComplexRect.h"

#include "BitmapInfomation.h"
#include "TVPBitmap.h"
#include "TextureInfo.h"

//---------------------------------------------------------------------------
extern void TVPSetFontCacheForLowMem();
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// tTVPNativeBaseBitmap
//---------------------------------------------------------------------------
class tTVPBitmap;
class tTVPComplexRect;
class tTVPCharacterData;
struct tTVPDrawTextData;
class tTVPPrerenderedFont;
class tTVPNativeBaseBitmap
{
	using BitmapLockType = typename tvp::bitmap::LockType;
public:
	tTVPNativeBaseBitmap(tjs_uint w, tjs_uint h, tjs_uint bpp, bool unpadding=false);
	tTVPNativeBaseBitmap(const tTVPNativeBaseBitmap & r);
	virtual ~tTVPNativeBaseBitmap();

	/* metrics */
	tjs_uint GetWidth() const ;
	void SetWidth(tjs_uint w);
	tjs_uint GetHeight() const;
	void SetHeight(tjs_uint h);

	void SetSize(tjs_uint w, tjs_uint h, bool keepimage = true);
	// for async load
	// @param bits : tTVPBitmapBitsAlloc::Allocで確保したものを使用すること
	void SetSizeAndImageBuffer( tjs_uint width, tjs_uint height, void* bits );

	/* color depth */
	tjs_uint GetBPP() const;

	bool Is32BPP() const;
	bool Is8BPP() const;

	/* assign */
	bool Assign(const tTVPNativeBaseBitmap &rhs) ;
	bool AssignBitmap(const tTVPNativeBaseBitmap &rhs) ; // assigns only bitmap

	/* scan line */
	const void * GetScanLine(tjs_uint l) const;
	void * GetScanLineForWrite(tjs_uint l);
	tjs_int GetPitchBytes() const;


	/* object lifetime management */
	void Independ();
	void IndependNoCopy();
	void Recreate();
	void Recreate(tjs_uint w, tjs_uint h, tjs_uint bpp, bool unpadding=false);

	bool IsIndependent() const { return Bitmap->IsIndependent(); }

	/* other utilities */
	tTVPBitmap* GetBitmap() const {
		if (Bitmap->IsMemoryBitmap()) {
			return reinterpret_cast<tTVPBitmap*>( const_cast<iTVPBitmap*>( Bitmap ) );
		} else {
			return nullptr;
		}
	}

	tjs_uint GetPalette( tjs_uint index ) const;
	void SetPalette( tjs_uint index, tjs_uint color );

	/* font and text functions */
private:
	tTVPFont Font;
	bool FontChanged;
	tjs_int GlobalFontState;

	// v--- these can be recreated in ApplyFont if FontChanged flag is set
	tTVPPrerenderedFont *PrerenderedFont;
	tjs_int AscentOfsX;
	tjs_int AscentOfsY;
	double RadianAngle;
	tjs_uint32 FontHash;
	// ^---

	void ApplyFont();

public:
	void SetFont(const tTVPFont &font);
	const tTVPFont & GetFont() const { return Font; };

	void GetFontList(tjs_uint32 flags, std::vector<ttstr> &list);

	void MapPrerenderedFont(const ttstr & storage);
	void UnmapPrerenderedFont();

private:
	bool InternalDrawText(tTVPCharacterData *data, tjs_int x,
		tjs_int y, tjs_uint32 shadowcolor,tTVPDrawTextData *dtdata, tTVPRect &drect);

public:
	void DrawTextSingle(const tTVPRect &destrect, tjs_int x, tjs_int y, const ttstr &text,
		tjs_uint32 color, tTVPBBBltMethod bltmode, tjs_int opa = 255,
			bool holdalpha = true, bool aa = true, tjs_int shlevel = 0,
			tjs_uint32 shadowcolor = 0,
			tjs_int shwidth = 0, tjs_int shofsx = 0, tjs_int shofsy = 0,
			tTVPComplexRect *updaterects = NULL);
	void DrawTextMultiple(const tTVPRect &destrect, tjs_int x, tjs_int y, const ttstr &text,
		tjs_uint32 color, tTVPBBBltMethod bltmode, tjs_int opa = 255,
			bool holdalpha = true, bool aa = true, tjs_int shlevel = 0,
			tjs_uint32 shadowcolor = 0,
			tjs_int shwidth = 0, tjs_int shofsx = 0, tjs_int shofsy = 0,
			tTVPComplexRect *updaterects = NULL);
	void DrawText(const tTVPRect &destrect, tjs_int x, tjs_int y, const ttstr &text,
		tjs_uint32 color, tTVPBBBltMethod bltmode, tjs_int opa = 255,
			bool holdalpha = true, bool aa = true, tjs_int shlevel = 0,
			tjs_uint32 shadowcolor = 0,
			tjs_int shwidth = 0, tjs_int shofsx = 0, tjs_int shofsy = 0,
			tTVPComplexRect *updaterects = NULL)
	{
		tjs_int len = text.GetLen();
		if(len == 0) return;
		if(len >= 2)
			DrawTextMultiple(
				destrect, x, y, text,
				color, bltmode, opa,
				holdalpha, aa, shlevel,
				shadowcolor, shwidth, shofsx, shofsy,
				updaterects);
		else    /* if len == 1 */
			DrawTextSingle(
				destrect, x, y, text,
				color, bltmode, opa,
				holdalpha, aa, shlevel,
				shadowcolor, shwidth, shofsx, shofsy,
				updaterects);
	}
	void DrawGlyph(iTJSDispatch2* glyph, const tTVPRect &destrect, tjs_int x, tjs_int y,
			tjs_uint32 color, tTVPBBBltMethod bltmode, tjs_int opa = 255,
			bool holdalpha = true, bool aa = true, tjs_int shlevel = 0,
			tjs_uint32 shadowcolor = 0,
			tjs_int shwidth = 0, tjs_int shofsx = 0, tjs_int shofsy = 0,
			tTVPComplexRect *updaterects = NULL);


private:
	tjs_int TextWidth;
	tjs_int TextHeight;
	ttstr CachedText;

	void GetTextSize(const ttstr & text);

public:
	tjs_int GetTextWidth(const ttstr & text);
	tjs_int GetTextHeight(const ttstr & text);
	double GetEscWidthX(const ttstr & text);
	double GetEscWidthY(const ttstr & text);
	double GetEscHeightX(const ttstr & text);
	double GetEscHeightY(const ttstr & text);
	void GetFontGlyphDrawRect( const ttstr & text, struct tTVPRect& area );

protected:
private:
	iTVPBitmap	*Bitmap;

public:
	void operator =(const tTVPNativeBaseBitmap &rhs) { Assign(rhs); }
};
//---------------------------------------------------------------------------
extern tjs_uint32 MakeFontHash(tTVPFont font);
//---------------------------------------------------------------------------
#endif
