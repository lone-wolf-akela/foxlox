#include <gtest/gtest.h>
import foxlox;

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
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 3);
  ASSERT_EQ(v[0], "bar");
  ASSERT_EQ(v[1], 1_i64);
  ASSERT_EQ(v[2], 2_i64);
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
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 79);
  int i = 0;
  ASSERT_EQ(v[i++], "apple");
  ASSERT_EQ(v[i++], "apricot");
  ASSERT_EQ(v[i++], "avocado");
  ASSERT_EQ(v[i++], "banana");
  ASSERT_EQ(v[i++], "bilberry");
  ASSERT_EQ(v[i++], "blackberry");
  ASSERT_EQ(v[i++], "blackcurrant");
  ASSERT_EQ(v[i++], "blueberry");
  ASSERT_EQ(v[i++], "boysenberry");
  ASSERT_EQ(v[i++], "cantaloupe");
  ASSERT_EQ(v[i++], "cherimoya");
  ASSERT_EQ(v[i++], "cherry");
  ASSERT_EQ(v[i++], "clementine");
  ASSERT_EQ(v[i++], "cloudberry");
  ASSERT_EQ(v[i++], "coconut");
  ASSERT_EQ(v[i++], "cranberry");
  ASSERT_EQ(v[i++], "currant");
  ASSERT_EQ(v[i++], "damson");
  ASSERT_EQ(v[i++], "date");
  ASSERT_EQ(v[i++], "dragonfruit");
  ASSERT_EQ(v[i++], "durian");
  ASSERT_EQ(v[i++], "elderberry");
  ASSERT_EQ(v[i++], "feijoa");
  ASSERT_EQ(v[i++], "fig");
  ASSERT_EQ(v[i++], "gooseberry");
  ASSERT_EQ(v[i++], "grape");
  ASSERT_EQ(v[i++], "grapefruit");
  ASSERT_EQ(v[i++], "guava");
  ASSERT_EQ(v[i++], "honeydew");
  ASSERT_EQ(v[i++], "huckleberry");
  ASSERT_EQ(v[i++], "jabuticaba");
  ASSERT_EQ(v[i++], "jackfruit");
  ASSERT_EQ(v[i++], "jambul");
  ASSERT_EQ(v[i++], "jujube");
  ASSERT_EQ(v[i++], "juniper");
  ASSERT_EQ(v[i++], "kiwifruit");
  ASSERT_EQ(v[i++], "kumquat");
  ASSERT_EQ(v[i++], "lemon");
  ASSERT_EQ(v[i++], "lime");
  ASSERT_EQ(v[i++], "longan");
  ASSERT_EQ(v[i++], "loquat");
  ASSERT_EQ(v[i++], "lychee");
  ASSERT_EQ(v[i++], "mandarine");
  ASSERT_EQ(v[i++], "mango");
  ASSERT_EQ(v[i++], "marionberry");
  ASSERT_EQ(v[i++], "melon");
  ASSERT_EQ(v[i++], "miracle");
  ASSERT_EQ(v[i++], "mulberry");
  ASSERT_EQ(v[i++], "nance");
  ASSERT_EQ(v[i++], "nectarine");
  ASSERT_EQ(v[i++], "olive");
  ASSERT_EQ(v[i++], "orange");
  ASSERT_EQ(v[i++], "papaya");
  ASSERT_EQ(v[i++], "passionfruit");
  ASSERT_EQ(v[i++], "peach");
  ASSERT_EQ(v[i++], "pear");
  ASSERT_EQ(v[i++], "persimmon");
  ASSERT_EQ(v[i++], "physalis");
  ASSERT_EQ(v[i++], "pineapple");
  ASSERT_EQ(v[i++], "plantain");
  ASSERT_EQ(v[i++], "plum");
  ASSERT_EQ(v[i++], "plumcot");
  ASSERT_EQ(v[i++], "pomegranate");
  ASSERT_EQ(v[i++], "pomelo");
  ASSERT_EQ(v[i++], "quince");
  ASSERT_EQ(v[i++], "raisin");
  ASSERT_EQ(v[i++], "rambutan");
  ASSERT_EQ(v[i++], "raspberry");
  ASSERT_EQ(v[i++], "redcurrant");
  ASSERT_EQ(v[i++], "salak");
  ASSERT_EQ(v[i++], "salmonberry");
  ASSERT_EQ(v[i++], "satsuma");
  ASSERT_EQ(v[i++], "strawberry");
  ASSERT_EQ(v[i++], "tamarillo");
  ASSERT_EQ(v[i++], "tamarind");
  ASSERT_EQ(v[i++], "tangerine");
  ASSERT_EQ(v[i++], "tomato");
  ASSERT_EQ(v[i++], "watermelon");
  ASSERT_EQ(v[i++], "yuzu");
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
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, "bararg");
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
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 2);
  ASSERT_EQ(v[0], "foo1");
  ASSERT_EQ(v[1], 1_i64);
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
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 4);
  ASSERT_EQ(v[0], "bar value");
  ASSERT_EQ(v[1], "baz value");
  ASSERT_EQ(v[2], "bar value");
  ASSERT_EQ(v[3], "baz value");
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
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, nil);
}