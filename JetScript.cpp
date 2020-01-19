#include "JetScript.h"
using namespace PaintsNow;
using namespace Napi;
using PaintsNow::String;

// Request
JetScript::Request::Request(const Napi::Env& e) : env(e) {}
JetScript::Request::Request(const Napi::CallbackInfo& callbackInfo) : env(callbackInfo.Env()) {
	// Copy stack info
	valueStack.resize(callbackInfo.Length());
	for (size_t i = 0; i < callbackInfo.Length(); i++) {
		valueStack[i] = callbackInfo[i];
	}
}

JetScript::Request::~Request() {}

IScript* JetScript::Request::GetScript() {
	return nullptr;
}

// single-threaded model, no lock needed.
void JetScript::Request::DoLock() {}
void JetScript::Request::UnLock() {}

bool JetScript::Request::Call(const IScript::Request::AutoWrapperBase& defer, const IScript::Request::Ref& g) {
	napi_ref ref = reinterpret_cast<napi_ref>(g.value);
	Napi::Reference<Function> function(env, ref);
	function.SuppressDestruct();

	if (function) {
		Value retValue;
		if (initCount == 0) {
			retValue = function.Value().Call(std::move(valueStack));
			valueStack.clear();
		} else {
			std::vector<napi_value> values;
			values.resize(valueStack.size() - initCount);
			std::move(valueStack.begin() + initCount, valueStack.end(), values.begin());
			retValue = function.Value().Call(std::move(values));
			valueStack.resize(initCount);
		}

		idx = initCount;

		// set result
		if (!retValue.IsEmpty()) {
			valueStack.emplace_back(std::move(retValue));
		}

		return true;
	} else {
		return false;
	}
}

std::vector<IScript::Request::Key> JetScript::Request::Enumerate() {
	assert(false);
	return{};
}

IScript::Request::TYPE GetType(const Value& value) {
	switch (value.Type()) {
	case napi_undefined:
		return IScript::Request::NIL;
	case napi_null:
		return IScript::Request::NIL;
	case napi_boolean:
		return IScript::Request::BOOLEAN;
	case napi_number:
		return IScript::Request::NUMBER;
	case napi_string:
		return IScript::Request::STRING;
	case napi_object:
		return IScript::Request::TABLE;
	case napi_function:
		return IScript::Request::FUNCTION;
	case napi_external:
		return IScript::Request::OBJECT;
	case napi_bigint:
		return IScript::Request::INTEGER;
	default:
		return IScript::Request::NIL;
	}
}

IScript::Request::TYPE JetScript::Request::GetCurrentType() {
	return GetType(Value(env, valueStack[idx]));
}

IScript::Request::Ref JetScript::Request::Load(const String& script, const String& pathname) {
	assert(false); // Not supported
	return 0;
}

uint32_t JetScript::Request::IncreaseTableIndex(int32_t count) {
	assert(!indexStack.empty());

	uint32_t& index = indexStack.back();
	if (count == 0) {
		index = 0;
	} else {
		index += count;
	}

	return index - 1;
}

void JetScript::Request::Write(const Value& value) {
	if (tableLevel != 0) {
		if (key.empty()) {
			uint32_t index = IncreaseTableIndex();
			Array array(env, valueStack[valueStack.size() - 1]);
			assert(array.Length() == index);
			array[index] = value;
		} else {
			Object object(env, valueStack[valueStack.size() - 1]);
			object.Set(key, value);
			key = "";
		}
	} else {
		assert(key.empty());
		valueStack.push_back(value);
	}
}

Value JetScript::Request::Read() {
	if (tableLevel != 0) {
		if (key.empty()) {
			uint32_t index = IncreaseTableIndex();
			Array array(env, valueStack[valueStack.size() - 1]);
			return array[index];
		} else {
			String k;
			std::swap(k, key);
			Object object(env, valueStack[valueStack.size() - 1]);
			return object.Get(k);
		}
	} else {
		return Value(env, valueStack[idx++]);
	}
}

