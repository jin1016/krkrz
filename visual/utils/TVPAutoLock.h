

#ifndef __TVP_AUTO_LOCK_H__
#define __TVP_AUTO_LOCK_H__

namespace tvp {

template<class T>
struct BitmapAutoLock {
	T* lock_target_;
	BitmapAutoLock( T* target ) : lock_target_(target) {}
	~BitmapAutoLock() {
		lock_target_->UnlockBits();
	}
};

}

#endif // _TVP_AUTO_LOCK_H__
