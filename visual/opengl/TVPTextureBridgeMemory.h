#ifndef __TVP_TEXTURE_BRIDGE_MEMORY_H__
#define __TVP_TEXTURE_BRIDGE_MEMORY_H__

/**
 * Texture と CPU 間の画像転送を OpenGL ES2.0 と ES3.0 で抽象化し、PBO が使えない環境と使える環境で違和感なく操作できるようにする
 * PBO が使えたら、DMA 転送されるので CPU に余裕が出来る。
 */
class TVPTextureBridgeMemory {
	std::unique_ptr<GLPixelBufferObject> PixelBuffer;	// OpenGL ES 3.0 で使用、DMA で CPU GPU 間の画像転送を効率化する
	tTVPBitmap* Bitmap = nullptr;		// ファイル読み込み時とOpenGL ES 2.0の時はテンポラリ画像データとして使用される

	tTVPBitmapLockType	LockType = tTVPBitmapLockType::WRITE_ONLY;
	tjs_offset			LockOffset = 0;
	tjs_size			LockLength = 0;
	tTVPRect			DirtyRect { -1, -1, -1, -1 };

public:
	/**
	 * @param w 
	 * @param h
	 * @param bpp 1pixel 辺りのバイト数
	 * @param src 初期読み込み画像データ、null の時は未初期化
	 */
	TVPTextureBridgeMemory( tjs_int w, tjs_int h, tjs_int bpp = 4, const void* src = nullptr );

	// コピーは不許可、ムーブは許可
	TVPTextureBridgeMemory( const TVPTextureBridgeMemory& ref ) = delete;
	TVPTextureBridgeMemory( TVPTextureBridgeMemory&& ref );

	~TVPTextureBridgeMemory();

	// コピーは不許可、ムーブは許可
	TVPTextureBridgeMemory& operator=(TVPTextureBridgeMemory&& rhs);
	TVPTextureBridgeMemory& operator=(const TVPTextureBridgeMemory& rhs) = delete;

	/**
	 * @return width
	 */
	tjs_int GetWidth() const;

	/**
	 * @return height
	 */
	tjs_int GetHeight() const;

	/**
	 * @return byte per pixel
	 */
	tjs_int GetBPP() const;

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
	void* LockBuffer(tTVPBitmapLockType type, tjs_offset offset, tjs_size length);

	void* LockBuffer(tTVPBitmapLockType type = tTVPBitmapLockType::WRITE_ONLY, tTVPRect* area = nullptr);

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
};

#endif
