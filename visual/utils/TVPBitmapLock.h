#ifndef __TVP_BITMAP_LOCK_H__
#define __TVP_BITMAP_LOCK_H__

struct tTVPRect;

namespace tvp {
namespace bitmap {

/**
 * Lock 処理タイプ ( 実質的にOpenGL ES 3.0 以降の時のみ意味がある )
 */
enum class LockType : tjs_int {
	WRITE_ONLY = 0,	// 書き込みのみなので、Lock 時に値のコピーは行わない
	OVERWRITE_ONLY = 1,	// 指定領域全てを上書きする、上書きしなかった領域がどうなるかは不定
	READ_ONLY = 2,	// 読み込みのみなので、Lock 時に値のコピーを行う、Unlock 時は何もせずメモリ開放
	READ_WRITE = 3,	// 読み書きなので、Loack 時のコピーと Unlock 時の書き込みを行う
};

/**
 * ロック処理用インターフェイス
 */
struct LockInterface {
	/**
	 * 画像の生データへのポインタを取得する(バイトオフセット版)
	 * 使用後 UnlockBits をコールすること
	 * @return 画像データ実体へのポイント
	 */
	virtual void* LockBits( LockType type, tjs_offset offset, tjs_size length ) = 0;

	/**
	* 画像の生データへのポインタを取得する(矩形指定版)
	* 使用後 UnlockBits をコールすること
	* 指定行以降全てが更新されたとみなされる
	* @param type ロック方法指定
	* @param area ロック範囲矩形
	*/
	virtual void* LockBits( LockType type = LockType::WRITE_ONLY, tTVPRect* area = nullptr) = 0;

	/**
	 * 画像データアクセス終了時にロックを解除する
	 */
	virtual void UnlockBits() = 0;
};

}	// bitmap
}	// tvp



#endif // __TVP_BITMAP_LOCK_H__
