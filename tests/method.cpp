#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(method, arity)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
class Foo {
  method0() { return "no args"; }
  method1(a) { return a; }
  method2(a, b) { return a + b; }
  method3(a, b, c) { return a + b + c; }
  method4(a, b, c, d) { return a + b + c + d; }
  method5(a, b, c, d, e) { return a + b + c + d + e; }
  method6(a, b, c, d, e, f) { return a + b + c + d + e + f; }
  method7(a, b, c, d, e, f, g) { return a + b + c + d + e + f + g; }
  method8(a, b, c, d, e, f, g, h) { return a + b + c + d + e + f + g + h; }
}
var foo = Foo();
r += foo.method0(); 
r += foo.method1(1); 
r += foo.method2(1, 2); 
r += foo.method3(1, 2, 3); 
r += foo.method4(1, 2, 3, 4);
r += foo.method5(1, 2, 3, 4, 5); 
r += foo.method6(1, 2, 3, 4, 5, 6);
r += foo.method7(1, 2, 3, 4, 5, 6, 7); 
r += foo.method8(1, 2, 3, 4, 5, 6, 7, 8);
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 9);
  ASSERT_EQ(v[0], "no args");
  ASSERT_EQ(v[1], 1);
  ASSERT_EQ(v[2], 3);
  ASSERT_EQ(v[3], 6);
  ASSERT_EQ(v[4], 10);
  ASSERT_EQ(v[5], 15);
  ASSERT_EQ(v[6], 21);
  ASSERT_EQ(v[7], 28);
  ASSERT_EQ(v[8], 36);
}

TEST(method, empty_block)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class Foo {
  bar() {}
}
return Foo().bar();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, nil);
}

TEST(method, extra_arguments)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class Foo {
  method(a, b) {
    this.a = a;
    this.b = b;
  }
}
Foo().method(1, 2, 3, 4);
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(method, missing_arguments)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class Foo {
  method(a, b) {}
}
Foo().method(1);
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(method, not_found)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class Foo {}
Foo().unknown();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(method, get_bound_method)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class Foo {
  method() { }
}
var foo = Foo();
return foo.method;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  auto is_method = v.is<std::pair<Instance*, Subroutine*>>();
  ASSERT_TRUE(is_method);
  auto method = v.get<std::pair<Instance*, Subroutine*>>();
  ASSERT_NE(method.first, nullptr);
  ASSERT_NE(method.second, nullptr);
}

