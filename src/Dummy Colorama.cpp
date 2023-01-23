/*******************************************************************/
/*                                                                 */
/*                      ADOBE CONFIDENTIAL                         */
/*                   _ _ _ _ _ _ _ _ _ _ _ _ _                     */
/*                                                                 */
/* Copyright 2007 Adobe Systems Incorporated                       */
/* All Rights Reserved.                                            */
/*                                                                 */
/* NOTICE:  All information contained herein is, and remains the   */
/* property of Adobe Systems Incorporated and its suppliers, if    */
/* any.  The intellectual and technical concepts contained         */
/* herein are proprietary to Adobe Systems Incorporated and its    */
/* suppliers and may be covered by U.S. and Foreign Patents,       */
/* patents in process, and are protected by trade secret or        */
/* copyright law.  Dissemination of this information or            */
/* reproduction of this material is strictly forbidden unless      */
/* prior written permission is obtained from Adobe Systems         */
/* Incorporated.                                                   */
/*                                                                 */
/*******************************************************************/

/*
	Gamma_Table.cpp

	This demonstrates:
		using a sequence data Handle
		iterating over the image pixels (using PF_ITERATE)
		calling the progress callback
		a Fixed Slider control
		the Extent Hint rectangle (from the InData structure)
		making the application display an error alert for us
		using the PF_ITERATE callback

	Revision History

	Version		Change													Engineer	Date
	=======		======													========	======
	1.0			Created													rb			3/12/1993
	1.1			Rewritten												bbb
	1.2			Added Windows entry point, made most functions static
	1.5			Added new entry point									zal			9/15/2017
*/

#include "Dummy Colorama.h"
#include"AEGP_SuiteHandler.h"

BOOL WINAPI dummyChooseColor(const LPCHOOSECOLOR lpcc) {
	PF_PixelFloat nCol{};
	const DWORD initCol = lpcc->rgbResult;
	nCol.red = (initCol >> 0 & 0x00FF) / 255.0f;
	nCol.green = ((initCol >> 8) & 0x00FF) / 255.0f;
	nCol.blue = ((initCol >> 16) & 0x00FF) / 255.0f;
	nCol.alpha = 1.0f;
	DWORD resCol = initCol;
	if (lpColorPickerfn("Dummy Color Picker", &nCol, true, &nCol) != PF_Interrupt_CANCEL) {
		unsigned char r = 0, g = 0, b = 0;
		r = nCol.red * 255;
		g = nCol.green * 255;
		b = nCol.blue * 255;
		resCol = b << 16 | g << 8 | r;
	}
	lpcc->rgbResult = resCol;
	return TRUE;
};

static PF_Err
About(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output)
{
	PF_SPRINTF(out_data->return_msg,
		"%s, v%d.%d\r%s",
		NAME,
		MAJOR_VERSION,
		MINOR_VERSION,
		DESCRIPTION);

	return PF_Err_NONE;
}

static PF_Err
GlobalSetup(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output)
{
	PF_Err	err = PF_Err_NONE;

	out_data->my_version = PF_VERSION(MAJOR_VERSION,
		MINOR_VERSION,
		BUG_VERSION,
		STAGE_VERSION,
		BUILD_VERSION);


	out_data->out_flags |= PF_OutFlag_PIX_INDEPENDENT |
		PF_OutFlag_USE_OUTPUT_EXTENT |
		PF_OutFlag_CUSTOM_UI;

	out_data->out_flags2 = PF_OutFlag2_SUPPORTS_THREADED_RENDERING;

	AEGP_SuiteHandler	suites(in_data->pica_basicP);

	//PF_Err err = PF_Err_NONE;
	//AEGP_SuiteHandler suites(in_data->pica_basicP);

	// Premiere Pro/Elements don't support PF_AppColorPickerDialog
	lpColorPickerfn = suites.AppSuite5()->PF_AppColorPickerDialog;

	if (lpColorPickerfn != NULL) {

		// get target function adress
		auto hComdlgDll = LoadLibrary("comdlg32.dll");
		FARPROC lpChooseColor = NULL;
		if (hComdlgDll != NULL) {

			lpChooseColor = GetProcAddress(hComdlgDll, "ChooseColorW");

			if (lpChooseColor != NULL) {
				// start hijack
				DetourTransactionBegin();
				DetourAttach(&(PVOID&)lpChooseColor, dummyChooseColor);
				DetourTransactionCommit();
			}
		}
	}
	return err;
}

