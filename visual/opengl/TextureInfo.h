
#ifndef TextureInfoH
#define TextureInfoH

#include "TVPBitmapLock.h"
#include "BitmapTextDrawer.h"

class iTVPTextureInfoIntrface : public tvp::bitmap::LockInterface {
	using BitmapLockType = typename tvp::bitmap::LockType;
public:

	/**
	 * 幅を取得
	 * @return テクスチャ幅
	 */
	virtual tjs_uint GetWidth() const = 0;

	/**
	 * 高さを取得
	 * @return テクスチャ高さ
	 */
	virtual tjs_uint GetHeight() const = 0;

	/**
	 * ネイティブハンドルを取得。OpenGL ES2/3実装ではテクスチャID
	 * @return ネイティブハンドル
	 */
	virtual tjs_int64 GetNativeHandle() const = 0;

	/**
	 * テクスチャ全体を表す頂点データのVBOをハンドルを返す。
	 * @return VBO ID、0の時VBOがない
	 */
	virtual tjs_int64 GetVBOHandle() const = 0;

	/**
	 * テクスチャフォーマットを取得
	 * @return テクスチャフォーマット
	 */
	virtual tjs_int GetImageFormat() const = 0;
#if 0
	// 以下 TVPBitmapLock.h に記述
	/**
	 * 画像の生データへのポインタを取得する(バイトオフセット版)
	 * 使用後 UnlockBits をコールすること
	 * @return 画像データ実体へのポイント
	 */
	virtual void* LockBits(BitmapLockType type, tjs_offset offset, tjs_size length ) = 0;

	/**
	* 画像の生データへのポインタを取得する(矩形指定版)
	* 使用後 UnlockBits をコールすること
	* 指定行以降全てが更新されたとみなされる
	* @param type ロック方法指定
	* @param area ロック範囲矩形
	*/
	virtual void* LockBits(BitmapLockType type = BitmapLockType::WRITE_ONLY, tTVPRect* area = nullptr) = 0;

	/**
	 * 画像データアクセス終了時にロックを解除する
	 */
	virtual void UnlockBits() = 0;
#endif
};

#if 0
旧実装の以下に戻した方が良い
class iTVPTextureInfoIntrface {
public:

	/**
	 * 幅を取得
	 * @return テクスチャ幅
	 */
	virtual tjs_uint GetWidth() const = 0;

	/**
	 * 高さを取得
	 * @return テクスチャ高さ
	 */
	virtual tjs_uint GetHeight() const = 0;

	/**
	 * ネイティブハンドルを取得。OpenGL ES2/3実装ではテクスチャID
	 * @return ネイティブハンドル
	 */
	virtual tjs_int64 GetNativeHandle() const = 0;

	/**
	 * テクスチャ全体を表す頂点データのVBOをハンドルを返す。
	 * @return VBO ID、0の時VBOがない
	 */
	virtual tjs_int64 GetVBOHandle() const = 0;

	/**
	 * テクスチャフォーマットを取得
	 * @return テクスチャフォーマット
	 */
	virtual tjs_int GetImageFormat() const = 0;
};

#endif

#endif
