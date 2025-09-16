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

        if (rel.type() == ivypo::RelationType::ChildOf)
        {
          om.AddRelation(target_id, ivy::BaseObjectManager::ChildOf, it.second);
        }
        else if (rel.type() == ivypo::RelationType::DependsOn)
        {
          om.AddRelation(it.second, ivy::BaseObjectManager::DependsOn, target_id);
        }