TEST(method, refer_to_name)
{
  auto [res, chunk] = compile(R"(
class Foo {
  method() {
    return method;
  }
}
Foo().method();
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(method, too_many_arguments)
{
  auto [res, chunk] = compile(R"(
{
  var a = 1;
  true.method(
     a, # 1
     a, # 2
     a, # 3
     a, # 4
     a, # 5
     a, # 6
     a, # 7
     a, # 8
     a, # 9
     a, # 10
     a, # 11
     a, # 12
     a, # 13
     a, # 14
     a, # 15
     a, # 16
     a, # 17
     a, # 18
     a, # 19
     a, # 20
     a, # 21
     a, # 22
     a, # 23
     a, # 24
     a, # 25
     a, # 26
     a, # 27
     a, # 28
     a, # 29
     a, # 30
     a, # 31
     a, # 32
     a, # 33
     a, # 34
     a, # 35
     a, # 36
     a, # 37
     a, # 38
     a, # 39
     a, # 40
     a, # 41
     a, # 42
     a, # 43
     a, # 44
     a, # 45
     a, # 46
     a, # 47
     a, # 48
     a, # 49
     a, # 50
     a, # 51
     a, # 52
     a, # 53
     a, # 54
     a, # 55
     a, # 56
     a, # 57
     a, # 58
     a, # 59
     a, # 60
     a, # 61
     a, # 62
     a, # 63
     a, # 64
     a, # 65
     a, # 66
     a, # 67
     a, # 68
     a, # 69
     a, # 70
     a, # 71
     a, # 72
     a, # 73
     a, # 74
     a, # 75
     a, # 76
     a, # 77
     a, # 78
     a, # 79
     a, # 80
     a, # 81
     a, # 82
     a, # 83
     a, # 84
     a, # 85
     a, # 86
     a, # 87
     a, # 88
     a, # 89
     a, # 90
     a, # 91
     a, # 92
     a, # 93
     a, # 94
     a, # 95
     a, # 96
     a, # 97
     a, # 98
     a, # 99
     a, # 100
     a, # 101
     a, # 102
     a, # 103
     a, # 104
     a, # 105
     a, # 106
     a, # 107
     a, # 108
     a, # 109
     a, # 110
     a, # 111
     a, # 112
     a, # 113
     a, # 114
     a, # 115
     a, # 116
     a, # 117
     a, # 118
     a, # 119
     a, # 120
     a, # 121
     a, # 122
     a, # 123
     a, # 124
     a, # 125
     a, # 126
     a, # 127
     a, # 128
     a, # 129
     a, # 130
     a, # 131
     a, # 132
     a, # 133
     a, # 134
     a, # 135
     a, # 136
     a, # 137
     a, # 138
     a, # 139
     a, # 140
     a, # 141
     a, # 142
     a, # 143
     a, # 144
     a, # 145
     a, # 146
     a, # 147
     a, # 148
     a, # 149
     a, # 150
     a, # 151
     a, # 152
     a, # 153
     a, # 154
     a, # 155
     a, # 156
     a, # 157
     a, # 158
     a, # 159
     a, # 160
     a, # 161
     a, # 162
     a, # 163
     a, # 164
     a, # 165
     a, # 166
     a, # 167
     a, # 168
     a, # 169
     a, # 170
     a, # 171
     a, # 172
     a, # 173
     a, # 174
     a, # 175
     a, # 176
     a, # 177
     a, # 178
     a, # 179
     a, # 180
     a, # 181
     a, # 182
     a, # 183
     a, # 184
     a, # 185
     a, # 186
     a, # 187
     a, # 188
     a, # 189
     a, # 190
     a, # 191
     a, # 192
     a, # 193
     a, # 194
     a, # 195
     a, # 196
     a, # 197
     a, # 198
     a, # 199
     a, # 200
     a, # 201
     a, # 202
     a, # 203
     a, # 204
     a, # 205
     a, # 206
     a, # 207
     a, # 208
     a, # 209
     a, # 210
     a, # 211
     a, # 212
     a, # 213
     a, # 214
     a, # 215
     a, # 216
     a, # 217
     a, # 218
     a, # 219
     a, # 220
     a, # 221
     a, # 222
     a, # 223
     a, # 224
     a, # 225
     a, # 226
     a, # 227
     a, # 228
     a, # 229
     a, # 230
     a, # 231
     a, # 232
     a, # 233
     a, # 234
     a, # 235
     a, # 236
     a, # 237
     a, # 238
     a, # 239
     a, # 240
     a, # 241
     a, # 242
     a, # 243
     a, # 244
     a, # 245
     a, # 246
     a, # 247
     a, # 248
     a, # 249
     a, # 250
     a, # 251
     a, # 252
     a, # 253
     a, # 254
     a, # 255
     a); 
}
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}

TEST(method, too_many_parameters)
{
  auto [res, chunk] = compile(R"(
class Foo {
  // 256 parameters.
  method(
    a1,
    a2,
    a3,
    a4,
    a5,
    a6,
    a7,
    a8,
    a9,
    a10,
    a11,
    a12,
    a13,
    a14,
    a15,
    a16,
    a17,
    a18,
    a19,
    a20,
    a21,
    a22,
    a23,
    a24,
    a25,
    a26,
    a27,
    a28,
    a29,
    a30,
    a31,
    a32,
    a33,
    a34,
    a35,
    a36,
    a37,
    a38,
    a39,
    a40,
    a41,
    a42,
    a43,
    a44,
    a45,
    a46,
    a47,
    a48,
    a49,
    a50,
    a51,
    a52,
    a53,
    a54,
    a55,
    a56,
    a57,
    a58,
    a59,
    a60,
    a61,
    a62,
    a63,
    a64,
    a65,
    a66,
    a67,
    a68,
    a69,
    a70,
    a71,
    a72,
    a73,
    a74,
    a75,
    a76,
    a77,
    a78,
    a79,
    a80,
    a81,
    a82,
    a83,
    a84,
    a85,
    a86,
    a87,
    a88,
    a89,
    a90,
    a91,
    a92,
    a93,
    a94,
    a95,
    a96,
    a97,
    a98,
    a99,
    a100,
    a101,
    a102,
    a103,
    a104,
    a105,
    a106,
    a107,
    a108,
    a109,
    a110,
    a111,
    a112,
    a113,
    a114,
    a115,
    a116,
    a117,
    a118,
    a119,
    a120,
    a121,
    a122,
    a123,
    a124,
    a125,
    a126,
    a127,
    a128,
    a129,
    a130,
    a131,
    a132,
    a133,
    a134,
    a135,
    a136,
    a137,
    a138,
    a139,
    a140,
    a141,
    a142,
    a143,
    a144,
    a145,
    a146,
    a147,
    a148,
    a149,
    a150,
    a151,
    a152,
    a153,
    a154,
    a155,
    a156,
    a157,
    a158,
    a159,
    a160,
    a161,
    a162,
    a163,
    a164,
    a165,
    a166,
    a167,
    a168,
    a169,
    a170,
    a171,
    a172,
    a173,
    a174,
    a175,
    a176,
    a177,
    a178,
    a179,
    a180,
    a181,
    a182,
    a183,
    a184,
    a185,
    a186,
    a187,
    a188,
    a189,
    a190,
    a191,
    a192,
    a193,
    a194,
    a195,
    a196,
    a197,
    a198,
    a199,
    a200,
    a201,
    a202,
    a203,
    a204,
    a205,
    a206,
    a207,
    a208,
    a209,
    a210,
    a211,
    a212,
    a213,
    a214,
    a215,
    a216,
    a217,
    a218,
    a219,
    a220,
    a221,
    a222,
    a223,
    a224,
    a225,
    a226,
    a227,
    a228,
    a229,
    a230,
    a231,
    a232,
    a233,
    a234,
    a235,
    a236,
    a237,
    a238,
    a239,
    a240,
    a241,
    a242,
    a243,
    a244,
    a245,
    a246,
    a247,
    a248,
    a249,
    a250,
    a251,
    a252,
    a253,
    a254,
    a255, a) {} 
}
)");
  ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
}