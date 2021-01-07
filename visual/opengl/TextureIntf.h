/**
 * Texture クラス
 */

#ifndef TextureIntfH
#define TextureIntfH

#include "tjsNative.h"
#include "GLTexture.h"
#include "TextureInfo.h"
#include "GLVertexBufferObject.h"
#include "ComplexRect.h"
#include "LayerBitmapIntf.h"
#include "TVPTextureBridgeMemory.h"

#include "TVPTexture.h"


enum class tTVPTextureColorFormat : tjs_int {
	RGBA = 0,
	Alpha = 1,
	// Luminance or Compressed texture
};


class tTJSNI_Texture : public tTJSNativeInstance, public iTVPTextureInfoIntrface
{
	std::unique_ptr<tTVPTexture> Texture;

	tjs_uint SrcWidth = 0;
	tjs_uint SrcHeight = 0;

	// 9patch描画用情報
	tTVPRect Scale9Patch = { -1, -1, -1, -1 };
	tTVPRect Margin9Patch = { -1, -1, -1, -1 };

	tTJSVariant MarginRectObject;
	class tTJSNI_Rect* MarginRectInstance = nullptr;

	// このクラスに書かれているいくつかのメソッドは tTVPTexture に移動した方が良いかもしれない
	// tTJSNI_Offscreen でほぼ同等のメソッドを記述するのなら、tTVPTexture に移動する
	void LoadTexture( const class tTVPBaseBitmap* bitmap, tTVPTextureColorFormat color, bool rbswap = false );
	tjs_error LoadMipmapTexture( const class tTVPBaseBitmap* bitmap, class tTJSArrayNI* sizeList, enum tTVPBBStretchType type, tjs_real typeopt );
	GLint ColorToGLColor( tTVPTextureColorFormat color );

	void SetMarginRectObject( const tTJSVariant & val );
public:
	tTJSNI_Texture();
	~tTJSNI_Texture() override;
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) override;
	void TJS_INTF_METHOD Invalidate() override;

	const tTJSVariant& GetMarginRectObject() const { return MarginRectObject; }

	void CopyBitmap( tjs_int left, tjs_int top, const class tTVPBaseBitmap* bitmap, const tTVPRect& srcRect );
	void CopyBitmap( const class tTVPBaseBitmap* bitmap );

	tjs_uint GetWidth() const override { return SrcWidth; }
	tjs_uint GetHeight() const override { return SrcHeight; }

	// 以下、各メソッドを包含
	tjs_uint GetMemoryWidth() const { return Texture->GetWidth(); }
	tjs_uint GetMemoryHeight() const { return Texture->GetHeight(); }
	tjs_int64 GetNativeHandle() const override { return Texture->GetTextureId(); }
	tjs_int64 GetVBOHandle() const override { return Texture->GetVBOHandle(); }
	// VBOに描画サイズを設定しておき、テクスチャサイズ以外で描画させる
	void SetDrawSize(tjs_uint width, tjs_uint height) { Texture->SetDrawSize(width, height); }
	tjs_int GetImageFormat() const override { return Texture->GetTextureFormat(); }

	bool IsGray() const { return Texture->GetTextureFormat() == GL_ALPHA; }
	bool IsPowerOfTwo() const { return IsPowerOfTwo(Texture->GetWidth()) && IsPowerOfTwo(Texture->GetHeight()); }

	static inline bool IsPowerOfTwo( tjs_uint x ) { return (x & (x - 1)) == 0; }
	static inline tjs_uint ToPowerOfTwo( tjs_uint x ) {
		// 組み込み関数等でMSBを取得してシフトしてもいいが、32からシフトしてループで得ることにする。
		if( IsPowerOfTwo( x ) == false ) {
			tjs_uint r = 32;
			while( r < x ) r = r << 1;
			return r;
		}
		return x;
	}
	
	tjs_int GetStretchType() const {
		return static_cast<tjs_int>( Texture->GetStretchType() );
	}
	void SetStretchType(tjs_int v) {
		Texture->SetStretchType(static_cast<GLenum>( v ));
	}
	tjs_int GetWrapModeHorizontal() const {
		return static_cast<tjs_int>( Texture->GetWrapModeHorizontal() );
	}
	void SetWrapModeHorizontal(tjs_int v) {
		Texture->SetWrapModeHorizontal(static_cast<GLenum>( v ));
	}
	tjs_int GetWrapModeVertical() const {
		return static_cast<tjs_int>( Texture->GetWrapModeVertical() );
	}
	void SetWrapModeVertical(tjs_int v) {
		Texture->SetWrapModeVertical(static_cast<GLenum>( v ));
	}


	const tTVPRect& GetScale9Patch() const { return Scale9Patch; }
	const tTVPRect& GetMargin9Patch() const { return Margin9Patch; }

	tjs_int64 GetTextureId() const { return Texture->GetTextureId(); }
	void SetTextureId(tjs_int64 id) { Texture->SetTextureId(id); }

	// iTVPBitmap intreface override
	tjs_int GetStride() const override  {
		return Texture->GetStride();
	}
	tjs_uint GetLineBytes() const override {
		return Texture->GetLineBytes();
	}
	void* LockBits(tTVPBitmapLockType type, tjs_offset offset, tjs_size length) override {
		return Texture->LockBits(type, offset, length);
	}
	void* LockBits(tTVPBitmapLockType type = tTVPBitmapLockType::WRITE_ONLY, tTVPRect* area = nullptr) override {
		return Texture->LockBits(type, area );
	}
	void UnlockBits() override {
		Texture->UnlockBits();
	}

	friend class tTJSNI_Offscreen;
};


//---------------------------------------------------------------------------
// tTJSNC_Texture : TJS Texture class
//---------------------------------------------------------------------------
class tTJSNC_Texture : public tTJSNativeClass
{
public:
	tTJSNC_Texture();
	static tjs_uint32 ClassID;

protected:
	tTJSNativeInstance *CreateNativeInstance() override { return new tTJSNI_Texture(); }
};
//---------------------------------------------------------------------------
extern tTJSNativeClass * TVPCreateNativeClass_Texture();
#endif
