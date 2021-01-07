#include "tjsCommHead.h"

#include "OpenGLHeader.h"

#include "MsgIntf.h"	// TVPThrowExceptionMessage
#include "BitmapIntf.h"
#include "LayerBitmapIntf.h"
#include "tvpgl.h"
#include "DebugIntf.h"
#include "RectItf.h"
#include <memory>

#include "TextureIntf.h"
#include "TVPOffscreen.h"


extern bool TVPCopyBitmapToTexture( const iTVPTextureInfoIntrface* texture, tjs_int left, tjs_int top, const tTVPBaseBitmap* bitmap, const tTVPRect& srcRect );

// 引数が1つだけの時は、全体コピーでBitmap側のサイズをOffscreenに合わせる
void tTVPOffscreen::CopyToBitmap( class tTVPBaseBitmap* bmp ) {
}

void tTVPOffscreen::CopyToBitmap( class tTVPBaseBitmap* bmp, const tTVPRect& srcRect, tjs_int dleft, tjs_int dtop ) {
	if( !bmp ) return;
	if( bmp->Is8BPP() ) {
		TVPAddLog(TJS_W("unsupported format"));
		return;
	}
	FrameBuffer.readTextureToBitmap( bmp, srcRect, dleft, dtop );
}

void tTVPOffscreen::CopyFromBitmap( tjs_int left, tjs_int top, const tTVPBaseBitmap* bitmap, const tTVPRect& srcRect ) {
	TVPCopyBitmapToTexture( this, left, top, bitmap, srcRect );
}

void tTVPOffscreen::ExchangeTexture( tTJSNI_Texture* texture ) {
	if( FrameBuffer.width() == texture->GetMemoryWidth() && FrameBuffer.height() == texture->GetMemoryHeight() && FrameBuffer.format() == texture->GetImageFormat() ) {
		GLuint oldTex = FrameBuffer.textureId();
		bool result = FrameBuffer.exchangeTexture( (GLuint)texture->GetNativeHandle() );
		if( !result ) {
			TVPThrowExceptionMessage( TJS_W( "Cannot exchange texture." ) );
		} else {
			texture->SetTextureId( oldTex );
		}
	} else {
		TVPThrowExceptionMessage( TJS_W( "Incompatible texture." ) );
	}
}

tjs_int64 tTVPOffscreen::GetVBOHandle() const {
	if( VertexBuffer.isCreated() ) {
		return VertexBuffer.id();
	} else {
		const float w = (float)FrameBuffer.width();
		const float h = (float)FrameBuffer.height();
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

