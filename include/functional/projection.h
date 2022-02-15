#pragma once
#include "utility/value_wrapper.h"
#include "functional/invocables.h"
#include "functional/cpo.h"

namespace stdsharp::functional
{
    inline constexpr struct projected_invoke_fn
    {
    private:
        struct impl
        {
            template<typename Fn, typename Projector, typename... Args>
                requires(
                    (::std::invocable<Projector&, Args> && ...) &&
                    ::std::invocable<Fn, ::std::invoke_result_t<Projector, Args>...> //
                )
            constexpr decltype(auto) operator()(Fn&& fn, Projector projector, Args&&... args) const
                noexcept(
                    concepts::nothrow_invocable<Fn, ::std::invoke_result_t<Projector, Args>...> //
                )

            {
                return ::std::invoke(
                    ::std::forward<Fn>(fn),
                    ::std::invoke(projector, ::std::forward<Args>(args))... //
                );
            }
        };

    public:
        template<typename... Args>
            requires(
                !functional::cpo_invocable<projected_invoke_fn, Args...> &&
                ::std::invocable<impl, Args...> //
            )
        constexpr decltype(auto) operator()(Args&&... args) const //
        {
            return impl{}(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires functional::cpo_invocable<projected_invoke_fn, Args...>
        constexpr decltype(auto) operator()(Args&&... args) const
        {
            return functional::cpo(*this, ::std::forward<Args>(args)...);
        }

    } projected_invoke{};

    template<typename... Args>
    concept projected_invocable = ::std::invocable<projected_invoke_fn, Args...>;

    template<typename... Args>
    concept projected_nothrow_invocable = concepts::nothrow_invocable<projected_invoke_fn, Args...>;

    template<typename Proj>
    struct projector : value_wrapper<Proj>
    {
        using base = value_wrapper<Proj>;
        using base::base;

#define BS_OPERATOR(const_, ref)                                                         \
    template<typename Func, typename... Args>                                            \
        requires(projected_invocable<Func, const_ Proj ref, Args...>)                    \
    constexpr decltype(auto) operator()(Func&& func, Args&&... args)                     \
        const_ ref noexcept(projected_nothrow_invocable<Func, const_ Proj ref, Args...>) \
    {                                                                                    \
        return projected_invoke(                                                         \
            ::std::forward<Func>(func),                                                  \
            static_cast<const_ Proj ref>(value_wrapper<Proj>::value),                    \
            ::std::forward<Args>(args)...);                                              \
    }

        BS_OPERATOR(, &)
        BS_OPERATOR(const, &)
        BS_OPERATOR(, &&)
        BS_OPERATOR(const, &&)

#undef BS_OPERATOR
    };

    inline constexpr auto make_projector = make_trivial_invocables(
        nodiscard_tag,
        []<typename Func>(Func&& func) //
        noexcept(noexcept(projector{::std::forward<Func>(func)}))
        {
            return projector{::std::forward<Func>(func)}; //
        } //
    );

    using make_projector_fn = decltype(make_projector);
}