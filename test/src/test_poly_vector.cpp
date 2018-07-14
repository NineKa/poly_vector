#include "catch_ext.hpp"
#include <algorithm>
#include <catch.hpp>
#include <cstring>
#include <iostream>
#include <vector>

#include "poly_vector.h"
#include "test_poly_vector.h"

std::atomic<size_t> Interface::last_id { 0 };

TEST_CASE(
    "default_constructed_poly_vec_is_empty_with_max_align_equals_one", "[poly_vector_basic_tests]")
{
    estd::poly_vector<Interface> v;
    REQUIRE(v.empty());
    REQUIRE(0 == v.size());
    REQUIRE(0 == v.sizes().first);
    REQUIRE(0 == v.sizes().second);
    REQUIRE(0 == v.capacity());
    REQUIRE(0 == v.capacities().first);
    REQUIRE(0 == v.capacities().second);
    constexpr auto alignment = estd::poly_vector<Interface>::default_alignement;
    REQUIRE(alignment == v.max_align());
}

TEST_CASE(
    "reserve_increases_capacity_with_default_align_as_max_align_t", "[poly_vector_basic_tests]")
{
    estd::poly_vector<Interface> v;
    constexpr size_t             n     = 16;
    constexpr size_t             avg_s = 64;
    v.reserve(n, avg_s);
    REQUIRE(v.empty());
    REQUIRE(0 == v.size());
    REQUIRE(0 == v.sizes().first);
    REQUIRE(0 == v.sizes().second);
    REQUIRE(n <= v.capacity());
    REQUIRE(n <= v.capacities().first);
    REQUIRE(n * avg_s <= v.capacities().second);
    REQUIRE(alignof(std::max_align_t) == v.max_align());
}

TEST_CASE("reserve_reallocates_capacity_when_max_align_changes", "[poly_vector_basic_tests]")
{
    estd::poly_vector<Interface> v;
    constexpr size_t             n     = 16;
    constexpr size_t             avg_s = 64;
    v.reserve(n, avg_s);
    auto old_data = v.data();
    v.reserve(n, avg_s, 32);
    REQUIRE(old_data != v.data());
}

TEST_CASE("reserve_does_not_increase_capacity_when_size_is_less_than_current_"
          "capacity",
    "[poly_vector_basic_tests]")
{
    estd::poly_vector<Interface> v;
    constexpr size_t             n     = 16;
    constexpr size_t             avg_s = 8;
    v.reserve(n, avg_s);
    const auto capacities = v.capacities();
    v.reserve(n / 2, avg_s);
    REQUIRE(capacities == v.capacities());
}

TEST_CASE(
    "descendants_of_interface_can_be_pushed_back_into_the_vector", "[poly_vector_basic_tests]")
{
    estd::poly_vector<Interface> v;
    v.push_back(Impl1(3.14));
    v.push_back(Impl2());
    REQUIRE(2 == v.size());
}

TEST_CASE(
    "descendants of interface can be emplaced back into the vector", "[poly_vector_basic_tests]")
{
    estd::poly_vector<Interface> v;

    auto& first = v.emplace_back<Impl1>(3.14, false);

    REQUIRE(std::addressof(first) == std::addressof(v[0]));

    auto& second = v.emplace_back<Impl2>();

    REQUIRE(std::addressof(second) == std::addressof(v[1]));

    REQUIRE(2 == v.size());
}

TEST_CASE("is_not_copyable_with_no_cloning_policy", "[poly_vector_basic_tests]")
{
    estd::poly_vector<Interface, std::allocator<Interface>, estd::no_cloning_policy<Interface>>
        v {};
    v.reserve(2, std::max(sizeof(Impl1), sizeof(Impl2)), std::max(alignof(Impl1), alignof(Impl2)));
    v.push_back(Impl1(3.14));
    v.push_back(Impl2());
    REQUIRE_THROWS_AS(v.push_back(Impl2()), estd::no_cloning_exception);
}

template <typename T> bool is_aligned_properly(T& obj)
{
    return reinterpret_cast<std::uintptr_t>(&obj) % alignof(T) == 0;
}

