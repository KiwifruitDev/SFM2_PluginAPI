/*
	SFM2_PluginAPI
	==============
	This software is licensed under the MIT License.

	MIT License

	Copyright (c) 2023 KiwifruitEngineDev

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
#include "feature.hpp"
#include "SPTLib\sptlib.hpp"
#include "SPTLib\Windows\detoursutils.hpp"
#include "SPTLib\Hooks.hpp"

static std::unordered_map<std::string, ModuleHookData> moduleHookData;
static std::unordered_map<uintptr_t, int> patternIndices;
static bool loadedOnce = false;
static bool reloadingFeatures = false;

static std::vector<Feature*>& GetFeatures()
{
	static std::vector<Feature*> features;
	return features;
}

void Feature::ReloadFeatures()
{
	reloadingFeatures = true;
	for (Feature* feature : GetFeatures())
	{
		auto instance = feature->CreateNewInstance();
		feature->Move(instance);
		delete instance;
	}
	reloadingFeatures = false;
}

void Feature::LoadFeatures()
{
	// This is a restart, reload the features
	if (loadedOnce)
	{
		ReloadFeatures();
	}

	Hooks::InitInterception(true);

	for (auto feature : GetFeatures())
	{
		if (!feature->moduleLoaded && feature->ShouldLoadFeature())
		{
			feature->startedLoading = true;
			feature->InitHooks();
		}
	}

	InitModules();

	for (auto feature : GetFeatures())
	{
		if (!feature->moduleLoaded && feature->startedLoading)
		{
			feature->PreHook();
		}
	}

	Hook();

	for (auto feature : GetFeatures())
	{
		if (!feature->moduleLoaded && feature->startedLoading)
		{
			feature->LoadFeature();
			feature->moduleLoaded = true;
		}
	}

	loadedOnce = true;
}

void Feature::UnloadFeatures()
{
	for (auto feature : GetFeatures())
	{
		if (feature->moduleLoaded)
		{
			feature->UnloadFeature();
			feature->moduleLoaded = false;
		}
	}

	Unhook();

	for (auto feature : GetFeatures())
	{
		if (feature->moduleLoaded)
		{
			feature->~Feature();
		}
	}

	moduleHookData.clear();
	patternIndices.clear();
}

void Feature::AddVFTableHook(VFTableHook hook, std::string moduleEnum)
{
	if (moduleHookData.find(moduleEnum) == moduleHookData.end())
	{
		moduleHookData[moduleEnum] = ModuleHookData();
	}

	auto& mhd = moduleHookData[moduleEnum];
	mhd.vftableHooks.push_back(hook);
}

Feature::Feature()
{
	moduleLoaded = false;
	startedLoading = false;
	if (!reloadingFeatures)
		GetFeatures().push_back(this);
}

void Feature::InitModules()
{
	for (auto& pair : moduleHookData)
	{
		pair.second.InitModule(Convert(pair.first + ".dll"));
	}
}

void Feature::Hook()
{
	for (auto& pair : moduleHookData)
	{
		pair.second.HookModule(Convert(pair.first + ".dll"));
	}
}

void Feature::Unhook()
{
	for (auto& pair : moduleHookData)
	{
		pair.second.UnhookModule(Convert(pair.first + ".dll"));
	}
}

void Feature::AddOffsetHook(std::string moduleEnum,
                            int offset,
                            const char* patternName,
                            void** origPtr,
                            void* functionHook)
{
	if (moduleHookData.find(moduleEnum) == moduleHookData.end())
	{
		moduleHookData[moduleEnum] = ModuleHookData();
	}

	auto& mhd = moduleHookData[moduleEnum];
	mhd.offsetHooks.push_back(OffsetHook{offset, patternName, origPtr, functionHook});
}

int Feature::GetPatternIndex(void** origPtr)
{
	uintptr_t ptr = reinterpret_cast<uintptr_t>(origPtr);
	if (patternIndices.find(ptr) != patternIndices.end())
	{
		return patternIndices[ptr];
	}
	else
	{
		return -1;
	}
}

void Feature::AddRawHook(std::string moduleName, void** origPtr, void* functionHook)
{
	if (moduleHookData.find(moduleName) == moduleHookData.end())
	{
		moduleHookData[moduleName] = ModuleHookData();
	}

	auto& hookData = moduleHookData[moduleName];
	hookData.funcPairs.emplace_back(origPtr, functionHook);
	hookData.hookedFunctions.emplace_back(origPtr);
}

void Feature::AddPatternHook(PatternHook hook, std::string moduleName)
{
	if (moduleHookData.find(moduleName) == moduleHookData.end())
	{
		moduleHookData[moduleName] = ModuleHookData();
	}

	auto& mhd = moduleHookData[moduleName];
	mhd.patternHooks.push_back(hook);
}

void Feature::AddMatchAllPattern(MatchAllPattern hook, std::string moduleName)
{
	if (moduleHookData.find(moduleName) == moduleHookData.end())
	{
		moduleHookData[moduleName] = ModuleHookData();
	}

	auto& mhd = moduleHookData[moduleName];
	mhd.matchAllPatterns.push_back(hook);
}

void ModuleHookData::UnhookModule(const std::wstring& moduleName)
{
	if (!hookedFunctions.empty())
		DetoursUtils::DetachDetours(moduleName, hookedFunctions.size(), &hookedFunctions[0]);

	for (auto& vft_hook : existingVTableHooks)
		MemUtils::HookVTable(vft_hook.vftable, vft_hook.index, *vft_hook.origPtr);
}

void ModuleHookData::InitModule(const std::wstring& moduleName)
{
	void* handle;
	void* moduleStart;
	size_t moduleSize;

	if (MemUtils::GetModuleInfo(moduleName, &handle, &moduleStart, &moduleSize))
	{
		EngineDevMsg("Hooking %s (start: %p; size: %x)...\n", Convert(moduleName).c_str(), moduleStart, moduleSize);
	}
	else
	{
		EngineDevMsg("Couldn't hook %s, not loaded\n", Convert(moduleName).c_str());
		return;
	}

	std::vector<std::future<patterns::PatternWrapper*>> hooks;
	std::vector<std::future<std::vector<patterns::MatchedPattern>>> mhooks;
	hooks.reserve(patternHooks.size());

	for (auto& mpattern : matchAllPatterns)
	{
		mhooks.emplace_back(MemUtils::find_all_sequences_async(moduleStart,
		                                                       moduleSize,
		                                                       mpattern.patternArr,
		                                                       mpattern.patternArr + mpattern.size));
	}

	for (auto& pattern : patternHooks)
	{
		hooks.emplace_back(MemUtils::find_unique_sequence_async(*pattern.origPtr,
		                                                        moduleStart,
		                                                        moduleSize,
		                                                        pattern.patternArr,
		                                                        pattern.patternArr + pattern.size));
	}

	funcPairs.reserve(funcPairs.size() + patternHooks.size());
	hookedFunctions.reserve(hookedFunctions.size() + patternHooks.size());

	for (std::size_t i = 0; i < mhooks.size(); ++i)
	{
		auto modulePattern = matchAllPatterns[i];
		*modulePattern.foundVec = std::move(mhooks[i].get());
		EngineDevMsg("[%s] Found %u instances of pattern %s\n",
		       Convert(moduleName).c_str(),
		       modulePattern.foundVec->size(),
		       modulePattern.patternName);
	}

	for (std::size_t i = 0; i < hooks.size(); ++i)
	{
		auto foundPattern = hooks[i].get();
		auto modulePattern = patternHooks[i];

		if (*modulePattern.origPtr)
		{
			if (modulePattern.functionHook)
			{
				funcPairs.emplace_back(modulePattern.origPtr, modulePattern.functionHook);
				hookedFunctions.emplace_back(modulePattern.origPtr);
			}

			EngineDevMsg("[%s] Found %s at %p (using the %s pattern).\n",
			       Convert(moduleName).c_str(),
			       modulePattern.patternName,
			       *modulePattern.origPtr,
			       foundPattern->name());
			patternIndices[reinterpret_cast<uintptr_t>(modulePattern.origPtr)] =
			    foundPattern - modulePattern.patternArr;
		}
		else
		{
			EngineDevWarning("[%s] Could not find %s.\n", Convert(moduleName).c_str(), modulePattern.patternName);
		}
	}

	for (auto& offset : offsetHooks)
	{
		*offset.origPtr = reinterpret_cast<char*>(moduleStart) + offset.offset;

		EngineDevMsg("[%s] Found %s at %p via a fixed offset.\n",
		       Convert(moduleName).c_str(),
		       offset.patternName,
		       *offset.origPtr);

		if (offset.functionHook)
		{
			funcPairs.emplace_back(offset.origPtr, offset.functionHook);
			hookedFunctions.emplace_back(offset.origPtr);
		}
	}
}

void ModuleHookData::HookModule(const std::wstring& moduleName)
{
	if (!vftableHooks.empty())
	{
		for (auto& vft_hook : vftableHooks)
		{
			*vft_hook.origPtr = vft_hook.vftable[vft_hook.index];
			MemUtils::HookVTable(vft_hook.vftable, vft_hook.index, vft_hook.functionToHook);
		}
	}

	if (!funcPairs.empty())
	{
		for (auto& entry : funcPairs)
			MemUtils::MarkAsExecutable(*(entry.first));

		DetoursUtils::AttachDetours(moduleName, funcPairs.size(), &funcPairs[0]);
	}

	// Clear any hooks that were added
	offsetHooks.clear();
	patternHooks.clear();
	// VTable hooks have to be stored for the unhooking code
	existingVTableHooks.insert(existingVTableHooks.end(), vftableHooks.begin(), vftableHooks.end());
	vftableHooks.clear();
}

VFTableHook::VFTableHook(void** vftable, int index, void* functionToHook, void** origPtr)
{
	this->vftable = vftable;
	this->index = index;
	this->functionToHook = functionToHook;
	this->origPtr = origPtr;
}
