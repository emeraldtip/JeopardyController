#ifndef EE_SYSTEMCTHREAD_H
#define EE_SYSTEMCTHREAD_H

#include <cstddef>
#include <eepp/core/noncopyable.hpp>

namespace EE { namespace System {

namespace Platform {
class ThreadImpl;
}
namespace Private {
struct ThreadFunc;
}

/** @brief Thread manager class */
class EE_API Thread : NonCopyable {
  public:
	typedef void ( *FuncType )( void* );

	/** @return The current thread id */
	static Uint32 getCurrentThreadId();

	/** @brief Construct the thread from a functor with no argument
	**	This constructor works for function objects, as well
	**	as free function.
	**	Use this constructor for this kind of function:
	 @code
	 void function();

	 --- or ----

	 struct Functor
	 {
		 void operator()();
	 };
	 @endcode
	 Note: this does *not* run the thread, use Launch().
	 @param function Functor or free function to use as the entry point of the thread
	*/
	template <typename F> Thread( F function );

	/**	@brief Construct the thread from a functor with an argument
	**	This constructor works for function objects, as well
	**	as free function.
	**	It is a template, which means that the argument can
	**	have any type (int, std::string, void*, Toto, ...).
	**	Use this constructor for this kind of function:
	**	@code
		void function(int arg);

		--- or ----

		struct Functor
		{
			void operator()(std::string arg);
		};
		@endcode
		Note: this does *not* run the thread, use Launch().
	**	@param function Functor or free function to use as the entry point of the thread
	**	@param argument argument to forward to the function */
	template <typename F, typename A> Thread( F function, A argument );

	/**	@brief Construct the thread from a member function and an object
	**	This constructor is template, which means that you can
	**	use it with any class.
	**	Use this constructor for this kind of function:
	**	@code
		class MyClass
		{
			public :
				void function();
		};
	**	@endcode
	**	Note: this does *not* run the thread, use Launch().
	**	@param function Entry point of the thread
	**	@param object Pointer to the object to use **/
	template <typename C> Thread( void ( C::*function )(), C* object );

	/** @brief Destructor
	**	This destructor calls Wait(), so that the internal thread
	**	cannot survive after its Thread instance is destroyed. */
	virtual ~Thread();

	/** @brief Run the thread
	**	This function starts the entry point passed to the
	**	thread's constructor, and returns immediately.
	**	After this function returns, the thread's function is
	**	running in parallel to the calling code. */
	void launch();

	/** @brief Wait until the thread finishes.
	**	This function will block the execution until the
	**	thread's function ends.
	**	Warning: if the thread function never ends, the calling
	**	thread will block forever.
	**	If this function is called from its owner thread, it
	**	returns without doing anything. */
	void wait();

	/** @brief Terminate the thread
	**	This function immediately stops the thread, without waiting
	**	for its function to finish.
	**	Terminating a thread with this function is not safe,
	**	and can lead to local variables not being destroyed
	**	on some operating systems. You should rather try to make
	**	the thread function terminate by itself. */
	void terminate();

	/** @return The id of the thread */
	Uint32 getId();

  protected:
	Thread();

  private:
	friend class Platform::ThreadImpl;

	/** The virtual function to run in the thread */
	virtual void run();

	Platform::ThreadImpl* mThreadImpl; ///< OS-specific implementation of the thread
	Private::ThreadFunc* mEntryPoint;  ///< Abstraction of the function to run
};

// Based on the SFML2 thread implementation.
namespace Private {

// Base class for abstract thread functions
struct ThreadFunc {
	virtual ~ThreadFunc() {}
	virtual void run() = 0;
};

// Specialization using a functor (including free functions) with no argument
template <typename T> struct ThreadFunctor : ThreadFunc {
	ThreadFunctor( T functor ) : mFunctor( functor ) {}

	virtual void run() { mFunctor(); }

	T mFunctor;
};

// Specialization using a functor (including free functions) with one argument
template <typename F, typename A> struct ThreadFunctorWithArg : ThreadFunc {
	ThreadFunctorWithArg( F function, A arg ) : mFunction( function ), mArg( arg ) {}

	virtual void run() { mFunction( mArg ); }

	F mFunction;
	A mArg;
};

// Specialization using a member function
template <typename C> struct ThreadMemberFunc : ThreadFunc {
	ThreadMemberFunc( void ( C::*function )(), C* object ) :
		mFunction( function ), mObject( object ) {}

	virtual void run() { ( mObject->*mFunction )(); }

	void ( C::*mFunction )();

	C* mObject;
};

} // namespace Private

template <typename F>
Thread::Thread( F functor ) :
	mThreadImpl( NULL ), mEntryPoint( new Private::ThreadFunctor<F>( functor ) ) {}

template <typename F, typename A>
Thread::Thread( F function, A argument ) :
	mThreadImpl( NULL ),
	mEntryPoint( new Private::ThreadFunctorWithArg<F eeCOMMA A>( function, argument ) ) {}

template <typename C>
Thread::Thread( void ( C::*function )(), C* object ) :
	mThreadImpl( NULL ), mEntryPoint( new Private::ThreadMemberFunc<C>( function, object ) ) {}

}} // namespace EE::System

#endif
