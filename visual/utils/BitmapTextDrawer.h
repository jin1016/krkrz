/**
 * 指定メモリ領域 ( LockInterface を実装した Bitmapクラス ) にテキストを描画する
 */
#ifndef __TVP_BITMAP_TEXT_DRAWER_H__
#define __TVP_BITMAP_TEXT_DRAWER_H__

#include "LayerBitmapIntf.h" // enum 等だけ別ヘッダーに分離した方が良い。plugin用にも公開されているから、perl 側の修正も必要。
#include "tvpfontstruc.h"
#include "ComplexRect.h"
#include "TVPBitmapLock.h"


namespace tvp {
namespace bitmap {


/**
 * テキスト描画オプション
 */
struct DrawTextOptions {
	LockInterface* destbmp;	// 描画先 Bitmap
	tjs_int bmppitch;		// 描画先画像ピッチ(横バイトサイズ)
	tTVPRect rect;			// 描画先矩形領域
	tjs_int x;				// 描画先位置 X
	tjs_int y;				// 描画先位置 Y
	const ttstr &text;		// 描画テキスト
	tjs_uint32 color;		// 文字色
	tTVPBBBltMethod bltmode = bmAlphaOnAlpha;	// 描画モード
	tjs_int opa = 255;			// 透明度
	bool holdalpha = true;		// アルファ維持
	bool aa = true;				// アンチエイリアシング
	tjs_int shlevel = 0;		// 影レベル
	tjs_uint32 shadowcolor = 0;	// 影色
	tjs_int shwidth = 0;		// 影の幅
	tjs_int shofsx = 0;			// 影の X 方向オフセット
	tjs_int shofsy = 0;			// 影の Y 方向オフセット
	tTVPComplexRect *updaterects = nullptr;	// テキスト描画領域取得先(必要な場合ポインタを渡す)
};

/**
 * 指定されたメモリ領域にテキストを描画するクラス
 * プリレンダーフォントは非サポート( FreeType ttf などが読めるので、bitmap フォントを作れば対応可能、将来はツール充実してそちら方向を検討 )
 */
class TextDrawer {
	tTVPFont Font;		// フォント
	bool FontChanged = true;	// フォントが変更されたかどうか
	tjs_int GlobalFontState = -1;	// 全体でフォント情報が更新されたかどうかをチェックするための変数

	tjs_int AscentOfsX = 0;		// テキスト送り量 X
	tjs_int AscentOfsY = 0;		// テキスト送り量 Y
	double RadianAngle = 0.0;		// テキスト送り方向ラジアン角度
	//tjs_uint32 FontHash;	// フォントハッシュ(変更チェック用)

	tjs_int TextWidth = 0;		// テキスト幅キャッシュ
	tjs_int TextHeight = 0;		// テキスト高さキャッシュ
	ttstr CachedText;		// 幅と高さがキャッシュされている文字列(変更比較用)

	tjs_uint32 BitPerPixel = 32;	// bit per pixe

private:
	/**
	 * このクラスにフォントを適用する。もしくは、このクラスのフォントをシステムに適用する。
	 * フォントを使用して描画する際に呼ばれる
	 */
	void ApplyFont();

	/**
	 * テキストの幅と高さを取得し、キャッシュする。
	 * GetTextWidth と GetTextHeight は連続で呼び出される可能性が高いので、両方を取得し保持しておく。
	 * またテキスト送り量もこれらの値を元に算出される。
	 */
	void GetTextSize(const ttstr & text);

	/**
	 * 内部テキスト描画関数
	 */
	bool InternalDrawText( tjs_uint8* sl, const class CharacterData* data, tjs_int x, tjs_int y, tjs_uint32 color, const DrawTextOptions& option, tTVPRect& drect );

	/**
	 * 描画領域矩形サイズを計算する
	 */
	bool CalculateDrawRect(const class CharacterData* data, tjs_int x, tjs_int y, const DrawTextOptions& option, tTVPRect& srect, tTVPRect& drect );

public:
	/**
	 * constructor
	 */
	TextDrawer();

	/**
	 * フォントを設定する
	 */
	void SetFont(const struct tTVPFont &font);
	/**
	 * 設定されているフォントを取得する
	 */
	const struct tTVPFont & GetFont() const { return Font; };

	/**
	 * 使用可能なシステム、読み込み済みフォントを取得する
	 */
	void GetFontList(tjs_uint32 flags, std::vector<ttstr>& list);

	/**
	 * 1文字描画
	 */
	void DrawTextSingle( DrawTextOptions& option );

	/**
	 * 複数文字描画
	 */
	void DrawTextMultiple( DrawTextOptions& option );

	/**
	 * 文字描画
	 */
	void DrawText( DrawTextOptions& option ) {
		tjs_int len = option.text.GetLen();
		if(len == 0) return;
		if(len >= 2)
			DrawTextMultiple( option );
		else    /* if len == 1 */
			DrawTextSingle( option );
	}

public:
	/**
	 * 描画テキストの幅を取得する
	 */
	tjs_int GetTextWidth(const ttstr & text);
	/**
	 * 描画テキストの高さを取得する
	 */
	tjs_int GetTextHeight(const ttstr & text);
	/**
	 * テキスト幅送り量 X
	 */
	double GetEscWidthX(const ttstr & text);
	/**
	 * テキスト幅送り量 Y
	 */
	double GetEscWidthY(const ttstr & text);
	/**
	 * テキスト高さ送り量 X
	 */
	double GetEscHeightX(const ttstr & text);
	/**
	 * テキスト高さ送り量 Y
	 */
	double GetEscHeightY(const ttstr & text);
	/**
	 * テキストが実際に描画される矩形領域を取得する
	 */
	void GetFontGlyphDrawRect( const ttstr & text, struct tTVPRect& area );
};

}	// bitmap
}	// tvp

#endif // __TVP_BITMAP_TEXT_DRAWER_H__
