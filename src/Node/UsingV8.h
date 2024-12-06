#pragma once
#include "Binding.hpp"
#include "CppObjectMapper.h"
#include "DataTransfer.h"
#include "ScriptBackend.hpp"
#include "TypeInfo.hpp"
#include "node.h"
#include "uv.h"
#include "v8-context.h"
#include "v8-isolate.h"
#include "v8-local-handle.h"
#include "v8-locker.h"
#include "v8-primitive.h"
#include "v8-value.h"


using node::CommonEnvironmentSetup;
using node::Environment;
using node::MultiIsolatePlatform;
using v8::Context;
using v8::HandleScope;
using v8::Isolate;
using v8::Locker;
using v8::MaybeLocal;
using v8::V8;
using v8::Value;
