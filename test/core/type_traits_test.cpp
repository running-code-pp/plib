/**
 * @brief 测试类型萃取工具
 */

#include "type_traits.hpp"
#include <iostream>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <string>

template<typename T>
void test_container_type(const std::string& name) {
    std::cout << name << ":\n";
    std::cout << "  has_key_type: " << has_key_type<T>::value << "\n";
    std::cout << "  is_sequence_container: " << is_sequence_container<T>::value << "\n";
    std::cout << "  is_associative_container: " << is_associative_container<T>::value << "\n";
    std::cout << "  is_set_like: " << is_set_like<T>::value << "\n";
    std::cout << "  is_map_like: " << is_map_like<T>::value << "\n";
    std::cout << std::endl;
}

int main() {
    
    std::cout << "=== 类型萃取测试 ===\n\n";
    
    std::cout << "--- 序列式容器 (应该: has_key_type=0, is_sequence=1, is_associative=0) ---\n";
    test_container_type<std::vector<int>>("std::vector<int>");
    test_container_type<std::list<int>>("std::list<int>");
    test_container_type<std::deque<int>>("std::deque<int>");
    
    std::cout << "--- 关联式容器 - set类 (应该: has_key_type=1, is_sequence=0) ---\n";
    test_container_type<std::set<int>>("std::set<int>");
    test_container_type<std::multiset<int>>("std::multiset<int>");
    test_container_type<std::unordered_set<int>>("std::unordered_set<int>");
    
    std::cout << "--- 关联式容器 - map类 (应该: has_key_type=1, is_sequence=0, is_associative=1) ---\n";
    test_container_type<std::map<int, std::string>>("std::map<int, string>");
    test_container_type<std::multimap<int, std::string>>("std::multimap<int, string>");
    test_container_type<std::unordered_map<int, std::string>>("std::unordered_map<int, string>");
    
    std::cout << "--- 非容器类型 (应该: 全部=0) ---\n";
    test_container_type<int>("int");
    test_container_type<std::string>("std::string");
    
    return 0;
}