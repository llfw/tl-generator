#include <tl/generator.hpp>
#include <catch2/catch.hpp>
#include <ranges>

template <class T>
tl::generator<T> firstn(T t) {
   T num = T{ 0 };
   while (num < t) {
      co_yield num;
      ++num;
   }
}

TEST_CASE("firstn") {
   auto i = 0;
   for (auto n : firstn(20)) {
      REQUIRE(n == i);
      ++i;
   }
}

template <class T>
tl::generator<T> iota(T t = T{ 0 }) {
   while (true) {
      co_yield t;
      t++;
   }
}

TEST_CASE("iota") {
   auto i = 0;
   for (auto n : iota<int>() | std::views::take(10)) {
      REQUIRE(n == i);
      ++i;
   }
   for (auto n : iota(10) | std::views::take(10)) {
      REQUIRE(n == i);
      ++i;
   }
}

#include <vector>
#include <string>
#include <string_view>

tl::generator<std::vector<std::string>> split_by_lines_and_whitespace(std::string_view sv) {
   std::vector<std::string> res;

   auto start = sv.begin();
   auto pos = sv.begin();

   while (pos != sv.end()) {
      if (*pos == ' ') {
         res.push_back(std::string(start, pos));
         start = pos+1;
      }
      if (*pos == '\n') {
         res.push_back(std::string(start, pos));
         co_yield res;
         res.clear();
         start = pos+1;
      }
      ++pos;
   }
}

template <class Range>
tl::generator<std::pair<std::size_t, std::ranges::range_reference_t<Range>>> enumerate(Range&& r) {
   std::size_t i = 0;
   for (auto&& val : std::forward<Range>(r)) {
      std::pair<std::size_t, std::ranges::range_reference_t<Range>> pair {i, std::forward<decltype(val)>(val)};
      co_yield pair;
      ++i;
   }
}

TEST_CASE("split") {
   auto string = R"(one two three
four five six
seven eight nine)";

   std::vector<std::vector<std::string>> result = {
      {"one", "two", "three"},
      {"four", "five", "six"},
      {"seven", "eight", "nine"}
   };
   auto gen = split_by_lines_and_whitespace(string);
   for (auto&& [i, val] : enumerate(gen)) {
      REQUIRE(val == result[i]);
   }
}

tl::generator<const char*> generate() {
   co_yield "one";
   co_yield "two";
   co_yield "three";
}

TEST_CASE("pointers") {
   std::vector<std::string> result = {
      "one", "two", "three"
   };
   std::size_t i = 0;
   for (auto&& val : generate()) {
      REQUIRE(val == result[i]);
      ++i;
   }
}

TEST_CASE("values") {
   auto ints = []() -> tl::generator<int> {
      co_yield 1;
      co_yield 2;
      co_yield 3;
   };

   std::vector<int> result = {1, 2, 3};
   std::size_t i = 0;
   for (auto &&val: ints()) {
      REQUIRE(val == result[i]);
      ++i;
   }
}

TEST_CASE("references") {
   int one = 1;
   int two = 2;
   int three = 3;

   auto references = [&]() -> tl::generator<int&> {
      co_yield one;
      co_yield two;
      co_yield three;
   };

   auto range = references();
   auto it = range.begin();
   REQUIRE(&*it == &one);
   ++it;
   REQUIRE(&*it == &two);
   ++it;
   REQUIRE(&*it == &three);
   ++it;
   REQUIRE(it == range.end());
}

TEST_CASE("const references") {
   const int one = 1;
   const int two = 2;
   const int three = 3;

   auto references = [&]() -> tl::generator<const int&> {
      co_yield one;
      co_yield two;
      co_yield three;
   };

   auto range = references();
   auto it = range.begin();
   REQUIRE(&*it == &one);
   ++it;
   REQUIRE(&*it == &two);
   ++it;
   REQUIRE(&*it == &three);
   ++it;
   REQUIRE(it == range.end());
}

struct move_only {
   explicit move_only(int i) : value(i) {}

   move_only(move_only&&) = default;
   move_only& operator=(move_only&&) = default;
   move_only(const move_only&) = delete;
   move_only& operator=(const move_only&) = delete;

   int value;
};

TEST_CASE("move-only objects") {
   auto references = [&]() -> tl::generator<move_only> {
      co_yield move_only(1);
      co_yield move_only(2);
      co_yield move_only(3);
   };

   auto range = references();
   auto it = range.begin();
   REQUIRE(it->value == 1);
   ++it;
   REQUIRE(it->value == 2);
   ++it;
   REQUIRE(it->value == 3);
   ++it;
   REQUIRE(it == range.end());
}