IScript::Request& JetScript::Request::Push() {
	indexStack.push_back(idx);
	indexStack.push_back(tableLevel);
	indexStack.push_back(initCount);

	initCount = safe_cast<uint32_t>(valueStack.size());
	idx = initCount;
	tableLevel = 0;
	return *this;
}

IScript::Request& JetScript::Request::Pop() {
	assert(key.empty());
	assert(tableLevel == 0);

	valueStack.resize(initCount);
	initCount = indexStack[indexStack.size() - 1];
	tableLevel = indexStack[indexStack.size() - 2];
	idx = indexStack[indexStack.size() - 3];
	indexStack.resize(indexStack.size() - 3);

	return *this;
}

IScript::Request& JetScript::Request::operator >> (IScript::Request::Ref& r) {
	Value value = Read();
	Napi::Reference<Value> reference = Napi::Reference<Value>::New(value, 1);
	r.value = reinterpret_cast<size_t>((napi_ref)reference);
	reference.SuppressDestruct();

	return *this;
}

IScript::Request& JetScript::Request::operator << (const IScript::Request::Ref& r) {
	Napi::Reference<Value> reference(env, reinterpret_cast<napi_ref>(r.value));
	reference.SuppressDestruct();
	Write(reference.Value());

	return *this;
}

IScript::Request& JetScript::Request::operator << (const IScript::Request::Nil&) {
	Write(Value());
	return *this;
}

IScript::Request& JetScript::Request::operator << (const IScript::BaseDelegate& d) {
	IScript::Object* object = d.GetRaw();
	object->ScriptInitialize(*this);
	External<IScript::Object> value = External<IScript::Object>::New(env, object, [](Env env, IScript::Object* object) {
		Request request(env);
		object->ScriptUninitialize(request);
	});

	Write(value);
	return *this;
}

IScript::Request& JetScript::Request::operator >> (IScript::BaseDelegate& d) {
	External<IScript::Object> value = Read().As<External<IScript::Object>>();
	d = IScript::BaseDelegate(value.Data());
	return *this;
}

void JetScript::Request::OpenValue(const Value& value) {
	valueStack.push_back(value);
	indexStack.push_back(0);
	tableLevel++;
}

void JetScript::Request::CloseValue() {
	assert(tableLevel != 0);
	assert(key.empty());

	valueStack.pop_back();
	indexStack.pop_back();
	tableLevel--;
}

IScript::Request& JetScript::Request::operator << (const IScript::Request::Global&) {
	Value v = env.Global();
	Write(v);
	OpenValue(v);
	return *this;
}

IScript::Request& JetScript::Request::operator << (const IScript::Request::Local&) {
	// accessing local variable is not allowed
	assert(false);
	return *this;
}

IScript::Request& JetScript::Request::operator << (const IScript::Request::TableStart&) {
	Value v = Object::New(env);
	Write(v);
	OpenValue(v);
	return *this;
}

IScript::Request& JetScript::Request::operator >> (IScript::Request::TableStart& tn) {
	OpenValue(Read());
	
	tn.count = 0; // JavaScript hold no array elements for a table
	return *this;
}

IScript::Request& JetScript::Request::operator << (const IScript::Request::TableEnd&) {
	CloseValue();
	return *this;
}

IScript::Request& JetScript::Request::operator >> (const IScript::Request::TableEnd&) {
	CloseValue();
	return *this;
}

IScript::Request& JetScript::Request::operator << (const IScript::Request::ArrayStart&) {
	Value v = Array::New(env);
	Write(v);
	OpenValue(v);
	return *this;
}

IScript::Request& JetScript::Request::operator >> (IScript::Request::ArrayStart& t) {
	Value v = Read();
	OpenValue(v);

	Array arr(env, v);
	t.count = arr.Length();
	return *this;
}

IScript::Request& JetScript::Request::operator << (const IScript::Request::ArrayEnd&) {
	CloseValue();
	return *this;
}

IScript::Request& JetScript::Request::operator >> (const IScript::Request::ArrayEnd&) {
	CloseValue();
	return *this;
}

IScript::Request& JetScript::Request::operator << (const IScript::Request::Key& k) {
	key = k.GetKey();
	return *this;
}

