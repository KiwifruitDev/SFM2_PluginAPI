/*
	SFM2_PluginAPI
	==============
	This software is licensed under the MIT License.

	MIT License

	Copyright (c) 2023 KiwifruitDev

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include "pch.hpp"
#include "helpers.hpp"
#include <feature.hpp>
#include <SPTLib/sptlib.hpp>

HMODULE hTier0;
HMODULE hSFM;

typedef void* (*BinaryPropertiesGetValueType)(void* param1);
BinaryPropertiesGetValueType BinaryPropertiesGetValue_fn;

typedef void* (*CreateInterfaceType)(void* param1, void* param2);
CreateInterfaceType CreateInterface_fn;

typedef void* (*GetResourceManifestCountType)();
GetResourceManifestCountType GetResourceManifestCount_fn;

typedef void* (*GetResourceManifestsType)(void* param1, void* param2, void* param3);
GetResourceManifestsType GetResourceManifests_fn;

typedef void* (*InstallSchemaBindingsType)(void* param1, void* param2);
InstallSchemaBindingsType InstallSchemaBindings_fn;

typedef void* (*NvOptimusEnablementType)();
NvOptimusEnablementType NvOptimusEnablement_fn;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		// Load library "tier0.dll"
		hTier0 = LoadLibraryA("tier0.dll");
		// Load library "sfm.dll"
		hSFM = LoadLibraryA("tools\\sfm.dll");
		if (hTier0 && hSFM)
		{
			// Set message functions
			_EngineMsg = (_MsgFormat)GetProcAddress(hTier0, "Msg");
			_EngineDevMsg = _EngineMsg;
			_EngineWarning = _EngineMsg;
			_EngineDevWarning = _EngineMsg;
			// Load features
			Feature::LoadFeatures();
			// Get address of "BinaryPropertiesGetValue" function
			BinaryPropertiesGetValue_fn = (BinaryPropertiesGetValueType)GetProcAddress(hSFM, "BinaryProperties_GetValue");
			// Get address of "CreateInterface" function
			CreateInterface_fn = (CreateInterfaceType)GetProcAddress(hSFM, "CreateInterface");
			// Get address of "GetResourceManifestCount" function
			GetResourceManifestCount_fn = (GetResourceManifestCountType)GetProcAddress(hSFM, "GetResourceManifestCount");
			// Get address of "GetResourceManifests" function
			GetResourceManifests_fn = (GetResourceManifestsType)GetProcAddress(hSFM, "GetResourceManifests");
			// Get address of "InstallSchemaBindings" function
			InstallSchemaBindings_fn = (InstallSchemaBindingsType)GetProcAddress(hSFM, "InstallSchemaBindings");
			// Get address of "NvOptimusEnablement" function
			NvOptimusEnablement_fn = (NvOptimusEnablementType)GetProcAddress(hSFM, "NvOptimusEnablement");
			//QMessageBox.warning(0, "Funny", "This is a funny message box because it's on Source 2.");
		}
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
		// Unload features
		//Feature::UnloadFeatures();
        break;
    }
    return TRUE;
}

void* BinaryProperties_GetValue(void* param1)
{
	if (BinaryPropertiesGetValue_fn)
	{
		return BinaryPropertiesGetValue_fn(param1);
	}
}

void* CreateInterface(void* param1, void* param2)
{
	if (CreateInterface_fn)
	{
		return CreateInterface_fn(param1, param2);
	}
}

void* GetResourceManifestCount()
{
	if (GetResourceManifestCount_fn)
	{
		return GetResourceManifestCount_fn();
	}
}

void* GetResourceManifests(void* param1, void* param2, void* param3)
{
	if (GetResourceManifests_fn)
	{
		return GetResourceManifests_fn(param1, param2, param3);
	}
}

void* InstallSchemaBindings(void* param1, void* param2)
{
	if (InstallSchemaBindings_fn)
	{
		return InstallSchemaBindings_fn(param1, param2);
	}
}

void* NvOptimusEnablement()
{
	if (NvOptimusEnablement_fn)
	{
		return NvOptimusEnablement_fn();
	}
}
