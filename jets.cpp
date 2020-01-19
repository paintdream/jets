#include <napi.h>
#include "JetScript.h"
#include "Core/System/Tiny.h"
using namespace PaintsNow;

class CustomObject : public TReflected<CustomObject, SharedTiny> {
public:
	virtual ~CustomObject() {
		printf("Destructor called!\n");
	}
};

class Proxy : public TReflected<Proxy, IScript::Library> {
public:
	virtual TObject<IReflect>& operator () (IReflect& reflect) {
		BaseClass::operator () (reflect);

		if (reflect.IsReflectMethod()) {
			ReflectMethod(RequestAdd)[ScriptMethod = "add"];
			ReflectMethod(RequestSum)[ScriptMethod = "sum"];
			ReflectMethod(RequestMakeTable)[ScriptMethod = "maketable"];
			ReflectMethod(RequestForward)[ScriptMethod = "forward"];
			ReflectMethod(RequestMakeObject)[ScriptMethod = "makeobject"];
			ReflectMethod(RequestGetObjectAddress)[ScriptMethod = "getobjectaddress"];
			ReflectMethod(RequestCallback)[ScriptMethod = "callback"];
		}

		return *this;
	}

	void RequestAdd(IScript::Request& request, double a, double b) {
		request << a + b;
	}

	void RequestSum(IScript::Request& request, std::vector<int>&& data) {
		int sum = 0;
		for (auto&& i : data) sum += i;
		request << sum;
	}

	void RequestMakeTable(IScript::Request& request, String&& k, String&& v) {
		request << begintable;
		request << key(k) << v;
		request << endtable;
	}

	void RequestForward(IScript::Request& request, IScript::Request::Ref ref) {
		request << ref;
		request.Dereference(ref);
	}

	void RequestMakeObject(IScript::Request& request) {
		CustomObject* object = new CustomObject();
		request << object;
		object->ReleaseObject();
	}

	void RequestGetObjectAddress(IScript::Request& request, IScript::Delegate<SharedTiny> obj) {
		request << (uint64_t)(obj ? obj.Get() : nullptr);
	}

	void RequestCallback(IScript::Request& request, IScript::Request::Ref ref) {
		request.Push();
		request.Call(sync, ref, "callback");
		// get return value
		String retValue;
		request >> retValue;
		request.Pop();

		request << retValue;
		request.Dereference(ref);
	}
};

Napi::Object Init(Napi::Env env, Napi::Object exports) {
	static Proxy proxy;
	JetScript::Request request(env);
	request << exports;
	request >> begintable;
	proxy.Register(request);
	request >> endtable;

	return exports;
}

NODE_API_MODULE(addon, Init)