static PF_Err
GlobalSetdown(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output)
{
	PF_Err	err = PF_Err_NONE;
	auto hComdlgDll = LoadLibrary("comdlg32.dll");
	FARPROC lpChooseColor = NULL;
	if (hComdlgDll != NULL) {

		lpChooseColor = GetProcAddress(hComdlgDll, "ChooseColorW");

		if (lpChooseColor != NULL) {
			DetourTransactionBegin();
			DetourDetach(&(PVOID&)lpChooseColor, dummyChooseColor);
			DetourTransactionCommit();
		}
	}
	return err;
}

static PF_Err
ParamsSetup(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output)
{
	PF_Err			err = PF_Err_NONE;
	out_data->num_params = 1;
	return err;
}

static PF_Err
SequenceResetup(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output)
{
	std::cout << "seq resetup\n";
	if (lpColorPickerfn == NULL) {
		AEGP_SuiteHandler	suites(in_data->pica_basicP);
		lpColorPickerfn = suites.AppSuite5()->PF_AppColorPickerDialog;
	}
	else {

		// get target function adress
		auto hComdlgDll = LoadLibrary("comdlg32.dll");
		FARPROC lpChooseColor = NULL;
		if (hComdlgDll != NULL) {

			lpChooseColor = GetProcAddress(hComdlgDll, "ChooseColorW");

			if (lpChooseColor != NULL) {
				DetourTransactionBegin();
				DetourAttach(&(PVOID&)lpChooseColor, dummyChooseColor);
				DetourTransactionCommit();
			}
		}
	}
	return PF_Err_NONE;
}

static PF_Err
Render(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output)
{
	in_data->utils->copy(in_data->effect_ref, &params[0]->u.ld, output, &in_data->extent_hint, &in_data->extent_hint);
	return 0;
}


extern "C" DllExport
PF_Err PluginDataEntryFunction(
	PF_PluginDataPtr inPtr,
	PF_PluginDataCB inPluginDataCallBackPtr,
	SPBasicSuite * inSPBasicSuitePtr,
	const char* inHostName,
	const char* inHostVersion)
{
	PF_Err result = PF_Err_INVALID_CALLBACK;

	result = PF_REGISTER_EFFECT(
		inPtr,
		inPluginDataCallBackPtr,
		"Colorama(dummy)", // Name
		"ADBE Colorama(dummy)", // Match Name
		"Dolag Plug-ins", // Category
		AE_RESERVED_INFO); // Reserved Info

	return result;
}


PF_Err
EffectMain(
	PF_Cmd			cmd,
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output)
{
	PF_Err		err = PF_Err_NONE;

	switch (cmd) {
	case PF_Cmd_ABOUT:
		err = About(in_data, out_data, params, output);
		break;
	case PF_Cmd_GLOBAL_SETUP:
		err = GlobalSetup(in_data, out_data, params, output);
		break;
	case PF_Cmd_GLOBAL_SETDOWN:
		err = GlobalSetdown(in_data, out_data, params, output);
		break;
	case PF_Cmd_PARAMS_SETUP:
		err = ParamsSetup(in_data, out_data, params, output);
		break;
	case PF_Cmd_SEQUENCE_RESETUP:
		err = SequenceResetup(in_data, out_data, params, output);
		break;
	case PF_Cmd_RENDER:
		err = Render(in_data, out_data, params, output);
		break;
	}
	return err;
}
