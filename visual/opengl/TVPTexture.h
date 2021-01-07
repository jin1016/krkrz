/**
 * テクスチャを管理するクラス
 * OpenGLのテクスチャを直接的に管理するのはGLTextureクラスで、本クラスはテクスチャをより扱いやすくするために存在する
 * テクスチャ描画のための VBO 管理や画像転送のための PBO、ES 2.0 ではメモリ内画像データを保持する。
 * メモリ内画像は現在テクスチャが変更されない前提で作られているため、フレームバッファからテクスチャ直接転送する場合など
 * テクスチャとメモリ内画像で不整合が起きる可能性がある。
 * 将来、変更に追従するよう対応する可能性もある(テクスチャとメモリ感で転送のオーバーヘッドが発生するのであまり好ましくないため、
 * テクスチャからのコピーは明示的に行う実装とする可能性もある)。
 * フレームバッファからメモリ内画像へ転送後、テクスチャへ転送する間接的転送を実装するかもしれない。
 */
#ifndef __TVP_TEXTURE_H__
#define __TVP_TEXTURE_H__

#include "TextureInfo.h"

class tTVPTexture : public iTVPTextureInfoIntrface {
	std::shared_ptr<GLTexture> Texture;	// OpenGL ES テクスチャを表すクラス
	GLVertexBufferObject VertexBuffer;	// テクスチャを2D描画するために使うVertex Buffer
	GLint TextureFormat;				// OpenGL ES テクスチャのフォーマット

	// Texture に画像転送したり、フレームバッファから取り込んだりするためのメモリ (PBO or Bitmap)
	TVPTextureBridgeMemory	TextureBuffer;

public:
	tTVPTexture(tjs_uint width, tjs_uint height, GLint format=GL_RGBA, const GLvoid* bits=nullptr );

	// コピーコンストラクタについては後で考える
	// コピーは copy on write で実装できると良いか？
	// tTVPTexture( const tTVPTexture& r ) : Texture(r.Texture) {}

	~tTVPTexture();

	/**
	 * ネイティブハンドルを取得。OpenGL ES2/3実装ではテクスチャID
	 * @return ネイティブハンドル
	 */
	tjs_int64 GetNativeHandle() const { return static_cast<tjs_int64>( Texture->id() ); }

	/**
	 * テクスチャ全体を表す頂点データのVBOをハンドルを返す。
	 * @return VBO ID、0の時VBOがない
	 */
	tjs_int64 GetVBOHandle() const;

	void SetDrawSize( tjs_uint width, tjs_uint height );

	/**
	 * テクスチャフォーマットを取得
	 * @return テクスチャフォーマット
	 */
	tjs_int GetImageFormat() const { return TextureFormat; } // Texture->format(); }

	/**
	 * 1列のストライド(バイト数)を取得
	 * 負の値の時は上下反転している(最新環境ではOpenGL用に正順格納されている)
	 */
	tjs_int GetStride() const {
		return GetLineBytes();
	}

	/**
	 * 1列のバイト数を取得
	 */
	tjs_uint GetLineBytes() const;

	/**
	 * 画像の生データへのポインタを取得する
	 * 使用後 UnlockBits をコールすること
	 * @return 画像データ実体へのポイント
	 */
	//virtual void* LockBits( tTVPBitmapLockType type = tTVPBitmapLockType::WRITE_ONLY, tTVPRect* area = nullptr ) = 0;
	void* LockBits(tTVPBitmapLockType type, tjs_offset offset, tjs_size length);

	void* LockBits(tTVPBitmapLockType type = tTVPBitmapLockType::WRITE_ONLY, tTVPRect* area = nullptr);

	/**
	 * 画像データアクセス終了時にロックを解除する
	 */
	void UnlockBits();

	// 再生成可能なテクスチャどうか、フレームバッファの時は失敗する
	bool CanItRecreateTexture() const { return true; }

	tjs_uint GetWidth() const;
	tjs_uint GetHeight() const;

	GLint GetTextureFormat() const { return TextureFormat; }
	tjs_int64 GetTextureId() const { return Texture->id(); }
	void SetTextureId(tjs_int64 id) { return Texture->set_id(static_cast<GLuint>(id)); }
	
	void createMipmapTexture(std::vector<GLTextreImageSet>& img) {
		Texture->createMipmapTexture( img );
	}

	GLenum GetStretchType() const { return Texture->stretchType(); }
	void SetStretchType(GLenum s) { Texture->setStretchType( s ); }
	
	GLenum GetWrapModeHorizontal() const { return Texture->wrapS(); }
	void SetWrapModeHorizontal(GLenum v) { Texture->setWrapS( v ); }
	GLenum GetWrapModeVertical() const { return Texture->wrapT(); }
	void SetWrapModeVertical(GLenum v) { Texture->setWrapT( v ); }
};

#endif
