#include <gtest/gtest.h>

#include <foxlox/vm.h>
#include <foxlox/compiler.h>
#include <foxlox/except.h>
#include <foxlox/cppinterop.h>

using namespace foxlox;

TEST(field, call_function_field)
{
  auto [res, chunk] = compile(R"(
var r = ();
class Foo {}
fun bar(a, b) {
  r += "bar";
  r += a;
  r += b;
}
var foo = Foo();
foo.bar = bar;
foo.bar(1, 2);
return r; 
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = to_variant(vm.run(chunk));
  ASSERT_TRUE(std::holds_alternative<TupleSpan>(v));
  auto s = std::get<TupleSpan>(v);
  ASSERT_EQ(to_variant(s[0]), FoxValue("bar"));
  ASSERT_EQ(to_variant(s[1]), FoxValue(1_i64));
  ASSERT_EQ(to_variant(s[2]), FoxValue(2_i64));
}

TEST(field, call_nonfunction_field)
{
  VM vm;
  auto [res, chunk] = compile(R"(
class Foo {}
var foo = Foo();
foo.bar = "not fn";
foo.bar();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(field, rewrite_method)
{
  auto [res, chunk] = compile(R"(
class Foo {
  method() {}
  other() {}
}
var foo = Foo();
foo.method = foo.other;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(field, get_on_bool)
{
  auto [res, chunk] = compile(R"(
true.foo;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(field, get_on_class)
{
  auto [res, chunk] = compile(R"(
class Foo {}
Foo.bar;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(field, get_on_function)
{
  auto [res, chunk] = compile(R"(
fun foo() {}
foo.bar;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(field, get_on_nil)
{
  auto [res, chunk] = compile(R"(
nil.foo;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(field, get_on_num)
{
  auto [res, chunk] = compile(R"(
123.foo;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(field, get_on_string)
{
  auto [res, chunk] = compile(R"(
"str".foo;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(field, many)
{
  auto [res, chunk] = compile(R"(
var r = ();
class Foo {}
var foo = Foo();
fun setFields() {
  foo.bilberry = "bilberry";
  foo.lime = "lime";
  foo.elderberry = "elderberry";
  foo.raspberry = "raspberry";
  foo.gooseberry = "gooseberry";
  foo.longan = "longan";
  foo.mandarine = "mandarine";
  foo.kiwifruit = "kiwifruit";
  foo.orange = "orange";
  foo.pomegranate = "pomegranate";
  foo.tomato = "tomato";
  foo.banana = "banana";
  foo.juniper = "juniper";
  foo.damson = "damson";
  foo.blackcurrant = "blackcurrant";
  foo.peach = "peach";
  foo.grape = "grape";
  foo.mango = "mango";
  foo.redcurrant = "redcurrant";
  foo.watermelon = "watermelon";
  foo.plumcot = "plumcot";
  foo.papaya = "papaya";
  foo.cloudberry = "cloudberry";
  foo.rambutan = "rambutan";
  foo.salak = "salak";
  foo.physalis = "physalis";
  foo.huckleberry = "huckleberry";
  foo.coconut = "coconut";
  foo.date = "date";
  foo.tamarind = "tamarind";
  foo.lychee = "lychee";
  foo.raisin = "raisin";
  foo.apple = "apple";
  foo.avocado = "avocado";
  foo.nectarine = "nectarine";
  foo.pomelo = "pomelo";
  foo.melon = "melon";
  foo.currant = "currant";
  foo.plum = "plum";
  foo.persimmon = "persimmon";
  foo.olive = "olive";
  foo.cranberry = "cranberry";
  foo.boysenberry = "boysenberry";
  foo.blackberry = "blackberry";
  foo.passionfruit = "passionfruit";
  foo.mulberry = "mulberry";
  foo.marionberry = "marionberry";
  foo.plantain = "plantain";
  foo.lemon = "lemon";
  foo.yuzu = "yuzu";
  foo.loquat = "loquat";
  foo.kumquat = "kumquat";
  foo.salmonberry = "salmonberry";
  foo.tangerine = "tangerine";
  foo.durian = "durian";
  foo.pear = "pear";
  foo.cantaloupe = "cantaloupe";
  foo.quince = "quince";
  foo.guava = "guava";
  foo.strawberry = "strawberry";
  foo.nance = "nance";
  foo.apricot = "apricot";
  foo.jambul = "jambul";
  foo.grapefruit = "grapefruit";
  foo.clementine = "clementine";
  foo.jujube = "jujube";
  foo.cherry = "cherry";
  foo.feijoa = "feijoa";
  foo.jackfruit = "jackfruit";
  foo.fig = "fig";
  foo.cherimoya = "cherimoya";
  foo.pineapple = "pineapple";
  foo.blueberry = "blueberry";
  foo.jabuticaba = "jabuticaba";
  foo.miracle = "miracle";
  foo.dragonfruit = "dragonfruit";
  foo.satsuma = "satsuma";
  foo.tamarillo = "tamarillo";
  foo.honeydew = "honeydew";
}
setFields();
fun getFields() {
  r += foo.apple; # expect: apple
  r += foo.apricot; # expect: apricot
  r += foo.avocado; # expect: avocado
  r += foo.banana; # expect: banana
  r += foo.bilberry; # expect: bilberry
  r += foo.blackberry; # expect: blackberry
  r += foo.blackcurrant; # expect: blackcurrant
  r += foo.blueberry; # expect: blueberry
  r += foo.boysenberry; # expect: boysenberry
  r += foo.cantaloupe; # expect: cantaloupe
  r += foo.cherimoya; # expect: cherimoya
  r += foo.cherry; # expect: cherry
  r += foo.clementine; # expect: clementine
  r += foo.cloudberry; # expect: cloudberry
  r += foo.coconut; # expect: coconut
  r += foo.cranberry; # expect: cranberry
  r += foo.currant; # expect: currant
  r += foo.damson; # expect: damson
  r += foo.date; # expect: date
  r += foo.dragonfruit; # expect: dragonfruit
  r += foo.durian; # expect: durian
  r += foo.elderberry; # expect: elderberry
  r += foo.feijoa; # expect: feijoa
  r += foo.fig; # expect: fig
  r += foo.gooseberry; # expect: gooseberry
  r += foo.grape; # expect: grape
  r += foo.grapefruit; # expect: grapefruit
  r += foo.guava; # expect: guava
  r += foo.honeydew; # expect: honeydew
  r += foo.huckleberry; # expect: huckleberry
  r += foo.jabuticaba; # expect: jabuticaba
  r += foo.jackfruit; # expect: jackfruit
  r += foo.jambul; # expect: jambul
  r += foo.jujube; # expect: jujube
  r += foo.juniper; # expect: juniper
  r += foo.kiwifruit; # expect: kiwifruit
  r += foo.kumquat; # expect: kumquat
  r += foo.lemon; # expect: lemon
  r += foo.lime; # expect: lime
  r += foo.longan; # expect: longan
  r += foo.loquat; # expect: loquat
  r += foo.lychee; # expect: lychee
  r += foo.mandarine; # expect: mandarine
  r += foo.mango; # expect: mango
  r += foo.marionberry; # expect: marionberry
  r += foo.melon; # expect: melon
  r += foo.miracle; # expect: miracle
  r += foo.mulberry; # expect: mulberry
  r += foo.nance; # expect: nance
  r += foo.nectarine; # expect: nectarine
  r += foo.olive; # expect: olive
  r += foo.orange; # expect: orange
  r += foo.papaya; # expect: papaya
  r += foo.passionfruit; # expect: passionfruit
  r += foo.peach; # expect: peach
  r += foo.pear; # expect: pear
  r += foo.persimmon; # expect: persimmon
  r += foo.physalis; # expect: physalis
  r += foo.pineapple; # expect: pineapple
  r += foo.plantain; # expect: plantain
  r += foo.plum; # expect: plum
  r += foo.plumcot; # expect: plumcot
  r += foo.pomegranate; # expect: pomegranate
  r += foo.pomelo; # expect: pomelo
  r += foo.quince; # expect: quince
  r += foo.raisin; # expect: raisin
  r += foo.rambutan; # expect: rambutan
  r += foo.raspberry; # expect: raspberry
  r += foo.redcurrant; # expect: redcurrant
  r += foo.salak; # expect: salak
  r += foo.salmonberry; # expect: salmonberry
  r += foo.satsuma; # expect: satsuma
  r += foo.strawberry; # expect: strawberry
  r += foo.tamarillo; # expect: tamarillo
  r += foo.tamarind; # expect: tamarind
  r += foo.tangerine; # expect: tangerine
  r += foo.tomato; # expect: tomato
  r += foo.watermelon; # expect: watermelon
  r += foo.yuzu; # expect: yuzu
}
getFields();

return r; 
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = to_variant(vm.run(chunk));
  ASSERT_TRUE(std::holds_alternative<TupleSpan>(v));
  auto s = std::get<TupleSpan>(v);
  ASSERT_EQ(ssize(s), 79);
  int i = 0;
  ASSERT_EQ(to_variant(s[i++]), FoxValue("apple"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("apricot"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("avocado"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("banana"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("bilberry"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("blackberry"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("blackcurrant"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("blueberry"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("boysenberry"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("cantaloupe"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("cherimoya"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("cherry"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("clementine"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("cloudberry"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("coconut"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("cranberry"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("currant"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("damson"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("date"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("dragonfruit"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("durian"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("elderberry"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("feijoa"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("fig"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("gooseberry"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("grape"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("grapefruit"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("guava"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("honeydew"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("huckleberry"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("jabuticaba"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("jackfruit"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("jambul"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("jujube"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("juniper"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("kiwifruit"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("kumquat"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("lemon"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("lime"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("longan"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("loquat"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("lychee"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("mandarine"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("mango"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("marionberry"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("melon"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("miracle"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("mulberry"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("nance"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("nectarine"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("olive"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("orange"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("papaya"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("passionfruit"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("peach"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("pear"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("persimmon"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("physalis"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("pineapple"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("plantain"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("plum"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("plumcot"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("pomegranate"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("pomelo"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("quince"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("raisin"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("rambutan"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("raspberry"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("redcurrant"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("salak"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("salmonberry"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("satsuma"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("strawberry"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("tamarillo"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("tamarind"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("tangerine"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("tomato"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("watermelon"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("yuzu"));
}

TEST(field, method)
{
  auto [res, chunk] = compile(R"(
var r;
class Foo {
  bar(arg) {
    r = "bar" + arg;
  }
}
var bar = Foo().bar;
bar("arg");          # expect: bararg
return r; 
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = to_variant(vm.run(chunk));
  ASSERT_EQ(v, FoxValue("bararg"));
}

TEST(field, method_binds_this)
{
  auto [res, chunk] = compile(R"(
var r = ();
class Foo {
  sayName(a) {
    r += this.name;
    r += a;
  }
}
var foo1 = Foo();
foo1.name = "foo1";
var foo2 = Foo();
foo2.name = "foo2";
# Store the method reference on another object.
foo2.fn = foo1.sayName;
# Still retains original receiver.
foo2.fn(1);
# expect: foo1
# expect: 1
return r; 
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = to_variant(vm.run(chunk));
  ASSERT_TRUE(std::holds_alternative<TupleSpan>(v));
  auto s = std::get<TupleSpan>(v);
  ASSERT_EQ(to_variant(s[0]), FoxValue("foo1"));
  ASSERT_EQ(to_variant(s[1]), FoxValue(1_i64));
}

TEST(field, on_instance)
{
  auto [res, chunk] = compile(R"(
var r = ();
class Foo {}
var foo = Foo();
r += foo.bar = "bar value"; # expect: bar value
r += foo.baz = "baz value"; # expect: baz value
r += foo.bar; # expect: bar value
r += foo.baz; # expect: baz value
return r; 
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = to_variant(vm.run(chunk));
  ASSERT_TRUE(std::holds_alternative<TupleSpan>(v));
  auto s = std::get<TupleSpan>(v);
  ASSERT_EQ(to_variant(s[0]), FoxValue("bar value"));
  ASSERT_EQ(to_variant(s[1]), FoxValue("baz value"));
  ASSERT_EQ(to_variant(s[2]), FoxValue("bar value"));
  ASSERT_EQ(to_variant(s[3]), FoxValue("baz value"));
}

TEST(field, set_on_bool)
{
  auto [res, chunk] = compile(R"(
true.foo = "value";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(field, set_on_class)
{
  auto [res, chunk] = compile(R"(
class Foo {}
Foo.bar = "value";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(field, set_on_function)
{
  auto [res, chunk] = compile(R"(
fun foo() {}
foo.bar = "value";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(field, set_on_nil)
{
  auto [res, chunk] = compile(R"(
nil.foo = "value";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(field, set_on_num)
{
  auto [res, chunk] = compile(R"(
123.foo = "value";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(field, set_on_string)
{
  auto [res, chunk] = compile(R"(
"str".foo = "value";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(field, undefined)
{
  auto [res, chunk] = compile(R"(
class Foo {}
var foo = Foo();
return foo.bar;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = to_variant(vm.run(chunk));
  ASSERT_EQ(v, FoxValue(nil));
}