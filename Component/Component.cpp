#include "Component.h"

void Component::Execute(void* context) {}
void Component::Suspend(void* context) {}
void Component::Resume(void* context) {}
void Component::Abort(void* context) {}
bool Component::Continue() const { return true; }
