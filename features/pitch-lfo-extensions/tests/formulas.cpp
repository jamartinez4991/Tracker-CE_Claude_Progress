// Host-side checks of the formulas used by pitch-lfo-extensions (copies of the expressions in the
// firmware sources; they verify the maths, not the firmware build).
#include <cassert>
#include <cmath>
#include <cstdio>
static float ccToDepth(int cc) { return cc <= 64 ? (cc - 64) / 64.0f : (cc - 64) / 63.0f; }
static float shape(float c, bool uni, float depth) { return (uni ? (c + 1) * 0.5f : c) * depth; }
static float combined(int st, int ct) { return (st * 100 + ct) / 100.0f; }
int main()
{
    assert(ccToDepth(0) == -1.0f && ccToDepth(64) == 0.0f && ccToDepth(127) == 1.0f);
    float prev = -2; for (int i = 0; i <= 127; i++) { float v = ccToDepth(i); assert(v > prev); prev = v; }
    assert(combined(12, 50) == 12.5f && combined(-12, -50) == -12.5f && combined(12, -50) == 11.5f);
    assert(combined(1, -100) == 0.0f && combined(0, 75) == 0.75f && combined(24, 100) == 25.0f);
    // unipolar is a linear map, not rectification
    assert(shape(-1, true, 1) == 0 && shape(0, true, 1) == 0.5f && shape(1, true, 1) == 1);
    assert(shape(-1, true, -1) == 0 && shape(1, true, -1) == -1);          // unipolar, -100 %
    assert(shape(1, false, -1) == -1 && shape(-1, false, -1) == 1);        // bipolar inverted
    assert(shape(0.3f, false, 0) == 0);
    // pitch ratio
    assert(std::fabs(std::exp2(100.0 / 1200) - 1.0594631) < 1e-6);
    assert(std::fabs(std::exp2(2500.0 / 1200) - 4.2378) < 1e-3);           // +25 st
    // update() wrapper: modulation never accumulates into base; glide advance is converted back
    double base = 2.0, ratio = std::exp2(7.0 / 12), smooth = 0.001;
    for (int blk = 0; blk < 1000; blk++) {
        double eff0 = base * ratio, eff = eff0, es = smooth * ratio;
        for (int i = 0; i < 128; i++) eff += es;                           // glide inside the block
        base = base + (eff - eff0) / ratio;
    }
    double expect = 2.0 + 1000 * 128 * 0.001;
    assert(std::fabs(base - expect) < 1e-6);
    float b2 = 2.0f; for (int blk = 0; blk < 1000; blk++) { float e = b2 * ratio; b2 = e / ratio; }
    assert(std::fabs(b2 - 2.0f) < 1e-3f);                                  // no drift without glide
    puts("formulas OK");
}
