
#ifndef __TVP_COLOR_FORMAT_H__
#define __TVP_COLOR_FORMAT_H__

namespace tvp {

// tTVPGraphicLoadMode がカラーフォーマットを表しているが、ロード用なので違う

enum class COLOR_FORMAT : int32_t {
	RGBA = GL_RGBA,
	ALPHA = GL_ALPHA,
	BGRA = 1
};

}

#endif __TVP_COLOR_FORMAT_H__
