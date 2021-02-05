/**
 * 解放時に Release を使用する deleter
 * 吉里吉里Z の既存クラスは参照カウンタ方式が多いため、スマートポインタ類で管理する時に delete の代わりに Release を呼び出す deleter が必要
 */
#ifndef __TVP_RELEASE_DELETER_H__
#define __TVP_RELEASE_DELETER_H__

namespace tvp {

template<typename T = void>
struct release_deleter {
	constexpr release_deleter() noexcept = default;
	template< typename U, typename std::enable_if<std::is_convertible<U*, T*>::value, std::nullptr_t>::type = nullptr >
	release_deleter(const release_deleter<U>&) noexcept {}
	void operator()(T* ptr) const {
		ptr->Release();
	}
};
template<typename T>
struct release_deleter<T[]> {
	constexpr release_deleter() noexcept = default;
	template< typename U, typename std::enable_if<std::is_convertible<U(*)[], T(*)[]>::value, std::nullptr_t>::type = nullptr >
	release_deleter(const release_deleter<U[]>&) noexcept {}
	void operator()(T* ptr) const {
		for( auto i : ptr ) {
			i->Release();
		}
		delete[] ptr;
	}
};
}

#endif __TVP_RELEASE_DELETER_H__
