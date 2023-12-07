#ifndef DEFER_HPP
#define DEFER_HPP

// credit: https://stackoverflow.com/questions/32432450/what-is-standard-defer-finalizer-implementation-in-c
struct defer_dummy {};
template <class F> struct deferrer { F f; ~deferrer() { f(); } };
template <class F> deferrer<F> operator*(defer_dummy, F f) { return {f}; }
#define DEFER_(LINE) _defer_##LINE
#define DEFER(LINE) DEFER_(LINE)
// defer with noexcept (regular):
#define defer auto DEFER(__LINE__) = defer_dummy{} * [&]() noexcept -> void
// defer with exceptions (just in case you need it for some reason):
#define defer_ex auto DEFER(__LINE__) = defer_dummy{} * [&]() -> void

#endif
