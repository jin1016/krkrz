/**
 * Texture と CPU 間の画像転送を OpenGL ES2.0 と ES3.0 で抽象化し、PBO が使えない環境と使える環境で違和感なく操作できるようにする。
 * PBO が使える環境では、DMA 転送されるので CPU に余裕が出来る。
 * PBO が使えない環境では 従来の Bitmap クラスが用いられる。
 */

#ifndef __TVP_TEXTURE_BRIDGE_MEMORY_H__
#define __TVP_TEXTURE_BRIDGE_MEMORY_H__

#include "TVPBitmapLock.h"
#include "TVPReleaseDeleter.h"

class TVPTextureBridgeMemory {
	using BitmapLockType = typename tvp::bitmap::LockType;

	// OpenGL ES 3.0 で使用、DMA で CPU GPU 間の画像転送を効率化する
	std::unique_ptr<GLPixelBufferObject> PixelBuffer;

	// OpenGL ES 2.0の時はテンポラリ画像データとして使用される
	std::unique_ptr<tTVPBitmap,tvp::release_deleter<tTVPBitmap>> Bitmap = nullptr;

	BitmapLockType		LockType = BitmapLockType::WRITE_ONLY;
	tjs_offset			LockOffset = 0;
	tjs_size			LockLength = 0;
	tTVPRect			DirtyRect { -1, -1, -1, -1 };

	tjs_int Width;	// 幅
	tjs_int Height;	// 高さ
	tjs_int BPP;	// byte per pixel

	static GLbitfield GetRangeLockFlag(BitmapLockType type );
public:
	/**
	 * @param w witdh
	 * @param h height
	 * @param bpp byte per pixel
	 * @param src 初期読み込み画像データ、null の時は未初期化
	 */
	TVPTextureBridgeMemory( tjs_int w, tjs_int h, tjs_int bpp = 4, const void* src = nullptr );

	// コピーは不許可、ムーブは許可
	TVPTextureBridgeMemory( const TVPTextureBridgeMemory& ref ) = delete;
	TVPTextureBridgeMemory( TVPTextureBridgeMemory&& ref ) noexcept;

	~TVPTextureBridgeMemory();

	// コピーは不許可、ムーブは許可
	TVPTextureBridgeMemory& operator=(TVPTextureBridgeMemory&& rhs) noexcept;
	TVPTextureBridgeMemory& operator=(const TVPTextureBridgeMemory& rhs) = delete;

	bool HasImage() const;

	// Bitmap遅延生成のためのメソッド
	void CheckAndCreateBitmap();

	/**
	 * @return width
	 */
	tjs_int GetWidth() const { return Width; }

	/**
	 * @return height
	 */
	tjs_int GetHeight() const { return Height; }

	/**
	 * @return byte per pixel
	 */
	tjs_int GetBPP() const { return BPP; }

	/**
	 * メモリから work へデータ転送する
	 */
	void CopyFromMemory( const void* src );

	/*
	 * work をロックして、指定位置のポインタを取得する
	 * @param type 読み書き等の指定
	 * @param offset ポインタオフセット
	 * @param length バイト長さ
	 */
	void* LockBuffer(BitmapLockType type, tjs_offset offset, tjs_size length);

	void* LockBuffer(BitmapLockType type = BitmapLockType::WRITE_ONLY, tTVPRect* area = nullptr);

	/**
	 * work のロックを解除する
	 * ロック中に他の work へのアクセスはしないこと
	 */
	void UnlockBuffer();

	/**
	 * work からテクスチャへデータ転送する
	 */
	void CopyToTexture( GLuint tex );

	/**
	 * work からテクスチャへデータ転送する(初回のみ)
	 */
	void CopyToTexture( GLuint tex, GLint format );

	/**
	 * フレームバッファから work へデータ転送する
	 */
	bool CopyFromFramebuffer( GLsizei width, GLsizei height, bool front );

	/**
	 * TextureBridgeMemoryをコピーする (どちらかがLock中にコピーを呼び出した場合の動作は現状不定)
	 */
	void CopyFrom( const TVPTextureBridgeMemory& src );
};

#endif
