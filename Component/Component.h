#pragma once

#include "../Core/System/Tiny.h"
#include "../Core/Interface/ITask.h"

class Component : public PaintsNow::TReflected<Component, PaintsNow::SharedTiny>, PaintsNow::ITask {
public:
	virtual void Execute(void* context);
	virtual void Suspend(void* context);
	virtual void Resume(void* context);
	virtual void Abort(void* context);
	virtual bool Continue() const;
};