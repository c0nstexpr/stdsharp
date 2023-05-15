#include "test.h"
#include "stdsharp/memory/object_allocation.h"

using namespace stdsharp;
using namespace std;

using allocator_t = allocator<unsigned char>;

SCENARIO("object allocation basic requirements", "[memory][object allocation]") // NOLINT
{
    struct local
    {
        local() = default;
        ~local() = default;

    private:
        local(const local&) = default;
        local(local&&) = default;
        local& operator=(const local&) = default;
        local& operator=(local&&) = default;
    };

    using normal_t = normal_obj_allocation<allocator_t>;
    using unique_t = unique_obj_allocation<allocator_t>;
    using worst_t = obj_allocation_for<local, allocator_t>;

    STATIC_REQUIRE(default_initializable<obj_allocation_for<int, allocator_t>>);
    STATIC_REQUIRE(default_initializable<trivial_obj_allocation<allocator_t>>);
    STATIC_REQUIRE(default_initializable<normal_t>);
    STATIC_REQUIRE(default_initializable<unique_t>);

    STATIC_REQUIRE(nothrow_movable<unique_t>);
    STATIC_REQUIRE(nothrow_swappable<unique_t>);
    STATIC_REQUIRE(copyable<normal_t>);

    STATIC_REQUIRE(default_initializable<worst_t>);
    STATIC_REQUIRE(nothrow_movable<worst_t>);
    STATIC_REQUIRE(nothrow_swappable<worst_t>);
}

SCENARIO("object allocation assign value", "[memory][object allocation]") // NOLINT
{
    GIVEN("a normal object allocation")
    {
        normal_obj_allocation<allocator_t> allocation;

        WHEN("emplace an int value")
        {
            auto value = allocation.emplace<int>(1);

            THEN("the return value should correct") { REQUIRE(value == 1); }

            AND_THEN("type should be expected") { REQUIRE(allocation.type() == type_id<int>); }
        }

        WHEN("emplace an int vector")
        {
            const auto list = {1, 2};
            const auto& value = allocation.emplace(vector<int>{list});

            THEN("the return value should correct")
            {
                REQUIRE_THAT(value, Catch::Matchers::RangeEquals(list));
            }

            AND_THEN("type should be expected")
            {
                REQUIRE(allocation.type() == type_id<vector<int>>);
            }
        }

        auto invoked = 0u;

        struct local : reference_wrapper<unsigned>
        {
            local(unsigned& value): reference_wrapper(value) { ++get(); }
        };

        WHEN("assign custom type twice")
        {
            INFO(fmt::format("custom type: {}", type_id<local>));

            allocation.emplace<local>(invoked);
            allocation.emplace<local>(invoked);

            THEN("assign operator should be invoked") { REQUIRE(invoked == 2); }

            AND_THEN("destroy allocation and check content")
            {
                allocation.destroy();
                REQUIRE(!allocation);
            }
        }
    }
}

SCENARIO("constexpr object allocation", "[memory][object allocation]") // NOLINT
{
    STATIC_REQUIRE(
        []
        {
            trivial_obj_allocation<allocator<int>> allocation{};
            auto& value = allocation.emplace(1);
            value = 42;
            return allocation.get<int>();
        }() == 42
    );
}

template<typename T>
static constexpr auto value()
{
    return allocation_obj_req{} >= allocation_value_type_req<allocator_t, T>;
}

template<allocation_obj_req Req, typename T>
    requires(Req >= allocation_value_type_req<allocator_t, ::std::decay_t<T>>)
void bar()
{
}

void foo() { bar<allocation_obj_req{}, int>(); }