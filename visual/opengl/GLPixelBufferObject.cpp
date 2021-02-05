
#include "tjsCommHead.h"
#include "DebugIntf.h"
#include "OpenGLScreen.h"
#include "BitmapIntf.h"
#include "LayerBitmapIntf.h"
#include "tvpgl.h"
#include <memory>

#include "GLPixelBufferObject.h"



/**
 * @param w 
 * @param h
 * @param bpp 1pixel 辺りのバイト数
 * @param src 初期読み込み画像データ、null の時は未初期化
 */
GLPixelBufferObject::GLPixelBufferObject( tjs_int w, tjs_int h, tjs_int bpp, const void* src )
: pbo_(0), width_(w), height_(h), bpp_(bpp), copied_(false) {
	glGenBuffers( 1, &pbo_ );
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_ );

	// バッファの作成と初期化, srcが非nullならデータコピー
	GLsizeiptr length = w * h * bpp;
	glBufferData(GL_PIXEL_UNPACK_BUFFER, length, 0, GL_DYNAMIC_DRAW );
	if( src ) {
		unsigned char* ptr = static_cast<unsigned char*>( glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, length, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT) );
		if( ptr ) {
			memcpy(ptr, src, length);
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
			copied_ = true;
		} else {
			tTVPOpenGLScreen::CheckGLErrorAndLog( TJS_W("GLPixelBufferObject::GLPixelBufferObject") );
		}
	} else {
		copied_ = false;
	}
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0 );
}
GLPixelBufferObject::GLPixelBufferObject( GLPixelBufferObject&& ref ) noexcept {
	*this = std::move( ref );
}


GLPixelBufferObject::~GLPixelBufferObject() {
    glBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );
	glBindBuffer( GL_ARRAY_BUFFER, pbo_ );
	glDeleteBuffers( 1, &pbo_ );
	pbo_ = 0;
}

GLPixelBufferObject& GLPixelBufferObject::operator=(GLPixelBufferObject&& rhs) noexcept {
	pbo_ = rhs.pbo_;
	width_ = rhs.width_;
	height_ = rhs.height_;
	bpp_ = rhs.bpp_;
	copied_ = rhs.copied_;
	return *this;
}


/*
 * メモリから PBO へデータ転送する
 */
void GLPixelBufferObject::CopyFromMemory( const void* src ) {
	if( src == nullptr ) return;

    GLenum target = GL_PIXEL_UNPACK_BUFFER;
	glBindBuffer( target, pbo_ );
	GLintptr offset = 0;
	GLsizeiptr length = width_*height_*bpp_;
	unsigned char *ptr = (unsigned char*)glMapBufferRange( target, offset, length, GL_MAP_WRITE_BIT|GL_MAP_INVALIDATE_BUFFER_BIT );
    //unsigned char *ptr = (unsigned char*)glMapBufferRange( target, offset, length, GL_MAP_WRITE_BIT );
	if( ptr ) {
		memcpy( ptr, src, length );
		glUnmapBuffer( target );
		copied_ = true;
	}
	glBindBuffer( target, 0 );
}



// https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glMapBufferRange.xhtml
/*
 * PBO をロックして、指定位置のポインタを取得する
 * @param type 読み書き等の指定
 * @param offset ポインタオフセット
 * @param length バイト長さ
 */
void* GLPixelBufferObject::LockBuffer( GLbitfield type, GLintptr offset, GLsizeiptr length ) {
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER, pbo_ );
	return glMapBufferRange( GL_PIXEL_UNPACK_BUFFER, offset, length, type );
}

/**
 * PBO のロックを解除する
 * ロック中に他のPBOへのアクセスはしないこと
 */
void GLPixelBufferObject::UnlockBuffer() {
	glUnmapBuffer( GL_PIXEL_UNPACK_BUFFER );
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );
	copied_ = true;
}


/**
 * PBO からテクスチャへデータ転送する
 */
void GLPixelBufferObject::CopyToTexture( GLuint tex )
{
	if (copied_) {
		// 既に PBO にデータが転送されている場合、テクスチャへデータ転送する
		// PBOからテクスチャへ転送
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		//glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

/**
 * PBO からテクスチャへデータ転送する(初回転送)
 */
void GLPixelBufferObject::CopyToTexture( GLuint tex, GLint format )
{
	if( copied_ ) {
		// PBOからテクスチャへ転送
		glBindBuffer( GL_PIXEL_UNPACK_BUFFER, pbo_ );
		glBindTexture( GL_TEXTURE_2D, tex );
		glTexImage2D( GL_TEXTURE_2D, 0, format, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
		glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );
		glBindTexture( GL_TEXTURE_2D, 0 );
	}
}


/**
 * フレームバッファから PBO へデータ転送する
 */
bool GLPixelBufferObject::CopyFromFramebuffer( GLsizei width, GLsizei height, bool front )
{
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_);

	// バッファ指定
	glReadBuffer( front ? GL_FRONT : GL_BACK );

	// PBO にロード
	glReadPixels( 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0 );

	copied_ = true;
	return true;
}


/**
 * PBO からデータをコピーする
 */
void GLPixelBufferObject::CopyFrom( const GLPixelBufferObject& pbo )
{
	GLint size;
	glBindBuffer( GL_PIXEL_PACK_BUFFER, pbo.pbo_ );
	glGetBufferParameteriv( GL_PIXEL_PACK_BUFFER, GL_BUFFER_SIZE, &size );

	glBindBuffer( GL_PIXEL_UNPACK_BUFFER, pbo_ );
	glBufferData( GL_PIXEL_UNPACK_BUFFER, size, nullptr, GL_DYNAMIC_DRAW );

	glCopyBufferSubData( GL_PIXEL_UNPACK_BUFFER, GL_PIXEL_PACK_BUFFER, 0, 0, size );

	glBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );

	width_ = pbo.width_;
	height_ = pbo.height_;
	bpp_ = pbo.bpp_;
	copied_ = true;
}


