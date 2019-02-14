// Copyright 2017-2019 Elias Kosunen
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// This file is a part of scnlib:
//     https://github.com/eliaskosunen/scnlib

#include <doctest.h>
#include <scn/scn.h>

struct non_default_constructible {
    non_default_constructible(int v) : val(v) {}

    operator int() const
    {
        return val;
    }

    int val;
};

template <typename T, size_t N>
struct small_vector_size_construct {
    static scn::detail::small_vector<T, N> make(size_t n)
    {
        return scn::detail::small_vector<T, N>(n);
    }
};
template <size_t N>
struct small_vector_size_construct<non_default_constructible, N> {
    static scn::detail::small_vector<non_default_constructible, N> make(
        size_t n)
    {
        return scn::detail::small_vector<non_default_constructible, N>(
            n, non_default_constructible{0});
    }
};

TEST_CASE_TEMPLATE_DEFINE("small_vector", T, small_vector_test)
{
    SUBCASE("default construct stack")
    {
        scn::detail::small_vector<T, 64> vec;
        CHECK(vec.is_small());
        CHECK(vec.size() == 0);
        CHECK(vec.capacity() == 64);
    }
    SUBCASE("default construct heap")
    {
        scn::detail::small_vector<T, 0> vec;
        CHECK(!vec.is_small());
        CHECK(vec.size() == 0);
        CHECK(vec.capacity() == 0);
        CHECK(vec.data() == nullptr);
    }

    SUBCASE("size construct stack")
    {
        auto vec = small_vector_size_construct<T, 64>::make(32);
        CHECK(vec.is_small());
        CHECK(vec.size() == 32);
        CHECK(vec.capacity() == 64);

        CHECK(vec.front() == 0);
        CHECK(vec.back() == 0);
        CHECK(*vec.begin() == vec.front());
        CHECK(
            std::all_of(vec.begin(), vec.end(), [](int v) { return v == 0; }));
    }
    SUBCASE("size construct heap")
    {
        auto vec = small_vector_size_construct<T, 64>::make(128);
        CHECK(!vec.is_small());
        CHECK(vec.size() == 128);
        CHECK(vec.capacity() >= vec.size());

        CHECK(vec.front() == 0);
        CHECK(vec.back() == 0);
        CHECK(*vec.begin() == vec.front());
        CHECK(
            std::all_of(vec.begin(), vec.end(), [](int v) { return v == 0; }));
    }

    SUBCASE("size+value construct stack")
    {
        scn::detail::small_vector<T, 64> vec(32, 42);
        CHECK(vec.is_small());
        CHECK(vec.size() == 32);
        CHECK(vec.capacity() == 64);

        CHECK(vec.front() == 42);
        CHECK(vec.back() == 42);
        CHECK(*vec.begin() == vec.front());
        CHECK(
            std::all_of(vec.begin(), vec.end(), [](int v) { return v == 42; }));
    }
    SUBCASE("size+value construct heap")
    {
        scn::detail::small_vector<T, 64> vec(128, 42);
        CHECK(!vec.is_small());
        CHECK(vec.size() == 128);
        CHECK(vec.capacity() >= vec.size());

        CHECK(vec.front() == 42);
        CHECK(vec.back() == 42);
        CHECK(*vec.begin() == vec.front());
        CHECK(
            std::all_of(vec.begin(), vec.end(), [](int v) { return v == 42; }));
    }

    SUBCASE("accessors stack")
    {
        scn::detail::small_vector<T, 64> vec(16, 42);
        CHECK(vec.front() == 42);
        CHECK(vec.back() == 42);
        CHECK(vec[0] == vec.front());
        CHECK(vec[vec.size() - 1] == vec.back());
        CHECK(&vec[0] == vec.data());
        CHECK(&vec.front() == vec.data());
    }
    SUBCASE("accessors heap")
    {
        scn::detail::small_vector<T, 64> vec(128, 42);
        CHECK(vec.front() == 42);
        CHECK(vec.back() == 42);
        CHECK(vec[0] == vec.front());
        CHECK(vec[vec.size() - 1] == vec.back());
        CHECK(&vec[0] == vec.data());
        CHECK(&vec.front() == vec.data());
    }

    SUBCASE("capacity stack")
    {
        scn::detail::small_vector<T, 64> vec;
        CHECK(vec.empty());
        CHECK(vec.size() == 0);
        CHECK(vec.capacity() == 64);
        CHECK(vec.max_size() == std::numeric_limits<size_t>::max());

        vec = small_vector_size_construct<T, 64>::make(16);
        CHECK(!vec.empty());
        CHECK(vec.size() == 16);
        CHECK(vec.capacity() == 64);
        CHECK(vec.max_size() == std::numeric_limits<size_t>::max());
    }
    SUBCASE("capacity heap")
    {
        scn::detail::small_vector<T, 0> vec;
        CHECK(vec.empty());
        CHECK(vec.size() == 0);
        CHECK(vec.capacity() == 0);
        CHECK(vec.max_size() == std::numeric_limits<size_t>::max());

        vec = small_vector_size_construct<T, 0>::make(16);
        CHECK(!vec.empty());
        CHECK(vec.size() == 16);
        CHECK(vec.capacity() >= vec.size());
        CHECK(vec.max_size() == std::numeric_limits<size_t>::max());
    }

    SUBCASE("push_back stack")
    {
        scn::detail::small_vector<T, 64> vec;
        vec.push_back(1);
        CHECK(vec.size() == 1);
        CHECK(vec.back() == 1);

        vec.push_back(2);
        CHECK(vec.size() == 2);
        CHECK(vec.back() == 2);
    }
    SUBCASE("push_back overflow")
    {
        auto vec = small_vector_size_construct<T, 64>::make(64);
        CHECK(vec.is_small());

        vec.push_back(1);
        CHECK(vec.size() == 65);
        CHECK(vec.capacity() >= vec.size());
        CHECK(vec.back() == 1);
        CHECK(!vec.is_small());

        vec.push_back(2);
        CHECK(vec.size() == 66);
        CHECK(vec.back() == 2);
    }
    SUBCASE("push_back heap")
    {
        scn::detail::small_vector<T, 0> vec;
        vec.push_back(1);
        CHECK(vec.size() == 1);
        CHECK(vec.back() == 1);

        vec.push_back(2);
        CHECK(vec.size() == 2);
        CHECK(vec.back() == 2);
    }

    SUBCASE("reserve")
    {
        scn::detail::small_vector<T, 64> vec;
        CHECK(vec.size() == 0);
        CHECK(vec.capacity() == 64);
        CHECK(vec.is_small());

        vec.reserve(64);
        CHECK(vec.size() == 0);
        CHECK(vec.capacity() == 64);
        CHECK(vec.is_small());

        vec.reserve(256);
        CHECK(vec.size() == 0);
        CHECK(vec.capacity() == 256);
        CHECK(!vec.is_small());
    }

    SUBCASE("shrink_to_fit")
    {
        auto vec = small_vector_size_construct<T, 64>::make(64);
        vec.shrink_to_fit();
        CHECK(vec.size() == 64);
        CHECK(vec.capacity() == 64);
        CHECK(vec.is_small());

        vec.push_back(1);
        vec.shrink_to_fit();
        CHECK(vec.size() == 65);
        CHECK(vec.capacity() >= vec.size());
        CHECK(!vec.is_small());

        vec = scn::detail::small_vector<T, 64>();
        vec.reserve(64);
        vec.shrink_to_fit();
        CHECK(vec.size() == 0);
        CHECK(vec.capacity() == 64);
        CHECK(vec.is_small());

        vec.reserve(256);
        vec.shrink_to_fit();
        CHECK(vec.size() == 0);
        CHECK(vec.capacity() == 64);
        CHECK(vec.is_small());
    }
}

TYPE_TO_STRING(non_default_constructible);
TEST_CASE_TEMPLATE_INSTANTIATE(small_vector_test,
                               signed char,
                               int,
                               unsigned,
                               long long,
                               unsigned long long,
                               non_default_constructible);
