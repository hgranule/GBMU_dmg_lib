/**
 * @file GB_clock.h
 * @brief File describes some functionality to deal with clock cycles
 */

#ifndef COMMON_GB_CLOCK_H_
# define COMMON_GB_CLOCK_H_

# include <algorithm>
# include <type_traits>

using clk_cycle_t = int64_t;  ///< type for representing clock cycles

constexpr static auto MCYCLE_TO_CLK_CYCLE = 0x4;

constexpr inline uint64_t
operator "" _CLKCycles(uint64_t cCycles) { return cCycles; }    // TODO(hgranule) are cCycles correct by code style

constexpr inline uint64_t
operator "" _MCycles(uint64_t mCycles) { return mCycles * MCYCLE_TO_CLK_CYCLE; }

namespace devsync {

/**
 * @brief returns true when we have cycles to do something
 */
inline bool
is_ready(const clk_cycle_t clock) {
    return clock <= 0;   // TODO i changed it so the counter starts working
}

/**
 * @brief pay for action
 */
inline void
pay(clk_cycle_t& clock, clk_cycle_t ccls) {
    clock -= ccls;
}

/**
 * @brief give cycles to a clock
 */
inline void
step(clk_cycle_t& clock, clk_cycle_t ccls) {
    clock = std::max(clock + ccls, clk_cycle_t(0));  // TODO(hgranule) you told we use positive cycles when call step,
                                                     //                but in this case clock will always 0
                                                     //                also pay function takes away clocks so it's error
}

/**
 * @brief counter class which counts passed cycles and could be used for action synchronization
 */
class counter_t {
 protected:
    clk_cycle_t __counter;

 public:
    explicit
    counter_t(clk_cycle_t init_val = 0) : __counter(init_val) {}

    inline void
    step(clk_cycle_t time_for_step = 4_CLKCycles) {
        ::devsync::step(__counter, time_for_step);
    }

    inline void
    pay(clk_cycle_t time_for_action) {
        ::devsync::pay(__counter, time_for_action);
    }

    inline bool
    is_ready() const {
        return ::devsync::is_ready(__counter);
    }

    inline operator clk_cycle_t&() {
        return __counter;
    }
};

/**
 * @brief synchronized to clock decorator for a synced action
 */
template< clk_cycle_t _cyclesPrice >
struct Action {

    /** Decorator for a not method function */
    template<typename _Func, typename... _CBArgs>
    inline auto
    operator()(clk_cycle_t& clock, _Func cbFunc, _CBArgs... cbArgs) const {
        pay(clock, _cyclesPrice);
        return cbFunc(cbArgs...);
    }

    /** Decorator for a method function */
    template<typename _Ret, class _Class, typename... _CBArgs>
    inline _Ret
    operator()(clk_cycle_t& clock, _Ret (_Class::*cbFunc)(_CBArgs...),
                           _Class* cbOwner, _CBArgs... cbArgs) const {

        pay(clock, _cyclesPrice);
        return (cbOwner->*cbFunc)(cbArgs...);
    }

};

}  // namespace devsync

#endif  // COMMON_GB_CLOCK_H_
