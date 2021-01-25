//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Bitmap interface
//---------------------------------------------------------------------------
#ifndef _TVP_Bitmap_H__
#define _TVP_Bitmap_H__

#include "tvpfontstruc.h"
#include "ComplexRect.h"

#include "BitmapInfomation.h"

/**
 * Lock 処理タイプ ( 実質的にOpenGL ES2.0 の時のみ意味がある )
 */
enum class tTVPBitmapLockType : tjs_int {
	WRITE_ONLY = 0,	// 書き込みのみなので、Lock 時に値のコピーは行わない
	READ_ONLY = 1,	// 読み込みのみなので、Lock 時に値のコピーを行う、Unlock 時は何もせずメモリ開放
	READ_WRITE = 2,	// 読み書きなので、Loack 時のコピーと Unlock 時の書き込みを行う
};


// tTVPBitmap のインターフェイスを別途定義し、Texture クラスと Bitmap クラスでインターフェイスを共通化
// フォント描画と copy on write を両クラスで実現できる構造にする
class iTVPBitmap
{
protected:
	tjs_int RefCount = 1;

	// DIB information
	// std::uniq_ptr<BitmapInfomation> BitmapInfo;

public:
	iTVPBitmap() {}

	void AddRef(void) {
		RefCount ++;
	}

	void Release(void) {
		if(RefCount == 1)
			delete this;
		else
			RefCount--;
	}

	bool IsIndependent() const { return RefCount == 1; }

	/**
	 * 幅を取得
	 * @return 画像幅
	 */
	virtual tjs_uint GetWidth() const = 0;

	/**
	 * 高さを取得
	 * @return 画像高さ
	 */
	virtual tjs_uint GetHeight() const = 0;

	/**
	 * 1pixel当たりのビット数を取得
	 * @return 1pixel当たりのビット数
	 */
	virtual tjs_uint GetBPP() const = 0;

	/**
	 * 32bit画像形式かどうか
	 */
	virtual bool Is32bit() const = 0;

	/**
	 * 8bit画像形式かどうか
	 */
	virtual bool Is8bit() const = 0;

	/**
	 * 1列のストライド(バイト数)を取得
	 * 負の値の時は上下反転している(最新環境ではOpenGLように正順格納されている)
	 */
	virtual tjs_int GetStride() const = 0;

	/**
	 * 1列のバイト数を取得
	 */
	virtual tjs_uint GetLineBytes() const = 0;

	/**
	 * 画像の生データへのポインタを取得する(バイトオフセット版)
	 * 使用後 UnlockBits をコールすること
	 * @return 画像データ実体へのポイント
	 */
	virtual void* LockBits( tTVPBitmapLockType type, tjs_offset offset, tjs_size length ) = 0;

	/**
	* 画像の生データへのポインタを取得する(矩形指定版)
	* 使用後 UnlockBits をコールすること
	* 指定行以降全てが更新されたとみなされる
	* @param type ロック方法指定
	* @param area ロック範囲矩形
	*/
	virtual void* LockBits(tTVPBitmapLockType type = tTVPBitmapLockType::WRITE_ONLY, tTVPRect* area = nullptr) = 0;

	/**
	 * 画像データアクセス終了時にロックを解除する
	 */
	virtual void UnlockBits() = 0;

	// 指定行の画像データ列へのポインタを取得する
	virtual void* GetScanLine(tjs_uint l) const = 0;

	virtual const tjs_uint* GetPalette() const = 0;
	virtual tjs_uint* GetPalette() = 0;
	virtual tjs_uint GetPaletteCount() const = 0;
	virtual void SetPaletteCount( tjs_uint count ) = 0;

	// unlock が不要かどうか。
	// true の時、unlock が不要なので、自由にスキャンライン取得が可能
	virtual bool IsNoNeedToUnlock() const = 0;

	// メモリ上に確保されたBitmapかどうか
	virtual bool IsMemoryBitmap() const { return true; }

	// 再生成可能なテクスチャどうか、フレームバッファの時は失敗する
	virtual bool CanItRecreateTexture() const { return false; }
};

// LockBits でポインタ取得後、このクラスでロックを自動解除させる
class tTVPBitmapAutoLock {
	iTVPBitmap* Bitmap;
public:
	tTVPBitmapAutoLock(iTVPBitmap* bmp) : Bitmap(bmp) {}
	~tTVPBitmapAutoLock() {
		Bitmap->UnlockBits();
	}
};

//---------------------------------------------------------------------------
// tTVPBitmap : internal bitmap object
//---------------------------------------------------------------------------
class tTVPBitmap : public iTVPBitmap
{
public:
	static const tjs_int DEFAULT_PALETTE_COUNT = 256;

private:
	void * Bits; // pointer to bitmap bits
	BitmapInfomation *BitmapInfo; // DIB information

	tjs_uint PitchBytes; // bytes required in a line
	tjs_int PitchStep; // step bytes to next(below) line
	tjs_int Width; // actual width
	tjs_int Height; // actual height

	tjs_int ActualPalCount;
	tjs_uint* Palette;

public:
	tTVPBitmap(tjs_uint width, tjs_uint height, tjs_uint bpp, bool unpadding=false);
	// for async load
	// @param bits : tTVPBitmapBitsAlloc::Allocで確保したものを使用すること
	tTVPBitmap(tjs_uint width, tjs_uint height, tjs_uint bpp, void* bits);

	tTVPBitmap(const tTVPBitmap & r);

	~tTVPBitmap();

	void Allocate(tjs_uint width, tjs_uint height, tjs_uint bpp, bool unpadding=false);

	tjs_uint GetWidth() const { return Width; }
	tjs_uint GetHeight() const { return Height; }

	tjs_uint GetBPP() const { return BitmapInfo->GetBPP(); }
	bool Is32bit() const { return BitmapInfo->Is32bit(); }
	bool Is8bit() const { return BitmapInfo->Is8bit(); }

	// 次の行までのストライド(負数の可能性あり)
	tjs_int GetStride() const { return PitchStep; }

	// 1行当たりのバイト数
	tjs_uint GetLineBytes() const { return PitchBytes; }

	const BitmapInfomation* GetBitmapInfomation() const { return BitmapInfo; }

#ifdef _WIN32
	const BITMAPINFO * GetBITMAPINFO() const { return BitmapInfo->GetBITMAPINFO(); }
	const BITMAPINFOHEADER * GetBITMAPINFOHEADER() const { return (const BITMAPINFOHEADER*)( BitmapInfo->GetBITMAPINFO() ); }
#endif

	const void * GetBits() const { return Bits; }

	void* LockBits(tTVPBitmapLockType type, tjs_offset offset, tjs_size length) {
		return static_cast<tjs_uint8*>(Bits) + offset;
	}
	void* LockBits(tTVPBitmapLockType type = tTVPBitmapLockType::WRITE_ONLY, tTVPRect* area = nullptr);
	void* GetScanLine(tjs_uint l) const;
	// unlock は必要ない
	void UnlockBits() {}
	// unlock は必要ない ので true
	bool IsNoNeedToUnlock() const { return true; }

	const tjs_uint* GetPalette() const { return Palette; };
	tjs_uint* GetPalette() { return Palette; };
	tjs_uint GetPaletteCount() const { return ActualPalCount; };
	void SetPaletteCount( tjs_uint count );

};
//---------------------------------------------------------------------------

#endif