IScript::Request& JetScript::Request::operator >> (const IScript::Request::Key& k) {
	key = k.GetKey();
	return *this;
}

IScript::Request& JetScript::Request::operator << (double value) {
	Write(Number::New(env, value));
	return *this;
}

IScript::Request& JetScript::Request::operator >> (double& value) {
	value = Read().As<Number>().DoubleValue();
	return *this;
}

IScript::Request& JetScript::Request::operator << (const String& str) {
	Write(Napi::String::New(env, str));
	return *this;
}

IScript::Request& JetScript::Request::operator >> (String& str) {
	str = Read().As<Napi::String>().Utf8Value();
	return *this;
}

IScript::Request& JetScript::Request::operator << (const char* str) {
	Write(Napi::String::New(env, str));
	return *this;
}

IScript::Request& JetScript::Request::operator >> (const char*& str) {
	assert(false); // not allowed
	return *this;
}

IScript::Request& JetScript::Request::operator << (int64_t value) {
	Write(Number::New(env, value));
	return *this;
}

IScript::Request& JetScript::Request::operator >> (int64_t& value) {
	value = Read().As<Number>().Int64Value();
	return *this;
}

IScript::Request& JetScript::Request::operator << (bool b) {
	Write(Boolean::New(env, b));
	return *this;
}

IScript::Request& JetScript::Request::operator >> (bool& b) {
	b = Read().As<Boolean>().Value();
	return *this;
}

IScript::Request& JetScript::Request::operator << (const AutoWrapperBase& wrapper) {
	AutoWrapperBase* copy = wrapper.Clone();
	Function func = Function::New(env, [](const CallbackInfo& callbackInfo) {
		Request::AutoWrapperBase* wrapper = reinterpret_cast<AutoWrapperBase*>(callbackInfo.Data());
		Request request(callbackInfo);
		size_t count = request.valueStack.size();
		(*wrapper).Execute(request);

		return request.valueStack.size() > count ? Value(callbackInfo.Env(), request.valueStack[count]) : Value();
	}, nullptr, copy);

	func.AddFinalizer([](Env env, AutoWrapperBase* w) {
		delete w;
	}, copy);

	Write(func);
	return *this;
}

IScript::Request& JetScript::Request::operator >> (const Skip& skip) {
	if (tableLevel != 0) {
		if (key.empty()) {
			IncreaseTableIndex(skip.count);
		}
	} else {
		idx += skip.count;
	}

	return *this;
}

IScript::Request& JetScript::Request::MoveVariables(IScript::Request& target, size_t count) {
	JetScript::Request& r = static_cast<JetScript::Request&>(target);
	size_t start = valueStack.size() - count;
	r.valueStack.reserve(r.valueStack.size() + count);
	std::move(valueStack.begin() + start, valueStack.end(), std::back_inserter(r.valueStack));
	valueStack.resize(start);

	return *this;
}

void JetScript::Request::Dereference(IScript::Request::Ref& ref) {
	Napi::Reference<Value> reference(env, reinterpret_cast<napi_ref>(ref.value));
	if (reference.Unref() != 0) {
		reference.SuppressDestruct();
	}
}

IScript::Request::Ref JetScript::Request::Reference(const IScript::Request::Ref& d) {
	Napi::Reference<Value> reference(env, reinterpret_cast<napi_ref>(ref.value));
	reference.SuppressDestruct();
	reference.Ref();
	return d;
}

IScript::Request::TYPE JetScript::Request::GetReferenceType(const IScript::Request::Ref& d) {
	Napi::Reference<Value> reference(env, reinterpret_cast<napi_ref>(ref.value));
	return GetType(reference.Value());
}

int JetScript::Request::GetCount() {
	return safe_cast<int32_t>(valueStack.size()) - initCount;
}

JetScript::Request& JetScript::Request::operator << (const Napi::Value& value) {
	Write(value);
	return *this;
}

JetScript::Request& JetScript::Request::operator >> (Napi::Value& value) {
	value = Read();
	return *this;
}
