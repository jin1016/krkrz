
#include "tjsCommHead.h"
#include "DebugIntf.h"
#include "OpenGLScreen.h"
#include <memory>

#include "TVPBitmap.h"
#include "GLPixelBufferObject.h"
#include "TVPTextureBridgeMemory.h"
#include "GLTexture.h"


// PBO/Bitmap を使って転送する
GLTexture::GLTexture( const TVPTextureBridgeMemory& src, GLint format )
 : width_(src.GetWidth()), height_(src.GetHeight()), format_(format), stretchType_(GL_LINEAR), wrapS_(GL_CLAMP_TO_EDGE), wrapT_(GL_CLAMP_TO_EDGE) 
{
	glPixelStorei( GL_UNPACK_ALIGNMENT, src.GetBPP() );
	glGenTextures( 1, &texture_id_ );
	glBindTexture( GL_TEXTURE_2D, texture_id_ );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, stretchType_ );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, stretchType_ );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS_ );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT_ );

	//glTexImage2D( GL_TEXTURE_2D, 0, format, width_, height_, 0, format, GL_UNSIGNED_BYTE, bits );
	createFromBridgeMemory( src, format);

	glBindTexture( GL_TEXTURE_2D, 0 );
}
// move
GLTexture& GLTexture::operator=(GLTexture&& rhs) {
	texture_id_ = rhs.texture_id_;
	format_ = rhs.format_;
	width_ = rhs.width_;
	height_ = rhs.height_;
	stretchType_ = rhs.stretchType_;
	wrapS_ = rhs.wrapS_;
	wrapT_ = rhs.wrapT_;
	hasMipmap_ = rhs.hasMipmap_;
	return *this;
}

void GLTexture::createFromBridgeMemory(const TVPTextureBridgeMemory& src, GLint format ) {
	const_cast<TVPTextureBridgeMemory&>(src).CopyToTexture( texture_id_, format_ );
}

void GLTexture::create( GLuint w, GLuint h, const GLvoid* bits, GLint format ) {
	glPixelStorei( GL_UNPACK_ALIGNMENT, format == GL_RGBA ? 4 : 1 );
	glGenTextures( 1, &texture_id_ );
	glBindTexture( GL_TEXTURE_2D, texture_id_ );
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,stretchType_);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,stretchType_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT_);
	glTexImage2D( GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, bits );
	glBindTexture( GL_TEXTURE_2D, 0 );
	format_ = format;
	width_ = w;
	height_ = h;
}

/**
 * ミップマップを持つテクスチャを生成する
 * 今のところ GL_RGBA 固定
 */
void GLTexture::createMipmapTexture( std::vector<GLTextreImageSet>& img ) {
	if( img.size() > 0 ) {
		GLuint w = img[0].width;
		GLuint h = img[0].height;
		glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
		glGenTextures( 1, &texture_id_ );
		glBindTexture( GL_TEXTURE_2D, texture_id_ );

		GLint count = img.size();
		if( count > 1 ) hasMipmap_ = true;

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, stretchType_ );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, getMipmapFilter( stretchType_ ) );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS_ );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT_ );
		// ミップマップの最小と最大レベルを指定する、これがないと存在しないレベルを参照しようとすることが発生しうる
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, count - 1 );
		if( TVPOpenGLESVersion == 200 ) {
			// OpenGL ES2.0 の時は、glGenerateMipmap しないと正しくミップマップ描画できない模様
			GLTextreImageSet& tex = img[0];
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits );
			glHint( GL_GENERATE_MIPMAP_HINT, GL_FASTEST );
			glGenerateMipmap( GL_TEXTURE_2D );
			// 自前で生成したものに一部置き換える
			for( GLint i = 1; i < count; i++ ) {
				GLTextreImageSet& tex = img[i];
				glTexSubImage2D( GL_TEXTURE_2D, i, 0, 0, tex.width, tex.height, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits );
			}
		} else {
			for( GLint i = 0; i < count; i++ ) {
				GLTextreImageSet& tex = img[i];
				glTexImage2D( GL_TEXTURE_2D, i, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits );
			}

		}
		glBindTexture( GL_TEXTURE_2D, 0 );
		format_ = GL_RGBA;
		width_ = w;
		height_ = h;
	}
}
void GLTexture::destory() {
	if( texture_id_ != 0 ) {
		glDeleteTextures( 1, &texture_id_ );
		texture_id_ = 0;
		hasMipmap_ = false;
	}
}
/**
 * フィルタタイプに応じたミップマップテクスチャフィルタを返す
 * GL_NEAREST_MIPMAP_LINEAR/GL_LINEAR_MIPMAP_LINEAR は使用していない
 */
GLint GLTexture::getMipmapFilter( GLint filter ) {
	switch( filter ) {
	case GL_NEAREST:
		return GL_NEAREST_MIPMAP_NEAREST;
	case GL_LINEAR:
		return GL_LINEAR_MIPMAP_NEAREST;
	case GL_NEAREST_MIPMAP_NEAREST:
		return GL_NEAREST_MIPMAP_NEAREST;
	case GL_LINEAR_MIPMAP_NEAREST:
		return GL_LINEAR_MIPMAP_NEAREST;
	case GL_NEAREST_MIPMAP_LINEAR:
		return GL_NEAREST_MIPMAP_LINEAR;
	case GL_LINEAR_MIPMAP_LINEAR:
		return GL_LINEAR_MIPMAP_LINEAR;
	default:
		return GL_LINEAR_MIPMAP_NEAREST;
	}
}
void GLTexture::copyImage( GLint x, GLint y, GLint w, GLint h, const GLvoid* bits ) {
	glPixelStorei( GL_UNPACK_ALIGNMENT, format_ == GL_RGBA ? 4 : 1 );
	glTexSubImage2D( GL_TEXTURE_2D, 0, x, y, w, h, format_, GL_UNSIGNED_BYTE, bits );
}
int GLTexture::getMaxTextureSize() {
	GLint maxTex;
	glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxTex );
	return maxTex;
}
	
void GLTexture::setStretchType( GLenum s ) {
	if( texture_id_ && stretchType_ != s ) {
		glBindTexture( GL_TEXTURE_2D, texture_id_ );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, s );
		if( hasMipmap_ == false ) {
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, s );
		} else {
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, getMipmapFilter(s) );
		}
		glBindTexture( GL_TEXTURE_2D, 0 );
	}
	stretchType_ = s;
}
	
void GLTexture::setWrapS( GLenum s ) {
	if( texture_id_ && wrapS_ != s ) {
		glBindTexture( GL_TEXTURE_2D, texture_id_ );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, s );
		glBindTexture( GL_TEXTURE_2D, 0 );
	}
	wrapS_ = s;
}
void GLTexture::setWrapT( GLenum s ) {
	if( texture_id_ && wrapT_ != s ) {
		glBindTexture( GL_TEXTURE_2D, texture_id_ );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, s );
		glBindTexture( GL_TEXTURE_2D, 0 );
	}
	wrapT_ = s;
}

