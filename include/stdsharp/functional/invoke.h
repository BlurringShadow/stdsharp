#pragma once

#include "invocables.h"

namespace stdsharp::functional
{
    inline constexpr auto empty_invoke = [](const auto&...) noexcept
    {
        return type_traits::empty; //
    };

    using empty_invoke_fn = decltype(empty_invoke);

    inline constexpr sequenced_invocables optional_invoke{::ranges::invoke, empty_invoke};

    template<typename... Args>
    concept nothrow_optional_invocable = noexcept(optional_invoke(::std::declval<Args>()...));

    template<bool Condition>
    struct conditional_invoke_fn
    {
        template<::std::invocable Func>
        requires Condition constexpr decltype(auto) operator()(Func&& func, const auto&) const
            noexcept(concepts::nothrow_invocable<Func>)
        {
            return func();
        }

        template<::std::invocable Func>
        constexpr decltype(auto) operator()(const auto&, Func&& func) const
            noexcept(concepts::nothrow_invocable<Func>)
        {
            return func();
        }
    };

    template<bool Condition>
    inline constexpr conditional_invoke_fn<Condition> conditional_invoke{};

    template<bool Condition, typename T, typename U>
    concept conditional_invocable = ::std::invocable<conditional_invoke_fn<Condition>, T, U>;

    template<bool Condition, typename T, typename U>
    concept nothrow_conditional_invocable =
        concepts::nothrow_invocable<conditional_invoke_fn<Condition>, T, U>;

    template<concepts::not_same_as<void> ReturnT>
    inline constexpr nodiscard_invocable invoke_r(
        []<typename Func, typename... Args>(Func&& func, Args&&... args) //
        noexcept(concepts::nothrow_invocable_r<Func, ReturnT, Args...>) -> ReturnT //
        requires concepts::invocable_r<Func, ReturnT, Args...> //
        {
            return ::std::invoke(::std::forward<Func>(func), ::std::forward<Args>(args)...); //
        } //
    );

    inline constexpr nodiscard_invocable returnable_invoke(
        []<typename Func, typename... Args> // clang-format off
            requires ::std::invocable<Func, Args...>
        (Func&& func, Args&&... args)
            noexcept(concepts::nothrow_invocable<Func, Args...>)
            ->decltype(auto) // clang-format on
        {
            const auto invoker = [&]() -> decltype(auto)
            {
                return ::std::invoke(::std::forward<Func>(func), ::std::forward<Args>(args)...); //
            };
            if constexpr(::std::same_as<::std::invoke_result_t<decltype(invoker)>, void>)
            {
                invoker();
                return type_traits::empty;
            } // clang-format off
            else return invoker(); // clang-format on
        } //
    );

    template<template<typename...> typename Tuple = ::std::tuple>
    inline constexpr nodiscard_invocable merge_invoke( //
        []<::std::invocable... Func>(Func&&... func) noexcept( //
            noexcept( // clang-format off
                Tuple<::std::invoke_result_t<
                    decltype(returnable_invoke),
                    Func>...
                 >{returnable_invoke(::std::forward<Func>(func))...}
            )
        ) -> Tuple<
            ::std::invoke_result_t<
                decltype(returnable_invoke),
                Func
            >...
        > // clang-format on
        {
            return {returnable_invoke(::std::forward<Func>(func))...}; //
        } //
    );
}