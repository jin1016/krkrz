
#include "tjsCommHead.h"
#include <memory>

#include "DebugIntf.h"
#include "TVPBitmapLock.h"

#include "tvpgl.h"
#include "BitmapIntf.h"
#include "LayerBitmapIntf.h"

#include "OpenGLScreen.h"
#include "GLTexture.h"
#include "GLPixelBufferObject.h"
#include "GLVertexBufferObject.h"
#include "TVPTextureBridgeMemory.h"
#include "TVPBitmapLock.h"

#include "TVPTexture.h"


tTVPTexture::tTVPTexture(tjs_uint width, tjs_uint height, GLint format, const GLvoid* bits )
: TextureBuffer( width, height, format == GL_RGBA ? 4 : 1, bits ), TextureFormat(format) {
	Texture = std::make_shared<GLTexture>( TextureBuffer, format );
}

tTVPTexture::tTVPTexture(tjs_uint width, tjs_uint height, enum class tvp::COLOR_FORMAT format, bool unpadding )
: TextureBuffer( width, height, format == tvp::COLOR_FORMAT::RGBA ? 4 : 1, nullptr ), TextureFormat(static_cast<GLint>(format)) {
	Texture = std::make_shared<GLTexture>( TextureBuffer, static_cast<GLint>(format) );
}
tTVPTexture::~tTVPTexture() {
}


/**
 * テクスチャ全体を表す頂点データのVBOをハンドルを返す。
 * @return VBO ID、0の時VBOがない
 */
tjs_int64 tTVPTexture::GetVBOHandle() const {
	if( VertexBuffer.isCreated() ) {
		return VertexBuffer.id();
	} else {
		const float w = (float)Texture->width();
		const float h = (float)Texture->height();
		const GLfloat vertices[] = {
			0.0f, 0.0f,	// 左上
			0.0f,    h,	// 左下
			   w, 0.0f,	// 右上
			   w,    h,	// 右下
		};
		GLVertexBufferObject& vbo = const_cast<GLVertexBufferObject&>( VertexBuffer );
		vbo.createStaticVertex( vertices, sizeof(vertices) );
		return VertexBuffer.id();
	}
}
void tTVPTexture::SetDrawSize(tjs_uint width, tjs_uint height) {
	const float w = (float)width;
	const float h = (float)height;
	const GLfloat vertices[] = {
		0.0f, 0.0f,	// 左上
		0.0f,    h,	// 左下
		   w, 0.0f,	// 右上
		   w,    h,	// 右下
	};
	GLVertexBufferObject& vbo = const_cast<GLVertexBufferObject&>( VertexBuffer );
	if (VertexBuffer.isCreated()) {
		vbo.copyBuffer(0, sizeof(vertices), vertices);
	}
	else {
		vbo.createStaticVertex(vertices, sizeof(vertices));
	}
}
/**
 * 1列のバイト数を取得
 */
tjs_uint tTVPTexture::GetLineBytes() const {
	tjs_int format = Texture->format();
	switch( format ) {
	case GL_ALPHA:
		return Texture->width();
	case GL_RGBA:
	default:
		return 4 * Texture->width();
	}
}

/**
 * 画像の生データへのポインタを取得する
 * 使用後 UnlockBits をコールすること
 * @return 画像データ実体へのポイント
 */
//void* tTVPTexture::LockBits( tTVPBitmapLockType type = tTVPBitmapLockType::WRITE_ONLY, tTVPRect* area = nullptr ) {
void* tTVPTexture::LockBits( BitmapLockType type, tjs_offset offset, tjs_size length ) {
	return TextureBuffer.LockBuffer(type, offset, length);
}
void* tTVPTexture::LockBits( BitmapLockType type, tTVPRect* area ) {
	return TextureBuffer.LockBuffer(type, area);
}
/**
 * 画像データアクセス終了時にロックを解除する
 */
void tTVPTexture::UnlockBits() {
	TextureBuffer.UnlockBuffer();
}

tjs_uint tTVPTexture::GetWidth() const { return TextureBuffer.GetWidth(); }
tjs_uint tTVPTexture::GetHeight() const { return TextureBuffer.GetHeight(); }

std::shared_ptr<tTVPTexture> tTVPTexture::clone() {
	std::shared_ptr<tTVPTexture> ret = std::make_shared<tTVPTexture>( GetWidth(), GetHeight(), static_cast<GLint>(GetImageFormat()), static_cast<const GLvoid*>(nullptr) );
	return ret;
}

	