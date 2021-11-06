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


#include "Gamma_Table.h"
#include"AEGP_SuiteHandler.h"

BOOL WINAPI dummyChooseColor(LPCHOOSECOLOR lpcc) {
	
	PF_PixelFloat nCol = { 0.0,0.0,0.0 };
	unsigned char r = 0, g = 0, b = 0;
	if (lpColorPickerfn("Dummy Color Picker", &nCol, 0, &nCol) != PF_Interrupt_CANCEL) {
		
		r = nCol.red * 255;
		g = nCol.green * 255;
		b = nCol.blue * 255;
	}
	const DWORD resCol = b << 16 | g << 8 | r;
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

	// out_data->out_flags2 = PF_OutFlag2_SUPPORTS_THREADED_RENDERING;

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
				// 开始事务
				DetourTransactionBegin();
				// 更新线程信息  
				DetourUpdateThread(GetCurrentThread());
				// 将拦截的函数附加到原函数的地址上,这里可以拦截多个函数。
				std::cout << "res code:" << DetourAttach(&(PVOID&)lpChooseColor, dummyChooseColor) << std::endl;
				// 结束事务
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

	if (lpColorPickerfn == NULL) {
	AEGP_SuiteHandler	suites(in_data->pica_basicP);
	// Premiere Pro/Elements don't support PF_AppColorPickerDialog
	lpColorPickerfn = suites.AppSuite5()->PF_AppColorPickerDialog;
	}
	else{

		// get target function adress
		auto hComdlgDll = LoadLibrary("comdlg32.dll");
		FARPROC lpChooseColor = NULL;
		if (hComdlgDll != NULL) {

			lpChooseColor = GetProcAddress(hComdlgDll, "ChooseColorW");

			if (lpChooseColor != NULL) {

				// start hijack
				// 开始事务
				DetourTransactionBegin();
				// 更新线程信息  
				DetourUpdateThread(GetCurrentThread());
				// 将拦截的函数附加到原函数的地址上,这里可以拦截多个函数。
				std::cout << "res code:" << DetourAttach(&(PVOID&)lpChooseColor, dummyChooseColor) << std::endl;
				// 结束事务
				DetourTransactionCommit();
			}
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
SequenceSetup(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output)
{
	return PF_Err_NONE;
}

static PF_Err
SequenceSetdown(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output)
{
	return PF_Err_NONE;
}


static PF_Err
SequenceResetup(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output)
{
	if (lpColorPickerfn == NULL) {
		AEGP_SuiteHandler	suites(in_data->pica_basicP);
		// Premiere Pro/Elements don't support PF_AppColorPickerDialog
		lpColorPickerfn = suites.AppSuite5()->PF_AppColorPickerDialog;
	}
	else {

		// get target function adress
		auto hComdlgDll = LoadLibrary("comdlg32.dll");
		FARPROC lpChooseColor = NULL;
		if (hComdlgDll != NULL) {

			lpChooseColor = GetProcAddress(hComdlgDll, "ChooseColorW");

			if (lpChooseColor != NULL) {

				// start hijack
				// 开始事务
				DetourTransactionBegin();
				// 更新线程信息  
				DetourUpdateThread(GetCurrentThread());
				// 将拦截的函数附加到原函数的地址上,这里可以拦截多个函数。
				std::cout << "res code:" << DetourAttach(&(PVOID&)lpChooseColor, dummyChooseColor) << std::endl;
				// 结束事务
				DetourTransactionCommit();
			}
		}
	}
	return PF_Err_NONE;
}

// Computes the gamma-corrected pixel given the lookup table.

static PF_Err
GammaFunc(
	void* refcon,
	A_long x,
	A_long y,
	PF_Pixel* inP,
	PF_Pixel* outP)
{
	PF_Err		err = PF_Err_NONE;
	GammaInfo* giP = (GammaInfo*)refcon;

	if (giP) {
		outP->alpha = inP->alpha;
		outP->red = giP->lut[inP->red];
		outP->green = giP->lut[inP->green];
		outP->blue = giP->lut[inP->blue];
	}
	else {
		err = PF_Err_BAD_CALLBACK_PARAM;
	}
	return err;
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
	case PF_Cmd_SEQUENCE_SETUP:
		err = SequenceSetup(in_data, out_data, params, output);
		break;
	case PF_Cmd_SEQUENCE_SETDOWN:
		err = SequenceSetdown(in_data, out_data, params, output);
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

#ifdef AE_OS_WIN
BOOL WINAPI DllMain(HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)
{
	HINSTANCE my_instance_handle = (HINSTANCE)0;

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		my_instance_handle = hDLL;
		break;

	case DLL_THREAD_ATTACH:
		my_instance_handle = hDLL;
		break;
	case DLL_THREAD_DETACH:
		my_instance_handle = 0;
		break;
	case DLL_PROCESS_DETACH:
		my_instance_handle = 0;
		break;
	}
	return(TRUE);
}
#endif

