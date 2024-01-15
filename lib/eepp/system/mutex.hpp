#ifndef EE_SYSTEMCMUTEX_H
#define EE_SYSTEMCMUTEX_H

#include <eepp/core.hpp>
#include <eepp/core/noncopyable.hpp>

namespace EE { namespace System {

namespace Platform {
class MutexImpl;
}

/** @brief Blocks concurrent access to shared resources from multiple threads */
class EE_API Mutex : NonCopyable {
  public:
	Mutex();

	~Mutex();

	/** @brief Lock the mutex
	**	If the mutex is already locked in another thread,
	**	this call will block the execution until the mutex
	**	is released. */
	void lock();

	/** @brief Unlock the mutex */
	void unlock();

	/** @brief Tries to lock de mutex if possible */
	int tryLock();

  private:
	Platform::MutexImpl* mMutexImpl;
};

}} // namespace EE::System

#endif
