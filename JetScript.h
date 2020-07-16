#pragma once
#include "Core/Interface/IScript.h"
#include "Core/Template/TBuffer.h"
#include <napi.h>

class JetScript {
public:
	using IScript = PaintsNow::IScript;
	using IReflect = PaintsNow::IReflect;
	using String = PaintsNow::String;

	class Request final : public PaintsNow::TReflected<Request, IScript::Request> {
	public:
		Request(const Napi::Env& env);
		Request(const Napi::CallbackInfo& callbackInfo);

		virtual ~Request() override;
		virtual IScript* GetScript() override;
		virtual void DoLock() override;
		virtual void UnLock() override;
		virtual bool Call(const AutoWrapperBase& defer, const Request::Ref& g) override;
		virtual std::vector<Key> Enumerate() override;
		virtual IScript::Request::TYPE GetCurrentType() override;
		virtual Request::Ref Load(const String& script, const String& pathname) override;
		virtual IScript::Request& Push() override;
		virtual IScript::Request& Pop() override;
		virtual IScript::Request& operator >> (IScript::Request::Arguments&) override;
		virtual IScript::Request& operator >> (IScript::Request::Ref&) override;
		virtual IScript::Request& operator << (const IScript::Request::Ref&) override;
		virtual IScript::Request& operator << (const IScript::Request::Nil&) override;
		virtual IScript::Request& operator << (const IScript::BaseDelegate&) override;
		virtual IScript::Request& operator >> (IScript::BaseDelegate&) override;
		virtual IScript::Request& operator << (const IScript::Request::Global&) override;
		virtual IScript::Request& operator << (const IScript::Request::TableStart&) override;
		virtual IScript::Request& operator >> (IScript::Request::TableStart&) override;
		virtual IScript::Request& operator << (const IScript::Request::TableEnd&) override;
		virtual IScript::Request& operator >> (const IScript::Request::TableEnd&) override;
		virtual IScript::Request& operator << (const IScript::Request::ArrayStart&) override;
		virtual IScript::Request& operator >> (IScript::Request::ArrayStart&) override;
		virtual IScript::Request& operator << (const IScript::Request::ArrayEnd&) override;
		virtual IScript::Request& operator >> (const IScript::Request::ArrayEnd&) override;
		virtual IScript::Request& operator << (const IScript::Request::Key&) override;
		virtual IScript::Request& operator >> (const IScript::Request::Key&) override;
		virtual IScript::Request& operator << (double value) override;
		virtual IScript::Request& operator >> (double& value) override;
		virtual IScript::Request& operator << (const String& str) override;
		virtual IScript::Request& operator >> (String& str) override;
		virtual IScript::Request& operator << (const char* str) override;
		virtual IScript::Request& operator >> (const char*& str) override;
		virtual IScript::Request& operator << (int64_t value) override;
		virtual IScript::Request& operator >> (int64_t& value) override;
		virtual IScript::Request& operator << (bool b) override;
		virtual IScript::Request& operator >> (bool& b) override;
		virtual IScript::Request& operator << (const AutoWrapperBase& wrapper) override;
		virtual IScript::Request& MoveVariables(IScript::Request& target, size_t count) override;
		virtual void Dereference(IScript::Request::Ref& ref) override;
		virtual IScript::Request::Ref Reference(const IScript::Request::Ref& d) override;
		virtual IScript::Request::TYPE GetReferenceType(const IScript::Request::Ref& d) override;
		virtual int GetCount() override;

		Request& operator << (const Napi::Value& value);
		Request& operator >> (Napi::Value& value);

	protected:
		uint32_t IncreaseTableIndex(int32_t count = 1);
		void Write(const Napi::Value& value);
		void OpenValue(const Napi::Value& value);
		void CloseValue();
		Napi::Value Read();

	protected:
		Napi::Env env;
		String key;
		uint32_t idx = 0;
		uint32_t initCount = 0;
		uint32_t tableLevel = 0;
		std::vector<napi_value> valueStack; // napi_value saves more storage than Napi::Value
		std::vector<uint32_t> indexStack;
	};
};
