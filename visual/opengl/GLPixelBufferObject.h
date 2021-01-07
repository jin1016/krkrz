/*
 * PBO
 * Pixel Buffer Objectについて
 * http://www.slis.tsukuba.ac.jp/~fujisawa.makoto.fu/cgi-bin/wiki/index.php?OpenGL%20-%20PBO
 * OpenGLの使い方:バッファ編
 * http://qiita.com/sugawara/items/b2bbaecc50a0e111ad08
 *
 * Android/iOS/OpenGL/Shader/Nexus/Kindle Version 一覧他
 * http://dench.flatlib.jp/start
 * OpenGL ES 3.0 Reference
 * https://www.khronos.org/registry/OpenGL-Refpages/es3.0/
 *
 *
 * Texture Layer Tree Owner用にES3.0はPBO, 2.0はメモリに一時コピーからの転送の2パターンが必要。
 * インターフェイス作って、継承でどちらが使われるか決める。
 * PBO の場合はダブルバッファリングして転送時間を少なくしたい。
 * 2.0のメモリコピー版はどちらでもいいが……
 * Texture が描画に使われいるかどうかの情報は必要だから、Canvasのswap通知は必要になる。
 *
 * いつ転送するかはユーザーマターにするか？
 * Dirty Rectを保持し、ユーザーのupdateで転送する。
 * 使用中テクスチャの更新は出来ない → ダブルバッファリングする必要がある。
 * ダブルバッファリングするとDirty Rect保持できないというか、2フレーム分のDirty Rectになる。
 *
 * 構造上、テキストの更新(ADVの1文字ずつ表示のような)に、Layer Treeを使った更新はあまり適さない。
 * TextureLayerTreeOwnerは、いったん取りやめ、BitmapLayerTreeOwner に、Dirty Rect管理を追加する。
 * BitmapLayerTreeOwner の Bitmap と Dirty Rect で Texture にスクリプトで部分転送して、使用すのが
 * タイミング的にブロッキングしてしまうかもしれないけれど、実装が早い。
 * 上述のBitmapLTOで処理時間上の問題が発生する可能性を考えると、ハードウェアを用いた文字描画も実装して
 * おいた方が得策。
 * TextureLayerTreeOwnerについては、ES3.0でPBOを使う＆テクスチャ全体更新前提で、将来実装とするのが無難。
 */

/**
 * フォントは PBO か Bitmap に一時的に書き出して処理することを考えていたけど、PBO はシリアルなメモリだから、malloc 的な処理が使える
 * malloc で確保したテクスチャメモリからバーテックスシェーダーを使って描画すれば、矩形確保処理が不要になる。
 * ES 2.0 の時は、そのような処理が使えないけど、
 */

/**
 * OpenGL ES 2.0 では Bitmap で対応
 * OpenGL ES 3.0 では PBO で対応
 * tTVPTexture クラスでどちらをワークとして使用するか決める
 * 読み込み処理は std::async で常に非同期読み込みさせる
 * Copy on write で転送する時 OpenGL ES 3.0 では PBO 間を glCopyBufferSubData で転送できる
 */
#ifndef __GL_PIXEL_BUFFER_OBJECT_H__
#define __GL_PIXEL_BUFFER_OBJECT_H__

class GLPixelBufferObject {
	GLuint pbo_;
	tjs_int width_;
	tjs_int height_;
	tjs_int bpp_;

public:
	/**
	 * @param w 
	 * @param h
	 * @param bpp 1pixel 辺りのバイト数
	 * @param src 初期読み込み画像データ、null の時は未初期化
	 */
	GLPixelBufferObject( tjs_int w, tjs_int h, tjs_int bpp = 4, const void* src = nullptr );

	// コピーは不許可、ムーブは許可
	GLPixelBufferObject( const GLPixelBufferObject& ref ) = delete;
	GLPixelBufferObject( GLPixelBufferObject&& ref );

	~GLPixelBufferObject();

	// コピーは不許可、ムーブは許可
	GLPixelBufferObject& operator=(GLPixelBufferObject&& rhs);
	GLPixelBufferObject& operator=(const GLPixelBufferObject& rhs) = delete;

	/**
	 * @return PBO ID
	 */
	GLuint GetPBO() const { return pbo_; }
	/**
	 * @return width
	 */
	tjs_int GetWidth() const { return width_; }
	/**
	 * @return height
	 */
	tjs_int GetHeight() const { return height_; }
	/**
	 * @return byte per pixel
	 */
	tjs_int GetBPP() const { return bpp_; }
	/**
	 * メモリから PBO へデータ転送する
	 */
	void CopyFromMemory( const void* src );

	/*
	 * PBO をロックして、指定位置のポインタを取得する
	 * @param type 読み書き等の指定
	 * @param offset ポインタオフセット
	 * @param length バイト長さ
	 */
	void* LockBuffer(GLbitfield type, GLintptr offset, GLsizeiptr length);

	/**
	 * PBO のロックを解除する
	 * ロック中に他のPBOへのアクセスはしないこと
	 */
	void UnlockBuffer();

	/**
	 * PBO からテクスチャへデータ転送する
	 */
	void CopyToTexture( GLuint tex );

	/**
	 * PBO からテクスチャへデータ転送する(初回転送)
	 */
	void CopyToTexture( GLuint tex, GLint format );

	/**
	 * フレームバッファから PBO へデータ転送する
	 */
	bool CopyFromFramebuffer( GLsizei width, GLsizei height, bool front );
};
#endif // __GL_PIXEL_BUFFER_OBJECT_H__