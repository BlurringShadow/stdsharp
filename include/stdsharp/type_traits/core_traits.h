// Created by BlurringShadow at 2021-03-01-下午 9:00

#pragma once

#include <array>
#include <string_view>
#include <functional>

#include <range/v3/utility/static_const.hpp>
#include <meta/meta.hpp>

using namespace ::std::literals;

namespace stdsharp::type_traits
{
    template<typename...>
    constexpr auto always_false(const auto&...) noexcept
    {
        return false;
    }

    using ignore_t = decltype(::std::ignore);

    inline constexpr struct empty_t : ignore_t
    {
        using ignore_t::operator=;

        constexpr empty_t(const auto&...) noexcept {}
    } empty;

    template<typename T>
    inline constexpr ::std::type_identity<T> type_identity_v{};

    template<typename T>
    using add_const_lvalue_ref_t = ::std::add_lvalue_reference_t<::std::add_const_t<T>>;

    template<auto Value>
    using constant = ::std::integral_constant<decltype(Value), Value>;

    template<auto Func, auto... Args>
        requires ::std::invocable<decltype(Func), decltype(Args)...>
    static constexpr decltype(auto) invoke_result = ::std::invoke(Func, Args...);

    template<auto Value>
    inline constexpr auto constant_v = Value;

    template<typename T>
    constexpr const auto& static_const_v = ::ranges::static_const<T>::value;

    template<bool conditional, auto Left, auto>
    inline constexpr auto conditional_v = Left;

    template<auto Left, auto Right>
    inline constexpr auto conditional_v<false, Left, Right> = Right;

    template<typename T>
    concept constant_value = requires
    {
        ::std::bool_constant<(::std::declval<T>().value, true)>{};
    };

    template<::std::size_t I>
    using index_constant = ::std::integral_constant<::std::size_t, I>;

    template<typename T>
    struct type_constant : ::std::type_identity<T>
    {
        template<template<typename> typename OtherT, typename U>
        friend constexpr auto operator==(const type_constant, const OtherT<U>&) noexcept
        {
            return ::std::same_as<T, U>;
        }

        template<template<typename> typename OtherT, typename U>
        friend constexpr auto operator!=(const type_constant l, const OtherT<U>& r) noexcept
        {
            return !(l == r);
        }
    };

    template<typename T>
    inline constexpr type_constant<T> type_constant_v{};

    template<typename T>
    using persist_t =
        ::std::conditional_t<::std::is_rvalue_reference_v<T>, ::std::remove_reference_t<T>, T&>;

    template<auto... V>
    struct regular_value_sequence
    {
        static constexpr auto size() noexcept { return sizeof...(V); }
    };

    template<typename T, typename U>
    using ref_align_t = ::std::conditional_t<
        ::std::is_lvalue_reference_v<T>,
        ::std::add_lvalue_reference_t<U>, // clang-format off
        ::std::conditional_t<::std::is_rvalue_reference_v<T>, ::std::add_rvalue_reference_t<U>, U>
    >; // clang-format on

    template<typename T, typename U>
    using const_align_t = ::std::conditional_t<
        ::std::is_const_v<::std::remove_reference_t<T>>,
        ::std::add_const_t<U>, // clang-format off
        U
    >; // clang-format on

    template<typename T, typename U>
    using const_ref_align_t = ref_align_t<T, const_align_t<T, U>>;

    template<typename... T>
    using regular_type_sequence = ::meta::list<T...>;

    inline namespace literals
    {
        template<::std::size_t Size>
        struct ltr : ::std::array<char, Size>
        {
        private:
            using array_t = const char (&)[Size]; // NOLINT(*-avoid-c-arrays)

        public:
            using base = ::std::array<char, Size>;
            using base::base;

            constexpr ltr(array_t arr) noexcept: base(::std::to_array(arr)) {}

            constexpr ltr& operator=(array_t arr) noexcept { *this = ::std::to_array(arr); }

            constexpr operator ::std::string_view() const noexcept
            {
                return {base::data(), Size - 1};
            }

            [[nodiscard]] constexpr auto to_string_view() const noexcept
            {
                return static_cast<::std::string_view>(*this); //
            }
        };

        template<::std::size_t Size>
        ltr(const char (&)[Size]) -> ltr<Size>; // NOLINT(*-avoid-c-arrays)

        template<ltr ltr>
        [[nodiscard]] constexpr auto operator"" _ltr() noexcept
        {
            return ltr;
        }
    }
}

namespace meta::extension
{
    template<invocable Fn, template<auto...> typename T, auto... V>
    struct apply<Fn, T<V...>> : lazy::invoke<Fn, ::stdsharp::type_traits::constant<V>...>
    {
    };
}

namespace stdsharp::inline literals
{
    using namespace stdsharp::type_traits::literals;
}