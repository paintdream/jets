{
	"targets": [
		{
			"target_name": "jets",
			"cflags!": [ "-fno-exceptions" ],
			"cflags_cc!": [ "-fno-exceptions" ],
			"sources": [
				"jets.cpp",
				"JetScript.cpp",
				"JetScript.h",
				"Core/Interface/IType.h",
				"Core/Interface/IType.cpp",
				"Core/Interface/IDevice.h",
				"Core/Interface/IDevice.cpp",
				"Core/Interface/IScript.h",
				"Core/Interface/IScript.cpp",
				"Core/Interface/IThread.h",
				"Core/Interface/IThread.cpp",
				"Core/Interface/IReflect.h",
				"Core/Interface/IReflect.cpp",
				"Core/System/Tiny.h",
				"Core/System/Tiny.cpp",
				"Component/Component.h",
				"Component/Component.cpp"
			],
			"include_dirs": [
				"<!@(node -p \"require('node-addon-api').include\")"
			],
			"defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
		}
	]
}
