#include <foxlox/cppinterop.h>

namespace foxlox
{
  FoxValue to_variant(const Value& v)
  {
    switch (v.type)
    {
    case ValueType::BOOL:
      return v.v.b;
    case ValueType::F64:
      return v.v.f64;
    case ValueType::I64:
      return v.v.i64;
    case ValueType::FUNC:
      return v.v.func;
    case ValueType::CPP_FUNC:
      return v.v.cppfunc;
    case ValueType::METHOD:
      return std::make_pair(v.v.instance, v.get_method_func());
    case ValueType::OBJ:
    {
      if (v.v.obj == nullptr)
      {
        return nil;
      }
      switch (v.v.obj->type)
      {
      case ObjType::STR:
        return v.v.str->get_view();
      case ObjType::TUPLE:
        return v.v.tuple->get_span();
      case ObjType::CLASS:
        return v.v.klass;
      case ObjType::INSTANCE:
        return v.v.instance;
      default:
        assert(false);
        return {};
      }
    }
    default:
      assert(false);
      return{};
    }
  }
}