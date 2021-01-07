
#ifndef __TVP_OFFSCREEN_H__
#define __TVP_OFFSCREEN_H__

#include "TextureInfo.h"

// フレームバッファから取り込む機会は少ないと考えられるのと、フレームバッファサイズを時々変更して使われることも考え、PBO は今のところ対応しない。
// 将来的に、フレームバッファから頻繁に取り込まれる可能性が出た場合に対応を考える
class tTVPOffscreen : public iTVPTextureInfoIntrface {
	GLFrameBufferObject	FrameBuffer;
	GLVertexBufferObject VertexBuffer;

public:
	bool Create( tjs_uint w, tjs_uint h ) { return FrameBuffer.create( static_cast<GLuint>(w), static_cast<GLuint>(h) ); }
	void Destory() {
		FrameBuffer.destory();
	}

	// 引数が1つだけの時は、全体コピーでBitmap側のサイズをOffscreenに合わせる
	void CopyToBitmap( class tTVPBaseBitmap* bmp );
	void CopyToBitmap( class tTVPBaseBitmap* bmp, const tTVPRect& srcRect, tjs_int dleft, tjs_int dtop );
	void CopyFromBitmap( tjs_int left, tjs_int top, const class tTVPBaseBitmap* bitmap, const tTVPRect& srcRect );
	void ExchangeTexture( class tTJSNI_Texture* texture );

	void BindFramebuffer() { FrameBuffer.bindFramebuffer(); }

	tjs_uint GetWidth() const override { return FrameBuffer.width(); }
	tjs_uint GetHeight() const override { return FrameBuffer.height(); }
	tjs_int64 GetNativeHandle() const override { return FrameBuffer.textureId(); }
	tjs_int64 GetVBOHandle() const override;
	tjs_int GetImageFormat() const override { return FrameBuffer.format(); }

	// iTVPBitmap intreface override
	// 以下の2つも使われないと思われる。動作未確認。
	tjs_int GetStride() const override { return -static_cast<tjs_int>(GetLineBytes()); }
	tjs_uint GetLineBytes() const override { return GetWidth() * 4; }

	// 以下非サポート
	void* LockBits(tTVPBitmapLockType type, tjs_offset offset, tjs_size length) override { return nullptr; }
	void* LockBits(tTVPBitmapLockType type = tTVPBitmapLockType::WRITE_ONLY, tTVPRect* area = nullptr) override { return nullptr; }
	void UnlockBits() override { }
};


#endif // __TVP_OFFSCREEN_H__
