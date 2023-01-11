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
#include "feature.hpp"
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <SPTLib/sptlib.hpp>

typedef void(__cdecl* _InitMainWindow)(QMainWindow* mainWindow);

class TestFeature : public FeatureWrapper<TestFeature>
{
public:
protected:
	virtual bool ShouldLoadFeature() override;

	virtual void InitHooks() override;

	virtual void LoadFeature() override;

	virtual void UnloadFeature() override;

private:
	_InitMainWindow ORIG_InitMainWindow = nullptr;
	static void __cdecl HOOKED_InitMainWindow(QMainWindow* mainWindow);
};

static TestFeature sfm2_testfeature;

namespace patterns
{
	PATTERNS(
		InitMainWindow,
		"steampal_8302839",
		"48 89 5C 24 ?? 56 48 83 EC 50 48 8B 05 ?? ?? ?? ??"
	)
} // namespace patterns

void TestFeature::InitHooks()
{
	HOOK_FUNCTION(sfm, InitMainWindow);
}

bool TestFeature::ShouldLoadFeature()
{
	return true;
}

void TestFeature::LoadFeature()
{
}

void TestFeature::UnloadFeature() {}

void __fastcall TestFeature::HOOKED_InitMainWindow(QMainWindow* mainWindow)
{
	// Open dialog box
	//QMessageBox::information(mainWindow, "Test", "This is a test message box.");
	// Call original function
	sfm2_testfeature.ORIG_InitMainWindow(mainWindow);
}
