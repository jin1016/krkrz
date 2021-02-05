

#ifndef __TVP_CHARACTER_DATA_H__
#define __TVP_CHARACTER_DATA_H__

#include "tjsCommHead.h"
#include <memory>
#include "tvpfontstruc.h"
#include "CharacterData.h"

namespace tvp {
/**
 * １グリフのメトリックを表す構造体
 */
struct GlyphMetrics
{
	tjs_int CellIncX = 0;		//!< 一文字進めるの必要なX方向のピクセル数
	tjs_int CellIncY = 0;		//!< 一文字進めるの必要なY方向のピクセル数
};

//---------------------------------------------------------------------------
/**
 * １グリフを表すクラス
 * tTVPCharacterData から リファレンスカウントを削除
 */
class CharacterData {
	// character data holder for caching:
	std::unique_ptr<tjs_uint8[]> Data;

public:
	tjs_int OriginX = 0;	//!< 文字Bitmapを描画するascent位置との横オフセット
	tjs_int OriginY = 0;	//!< 文字Bitmapを描画するascent位置との縦オフセット
	tGlyphMetrics	Metrics; //!< メトリック、送り幅と高さを保持
	tjs_int Pitch = 0;		//!< 保持している画像ピッチ
	tjs_uint BlackBoxX = 0;	//!< 保持している画像幅
	tjs_uint BlackBoxY = 0;	//!< 保持している画像高さ
	tjs_int BlurLevel = 0;
	tjs_int BlurWidth = 0;
	tjs_uint Gray = 65;		// 階調

	bool Antialiased = false;
	bool Blured = false;
	bool FullColored = false;

public:
	CharacterData( const tjs_uint8 * indata,
		tjs_int inpitch,
		tjs_int originx, tjs_int originy,
		tjs_uint blackboxw, tjs_uint blackboxh,
		const tGlyphMetrics & metrics,
		bool fullcolor = false );

	// 変換コピー
	CharacterData( tTVPCharacterData& src );

	// コピー
	CharacterData(const CharacterData & ref);
	CharacterData& operator=(const CharacterData&);

	// move
	CharacterData( CharacterData&& ref ) noexcept { *this = std::move(ref); }
	CharacterData& operator=(CharacterData&& rhs) noexcept {
		Data = std::move( rhs.Data );
		OriginX = std::move( rhs.OriginX );
		OriginY = std::move( rhs.OriginY );
		Metrics = std::move( rhs.Metrics );
		Pitch = std::move( rhs.Pitch );
		BlackBoxX = std::move( rhs.BlackBoxX );
		BlackBoxY = std::move( rhs.BlackBoxY );
		BlurLevel = std::move( rhs.BlurLevel );
		BlurWidth = std::move( rhs.BlurWidth );
		Gray = std::move( rhs.Gray );
		Antialiased = std::move( rhs.Antialiased );
		Blured = std::move( rhs.Blured );
		FullColored = std::move( rhs.FullColored );
		return *this;
	}

	void Alloc(tjs_int size) {
		Data = std::make_unique<tjs_uint8[]>( static_cast<size_t>(size) );
	}

	static inline tjs_int CalcAlignSize( tjs_int len ) {
		return ( ( ( ( len - 1 ) >> 3 ) + 1 ) << 3 );
	}

	const tjs_uint8* GetData() const { return Data.get(); }
	tjs_uint8* GetData() { return Data.get(); }


	void Expand();

	void Blur(tjs_int blurlevel, tjs_int blurwidth);
	void Blur();

	void Bold(tjs_int size);
	void Bold2(tjs_int size);

	void Resample4();
	void Resample8();

	/**
	 * 水平線を追加する(取り消し線、アンダーライン用)
	 * @param liney : ライン中心位置
	 * @param thickness : ライン太さ
	 * @param val : ライン値
	 */
	void AddHorizontalLine( tjs_int liney, tjs_int thickness, tjs_uint8 val );
};

//---------------------------------------------------------------------------
// Character Cache management
//---------------------------------------------------------------------------
struct FontAndCharacterData {
	tTVPFont Font;
	//size_t FontHash;
	tjs_char Character;
	tjs_int BlurLevel;
	tjs_int BlurWidth;
	bool Antialiased;
	bool Blured;
	bool Hinting;
	bool operator == (const FontAndCharacterData &rhs) const {
		return Character == rhs.Character && Font == rhs.Font &&
			Antialiased == rhs.Antialiased && BlurLevel == rhs.BlurLevel &&
			BlurWidth == rhs.BlurWidth && Blured == rhs.Blured &&
			Hinting == rhs.Hinting;
	}
};
struct FontAndCharacterDataHash {
	size_t operator()(const FontAndCharacterData& val) const {
		size_t seed = 0;
		seed ^= std::hash<int>()( val.Font.Height );
		seed ^= std::hash<unsigned int>()( val.Font.Flags );
		seed ^= std::hash<int>()( val.Font.Angle );
		seed ^= std::hash<tjs_string>()( val.Font.Face.AsStdString() );
		seed ^= std::hash<bool>()( val.Antialiased ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
		seed ^= std::hash<char16_t>()( val.Character ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
		seed ^= std::hash<bool>()( val.Blured ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
		seed ^= std::hash<int>()( val.BlurLevel ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
		seed ^= std::hash<int>()( val.BlurWidth ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
		return seed;
	}
};
//---------------------------------------------------------------------------
extern CharacterData * GetCharacter( const FontAndCharacterData & font, tjs_int aofsx, tjs_int aofsy);
//---------------------------------------------------------------------------

}	// namespace tvp

namespace std {
template <>
struct hash<tvp::FontAndCharacterData> {
	size_t operator()(const tvp::FontAndCharacterData &val) const {
		size_t seed = 0;
		seed ^= std::hash<int>()(val.Font.Height);
		seed ^= std::hash<unsigned int>()(val.Font.Flags);
		seed ^= std::hash<int>()(val.Font.Angle);
		seed ^= std::hash<tjs_string>()(val.Font.Face.AsStdString());
		seed ^= std::hash<bool>()(val.Antialiased) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= std::hash<char16_t>()(val.Character) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= std::hash<bool>()(val.Blured) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= std::hash<int>()(val.BlurLevel) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= std::hash<int>()(val.BlurWidth) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		return seed;
	}
};

template<>
struct default_delete<tTVPCharacterData> {
	void operator()(tTVPCharacterData* ch) const {
		ch->Release();
	}
};
}

#endif // __TVP_CHARACTER_DATA_H__
