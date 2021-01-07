
#ifndef TextureInfoH
#define TextureInfoH

#include "TVPBitmap.h"

class iTVPTextureInfoIntrface : public iTVPBitmap {
public:
	/**
	 * ネイティブハンドルを取得。OpenGL ES2/3実装ではテクスチャID
	 * @return ネイティブハンドル
	 */
	virtual tjs_int64 GetNativeHandle() const = 0;

	/**
	 * テクスチャ全体を表す頂点データのVBOをハンドルを返す。
	 * @return VBO ID、0の時VBOがない
	 */
	virtual tjs_int64 GetVBOHandle() const = 0;

	/**
	 * テクスチャフォーマットを取得
	 * @return テクスチャフォーマット
	 */
	virtual tjs_int GetImageFormat() const = 0;

	tjs_uint GetBPP() const;

	bool Is32bit() const;

	bool Is8bit() const;

	// パレットはサポートしない
	const tjs_uint* GetPalette() const { return nullptr; };
	tjs_uint* GetPalette() { return nullptr; };
	tjs_uint GetPaletteCount() const { return 0; };
	void SetPaletteCount( tjs_uint count ) {}

	void* GetScanLine(tjs_uint l) const;

	// unlock が不要かどうか。
	// true の時、unlock が不要なので、自由にスキャンライン取得が可能
	bool IsNoNeedToUnlock() const { return false; }

	// メモリ上に確保されたBitmapかどうか
	bool IsMemoryBitmap() const { return false; }
};

#endif
