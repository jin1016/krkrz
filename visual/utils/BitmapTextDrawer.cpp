
#define _USE_MATH_DEFINES
#include "tjsCommHead.h"
#include "MsgIntf.h"

#include "TVPAutoLock.h"
#include "TVPCharacterData.h"
#include "ComplexRect.h"
#include "LayerBitmapIntf.h"

#include "FontSystem.h"
#include "FreeType.h"
#include "FreeTypeFontRasterizer.h"

#include "BitmapTextDrawer.h"

extern tjs_int TVPGlobalFontStateMagic;
extern FontSystem* TVPFontSystem;
extern void TVPInializeFontRasterizers();

namespace tvp {
namespace bitmap {
//---------------------------------------------------------------------------
struct DrawTextData {
	tTVPRect rect;
	tjs_int bmppitch;
	tjs_int opa;
	bool holdalpha;
	tTVPBBBltMethod bltmode;
};
//---------------------------------------------------------------------------
// structure for holding data for a character
struct CharacterDrawData {
	CharacterData* Data; // main character data
	CharacterData* Shadow; // shadow character data
	tjs_int X, Y;
	tTVPRect ShadowRect;
	bool ShadowDrawn = false;

	CharacterDrawData( CharacterData* data, CharacterData* shadow, tjs_int x, tjs_int y)
	: Data(data), Shadow(shadow), X(x), Y(y) {
	}

	CharacterDrawData(const CharacterDrawData & rhs) {
		Data = rhs.Data;
		Shadow = rhs.Shadow;
		X = rhs.X;
		Y = rhs.Y;
		ShadowDrawn = rhs.ShadowDrawn;
		ShadowRect = rhs.ShadowRect;
	}
	CharacterDrawData( CharacterDrawData&& ref ) noexcept { *this = std::move(ref); }

