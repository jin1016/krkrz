#include "tjsCommHead.h"
#include <cmath>
#include "DebugIntf.h"
#include "OpenGLScreen.h"
#include "BitmapIntf.h"
#include "LayerBitmapIntf.h"
#include "tvpgl.h"
#include <memory>
#include "MsgIntf.h"
#include "GLPixelBufferObject.h"
#include "GLFrameBufferObject.h"

#include "TVPTextureBridgeMemory.h"

extern int TVPGetOpenGLESVersion();
static bool InitCheckPBO = false;
static bool CanUsePBO = false;
/**
 * @param w 幅
 * @param h 高さ
 * @param bpp 1pixel 辺りのバイト数
 * @param src 初期読み込み画像データ、null の時は未初期化
 */
TVPTextureBridgeMemory::TVPTextureBridgeMemory( tjs_int w, tjs_int h, tjs_int bpp, const void* src )
	: PixelBuffer(nullptr), Width(w), Height(h), BPP(bpp)
{
	if( InitCheckPBO == false ) {
		CanUsePBO = TVPGetOpenGLESVersion() >= 300;
		InitCheckPBO = true;
	}

	if( CanUsePBO ) {
		PixelBuffer.reset( new GLPixelBufferObject( w, h, bpp, src ) );
	} else {
		if( src ) {
			// データが渡された時のみ生成
			Bitmap.reset( new tTVPBitmap( w, h, bpp, true ) ); // padding なしで生成する
			CopyFromMemory( src );
		}
	}
}

// move constoractor
TVPTextureBridgeMemory::TVPTextureBridgeMemory( TVPTextureBridgeMemory&& ref ) noexcept
{
	*this = std::move( ref );
}

TVPTextureBridgeMemory::~TVPTextureBridgeMemory()
{
}

// move 
TVPTextureBridgeMemory& TVPTextureBridgeMemory::operator=(TVPTextureBridgeMemory&& rhs) noexcept
{
	if( rhs.PixelBuffer ) {
		PixelBuffer.reset( rhs.PixelBuffer.release() );
	} else {
		Bitmap = std::move(rhs.Bitmap);
		rhs.Bitmap.release();
	}

	LockType = rhs.LockType;
	LockOffset = rhs.LockOffset;
	LockLength = rhs.LockLength;
	DirtyRect = std::move( rhs.DirtyRect );
	return *this;
}

bool TVPTextureBridgeMemory::HasImage() const {
	if (PixelBuffer) {
		return PixelBuffer->HasImage();
	} else {
		return Bitmap.get() != nullptr;
	}
}


// Bitmap遅延生成のためのメソッド
void TVPTextureBridgeMemory::CheckAndCreateBitmap() {
	if( Bitmap.get() == nullptr ) {
		Bitmap.reset( new tTVPBitmap( Width, Height, BPP, true ) ); // padding なしで生成する
	}
}
/**
 * メモリから work へデータ転送する
 */
void TVPTextureBridgeMemory::CopyFromMemory( const void* src )
{
	if( PixelBuffer ) {
		PixelBuffer->CopyFromMemory( src );
	} else {
		CheckAndCreateBitmap();
		if( Bitmap->IsIndependent() ) {
			// 共有されていない時のみコピー可
			tjs_uint8* bits = static_cast<tjs_uint8*>(Bitmap->GetScanLine(0));
			const tjs_uint8* srcBits = static_cast<const tjs_uint8*>(src);
			tjs_int stride = Bitmap->GetStride();
			tjs_uint lineBytes = Bitmap->GetLineBytes();
			tjs_uint h = Bitmap->GetHeight();
			for( tjs_uint i = 0; i < h; i++ ) {
				memcpy( bits, srcBits, lineBytes );
				bits += stride;
				srcBits += lineBytes;
			}
			DirtyRect = std::move( tTVPRect( 0, 0, Bitmap->GetWidth(), h ) );
		} else {
			TVPThrowExceptionMessage(TJS_W("Work bitmap is not independent."));
		}
	}
}

GLbitfield TVPTextureBridgeMemory::GetRangeLockFlag( BitmapLockType type ) {
	switch( type ) {
	case BitmapLockType::WRITE_ONLY:
		return GL_MAP_WRITE_BIT;
	case BitmapLockType::OVERWRITE_ONLY:
		return GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
	case BitmapLockType::READ_ONLY:
		return GL_MAP_READ_BIT;
	case BitmapLockType::READ_WRITE:
		return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
	default:
		// フラグが不明な場合、読み書きとして処理する
		return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
	}
}
/*
 * work をロックして、指定位置のポインタを取得する
 * @param type 読み書き等の指定
 * @param offset ポインタオフセット
 * @param length バイト長さ
 */