TEST_CASE("objects_are_allocated_with_proper_alignment", "[poly_vector_basic_tests]")
{
    estd::poly_vector<Interface> v {};
    v.reserve(2, std::max(sizeof(Impl1), sizeof(Impl2)), std::max(alignof(Impl1), alignof(Impl2)));
    v.push_back(Impl1(3.14));
    v.push_back(Impl2());
    REQUIRE(is_aligned_properly(static_cast<Impl1&>(v[0])));
    REQUIRE(is_aligned_properly(static_cast<Impl2&>(v[1])));
}

TEST_CASE("no_cloning_policy_gives_e_what", "[poly_vector_basic_tests]")
{
    bool throwed = false;
    estd::poly_vector<Interface, std::allocator<Interface>, estd::no_cloning_policy<Interface>>
        v {};
    try {
        v.reserve(1, sizeof(Impl1));
        v.push_back(Impl1(3.14));
        v.push_back(Impl1(3.14));
    } catch (const estd::no_cloning_exception& e) {
        std::string emsg = e.what();
        throwed          = emsg.find("no_cloning_policy") != std::string::npos;
    }
    REQUIRE(throwed);
}

TEST_CASE("erase_from_begin_to_end_clears_the_vector", "[poly_vector_basic_tests]")
{
    estd::poly_vector<Interface> v {};
    v.push_back(Impl1(3.14));
    v.push_back(Impl2());
    auto it = v.erase(v.begin(), v.end());
    REQUIRE(it == v.end());
    REQUIRE(0 == v.size());
}

TEST_CASE(
    "erase_can_be_called_with_any_valid_iterator_to_a_vector_elem", "[poly_vector_basic_tests]")
{
    estd::poly_vector<Interface> v {};
    v.push_back(Impl1(3.14));
    v.push_back(Impl1(3.14));
    v.push_back(Impl2());
    v.push_back(Impl1(3.14));
    v.push_back(Impl2());
    v.push_back(Impl1(3.14));
    v.push_back(Impl2());
    const auto size = v.size();

    SECTION("when erase starts from the beginning"){
        auto it = v.erase(v.begin(), v.begin() + 2);
        auto expected_size = size - 2;
        REQUIRE(it == v.begin());
        REQUIRE(expected_size == v.size());
        REQUIRE_NOTHROW(dynamic_cast<Impl2&>(v[0]));
        REQUIRE_NOTHROW(dynamic_cast<Impl1&>(v[1]));
        REQUIRE_NOTHROW(dynamic_cast<Impl2&>(v[2]));
        REQUIRE_NOTHROW(dynamic_cast<Impl1&>(v[3]));
        REQUIRE_NOTHROW(dynamic_cast<Impl2&>(v[4]));
        SECTION("And then the vector is copied"){
            auto v_copy = v;
            auto v_copy_cap = v_copy.capacities();
            auto v_cap = v.capacities();
            REQUIRE(v.size() == v_copy.size());
            REQUIRE(v_copy_cap.first == v_cap.first);
            REQUIRE(v_copy_cap.second == v_cap.second);
        }
    }

    SECTION("when erase starts from the middle"){
        auto it = v.erase(v.begin()+2, v.begin() + 4);
        auto expected = v.begin() + 2;
        auto expected_size = size - 2;
        REQUIRE(it == expected);
        REQUIRE(expected_size == v.size());
        REQUIRE_NOTHROW(dynamic_cast<Impl1&>(v[0]));
        REQUIRE_NOTHROW(dynamic_cast<Impl1&>(v[1]));
        REQUIRE_NOTHROW(dynamic_cast<Impl2&>(v[2]));
        REQUIRE_NOTHROW(dynamic_cast<Impl1&>(v[3]));
        REQUIRE_NOTHROW(dynamic_cast<Impl2&>(v[4]));
        SECTION("And then the vector is copied"){
            auto v_copy = v;
            auto v_copy_cap = v_copy.capacities();
            auto v_cap = v.capacities();
            REQUIRE(v.size() == v_copy.size());
            REQUIRE(v_copy_cap.first == v_cap.first);
            REQUIRE(v_copy_cap.second == v_cap.second);
        }

    }

    SECTION("when erase starts from the middle and only one elem is erased"){
        auto it = v.erase(v.begin()+3, v.begin() + 4);
        auto expected = v.begin() + 3;
        auto expected_size = size - 1;
        int a;
        REQUIRE(it == expected);
        REQUIRE(expected_size == v.size());
        REQUIRE_NOTHROW(dynamic_cast<Impl1&>(v[0]));
        REQUIRE_NOTHROW(dynamic_cast<Impl1&>(v[1]));
        REQUIRE_NOTHROW(dynamic_cast<Impl2&>(v[2]));
        REQUIRE_NOTHROW(dynamic_cast<Impl2&>(v[3]));
        REQUIRE_NOTHROW(dynamic_cast<Impl1&>(v[4]));
        REQUIRE_NOTHROW(dynamic_cast<Impl2&>(v[5]));
        
        SECTION("And then the vector is copied"){
            auto v_copy = v;
            auto v_copy_cap = v_copy.capacities();
            auto v_cap = v.capacities();
            REQUIRE(v.size() == v_copy.size());
            REQUIRE(v_copy_cap.first == v_cap.first);
            REQUIRE(v_copy_cap.second == v_cap.second);
        }
    }
}

