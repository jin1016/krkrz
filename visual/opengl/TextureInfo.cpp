
#include "tjsCommHead.h"
#include "OpenGLScreen.h"
#include "MsgIntf.h"
#include "TextureInfo.h"

tjs_uint iTVPTextureInfoIntrface::GetBPP() const {
	switch( GetImageFormat() ) {
	case GL_RGBA:
		return 32;
	case GL_ALPHA:
		return 8;
	}
	// ここは例外出した方がいいか？
	return 32;	// 現在使われているのは上記2つだが、もしも違う時は32を返す。
}
bool iTVPTextureInfoIntrface::Is32bit() const {
	return GetImageFormat() == GL_RGBA;
}
bool iTVPTextureInfoIntrface::Is8bit() const {
	return GetImageFormat() == GL_ALPHA;
}

void* iTVPTextureInfoIntrface::GetScanLine(tjs_uint l) const {
	TVPThrowExceptionMessage(TJS_W("This operation is not possible with textures."));
	return nullptr;
}
