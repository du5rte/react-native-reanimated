//
// Created by Szymon Kapala on 2020-02-14.
//

#include "SharedString.h"
#include "SharedValueRegistry.h"

SharedString::SharedString(int id, std::string value) {
  this->value = value;
  this->id = id;
  this->type = SharedValueType::shared_string;
  this->parameter = jsi::Value::undefined();
}

SharedString::~SharedString() {}

jsi::Value SharedString::asValue(jsi::Runtime &rt) const {
  return jsi::String::createFromAscii(rt, value);
}

jsi::Value SharedString::asParameter(jsi::Runtime &rt) {
  if (!parameter.isUndefined()) {
    return parameter.getObject(rt);
  }

  class HO : public jsi::HostObject {
      public:
      std::string * value = nullptr;
      bool * dirty = nullptr;
      int id;

      HO(int id, std::string * val, bool * dirty) {
        this->value = val;
        this->dirty = dirty;
        this->id = id;
      }

      jsi::Value get(jsi::Runtime &rt, const jsi::PropNameID &name) {
        auto propName = name.utf8(rt);

        if (propName == "value") {
          return jsi::String::createFromAscii(rt, *value);
        } else if (propName == "set") {

          auto callback = [this, &rt](
            jsi::Runtime &runtime,
            const jsi::Value &thisValue,
            const jsi::Value *arguments,
            size_t count
          ) -> jsi::Value {
            (*dirty) = true;
            std::string newValue = arguments[0].asString(rt).utf8(rt);
            (*value) = newValue;
            return jsi::Value::undefined();
          };
          return jsi::Function::createFromHostFunction(rt, name, 1, callback);

        } else if (propName == "id") {
          return jsi::Value((double)id);
        }

        return jsi::Value::undefined();
      }

    };

    std::shared_ptr<jsi::HostObject> ptr(new HO(id, &value, &dirty));

    this->parameter = jsi::Object::createFromHostObject(rt, ptr);
    return parameter.getObject(rt);
}

void SharedString::setNewValue(std::shared_ptr<SharedValue> sv) {
  SharedString * sharedString = (SharedString*)sv.get();
  this->value = sharedString->value;
  this->dirty = true;
}

std::shared_ptr<SharedValue> SharedString::copy() {
  
  int id = SharedValueRegistry::NEXT_SHARED_VALUE_ID--;
  return std::make_shared<SharedString>(id,
                                        value);
}

std::vector<int> SharedString::getSharedValues() {
  std::vector<int> res;
  res.push_back(id);
  return res;
}
