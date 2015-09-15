// EntryPoint.cpp : Defines the entry point for the application.

#include "FirstTry.h"

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	// Use HeapSetInformation to specify that the process should terminate
	// if the heap manager detects an error in any heap used by the process
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	if (SUCCEEDED(CoInitialize(NULL)))
	{
		{
			FirstTry firstTry;
			if (SUCCEEDED(firstTry.Initialize()))
				firstTry.RunMessageLoop();
		}

		CoUninitialize();
	}

	return 0;
}