void* TVPTextureBridgeMemory::LockBuffer( BitmapLockType type, tjs_offset offset, tjs_size length )
{
	if( PixelBuffer ) {
		GLbitfield flag = GetRangeLockFlag( type );
		return PixelBuffer->LockBuffer( flag, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(length) );
/*
以下、追加オプションを使えばより最適化出来るが、複雑になるのでここでは扱わないものとする
GL_MAP_INVALIDATE_RANGE_BIT
GL_MAP_INVALIDATE_BUFFER_BIT
GL_MAP_FLUSH_EXPLICIT_BIT
GL_MAP_UNSYNCHRONIZED_BIT
*/
	} else {
		CheckAndCreateBitmap();
		LockType = type;
		LockOffset = offset;
		LockLength = length;
		return Bitmap->LockBits( type, offset, length );
	}
}
// 矩形指定でのアクセスは効率が悪い
void* TVPTextureBridgeMemory::LockBuffer( BitmapLockType type, tTVPRect* area )
{
	if (PixelBuffer) {
		GLbitfield flag = GetRangeLockFlag( type );
		tjs_offset offset = Bitmap->GetStride() * area->top + ( Bitmap->GetBPP() / 32 ) * area->left;
		tjs_size length = static_cast<tjs_size>(std::abs(static_cast<tjs_int64>( static_cast<tjs_int64>( Bitmap->GetStride()) * static_cast<tjs_int64>( area->bottom) + ( static_cast<tjs_int64>( Bitmap->GetBPP()) / 32 ) * static_cast<tjs_int64>( area->right) - offset )));
		return PixelBuffer->LockBuffer(flag, static_cast<GLintptr>( offset ), static_cast<GLsizeiptr>( length ));
	} else {
		CheckAndCreateBitmap();
		LockType = type;
		tjs_offset offset = Bitmap->GetStride() * area->top + ( Bitmap->GetBPP() / 32 ) * area->left;
		tjs_size length = static_cast<tjs_size>(std::abs( static_cast<tjs_int64>( static_cast<tjs_int64>(Bitmap->GetStride()) * static_cast<tjs_int64>( area->bottom) + ( static_cast<tjs_int64>( Bitmap->GetBPP()) / 32 ) * static_cast<tjs_int64>( area->right) - offset) ));
		LockOffset = offset;
		LockLength = length;
		return Bitmap->LockBits(type, offset, length);
	}
}
/**
 * work のロックを解除する
 */
void TVPTextureBridgeMemory::UnlockBuffer()
{
	if( PixelBuffer ) {
		PixelBuffer->UnlockBuffer();
	} else {
		CheckAndCreateBitmap();
		tjs_uint lineBytes = Bitmap->GetLineBytes();
		tjs_uint h = Bitmap->GetHeight();
		tjs_offset startLine = LockOffset / lineBytes;
		tjs_size height = LockLength / lineBytes + 1;	// 割り切れる場合などは考慮せず常に切り上げる
		if( (startLine + height ) > h ) {
			// 画像高さ超えているので、高さの範囲に収める
			height = h - startLine;
		}
		if( DirtyRect.is_empty() ) {
			// 未書き込みのデータはない
			DirtyRect = std::move( tTVPRect( 0, startLine, Bitmap->GetWidth(), startLine + height ) );
		} else {
			tTVPRect rt = tTVPRect( 0, startLine, Bitmap->GetWidth(), startLine + height );
			DirtyRect.do_union( rt );
		}
	}
}

/**
 * work からテクスチャへデータ転送する
 */
void TVPTextureBridgeMemory::CopyToTexture( GLuint tex )
{
	if( PixelBuffer ) {
		PixelBuffer->CopyToTexture( tex );
	} else {
		CheckAndCreateBitmap();	// 未生成の時、ゴミデータが転送される
		glBindTexture( GL_TEXTURE_2D, tex );
		glPixelStorei( GL_UNPACK_ALIGNMENT, BPP );
		glTexSubImage2D( GL_TEXTURE_2D, 0, DirtyRect.left, DirtyRect.top, DirtyRect.get_width(), DirtyRect.get_height(), Bitmap->Is32bit() ? GL_RGBA : GL_ALPHA, GL_UNSIGNED_BYTE, Bitmap->GetBits() );
		glBindTexture( GL_TEXTURE_2D, 0 );
		DirtyRect = std::move( tTVPRect( -1, -1, -1, -1 ) );
	}
}

/**
 * work からテクスチャへデータ転送する(初回のみ)
 */
void TVPTextureBridgeMemory::CopyToTexture( GLuint tex, GLint format )
{
	if( PixelBuffer ) {
		PixelBuffer->CopyToTexture( tex, format);
	} else {
		CheckAndCreateBitmap();	// 未生成の時、ゴミデータが転送される
		glBindTexture( GL_TEXTURE_2D, tex );
		glPixelStorei( GL_UNPACK_ALIGNMENT, Bitmap->Is32bit() ? 4 : 1 );
		glTexSubImage2D( GL_TEXTURE_2D, 0, DirtyRect.left, DirtyRect.top, DirtyRect.get_width(), DirtyRect.get_height(), Bitmap->Is32bit() ? GL_RGBA : GL_ALPHA, GL_UNSIGNED_BYTE, Bitmap->GetBits());
		glBindTexture( GL_TEXTURE_2D, 0 );
		DirtyRect = std::move( tTVPRect( -1, -1, -1, -1 ) );
	}
}
	
/**
 * フレームバッファから work へデータ転送する
 */
bool TVPTextureBridgeMemory::CopyFromFramebuffer( GLsizei width, GLsizei height, bool front )
{
	if( PixelBuffer ) {
		return PixelBuffer->CopyFromFramebuffer( width, height, front);
	} else {
		CheckAndCreateBitmap();
		return GLFrameBufferObject::readFrameBuffer( 0, 0, width, height, static_cast<tjs_uint8*>( Bitmap->GetScanLine(0) ), front );
	}
}

/**
 * TextureBridgeMemoryをコピーする
 */
void TVPTextureBridgeMemory::CopyFrom( const TVPTextureBridgeMemory& src )
{
	if( PixelBuffer ) {
		PixelBuffer->CopyFrom( *src.PixelBuffer.get() );
	} else {
		Bitmap.reset( new tTVPBitmap( *(src.Bitmap.get()) ) );
	}
}
