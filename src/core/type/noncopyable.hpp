#ifndef _core_type_noncopyable_hpp_
#define _core_type_noncopyable_hpp_
namespace plib::core::type {
	class noncopyable {
	protected:
		noncopyable() {}
		virtual ~noncopyable() {}

	private:
		noncopyable(const noncopyable&) = delete;
		noncopyable& operator=(const noncopyable&) = delete;
		noncopyable(noncopyable&&) = delete;
		noncopyable& operator=(noncopyable&&) = delete;
	};
} // namespace plib::core::type

#endif // _core_type_noncopyable_hpp_