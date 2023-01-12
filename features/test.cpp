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
#include <SPTLib/sptlib.hpp>

typedef void(*_TabWindow)();

typedef void(__fastcall* _InitMainWindow)(QMainWindow* mainWindow);
typedef void(__fastcall* _StartFilmmaker)(void* param_1);
typedef QDialog*(__fastcall* _ShowStartWizard)(QDialog* param_1, char param_2, uint* param_3, QFlags<Qt::WindowType> param_4);
typedef void*(__fastcall* _SetupTabWindows)(void* param_1);
typedef void(__fastcall* _RegisterTabWindow)(void* param_1, QString* parent, QString* name, int front, _TabWindow windowFunc, int show);
typedef QWidget* (__fastcall* _OpenUndoWindow)(QWidget* param_1);


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
	_StartFilmmaker ORIG_StartFilmmaker = nullptr;
	_ShowStartWizard ORIG_ShowStartWizard = nullptr;
	_SetupTabWindows ORIG_SetupTabWindows = nullptr;
	_RegisterTabWindow ORIG_RegisterTabWindow = nullptr;
	_OpenUndoWindow ORIG_OpenUndoWindow = nullptr;
	static void __fastcall HOOKED_InitMainWindow(QMainWindow* mainWindow);
	static void __fastcall HOOKED_StartFilmmaker(void* param_1);
	static QDialog* __fastcall HOOKED_ShowStartWizard(QDialog* param_1, char param_2, uint* param_3, QFlags<Qt::WindowType> param_4);
	static void* __fastcall HOOKED_SetupTabWindows(void* param_1);
	static QWidget* __fastcall HOOKED_OpenUndoWindow(QWidget* param_1);
};

static TestFeature sfm2_testfeature;

namespace patterns
{
	PATTERNS(
		InitMainWindow,
		"steampal_8302839",
		"48 89 5C 24 ?? 56 48 83 EC 50 48 8B 05 ?? ?? ?? ??"
	)
	PATTERNS(
		StartFilmmaker,
		"steampal_8302839",
		"40 53 48 83 EC 20 48 8B 41 ?? 48 8B D9 48 85 C0 74 ?? C6 80 ?? ?? ?? ?? 01"
	)
	PATTERNS(
		ShowStartWizard,
		"steampal_8302839",
		"48 89 5C 24 ?? 4C 89 44 24 ?? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ?? ?? ?? ?? 48 81 EC E0 05 00 00"
	)
	PATTERNS(
		SetupTabWindows,
		"steampal_8302839",
		"48 89 5C 24 ?? 55 56 57 41 56 41 57 48 8B EC 48 83 EC 30 48 8B D9"
	)
	PATTERNS(
		RegisterTabWindow,
		"steampal_8302839",
		"48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 57 41 54 41 55 41 56 41 57 48 83 EC 40 4C 8B E9"
	)
	PATTERNS(
		OpenUndoWindow,
		"steampal_8302839",
		"48 89 4C 24 ?? 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ?? 48 81 EC B8 00 00 00 45 33 E4"
	)
} // namespace patterns

void TestFeature::InitHooks()
{
	HOOK_FUNCTION(sfm, InitMainWindow);
	HOOK_FUNCTION(sfm, StartFilmmaker);
	HOOK_FUNCTION(sfm, ShowStartWizard);
	HOOK_FUNCTION(sfm, SetupTabWindows);
	FIND_PATTERN(sfm, RegisterTabWindow);
	HOOK_FUNCTION(sfm, OpenUndoWindow);
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
	// Set main window
	globalHelpers->mainWindow = mainWindow;
	// Call original function
	sfm2_testfeature.ORIG_InitMainWindow(mainWindow);
}

void __fastcall TestFeature::HOOKED_StartFilmmaker(void* param_1)
{
	// Call original function
	sfm2_testfeature.ORIG_StartFilmmaker(param_1);
}

QDialog* __fastcall TestFeature::HOOKED_ShowStartWizard(QDialog* param_1, char param_2, uint* param_3, QFlags<Qt::WindowType> param_4)
{
	// Call original function
	QDialog* startWizard = sfm2_testfeature.ORIG_ShowStartWizard(param_1, param_2, param_3, param_4);
	if (startWizard)
	{
		// Set custom window title
		startWizard->setWindowTitle("Start Wizard: SFM2_PluginAPI");
		// Increase height
		startWizard->setFixedHeight(startWizard->height() + 25);
		// Add footer text
		QLabel* footer = new QLabel(startWizard);
		footer->setText("SFM2_PluginAPI: Hello Twitter! I'm alive!");
		footer->setGeometry(70, startWizard->height() - 50, 200, 50);
		// Add GitHub button
		QPushButton* githubButton = new QPushButton(startWizard);
		githubButton->setText("GitHub");
		githubButton->setGeometry(10, startWizard->height() - 37.5, 50, 25);
		QObject::connect(githubButton, &QPushButton::clicked, []() { QDesktopServices::openUrl(QUrl("https://github.com/KiwifruitDev/SFM2_PluginAPI")); });
	}
	return startWizard;
}

void* __fastcall TestFeature::HOOKED_SetupTabWindows(void* param_1)
{
	// Call original function
	return sfm2_testfeature.ORIG_SetupTabWindows(param_1);
}

QWidget* __fastcall TestFeature::HOOKED_OpenUndoWindow(QWidget* param_1)
{
	// Call original function
	QWidget* undoWindow = sfm2_testfeature.ORIG_OpenUndoWindow(param_1);
	// Hide all elements (but don't remove them)
	for (auto child : undoWindow->findChildren<QWidget*>())
	{
		child->hide();
	}
	// Add custom text
	QLabel* customText = new QLabel(undoWindow);
	customText->setText("SFM2_PluginAPI: Hello Twitter! I'm alive!");
	customText->setGeometry(10, 10, 200, 50);
	// Add funny button
	QPushButton* funnyButton = new QPushButton(undoWindow);
	funnyButton->setText("Funny Button");
	funnyButton->setGeometry(10, 50, 100, 25);
	return undoWindow;
}
