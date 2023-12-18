#include "clib/Flexfixed.hpp"
#include "clib/Flexfloat.hpp"
#include "clib/logs.hpp"

namespace clib
{

// void Flexfloat::convert_fx_to_ff(const Flexfixed &value, Flexfloat &res)
// {
// #ifdef EN_LOGS
//     CLOG(trace) << "Convertion from Flexfixed to Flexfloat:";
//     CLOG(trace) << "Value: " << value;
//     CLOG(trace) << "Result hyperparams: " << res;
// #endif

// // res.e = Flexfloat::msb(res.get_e()) + res.get_B() - value.get_F();

// // res.m =
// #ifdef EN_LOGS
//     CLOG(trace) << "Result: " << res;
// #endif
// }

// void Flexfixed::convert_ff_to_fx(const Flexfloat &value, Flexfixed &res)
// {
//     // todo
//     assert(false);
// }

} // namespace clib