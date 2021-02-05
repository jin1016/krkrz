

#ifndef __CHARACTER_DATA_H__
#define __CHARACTER_DATA_H__

#include "tjsCommHead.h"
#include "tvpfontstruc.h"

/**
 * １グリフのメトリックを表す構造体
 */
struct tGlyphMetrics
{
	tjs_int CellIncX = 0;		//!< 一文字進めるの必要なX方向のピクセル数
	tjs_int CellIncY = 0;		//!< 一文字進めるの必要なY方向のピクセル数
};

//---------------------------------------------------------------------------
/**
 * １グリフを表すクラス
 */
class tTVPCharacterData
{
	// character data holder for caching
private:
	tjs_uint8 * Data;
	tjs_int RefCount;

public:
	tjs_int OriginX = 0; //!< 文字Bitmapを描画するascent位置との横オフセット
	tjs_int OriginY = 0; //!< 文字Bitmapを描画するascent位置との縦オフセット
	tGlyphMetrics	Metrics; //!< メトリック、送り幅と高さを保持
	tjs_int Pitch = 0; //!< 保持している画像ピッチ
	tjs_uint BlackBoxX = 0; //!< 保持している画像幅
	tjs_uint BlackBoxY = 0; //!< 保持している画像高さ
	tjs_int BlurLevel = 0;
	tjs_int BlurWidth = 0;
	tjs_uint Gray = 65; // 階調

	bool Antialiased = false;
	bool Blured = false;
	bool FullColored = false;

public:
	tTVPCharacterData() : Gray(65), FullColored(false), RefCount(1), Data(nullptr) {}
	tTVPCharacterData( const tjs_uint8 * indata,
		tjs_int inpitch,
		tjs_int originx, tjs_int originy,
		tjs_uint blackboxw, tjs_uint blackboxh,
		const tGlyphMetrics & metrics,
		bool fullcolor = false );
	// コピー禁止
	tTVPCharacterData(const tTVPCharacterData & ref) = delete;
	tTVPCharacterData& operator=(const tTVPCharacterData&) = delete;

	// move
	tTVPCharacterData( tTVPCharacterData&& ref ) noexcept { *this = std::move(ref); }
	tTVPCharacterData& operator=(tTVPCharacterData&& rhs) noexcept {
		Data = rhs.Data;
		RefCount = std::move( rhs.RefCount );

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

	~tTVPCharacterData() { if(Data) delete [] Data; }

	void Alloc(tjs_int size) {
		if(Data) delete [] Data, Data = NULL;
		Data = new tjs_uint8[size];
	}

	static inline tjs_int CalcAlignSize( tjs_int len ) {
		return ( ( ( ( len - 1 ) >> 3 ) + 1 ) << 3 );
	}

	tjs_uint8 * GetData() const { return Data; }

	void AddRef() { RefCount ++; }
	void Release() {
		if(RefCount == 1) {
			delete this;
		} else {
			RefCount--;
		}
	}

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
namespace tvp {
	struct FontAndCharacterData;
};
struct tTVPFontAndCharacterData
{
	tTVPFont Font;
	tjs_uint32 FontHash = -1;
	tjs_char Character = TJS_W(' ');
	tjs_int BlurLevel = 0;
	tjs_int BlurWidth = 0;
	bool Antialiased = true;
	bool Blured = false;
	bool Hinting = true;

	tTVPFontAndCharacterData() {}
	tTVPFontAndCharacterData(const struct tvp::FontAndCharacterData& ref);
	tTVPFontAndCharacterData(struct tvp::FontAndCharacterData&& ref ) noexcept;
	tTVPFontAndCharacterData& operator=(struct tvp::FontAndCharacterData&& rhs) noexcept;

	bool operator == (const tTVPFontAndCharacterData& rhs) const {
		return Character == rhs.Character && Font == rhs.Font &&
			Antialiased == rhs.Antialiased && BlurLevel == rhs.BlurLevel &&
			BlurWidth == rhs.BlurWidth && Blured == rhs.Blured &&
			Hinting == rhs.Hinting;
	}
};
//---------------------------------------------------------------------------
class tTVPFontHashFunc
{
public:
	static tjs_uint32 Make(const tTVPFontAndCharacterData &val)
	{
		tjs_uint32 v = val.FontHash;

		v ^= val.Antialiased?1:0;
		v ^= val.Character;
		v ^= val.Blured?1:0;
		v ^= val.BlurLevel ^ val.BlurWidth;
		return v;
	}
};





#endif // __CHARACTER_DATA_H__