	CharacterDrawData& operator = (const CharacterDrawData & rhs) {
		Data = rhs.Data;
		Shadow = rhs.Shadow;
		X = rhs.X;
		Y = rhs.Y;
		ShadowDrawn = rhs.ShadowDrawn;
		ShadowRect = rhs.ShadowRect;
		return *this;
	}
	CharacterDrawData& operator=(CharacterDrawData&& rhs) noexcept {
		Data = rhs.Data;
		Shadow = rhs.Shadow;
		X = rhs.X;
		Y = rhs.Y;
		ShadowDrawn = rhs.ShadowDrawn;
		ShadowRect = std::move( rhs.ShadowRect );
		return *this;
	}
};
//---------------------------------------------------------------------------
TextDrawer::TextDrawer() {
	TVPInializeFontRasterizers();
	Font = TVPFontSystem->GetDefaultFont();
}
//---------------------------------------------------------------------------
bool TextDrawer::CalculateDrawRect(const CharacterData* data, tjs_int x, tjs_int y, const DrawTextOptions& dtdata, tTVPRect& srect, tTVPRect& drect) {
	// setup destination and source rectangle
	drect.left = x + data->OriginX;
	drect.top = y + data->OriginY;
	drect.right = drect.left + data->BlackBoxX;
	drect.bottom = drect.top + data->BlackBoxY;

	srect.left = srect.top = 0;
	srect.right = data->BlackBoxX;
	srect.bottom = data->BlackBoxY;

	// check boundary
	if( drect.left < dtdata.rect.left ) {
		srect.left += ( dtdata.rect.left - drect.left );
		drect.left = dtdata.rect.left;
	}

	if( drect.right > dtdata.rect.right ) {
		srect.right -= ( drect.right - dtdata.rect.right );
		drect.right = dtdata.rect.right;
	}

	if( srect.left >= srect.right ) return false; // not drawable

	if( drect.top < dtdata.rect.top ) {
		srect.top += ( dtdata.rect.top - drect.top );
		drect.top = dtdata.rect.top;
	}

	if( drect.bottom > dtdata.rect.bottom ) {
		srect.bottom -= ( drect.bottom - dtdata.rect.bottom );
		drect.bottom = dtdata.rect.bottom;
	}

	if( srect.top >= srect.bottom ) return false; // not drawable

	return true;
}
//---------------------------------------------------------------------------
bool TextDrawer::InternalDrawText( tjs_uint8* sl, const CharacterData* data, tjs_int x, tjs_int y, tjs_uint32 color, const DrawTextOptions& dtdata, tTVPRect& drect ) {
	// setup destination and source rectangle
	drect.left = x + data->OriginX;
	drect.top = y + data->OriginY;
	drect.right = drect.left + data->BlackBoxX;
	drect.bottom = drect.top + data->BlackBoxY;

	tTVPRect srect;
	srect.left = srect.top = 0;
	srect.right = data->BlackBoxX;
	srect.bottom = data->BlackBoxY;

	// check boundary
	if( drect.left < dtdata.rect.left ) {
		srect.left += ( dtdata.rect.left - drect.left );
		drect.left = dtdata.rect.left;
	}

	if( drect.right > dtdata.rect.right ) {
		srect.right -= ( drect.right - dtdata.rect.right );
		drect.right = dtdata.rect.right;
	}

	if( srect.left >= srect.right ) return false; // not drawable

	if( drect.top < dtdata.rect.top ) {
		srect.top += ( dtdata.rect.top - drect.top );
		drect.top = dtdata.rect.top;
	}

	if( drect.bottom > dtdata.rect.bottom ) {
		srect.bottom -= ( drect.bottom - dtdata.rect.bottom );
		drect.bottom = dtdata.rect.bottom;
	}

	if( srect.top >= srect.bottom ) return false; // not drawable


	// blend to the bitmap
	tjs_int pitch = data->Pitch;
	tjs_int h = drect.bottom - drect.top;
	tjs_int w = drect.right - drect.left;
	const tjs_uint8* bp = data->GetData() + pitch * srect.top;
	if( BitPerPixel == 8 ) {
		// カラーフォントはコピーできない
		if( data->FullColored ) TVPThrowExceptionMessage(TVPInvalidOperationFor8BPP);

		// 8bit colorの時はただコピーするだけ
		while( h-- ) {
			memcpy( sl + drect.left, bp + srect.left, w );
			sl += dtdata.bmppitch;
			bp += pitch;
		}
		return true;
	}
	if( data->Gray == 256 ) {
		if( dtdata.bltmode == bmAlphaOnAlpha ) {
			if( dtdata.opa > 0 ) {
				if( dtdata.opa == 255 ) {
					while( h-- )
						TVPApplyColorMap_d((tjs_uint32*)sl + drect.left, bp + srect.left, w, color), sl += dtdata.bmppitch, bp += pitch;
				} else {
					while( h-- )
						TVPApplyColorMap_do((tjs_uint32*)sl + drect.left, bp + srect.left, w, color, dtdata.opa), sl += dtdata.bmppitch, bp += pitch;
				}
			} else {
				// opacity removal
				if( dtdata.opa == -255 ) {
					while( h-- )
						TVPRemoveOpacity((tjs_uint32*)sl + drect.left, bp + srect.left, w), sl += dtdata.bmppitch, bp += pitch;
				} else {
					while( h-- )
						TVPRemoveOpacity_o((tjs_uint32*)sl + drect.left, bp + srect.left, w, -dtdata.opa), sl += dtdata.bmppitch, bp += pitch;
				}
			}
		} else if( dtdata.bltmode == bmAlphaOnAddAlpha ) {
			if( dtdata.opa == 255 ) {
				while( h-- )
					TVPApplyColorMap_a((tjs_uint32*)sl + drect.left, bp + srect.left, w, color), sl += dtdata.bmppitch, bp += pitch;
			} else {
				while( h-- )
					TVPApplyColorMap_ao((tjs_uint32*)sl + drect.left, bp + srect.left, w, color, dtdata.opa), sl += dtdata.bmppitch, bp += pitch;
			}
		} else {
			if( dtdata.opa == 255 ) {
				if( dtdata.holdalpha )
					while( h-- )
						TVPApplyColorMap_HDA((tjs_uint32*)sl + drect.left, bp + srect.left, w, color), sl += dtdata.bmppitch, bp += pitch;
				else
					while( h-- )
						TVPApplyColorMap((tjs_uint32*)sl + drect.left, bp + srect.left, w, color), sl += dtdata.bmppitch, bp += pitch;
			} else {
				if( dtdata.holdalpha )
					while( h-- )
						TVPApplyColorMap_HDA_o((tjs_uint32*)sl + drect.left, bp + srect.left, w, color, dtdata.opa), sl += dtdata.bmppitch, bp += pitch;
				else
					while( h-- )
						TVPApplyColorMap_o((tjs_uint32*)sl + drect.left, bp + srect.left, w, color, dtdata.opa), sl += dtdata.bmppitch, bp += pitch;
			}
		}
	} else if( data->FullColored ) {
		if( dtdata.bltmode == bmAlphaOnAlpha ) {
			if( dtdata.opa > 0 ) {
				if( dtdata.opa == 255 ) {
					while( h-- )
						TVPAlphaBlend_d((tjs_uint32*)sl + drect.left, (tjs_uint32*)bp + srect.left, w), sl += dtdata.bmppitch, bp += pitch;
				} else {
					while( h-- )
						TVPAlphaBlend_do((tjs_uint32*)sl + drect.left, (tjs_uint32*)bp + srect.left, w, dtdata.opa), sl += dtdata.bmppitch, bp += pitch;
				}
			}
		} else if( dtdata.bltmode == bmAlphaOnAddAlpha ) {
			if( dtdata.opa == 255 ) {
				while( h-- )
					TVPAlphaBlend_a((tjs_uint32*)sl + drect.left, (tjs_uint32*)bp + srect.left, w), sl += dtdata.bmppitch, bp += pitch;
			} else {
				while( h-- )
					TVPAlphaBlend_ao((tjs_uint32*)sl + drect.left, (tjs_uint32*)bp + srect.left, w, dtdata.opa), sl += dtdata.bmppitch, bp += pitch;
			}
		} else {
			if( dtdata.opa == 255 ) {
				if( dtdata.holdalpha )
					while( h-- )
						TVPAlphaBlend_HDA((tjs_uint32*)sl + drect.left, (tjs_uint32*)bp + srect.left, w), sl += dtdata.bmppitch, bp += pitch;
				else
					while( h-- )
						TVPAlphaBlend((tjs_uint32*)sl + drect.left, (tjs_uint32*)bp + srect.left, w), sl += dtdata.bmppitch, bp += pitch;
			} else {
				if( dtdata.holdalpha )
					while( h-- )
						TVPAlphaBlend_HDA_o((tjs_uint32*)sl + drect.left, (tjs_uint32*)bp + srect.left, w, dtdata.opa), sl += dtdata.bmppitch, bp += pitch;
				else
					while( h-- )
						TVPAlphaBlend_o((tjs_uint32*)sl + drect.left, (tjs_uint32*)bp + srect.left, w, dtdata.opa), sl += dtdata.bmppitch, bp += pitch;
			}
		}
	} else {
		if( dtdata.bltmode == bmAlphaOnAlpha ) {
			if( dtdata.opa > 0 ) {
				if( dtdata.opa == 255 ) {
					while( h-- )
						TVPApplyColorMap65_d((tjs_uint32*)sl + drect.left, bp + srect.left, w, color), sl += dtdata.bmppitch, bp += pitch;
				} else {
					while( h-- )
						TVPApplyColorMap65_do((tjs_uint32*)sl + drect.left, bp + srect.left, w, color, dtdata.opa), sl += dtdata.bmppitch, bp += pitch;
				}
			} else {
				// opacity removal
				if( dtdata.opa == -255 ) {
					while( h-- )
						TVPRemoveOpacity65((tjs_uint32*)sl + drect.left, bp + srect.left, w), sl += dtdata.bmppitch, bp += pitch;
				} else {
					while( h-- )
						TVPRemoveOpacity65_o((tjs_uint32*)sl + drect.left, bp + srect.left, w, -dtdata.opa), sl += dtdata.bmppitch, bp += pitch;
				}
			}
		} else if( dtdata.bltmode == bmAlphaOnAddAlpha ) {
			if( dtdata.opa == 255 ) {
				while( h-- )
					TVPApplyColorMap65_a((tjs_uint32*)sl + drect.left, bp + srect.left, w, color), sl += dtdata.bmppitch, bp += pitch;
			} else {
				while( h-- )
					TVPApplyColorMap65_ao((tjs_uint32*)sl + drect.left, bp + srect.left, w, color, dtdata.opa), sl += dtdata.bmppitch, bp += pitch;
			}
		} else {
			if( dtdata.opa == 255 ) {
				if( dtdata.holdalpha )
					while( h-- )
						TVPApplyColorMap65_HDA((tjs_uint32*)sl + drect.left, bp + srect.left, w, color), sl += dtdata.bmppitch, bp += pitch;
				else
					while( h-- )
						TVPApplyColorMap65((tjs_uint32*)sl + drect.left, bp + srect.left, w, color), sl += dtdata.bmppitch, bp += pitch;
			} else {
				if( dtdata.holdalpha )
					while( h-- )
						TVPApplyColorMap65_HDA_o((tjs_uint32*)sl + drect.left, bp + srect.left, w, color, dtdata.opa), sl += dtdata.bmppitch, bp += pitch;
				else
					while( h-- )
						TVPApplyColorMap65_o((tjs_uint32*)sl + drect.left, bp + srect.left, w, color, dtdata.opa), sl += dtdata.bmppitch, bp += pitch;
			}
		}
	}
	return true;

}
//---------------------------------------------------------------------------
 void TextDrawer::DrawTextSingle( DrawTextOptions& option ) {
	// text drawing function for single character

	if( !option.destbmp ) return;

	if(option.bltmode == bmAlphaOnAlpha) {
		if(option.opa < -255) option.opa = -255;
		if(option.opa > 255) option.opa = 255;
	} else {
		if(option.opa < 0) option.opa = 0;
		if(option.opa > 255 ) option.opa = 255;
	}

	if(option.opa == 0) return; // nothing to do

	ApplyFont();

	const tjs_char *p = option.text.c_str();
	FontAndCharacterData font;
	font.Font = Font;
	font.Antialiased = option.aa;
	font.Hinting = true;
	font.BlurLevel = option.shlevel;
	font.BlurWidth = option.shwidth;
	font.Character = *p;
	font.Blured = false;

 	// CharacterData はキャッシュによって管理されているので、シングルスレッドで使われる限りはポインタをそのまま使って問題ない
	CharacterData* data = GetCharacter(font, AscentOfsX, AscentOfsY);
	CharacterData* shadow = nullptr;
	if( option.shlevel != 0) {
		if( option.shlevel == 255 && option.shwidth == 0) {
			// normal shadow
			shadow = data;
		} else {
			// blured shadow
			font.Blured = true;
			shadow = GetCharacter(font, AscentOfsX, AscentOfsY);
		}
	}

	if( data ) {
		if( data->BlackBoxX != 0 && data->BlackBoxY != 0 ) {
			tTVPRect drect;
			tTVPRect srect;
			tTVPRect shadowdrect;
			tTVPRect shadowsrect;

			// 描画領域矩形を取得する
			tTVPComplexRect darea;
			if( option.shlevel != 0 ) {
				CalculateDrawRect( shadow, option.x, option.y, option, shadowsrect, shadowdrect );
				darea.Or(shadowdrect );
			}
			CalculateDrawRect(data, option.x, option.y, option, srect, drect);
			darea.Or(drect);

			tTVPRect lockRect( darea.GetBound() );
			tjs_uint8* sl = (tjs_uint8*)option.destbmp->LockBits(LockType::READ_WRITE, &lockRect );
			if( sl ) {
				BitmapAutoLock<LockInterface> lock(option.destbmp);	// 書き込み完了後解放するためにロック

				bool shadowdrawn = false;

				if(shadow) {
					shadowdrawn = InternalDrawText( sl, shadow, option.x + option.shofsx, option.y + option.shofsy, option.shadowcolor, option, shadowdrect);
				}

				bool drawn = InternalDrawText( sl, data, option.x, option.y, option.color, option, drect );
				if( option.updaterects) {
					if(!shadowdrawn) {
						if(drawn) option.updaterects->Or(drect);
					} else {
						if(drawn) {
							tTVPRect d;
							TVPUnionRect(&d, drect, shadowdrect);
							option.updaterects->Or(d);
						} else {
							option.updaterects->Or(shadowdrect);
						}
					}
				}
			}
		}
	}
}
//---------------------------------------------------------------------------
// 

//---------------------------------------------------------------------------
void TextDrawer::DrawTextMultiple( DrawTextOptions& option ) {
	if( !option.destbmp ) return;

	if( option.bltmode == bmAlphaOnAlpha ) {
		if( option.opa < -255 ) option.opa = -255;
		if( option.opa > 255 ) option.opa = 255;
	} else {
		if( option.opa < 0 ) option.opa = 0;
		if( option.opa > 255 ) option.opa = 255;
	}

	if( option.opa == 0 ) return; // nothing to do

	ApplyFont();

	const tjs_char* p = option.text.c_str();
	FontAndCharacterData font;
	font.Font = Font;
	font.Antialiased = option.aa;
	font.Hinting = true;
	font.BlurLevel = option.shlevel;
	font.BlurWidth = option.shwidth;
	//font.FontHash = FontHash;

	std::vector<CharacterDrawData> drawdata;
	drawdata.reserve(option.text.GetLen());

	// prepare all drawn characters
	while( *p ) { // while input string is remaining
		font.Character = *p;
		font.Blured = false;
		CharacterData* data = GetCharacter( font, AscentOfsX, AscentOfsY );
		CharacterData* shadow = nullptr;
		if( data ) {
			if( option.shlevel != 0 ) {
				if( option.shlevel == 255 && option.shwidth == 0 ) {
					// normal shadow
					// shadow is the same as main character data
					shadow = data;
				} else {
					// blured shadow
					font.Blured = true;
					shadow = GetCharacter( font, AscentOfsX, AscentOfsY );
				}
			}

			if( data->BlackBoxX != 0 && data->BlackBoxY != 0 ) {
				// append to array
				drawdata.push_back( CharacterDrawData( data, shadow, option.x, option.y ) );
			}

			// step to the next character position
			option.x += data->Metrics.CellIncX;
			if( data->Metrics.CellIncY != 0 ) {
				// Windows 9x returns negative CellIncY.
				// so we must verify whether CellIncY is proper.
				if( Font.Angle < 1800 ) {
					if( data->Metrics.CellIncY > 0 ) data->Metrics.CellIncY = -data->Metrics.CellIncY;
				} else {
					if( data->Metrics.CellIncY < 0 ) data->Metrics.CellIncY = -data->Metrics.CellIncY;
				}
				option.y += data->Metrics.CellIncY;
			}
		}
		p++;
	}
	// 描画領域矩形を取得する
	tTVPComplexRect darea;
	tTVPRect drect, srect;
	if( option.shlevel != 0 ) {
		for( const auto& i : drawdata ) {
			CharacterData* shadow = i.Shadow;
			if( shadow ) {
				CalculateDrawRect( shadow, i.X + option.shofsx, i.Y + option.shofsy, option, srect, drect );
				darea.Or( drect );
			}
		}
	}
	for( auto& i : drawdata ) {
		CharacterData* data = i.Data;
		//tTVPRect drect(i.X + data->OriginX, i.Y + data->OriginY, i.X + data->OriginX + data->BlackBoxX, i.Y + data->OriginY + data->BlackBoxY );
		CalculateDrawRect( data, i.X, i.Y, option, srect, drect );
		darea.Or(drect);
	}
	tTVPRect lockRect( darea.GetBound() );
	tjs_uint8* sl = (tjs_uint8*)option.destbmp->LockBits( LockType::READ_WRITE, &lockRect );
	if( sl ) {
		BitmapAutoLock<LockInterface> lock(option.destbmp);	// 書き込み完了後解放するためにロック
		// draw shadows first
		if( option.shlevel != 0 ) {
			for( auto& i : drawdata ) {
				CharacterData* shadow = i.Shadow;
				if( shadow ) {
					i.ShadowDrawn = InternalDrawText( sl, shadow, i.X + option.shofsx, i.Y + option.shofsy, option.shadowcolor, option, i.ShadowRect);
				}
			}
		}

		// then draw main characters
		// and compute returning update rectangle
		for( auto& i : drawdata ) {
			CharacterData* data = i.Data;
			tTVPRect drect;

			bool drawn = InternalDrawText( sl, data, i.X, i.Y, option.color, option, drect );
			if( option.updaterects ) {
				if( !i.ShadowDrawn ) {
					if( drawn ) option.updaterects->Or(drect);
				} else {
					if( drawn ) {
						tTVPRect d;
						TVPUnionRect(&d, drect, i.ShadowRect);
						option.updaterects->Or(d);
					} else {
						option.updaterects->Or(i.ShadowRect);
					}
				}
			}
		}
		
	}

}
//---------------------------------------------------------------------------
void TextDrawer::SetFont(const tTVPFont &font)
{
	Font = font;
	FontChanged = true;
}
//---------------------------------------------------------------------------
void TextDrawer::GetFontList(tjs_uint32 flags, std::vector<ttstr> &list)
{
	ApplyFont();
	TVPFontSystem->GetFontList( list, flags, GetFont() );
}
//---------------------------------------------------------------------------
void TextDrawer::ApplyFont() {
	// apply font
	if(FontChanged || GlobalFontState != TVPGlobalFontStateMagic) {
		// Independ();

		FontChanged = false;
		GlobalFontState = TVPGlobalFontStateMagic;
		CachedText.Clear();
		TextWidth = TextHeight = 0;

		// compute ascent offset
		//GetCurrentRasterizer()->ApplyFont( this, true );
		GetCurrentRasterizer()->ApplyFont( Font );
		tjs_int ascent = GetCurrentRasterizer()->GetAscentHeight();
		RadianAngle = Font.Angle * (M_PI/1800);
		double angle90 = RadianAngle + M_PI_2;
		AscentOfsX = static_cast<tjs_int>(-std::cos(angle90) * ascent);
		AscentOfsY = static_cast<tjs_int>(std::sin(angle90) * ascent);

		// compute font hash
		//FontHash = tTJSHashFunc<ttstr>::Make(Font.Face);
		//FontHash ^= Font.Height ^ Font.Flags ^ Font.Angle;
	} else {
		//GetCurrentRasterizer()->ApplyFont( this, false );
		GetCurrentRasterizer()->ApplyFont( Font );
	}
}
//---------------------------------------------------------------------------
void TextDrawer::GetTextSize(const ttstr & text) {
	ApplyFont();

	if(text != CachedText) {
		CachedText = text;
		tjs_uint width = 0;
		const tjs_char *buf = text.c_str();

		while(*buf) {
			tjs_int w, h;
			GetCurrentRasterizer()->GetTextExtent( *buf, w, h );
			width += w;
			buf++;
		}
		TextWidth = width;
		TextHeight = std::abs(Font.Height);
	}
}
//---------------------------------------------------------------------------
tjs_int TextDrawer::GetTextWidth(const ttstr & text) {
	GetTextSize(text);
	return TextWidth;
}
//---------------------------------------------------------------------------
tjs_int TextDrawer::GetTextHeight(const ttstr & text) {
	GetTextSize(text);
	return TextHeight;
}
//---------------------------------------------------------------------------
double TextDrawer::GetEscWidthX(const ttstr & text) {
	GetTextSize(text);
	return std::cos(RadianAngle) * TextWidth;
}
//---------------------------------------------------------------------------
double TextDrawer::GetEscWidthY(const ttstr & text) {
	GetTextSize(text);
	return std::sin(RadianAngle) * (-TextWidth);
}
//---------------------------------------------------------------------------
double TextDrawer::GetEscHeightX(const ttstr & text) {
	GetTextSize(text);
	return std::sin(RadianAngle) * TextHeight;
}
//---------------------------------------------------------------------------
double TextDrawer::GetEscHeightY(const ttstr & text) {
	GetTextSize(text);
	return std::cos(RadianAngle) * TextHeight;
}
//---------------------------------------------------------------------------
void TextDrawer::GetFontGlyphDrawRect( const ttstr & text, struct tTVPRect& area ) {
	ApplyFont();
	GetCurrentRasterizer()->GetGlyphDrawRect( text, area );
}
//---------------------------------------------------------------------------

}	// bitmap
}	// tvp

