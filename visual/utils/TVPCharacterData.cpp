
#include "tjsCommHead.h"

#include <cstdlib>
#include <unordered_map>
#include <algorithm>
#include <utility>
#include <memory>

#include "tvpgl.h"
#include "MsgIntf.h"
#include "EventIntf.h"
#include "FontRasterizer.h"

#include "CharacterData.h"
#include "TVPCharacterData.h"
/*
namespace std {
	template<>
	struct default_delete<tTVPCharacterData> {
		void operator()(tTVPCharacterData* ch) const {
			ch->Release();
		}
	};
};
*/
namespace tvp {

//---------------------------------------------------------------------------
//using CharacterDataHolder = tTJSRefHolder<CharacterData>;
//using FontCache = tTJSHashCache<FontAndCharacterData, CharacterDataHolder, FontHashFunc, TVP_CH_MAX_CACHE_HASH_SIZE>;

using FontCache = std::unordered_map<FontAndCharacterData, std::unique_ptr<CharacterData>, FontAndCharacterDataHash>;
using FontCacheIterator = std::unordered_map<FontAndCharacterData, std::unique_ptr<CharacterData>>::iterator;
using FontCacheConstIterator = std::unordered_map<FontAndCharacterData, std::unique_ptr<CharacterData>>::const_iterator;

static FontCache FontCacheData = FontCache();

//---------------------------------------------------------------------------
void ClearFontCache() {
	//FontCacheData.Clear();
	FontCacheData.clear();
}
//---------------------------------------------------------------------------
struct ClearFontCacheFunc : public tTVPCompactEventCallbackIntf {
	virtual void TJS_INTF_METHOD OnCompact(tjs_int level) {
		if(level >= TVP_COMPACT_LEVEL_MINIMIZE) {
			// clear the font cache on application minimize
			ClearFontCache();
		}
	}
} static ClearFontCacheCallback;
static bool ClearFontCacheCallbackInit = false;
//---------------------------------------------------------------------------
CharacterData* GetCharacter(const FontAndCharacterData& font, tjs_int aofsx, tjs_int aofsy) {
	// returns specified character data.
	// draw a character if needed.

	// compact interface initialization
	if( !ClearFontCacheCallbackInit ) {
		TVPAddCompactEventHook(&ClearFontCacheCallback);
		ClearFontCacheCallbackInit = true;
	}

	auto itr = FontCacheData.find( font );
	if( itr != FontCacheData.end() ) {
		// found in the cache
		return itr->second.get();
	}
	tTVPFontAndCharacterData tvpfont(font);
	std::unique_ptr<tTVPCharacterData> tvpchdata = std::unique_ptr<tTVPCharacterData>( GetCurrentRasterizer()->GetBitmap(tvpfont, aofsx, aofsy) );
	std::unique_ptr<CharacterData> chdata = std::unique_ptr<CharacterData>( new CharacterData(*tvpchdata.get()) );
	FontAndCharacterData cacheFont = font;
	std::pair<tvp::FontAndCharacterData, std::unique_ptr<tvp::CharacterData>> mapmember = std::make_pair<tvp::FontAndCharacterData, std::unique_ptr<tvp::CharacterData>>(std::move(cacheFont), std::move(chdata));
	auto isadd = FontCacheData.insert( std::move(mapmember) );
	if( !isadd.second ) {
		// 既に追加済み (先に検索しているのであり得ないはず)
		// isadd.first 既にあった要素
	}
	return isadd.first->second.get();
}
//---------------------------------------------------------------------------
CharacterData::CharacterData( const tjs_uint8 * indata, tjs_int inpitch, tjs_int originx, tjs_int originy, tjs_uint blackboxw, tjs_uint blackboxh,
 const tGlyphMetrics & metrics, bool fullcolor )
 : Metrics( metrics ), OriginX( originx ), OriginY( originy ), BlackBoxX( blackboxw ), BlackBoxY( blackboxh ), Gray( 65 ), FullColored( fullcolor ) {
	// サイズのチェック
	if( BlackBoxX != 0 && BlackBoxY != 0 ) {
		try {
			// ビットマップをコピー
			if( fullcolor ) {
				// 横方向のピッチを計算
				// MMX 等の使用を考えて横方向は 8 バイトでアライン
				Pitch = CalcAlignSize( blackboxw*4 );

				// バイト数を計算してメモリを確保
				Alloc( Pitch * blackboxh );

				// ビットマップをコピー
				inpitch *= 4;
				for(tjs_uint y = 0; y < blackboxh; y++) {
					memcpy( Data.get() + Pitch * y, indata + inpitch * y, blackboxw*4);
				}
			} else {
				// 横方向のピッチを計算
				// MMX 等の使用を考えて横方向は 8 バイトでアライン
				Pitch = CalcAlignSize( blackboxw );

				// バイト数を計算してメモリを確保
				Alloc( Pitch * blackboxh );

				// ビットマップをコピー
				for(tjs_uint y = 0; y < blackboxh; y++) {
					memcpy( Data.get() + Pitch * y, indata + inpitch * y, blackboxw);
				}
			}
		} catch(...) {
			Data.reset( nullptr );
			throw;
		}
	}
}
//---------------------------------------------------------------------------
CharacterData::CharacterData( tTVPCharacterData& src ) 
 : OriginX(src.OriginX ), OriginY(src.OriginY ), Metrics(src.Metrics ), Pitch(src.Pitch ),
 BlackBoxX(src.BlackBoxX ), BlackBoxY(src.BlackBoxY ), BlurLevel(src.BlurLevel ), BlurWidth(src.BlurWidth ),
 Gray(src.Gray ), Antialiased(src.Antialiased ), Blured(src.Blured ), FullColored(src.FullColored ) {

	size_t size = Pitch * BlackBoxY;
	Data = std::make_unique<tjs_uint8[]>( size );
	memcpy( Data.get(), src.GetData(), size );
}
//---------------------------------------------------------------------------
CharacterData::CharacterData(const CharacterData& src)
	: OriginX(src.OriginX), OriginY(src.OriginY), Metrics(src.Metrics), Pitch(src.Pitch),
	BlackBoxX(src.BlackBoxX), BlackBoxY(src.BlackBoxY), BlurLevel(src.BlurLevel), BlurWidth(src.BlurWidth),
	Gray(src.Gray), Antialiased(src.Antialiased), Blured(src.Blured), FullColored(src.FullColored) {

	size_t size = Pitch * BlackBoxY;
	Data = std::make_unique<tjs_uint8[]>(size);
	memcpy(Data.get(), src.GetData(), size);
}
//---------------------------------------------------------------------------
CharacterData& CharacterData::operator=(const CharacterData& src) {
	OriginX = src.OriginX, OriginY = src.OriginY, Metrics = src.Metrics, Pitch = src.Pitch;
	BlackBoxX = src.BlackBoxX, BlackBoxY = src.BlackBoxY, BlurLevel = src.BlurLevel, BlurWidth = src.BlurWidth;
	Gray = src.Gray, Antialiased = src.Antialiased, Blured = src.Blured, FullColored = src.FullColored;

	size_t size = Pitch * BlackBoxY;
	Data = std::make_unique<tjs_uint8[]>(size);
	memcpy(Data.get(), src.GetData(), size);
	return *this;
}
//---------------------------------------------------------------------------
void CharacterData::Expand() {
	if( FullColored )
		TVPThrowExceptionMessage( TJS_W("unimplemented: CharacterData::Expand for FullColored") );

	// expand the bitmap stored in 1bpp, to 8bpp
	tjs_int newpitch = CalcAlignSize( BlackBoxX );
	std::unique_ptr<tjs_uint8[]> newdata = std::make_unique<tjs_uint8[]>( newpitch * BlackBoxY );
	tjs_uint8* nd = newdata.get();
	tjs_int h = BlackBoxY;
	tjs_uint8 *d = Data.get();

	tjs_int w = BlackBoxX;
	static tjs_uint32 pal[2] = {0, 64};
	while(h--) {
		TVPBLExpand1BitTo8BitPal(nd, d, w, pal);
		nd += newpitch, d += Pitch;
	}
	Data = std::move( newdata );
	Pitch = newpitch;
}
//---------------------------------------------------------------------------
void CharacterData::Blur(tjs_int blurlevel, tjs_int blurwidth) {
	if( FullColored )
		TVPThrowExceptionMessage( TJS_W("unimplemented: CharacterData::Blur for FullColored") );

	// blur the bitmap with given parameters
	// blur the bitmap
	if( !Data ) return;
	if(blurlevel == 255 && blurwidth == 0) return; // no need to blur
	if(blurwidth == 0) {
		// no need to blur but must be transparent
		if( Gray == 256 )
			TVPChBlurMulCopy(Data.get(), Data.get(), Pitch*BlackBoxY, BlurLevel<<10);
		else
			TVPChBlurMulCopy65(Data.get(), Data.get(), Pitch*BlackBoxY, BlurLevel<<10);
		return;
	}

	// simple blur ( need to optimize )
	tjs_int bw = std::abs(blurwidth);
	tjs_int newwidth = BlackBoxX + bw*2;
	tjs_int newheight = BlackBoxY + bw*2;
	tjs_int newpitch =  CalcAlignSize( newwidth );

	std::unique_ptr<tjs_uint8[]> newdata = std::make_unique<tjs_uint8[]>( newpitch * newheight );

	if( Gray == 256 )
		TVPChBlurCopy(newdata.get(), newpitch, newwidth, newheight, Data.get(), Pitch, BlackBoxX, BlackBoxY, bw, blurlevel);
	else
		TVPChBlurCopy65(newdata.get(), newpitch, newwidth, newheight, Data.get(), Pitch, BlackBoxX, BlackBoxY, bw, blurlevel);

	Data = std::move( newdata );
	BlackBoxX = newwidth;
	BlackBoxY = newheight;
	Pitch = newpitch;
	OriginX -= blurwidth;
	OriginY -= blurwidth;
}
//---------------------------------------------------------------------------
void CharacterData::Blur() {
	// blur the bitmap
	Blur(BlurLevel, BlurWidth);
}
//---------------------------------------------------------------------------
void CharacterData::Bold(tjs_int size) {
	if( FullColored )
		TVPThrowExceptionMessage( TJS_W("unimplemented: CharacterData::Bold for FullColored") );

	// enbold the bitmap for 65-level grayscale bitmap
	if(size < 0) size = -size;
	tjs_int level = (tjs_int)(size / 50) + 1;
	if(level > 8) level = 8;

	// compute new metrics
	tjs_int newwidth = BlackBoxX + level;
	tjs_int newheight = BlackBoxY;
	tjs_int newpitch =  CalcAlignSize( newwidth );
	std::unique_ptr<tjs_uint8[]> newdata = std::make_unique<tjs_uint8[]>( newpitch * newheight );

	// apply bold
	tjs_uint8 * srcp = Data.get();
	tjs_uint8 * destp = newdata.get();
	for(tjs_int y = 0; y<newheight; y++) {
		for(tjs_int i = 0; i<level; i++) destp[i] = srcp[i];
		destp[0] = srcp[0];
		for(tjs_int x = level; x<newwidth-level; x++) {
			tjs_uint largest = srcp[x];
			for(tjs_int xx = x-level; xx<x; xx++)
				if((tjs_uint)srcp[xx] > largest) largest = srcp[xx];
			destp[x] = largest;
		}
		for(tjs_int i = 0; i<level; i++) destp[newwidth-i-1] = srcp[BlackBoxX-1-i];

		srcp += Pitch;
		destp += newpitch;
	}

	// replace old data
	Data = std::move( newdata );
	BlackBoxX = newwidth;
	BlackBoxY = newheight;
	OriginX -= level /2;
	Pitch = newpitch;
}
//---------------------------------------------------------------------------
void CharacterData::Bold2(tjs_int size) {
	if( FullColored )
		TVPThrowExceptionMessage( TJS_W("unimplemented: CharacterData::Bold2 for FullColored") );

	// enbold the bitmap for black/white monochrome bitmap
	if(size < 0) size = -size;
	tjs_int level = (tjs_int)(size / 50) + 1;
	if(level > 8) level = 8;

	// compute new metrics
	tjs_int newwidth = BlackBoxX + level;
	tjs_int newheight = BlackBoxY;
	tjs_int newpitch =  (((newwidth -1)>>6)+1)<<3;
	std::unique_ptr<tjs_uint8[]> newdata = std::make_unique<tjs_uint8[]>( newpitch * newheight );

	// apply bold
	tjs_uint8 * srcp = Data.get();
	tjs_uint8 * destp = newdata.get();
	for(tjs_int y = 0; y<newheight; y++) {
		memcpy(destp, srcp, Pitch);
		if(newpitch > Pitch) destp[Pitch] = 0;

		for(tjs_int i = 1; i<=level; i++) {
			tjs_uint8 bollow = 0;
			tjs_int bl = 8 - i;
			for(tjs_int x = 0; x < Pitch; x++) {
				destp[x] |= (srcp[x] >> i) + bollow;
				bollow = srcp[x] << bl;
			}
			if(newpitch > Pitch) destp[Pitch] |= bollow;
		}

		srcp += Pitch;
		destp += newpitch;
	}

	// replace old data
	Data = std::move( newdata );
	BlackBoxX = newwidth;
	BlackBoxY = newheight;
	OriginX -= level /2;
	Pitch = newpitch;
}
//---------------------------------------------------------------------------
void CharacterData::Resample4() {
	if( FullColored )
		TVPThrowExceptionMessage( TJS_W("unimplemented: CharacterData::Resample4 for FullColored") );

	// down-sampling 4x4

	static tjs_uint16 bitcounter[256] = {0xffff};
	if(bitcounter[0] == 0xffff) {
		// initialize bitcounter table
		tjs_uint i;
		for(i = 0; i<256; i++) {
			tjs_uint16 v;
			tjs_int n;
			n = i & 0x0f;
			n = (n & 0x5) + ((n & 0xa)>>1);
			n = (n & 0x3) + ((n & 0xc)>>2);
			v = (n<<2);
			n = i >> 4;
			n = (n & 0x5) + ((n & 0xa)>>1);
			n = (n & 0x3) + ((n & 0xc)>>2);
			v |= ((n<<2)) << 8;
			bitcounter[i] = v;
		}
	}

	tjs_int newwidth = ((BlackBoxX-1)>>2)+1;
	tjs_int newheight = ((BlackBoxY-1)>>2)+1;
	tjs_int newpitch =  CalcAlignSize( newwidth );
	std::unique_ptr<tjs_uint8[]> newdata = std::make_unique<tjs_uint8[]>( newpitch * newheight );

	// resampling
	tjs_uint8 * srcp = Data.get();
	tjs_uint8 * destp = newdata.get();
	for(tjs_int y = 0; y<newheight; y++) {
		if(BlackBoxX & 7) srcp[BlackBoxX / 8] &= ((tjs_int8)0x80) >> ((BlackBoxX & 7) -1); // mask right fraction

		tjs_uint orgy = y*4;
		tjs_int rem = BlackBoxY - orgy;
		rem = rem > 4 ? 4 : rem;

		tjs_uint8 *dp = destp;
		tjs_int lim = (newwidth+1) >> 1;
		for(tjs_int i = 0; i<lim; i++) {
			tjs_uint32 n = 0;
			tjs_uint8 *sp = srcp + i;
			switch(rem) {
			case 4:	n += bitcounter[*sp]; sp += Pitch;
			case 3:	n += bitcounter[*sp]; sp += Pitch;
			case 2:	n += bitcounter[*sp]; sp += Pitch;
			case 1:	n += bitcounter[*sp];
			}
			dp[0] = n >> 8;
			dp[1] = n & 0xff;
			dp += 2;
		}

		srcp += Pitch * 4;
		destp += newpitch;
	}

	// replace old data
	Data = std::move( newdata );
	BlackBoxX = newwidth;
	BlackBoxY = newheight;
	OriginX = OriginX /4;
	OriginY = OriginY /4;
	Pitch = newpitch;
}
//---------------------------------------------------------------------------
void CharacterData::Resample8()
{
	if( FullColored )
		TVPThrowExceptionMessage( TJS_W("unimplemented: CharacterData::Resample8 for FullColored") );

	// down-sampling 8x8

	static tjs_uint8 bitcounter[256] = {0xff};
	if(bitcounter[0] == 0xff) {
		// initialize bitcounter table
		tjs_uint i;
		for(i = 0; i<256; i++) {
			tjs_int n;
			n = (i & 0x55) + ((i & 0xaa)>>1);
			n = (n & 0x33) + ((n & 0xcc)>>2);
			n = (n & 0x0f) + ((n & 0xf0)>>4);
			bitcounter[i] = (tjs_uint8)n;
		}
	}

	tjs_int newwidth = ((BlackBoxX-1)>>3)+1;
	tjs_int newheight = ((BlackBoxY-1)>>3)+1;
	tjs_int newpitch =  CalcAlignSize( newwidth );
	std::unique_ptr<tjs_uint8[]> newdata = std::make_unique<tjs_uint8[]>( newpitch * newheight );

	// resampling
	tjs_uint8 * srcp = Data.get();
	tjs_uint8 * destp = newdata.get();
	for(tjs_int y = 0;;) {
		if(BlackBoxX & 7) srcp[BlackBoxX / 8] &= ((tjs_int8)0x80) >> ((BlackBoxX & 7) -1); // mask right fraction

		tjs_uint orgy = y*8;
		tjs_int rem = BlackBoxY - orgy;
		rem = rem > 8 ? 8 : rem;

		for(tjs_int x = 0; x<newwidth; x++) {
			tjs_uint n = 0;
			tjs_uint8 *sp = srcp + x;
			switch(rem) {
			case 8:	n += bitcounter[*sp]; sp += Pitch;
			case 7:	n += bitcounter[*sp]; sp += Pitch;
			case 6:	n += bitcounter[*sp]; sp += Pitch;
			case 5:	n += bitcounter[*sp]; sp += Pitch;
			case 4:	n += bitcounter[*sp]; sp += Pitch;
			case 3:	n += bitcounter[*sp]; sp += Pitch;
			case 2:	n += bitcounter[*sp]; sp += Pitch;
			case 1:	n += bitcounter[*sp];
			}
			destp[x] = n;
		}

		y++;
		if(y >= newheight) break;
		srcp += Pitch * 8;
		destp += newpitch;
	}

	// replace old data
	Data = std::move( newdata );
	BlackBoxX = newwidth;
	BlackBoxY = newheight;
	OriginX = OriginX /8;
	OriginY = OriginY /8;
	Pitch = newpitch;
}
//---------------------------------------------------------------------------
void CharacterData::AddHorizontalLine( tjs_int liney, tjs_int thickness, tjs_uint8 val ) {
	if( FullColored )
		TVPThrowExceptionMessage( TJS_W("unimplemented: CharacterData::AddHorizontalLine for FullColored") );

	tjs_int linetop = liney - thickness/2;
	if( linetop < 0 ) linetop = 0;
	tjs_int linebottom = linetop + thickness;

	tjs_int newwidth = BlackBoxX;
	tjs_int newheight = BlackBoxY;
	tjs_int overlapx = 0;
	if( OriginX < 0 ) overlapx = -OriginX;	// 前の文字にかぶるように描画されることがある
	if( BlackBoxX != (Metrics.CellIncX+overlapx) ) {
		newwidth = Metrics.CellIncX+overlapx;
	}
	int top = OriginY;
	int bottom = top + BlackBoxY;
	if( linetop < top ) { // 上過ぎる
		top = linetop;
	} else if( linebottom > bottom ) { // 下過ぎる
		bottom = linebottom;
	}
	newheight = bottom - top;
	// 大きさが変化する時は作り直す
	if( newwidth != BlackBoxX || newheight != BlackBoxY ) {
		tjs_int newpitch =  CalcAlignSize( newwidth );
		std::unique_ptr<tjs_uint8[]> newdata = std::make_unique<tjs_uint8[]>( newpitch * newheight );
		memset( newdata.get(), 0, sizeof(tjs_uint8)*newpitch*newheight );
		// x は OriginX 分ずれる
		// y は OriginY - top分ずれる
		tjs_int offsetx = OriginX;
		if( offsetx < 0 ) offsetx =0;
		tjs_int offsety = OriginY - top;
		tjs_uint8 *sp = Data.get();
		tjs_uint8 *dp = newdata.get() + offsety*newpitch + offsetx;
		for( tjs_uint y = 0; y < BlackBoxY; y++ ) {
			for( tjs_uint x = 0; x < BlackBoxX; x++ ) {
				dp[x] = sp[x];
			}
			sp += Pitch;
			dp += newpitch;
		}
		Data = std::move( newdata );
		BlackBoxX = newwidth;
		BlackBoxY = newheight;
		if( OriginX > 0 ) OriginX = 0;
		OriginY = top;
		Pitch = newpitch;
	}
	tjs_int end = linetop-OriginY+thickness;
	tjs_uint8 *dp = Data.get() + (linetop-OriginY)*Pitch;
	for( tjs_int y = 0; y < thickness; y++ ) {
		for( tjs_uint x = 0; x < BlackBoxX; x++ ) {
			dp[x] = val;
		}
		dp += Pitch;
	}
}
//---------------------------------------------------------------------------

}	// namepsace tvp
