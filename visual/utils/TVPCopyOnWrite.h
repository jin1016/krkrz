
#ifndef __TVP_COPY_ON_WRITE_H__
#define __TVP_COPY_ON_WRITE_H__

#include <memory>
#include "TVPColorFormat.h"

namespace tvp {
/*
	このクラスを使用するためにテンプレート引数で渡されるクラスが実装するべきメソッド類
	class Bitmap : public std::enable_shared_from_this<Bitmap> {
	public:
		Bitmap(){}
		Bitmap( ( uint32_t w, uint32_t h, tvp::COLOR_FORMAT format, bool unpadding=false ){}

		std::shared_ptr<Bitmap> get_shared_this() {
			return shared_from_this();
		}
		// 自身を複製して返す。
		// 返すBitmapは保持している内容をそのままコピーして複製を作ること。
		// メモリ上の画像データは自身と切り離して生成する。
		std::shared_ptr<Bitmap> clone() {}
		uint32_t get_width() const;
		uint32_t get_height() const;
		enum COLOR_FORMAT get_format() const;
		bool is_padding() const;
	}
};
*/

/**
 * copy on write を実装しやすくするための補助クラス
 */
template<class T>
struct copy_on_write {
	/**
	 * 変更操作を行う前に呼び出し、Bitmap/Texture の共有状態を解除する
	 * 共有されていない場合はそのまま。
	 */
	static std::shared_ptr<T> independ( std::shared_ptr<T>& bmp ) {
		if( bmp.use_count() <= 1 ) return bmp;
		return bmp->get_shared_this();
	}
	/**
	 * 内容をコピーせずにBitmap/Texture の共有状態を解除する
	 * 共有されていない場合はそのまま。
	 */
	static std::shared_ptr<T> void independNoCopy( std::shared_ptr<T>& bmp ) {
		if( bmp.use_count() <= 1 ) return bmp;
		return bmp->clone();
	}
	static std::shared_ptr<T> recreate( std::shared_ptr<T>& bmp ) {
		return recreate( bmp->get_width(), bmp->get_height(), bmp->get_format(), !bmp->is_padding() );
	}
	static std::shared_ptr<T> recreate( uint32_t w, uint32_t h, enum class COLOR_FORMAT format, bool unpadding=false) {
		return std::make_shared<T>( w, h, format, unpadding );
	}

	static bool isIndependent() const { return bmp.use_count() <= 1; }
}
}

#endif __TVP_COPY_ON_WRITE_H__

