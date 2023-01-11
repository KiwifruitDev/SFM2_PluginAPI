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

#pragma once
#include <array>
#include <vector>
#include <functional>
#include <unordered_map>
#include "SPTLib\patterns.hpp"
#include "SPTLib\memutils.hpp"

#define DECL_MEMBER_CDECL(type, name, ...) \
	using _##name = type(__cdecl*)(__VA_ARGS__); \
	_##name ORIG_##name = nullptr;

#define DECL_HOOK_CDECL(type, name, ...) \
	DECL_MEMBER_CDECL(type, name, ##__VA_ARGS__) \
	static type __cdecl HOOKED_##name(__VA_ARGS__)

#define HOOK_CDECL(type, className, name, ...) type __cdecl className::HOOKED_##name(__VA_ARGS__)

#define DECL_MEMBER_THISCALL(type, name, ...) \
	using _##name = type(__fastcall*)(void* thisptr, int edx, ##__VA_ARGS__); \
	_##name ORIG_##name = nullptr;

#define DECL_HOOK_THISCALL(type, name, ...) \
	DECL_MEMBER_THISCALL(type, name, ##__VA_ARGS__) \
	static type __fastcall HOOKED_##name(void* thisptr, int edx, ##__VA_ARGS__)

#define HOOK_THISCALL(type, className, name, ...) \
	type __fastcall className::HOOKED_##name(void* thisptr, int edx, ##__VA_ARGS__)

#define ADD_RAW_HOOK(moduleName, name) \
	AddRawHook(#moduleName, reinterpret_cast<void**>(&ORIG_##name##), reinterpret_cast<void*>(HOOKED_##name##));
#define FIND_PATTERN(moduleName, name) \
	AddPatternHook(patterns::##name##, #moduleName, #name, reinterpret_cast<void**>(&ORIG_##name##), nullptr);
#define HOOK_FUNCTION(moduleName, name) \
	AddPatternHook(patterns::##name##, \
	               #moduleName, \
	               #name, \
	               reinterpret_cast<void**>(&ORIG_##name##), \
	               reinterpret_cast<void*>(HOOKED_##name##));
#define InitCommand(command) InitConcommandBase(command##_command)

struct VFTableHook
{
	VFTableHook(void** vftable, int index, void* functionToHook, void** origPtr);

	void** vftable;
	int index;
	void* functionToHook;
	void** origPtr;
};

struct PatternHook
{
	PatternHook(patterns::PatternWrapper* patternArr,
	            size_t size,
	            const char* patternName,
	            void** origPtr,
	            void* functionHook)
	{
		this->patternArr = patternArr;
		this->size = size;
		this->patternName = patternName;
		this->origPtr = origPtr;
		this->functionHook = functionHook;
	}

	patterns::PatternWrapper* patternArr;
	size_t size;
	const char* patternName;
	void** origPtr;
	void* functionHook;
};

struct MatchAllPattern
{
	MatchAllPattern(patterns::PatternWrapper* patternArr,
	                size_t size,
	                const char* patternName,
	                std::vector<patterns::MatchedPattern>* foundVec)
	{
		this->patternArr = patternArr;
		this->size = size;
		this->patternName = patternName;
		this->foundVec = foundVec;
	}

	patterns::PatternWrapper* patternArr;
	size_t size;
	const char* patternName;
	std::vector<patterns::MatchedPattern>* foundVec;
};

struct OffsetHook
{
	int32_t offset;
	const char* patternName;
	void** origPtr;
	void* functionHook;
};

struct RawHook
{
	const char* patternName;
	void** origPtr;
	void* functionHook;
};

struct ModuleHookData
{
	std::vector<PatternHook> patternHooks;
	std::vector<MatchAllPattern> matchAllPatterns;
	std::vector<VFTableHook> vftableHooks;
	std::vector<OffsetHook> offsetHooks;

	std::vector<std::pair<void**, void*>> funcPairs;
	std::vector<void**> hookedFunctions;
	std::vector<VFTableHook> existingVTableHooks;
	void InitModule(const std::wstring& moduleName);
	void HookModule(const std::wstring& moduleName);
	void UnhookModule(const std::wstring& moduleName);
};

class Feature
{
public:
	virtual ~Feature(){};
	virtual bool ShouldLoadFeature()
	{
		return true;
	};
	virtual void InitHooks(){};
	virtual void PreHook(){};
	virtual void LoadFeature(){};
	virtual void UnloadFeature(){};
	virtual Feature* CreateNewInstance() = 0;
	virtual void Move(Feature* instance) = 0;

	static void ReloadFeatures();
	static void LoadFeatures();
	static void UnloadFeatures();

	template<size_t PatternLength>
	static void AddPatternHook(const std::array<patterns::PatternWrapper, PatternLength>& patterns,
	                           std::string moduleName,
	                           const char* patternName,
	                           void** origPtr = nullptr,
	                           void* functionHook = nullptr);
	template<size_t PatternLength>
	static void AddMatchAllPattern(const std::array<patterns::PatternWrapper, PatternLength>& patterns,
	                               std::string moduleName,
	                               const char* patternName,
	                               std::vector<patterns::MatchedPattern>* foundVec);
	static void AddRawHook(std::string moduleName, void** origPtr, void* functionHook);
	static void AddPatternHook(PatternHook hook, std::string moduleEnum);
	static void AddMatchAllPattern(MatchAllPattern hook, std::string moduleName);
	static void AddVFTableHook(VFTableHook hook, std::string moduleEnum);
	static void AddOffsetHook(std::string moduleName,
	                          int offset,
	                          const char* patternName,
	                          void** origPtr = nullptr,
	                          void* functionHook = nullptr);
	static int GetPatternIndex(void** origPtr);

	Feature();

protected:
	bool moduleLoaded;
	bool startedLoading;

private:
	static void InitModules();
	static void Hook();
	static void Unhook();
};

template<typename T>
class FeatureWrapper : public Feature
{
public:
	virtual Feature* CreateNewInstance()
	{
		return new T();
	}

	virtual void Move(Feature* instance)
	{
		*((T*)this) = std::move(*(T*)instance);
	}
};

template<size_t PatternLength>
inline void Feature::AddPatternHook(const std::array<patterns::PatternWrapper, PatternLength>& p,
                                    std::string moduleEnum,
                                    const char* patternName,
                                    void** origPtr,
                                    void* functionHook)
{
	AddPatternHook(PatternHook(const_cast<patterns::PatternWrapper*>(p.data()),
	                           PatternLength,
	                           patternName,
	                           origPtr,
	                           functionHook),
	               moduleEnum);
}

template<size_t PatternLength>
inline void Feature::AddMatchAllPattern(const std::array<patterns::PatternWrapper, PatternLength>& patterns,
                                        std::string moduleName,
                                        const char* patternName,
                                        std::vector<patterns::MatchedPattern>* foundVec)
{
	AddMatchAllPattern(MatchAllPattern(const_cast<patterns::PatternWrapper*>(patterns.data()),
	                                   PatternLength,
	                                   patternName,
	                                   foundVec),
	                   moduleName);
}
