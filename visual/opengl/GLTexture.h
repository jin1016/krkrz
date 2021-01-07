
#ifndef __GL_TEXTURE_H__
#define __GL_TEXTURE_H__

#include "OpenGLHeader.h"

struct GLTextreImageSet {
	GLuint width;
	GLuint height;
	const GLvoid* bits;
	GLTextreImageSet( GLuint w, GLuint h, const GLvoid* b ) : width( w ), height( h ), bits( b ) {}
};
extern int TVPOpenGLESVersion;

class GLTexture {
protected:
	GLuint texture_id_;
	GLint format_;
	GLuint width_;
	GLuint height_;

	GLenum stretchType_;
	GLenum wrapS_;
	GLenum wrapT_;
	bool hasMipmap_ = false;

public:
	GLTexture() : texture_id_(0), width_(0), height_(0), format_(0), stretchType_(GL_LINEAR), wrapS_(GL_CLAMP_TO_EDGE), wrapT_(GL_CLAMP_TO_EDGE) {}
	GLTexture( GLuint w, GLuint h, const GLvoid* bits, GLint format=GL_RGBA ) : width_(w), height_(h), stretchType_(GL_LINEAR), wrapS_(GL_CLAMP_TO_EDGE), wrapT_(GL_CLAMP_TO_EDGE) {
		create( w, h, bits, format );
	}
	// PBO/Bitmap を使って転送する
	GLTexture( const class TVPTextureBridgeMemory& src, GLint format );

	// コピーは不許可、ムーブは許可
	GLTexture( const GLTexture& ref ) = delete;
	GLTexture( GLTexture&& ref ) { *this = std::move( ref ); }

	// GLTexture(const GLTexture& src) {}
	~GLTexture() {
		destory();
	}

	// コピーは不許可、ムーブは許可
	GLTexture& operator=(GLTexture&& rhs);
	GLTexture& operator=(const GLTexture& rhs) = delete;

	void create(GLuint w, GLuint h, const GLvoid* bits, GLint format = GL_RGBA);

	void createFromBridgeMemory( const class TVPTextureBridgeMemory& src, GLint format );

	/**
	* ミップマップを持つテクスチャを生成する
	* 今のところ GL_RGBA 固定
	*/
	void createMipmapTexture(std::vector<GLTextreImageSet>& img);

	void destory();

	/**
	 * フィルタタイプに応じたミップマップテクスチャフィルタを返す
	 * GL_NEAREST_MIPMAP_LINEAR/GL_LINEAR_MIPMAP_LINEAR は使用していない
	 */
	static GLint getMipmapFilter(GLint filter);

	void copyImage(GLint x, GLint y, GLint w, GLint h, const GLvoid* bits);

	static int getMaxTextureSize();

	GLuint width() const { return width_; }

	GLuint height() const { return height_; }

	GLuint id() const { return texture_id_; }
	void set_id(GLuint id) { texture_id_ = id; }

	GLint format() const { return format_; }

	GLenum stretchType() const { return stretchType_; }

	void setStretchType(GLenum s);

	GLenum wrapS() const { return wrapS_; }

	void setWrapS(GLenum s);

	GLenum wrapT() const { return wrapT_; }

	void setWrapT(GLenum s);

	friend class tTJSNI_Offscreen;
};


#endif // __GL_TEXTURE_H__