TEST_CASE("erase_includes_end_remove_elems_from_end", "[poly_vector_basic_tests]")
{
    estd::poly_vector<Interface> v {};
    v.push_back(Impl1(3.14));
    v.push_back(Impl2());
    auto id = v[0].getId();
    auto it = v.erase(++v.begin(), v.end());

    REQUIRE(it == v.end());
    REQUIRE(1 == v.size());
    REQUIRE(id == v[0].getId());
}

TEST_CASE("erase_from_end_position_is_same_as_pop_back", "[poly_vector_basic_tests]")
{
    estd::poly_vector<Interface> v {};
    v.push_back(Impl1(3.14));
    v.push_back(Impl2());
    auto id = v[0].getId();
    v.erase(++v.begin());
    REQUIRE(1 == v.size());
    REQUIRE(id == v[0].getId());
}

TYPE_P_TEST_CASE("poly vector modifiers test", "[poly_vector]", CloningPolicy,
    estd::virtual_cloning_policy<Interface>, estd::delegate_cloning_policy<Interface>)
{
    using vector = estd::poly_vector<Interface, std::allocator<Interface>, CloningPolicy>;

    Impl1                 obj1;
    Impl2T<CloningPolicy> obj2;
    vector                v;
    v.push_back(obj1);
    v.push_back(obj2);

    SECTION("back accesses the last element")
    {
        Interface& obj = obj2;
        REQUIRE(obj == v.back());
    }
    SECTION("front accesses the first element")
    {
        Interface& obj = obj1;
        REQUIRE(obj == v.front());
    }
    SECTION("pop_back_removes_the_last_element")
    {
        REQUIRE(obj2 == v.back());
        v.pop_back();
        REQUIRE(obj1 == v.back());
    }
    SECTION("pop_back_keeps_capacity_but_adjustes_size")
    {
        auto cap = v.capacities();

        v.pop_back();
        v.pop_back();

        REQUIRE(cap == v.capacities());
        REQUIRE(v.empty());
        REQUIRE(std::make_pair(size_t(0), size_t(0)) == v.sizes());
    }
    SECTION("clear_destroys_all_elements_but_keeps_capacity")
    {
        using Impl2 = Impl2T<CloningPolicy>;
        v.push_back(Impl2 {});
        v.push_back(Impl1 { 6.28 });
        auto cap = v.capacities();
        v.clear();
        REQUIRE(cap == v.capacities());
        REQUIRE(v.empty());
        REQUIRE(std::make_pair(size_t(0), size_t(0)) == v.sizes());
    }
    SECTION("at_operator_retrieves_nth_elem")
    {
        REQUIRE(static_cast<Interface&>(obj1) == v[0]);
        REQUIRE(static_cast<Interface&>(obj2) == v[1]);
    }
    SECTION("at_operator_throws_out_of_range_error_on_overindexing")
    {
        REQUIRE_NOTHROW(v.at(1));
        REQUIRE(static_cast<Interface&>(obj2) == v.at(1));
        REQUIRE_THROWS_AS(v.at(2), std::out_of_range);
    }
    SECTION("swap_swaps_the_contents_of_two_vectors")
    {
        using Impl2 = Impl2T<CloningPolicy>;
        vector v2;
        Impl1  obj1_2 { 6.28 };
        Impl2  obj2_2 {};
        v2.push_back(obj1_2);
        v2.push_back(obj2_2);

        v.swap(v2);

        REQUIRE(static_cast<Interface&>(obj1_2) == v[0]);
        REQUIRE(static_cast<Interface&>(obj2_2) == v[1]);
        REQUIRE(static_cast<Interface&>(obj1) == v2[0]);
        REQUIRE(static_cast<Interface&>(obj2) == v2[1]);
    }
    SECTION("copy_assignment_copies_contained_elems")
    {
        vector v2;

        v2 = v;

        REQUIRE(static_cast<Interface&>(obj1) == v2[0]);
        REQUIRE(static_cast<Interface&>(obj2) == v2[1]);
    }
    SECTION("move_assignment_moves_contained_elems")
    {
        vector v2;
        auto   size       = v2.sizes();
        auto   capacities = v2.capacities();

        v2 = std::move(v);

        REQUIRE(static_cast<Interface&>(obj1) == v2[0]);
        REQUIRE(static_cast<Interface&>(obj2) == v2[1]);
        REQUIRE(size == v.sizes());
        REQUIRE(capacities == v.capacities());
    }

    SECTION("strog_guarantee_when_there_is_an_exception_during_push_back_w_"
            "reallocation")
    {
        v.push_back(Impl1 {});
        v.back().set_throw_on_copy_construction(true);
        while (v.capacity() - v.size() > 0) {
            v.push_back(Impl1 {});
        }
        if (!std::is_same<CloningPolicy, estd::delegate_cloning_policy<Interface>> {}) {
            std::vector<uint8_t> ref_storage(
                static_cast<uint8_t*>(v.data().first), static_cast<uint8_t*>(v.data().second));
            auto data = v.data();
            auto size = v.sizes();
            auto caps = v.capacities();

            REQUIRE_THROWS_AS(v.push_back(Impl2 {}), std::exception);

            REQUIRE(data == v.data());
            REQUIRE(size == v.sizes());
            REQUIRE(caps == v.capacities());
            REQUIRE(0 == std::memcmp(ref_storage.data(), v.data().first, ref_storage.size()));
        } else {
            REQUIRE_NOTHROW(v.push_back(Impl2 {}));
        }
    }
    SECTION("basic_guarantee_when_there_is_an_exception_during_erase_and_last_"
            "element_is_not_included_in_erased_range")
    {
        v.push_back(Impl1 {});
        v.push_back(Impl1 {});
        v.push_back(Impl1 {});
        v.push_back(Impl1 {});
        v.push_back(Impl2 {});
        v.push_back(Impl1 {});
        v.push_back(Impl2 {});

        auto start = v.begin() + 3;
        auto end   = start + 1;

        if (!std::is_same<CloningPolicy, estd::delegate_cloning_policy<Interface>> {}) {
            v.at(5).set_throw_on_copy_construction(true);
            auto old_sizes = v.sizes();
            REQUIRE_THROWS_AS(v.erase(start, end), std::exception);
            REQUIRE(v.size() < old_sizes.first);
        } else {
            REQUIRE_NOTHROW(v.erase(start, end));
        }
    }

    SECTION("insert_at_end")
    {
        auto o        = Impl1();
        auto id       = o.getId();
        auto old_size = v.size();
        auto it       = v.insert(v.end(), std::move(o));
        REQUIRE(id == it->getId());
        REQUIRE(old_size + 1 == v.size());
    }
    SECTION("insert at begin")
    {
        auto o        = Impl1();
        auto id       = o.getId();
        auto old_size = v.size();
        auto it       = v.insert(v.begin(), std::move(o));
        REQUIRE(id == it->getId());
        REQUIRE(old_size + 1 == v.size());
    }
    SECTION("insert at the middle")
    {
        auto o        = Impl1();
        auto id       = o.getId();
        auto old_size = v.size();
        auto it       = v.insert(std::next(v.begin(), v.size() / 2), std::move(o));
        REQUIRE(id == it->getId());
        REQUIRE(old_size + 1 == v.size());
    }
}
