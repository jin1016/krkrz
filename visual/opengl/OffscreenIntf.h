/**
 * Offscreen クラス
 */

#ifndef OffscreenIntfH
#define OffscreenIntfH

#include "TextureInfo.h"
#include "TVPOffscreen.h"

class tTJSNI_Offscreen : public tTJSNativeInstance, public iTVPTextureInfoIntrface
{
	tTVPOffscreen Offscreen;

public:
	tTJSNI_Offscreen();
	~tTJSNI_Offscreen() override;
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) override;
	void TJS_INTF_METHOD Invalidate() override;

	// 引数が1つだけの時は、全体コピーでBitmap側のサイズをOffscreenに合わせる
	void CopyToBitmap( class tTVPBaseBitmap* bmp ) { Offscreen.CopyToBitmap( bmp ); }
	void CopyToBitmap(class tTVPBaseBitmap* bmp, const tTVPRect& srcRect, tjs_int dleft, tjs_int dtop) {
		Offscreen.CopyToBitmap( bmp, srcRect, dleft, dtop );
	}
	void CopyFromBitmap(tjs_int left, tjs_int top, const class tTVPBaseBitmap* bitmap, const tTVPRect& srcRect) {
		Offscreen.CopyFromBitmap( left, top, bitmap, srcRect );
	}
	void ExchangeTexture(class tTJSNI_Texture* texture) {
		Offscreen.ExchangeTexture(texture);
	}

	tjs_uint GetWidth() const override { return Offscreen.GetWidth(); }
	tjs_uint GetHeight() const override { return Offscreen.GetHeight(); }
	tjs_int64 GetNativeHandle() const override { return Offscreen.GetNativeHandle(); }
	tjs_int64 GetVBOHandle() const override { return Offscreen.GetVBOHandle(); }
	tjs_int GetImageFormat() const override { return Offscreen.GetImageFormat(); }

	/**
	 * 描画対象に設定する
	 */
	void BindFrameBuffer() { Offscreen.BindFramebuffer(); }

	// iTVPBitmap intreface override
	tjs_int GetStride() const override { return Offscreen.GetStride(); }
	tjs_uint GetLineBytes() const override { return Offscreen.GetLineBytes(); }
	void* LockBits(tTVPBitmapLockType type, tjs_offset offset, tjs_size length) override { return Offscreen.LockBits(type, offset, length); }
	void* LockBits(tTVPBitmapLockType type = tTVPBitmapLockType::WRITE_ONLY, tTVPRect* area = nullptr) override { return Offscreen.LockBits(type, area); }
	void UnlockBits() override { Offscreen.UnlockBits(); }
};


//---------------------------------------------------------------------------
// tTJSNC_Offscreen : TJS Offscreen class
//---------------------------------------------------------------------------
class tTJSNC_Offscreen : public tTJSNativeClass
{
public:
	tTJSNC_Offscreen();
	static tjs_uint32 ClassID;

protected:
	tTJSNativeInstance *CreateNativeInstance() override { return new tTJSNI_Offscreen(); }
};

//---------------------------------------------------------------------------
extern tTJSNativeClass * TVPCreateNativeClass_Offscreen();
#endif
