/**
 * \file opendlg.c
 * \brief Windows Vista+ open file dialog showing character details
 *
 * Copyright (c) 2026 Klaas van Aarsen
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "h-basic.h" /* bool, size_t */

#include "savefile.h" /* savefile_get_description */

/*
 * The Common Item Dialog (IFileOpenDialog) and SHCreateItemFromParsingName()
 * require Windows Vista (_WIN32_WINNT >= 0x0600).  This must come before any
 * include that may pull in windows.h.  If FORCE_XP_COMPAT is defined, the
 * Vista code is left out and the classic GetOpenFileName() dialog is used,
 * which has no character details.
 */
#ifndef FORCE_XP_COMPAT
#ifndef WINVER
#define WINVER 0x0600
#elif WINVER < 0x0600
#undef WINVER
#define WINVER 0x0600
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#elif _WIN32_WINNT < 0x0600
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#endif /* !FORCE_XP_COMPAT */

#include <windows.h>
#ifndef FORCE_XP_COMPAT
#include <shobjidl.h>   /* For IFileOpenDialog, IFileDialogCustomize, IFileDialogEvents */
#else
#include <commdlg.h>    /* For GetOpenFileName */
#endif

#include "opendlg.h"

/**
 * Tell the user that the open file dialog failed, as opposed to being
 * cancelled.  code is an HRESULT or a Windows error code.
 */
static void report_dialog_error(HWND hwnd_owner, const char *what, unsigned long code)
{
	char msg[256];

	snprintf(msg, sizeof(msg), "%s (error 0x%08lX).", what, code);
	MessageBoxA(hwnd_owner, msg, "Open Angband Save File", MB_OK | MB_ICONERROR);
}

#ifndef FORCE_XP_COMPAT

/* Control IDs for IFileDialogCustomize */
#define IDC_CHAR_GROUP   1001
#define IDC_CHAR_DETAILS 1002
#define IDC_CHAR_DETAILS2 1003

/* Texts shown in the character details controls */
#define TEXT_SELECT_FILE L"(Select a file)"
#define TEXT_NO_INFO     L"(No info found or empty)"

/**
 * Show two lines of text in the character details controls.  Two controls
 * are used since a single text control is too narrow for the whole description.
 */
static void set_details_text(IFileDialogCustomize *pfdc, const WCHAR *line1,
		const WCHAR *line2)
{
	pfdc->lpVtbl->SetControlLabel(pfdc, IDC_CHAR_DETAILS, line1);
	pfdc->lpVtbl->SetControlLabel(pfdc, IDC_CHAR_DETAILS2, line2);
}

/**
 * Convert an ANSI string of the given length to a wide string.
 */
static bool ansi_to_wide(const char *s, int len, WCHAR *out, int out_len)
{
	if (len == 0) {
		out[0] = L'\0';
		return true;
	}
	int n = MultiByteToWideChar(CP_ACP, 0, s, len, out, out_len - 1);
	if (n <= 0) return false;
	out[n] = L'\0';
	return true;
}

/* Struct representing our custom IFileDialogEvents COM listener */
typedef struct {
	IFileDialogEventsVtbl *lpVtbl;
	LONG ref_count;
	IFileDialogCustomize *pfdc;
} SaveFileDialogEvents;

/* --- IFileDialogEvents Interface Methods --- */

static HRESULT STDMETHODCALLTYPE Events_QueryInterface(
	IFileDialogEvents *this, REFIID riid, void **ppvObject)
{
	if (!ppvObject) return E_POINTER;
	if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IFileDialogEvents)) {
		*ppvObject = this;
		this->lpVtbl->AddRef(this);
		return S_OK;
	}
	*ppvObject = NULL;
	return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE Events_AddRef(IFileDialogEvents *this)
{
	SaveFileDialogEvents *self = (SaveFileDialogEvents *)this;
	return InterlockedIncrement(&self->ref_count);
}

static ULONG STDMETHODCALLTYPE Events_Release(IFileDialogEvents *this)
{
	SaveFileDialogEvents *self = (SaveFileDialogEvents *)this;
	return InterlockedDecrement(&self->ref_count);
}

static HRESULT STDMETHODCALLTYPE Events_OnFileOk(IFileDialogEvents *this, IFileDialog *pfd)
{
	(void)this; (void)pfd;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE Events_OnFolderChanging(
	IFileDialogEvents *this, IFileDialog *pfd, IShellItem *psiFolder)
{
	(void)this; (void)pfd; (void)psiFolder;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE Events_OnFolderChange(IFileDialogEvents *this, IFileDialog *pfd)
{
	(void)pfd;
	SaveFileDialogEvents *self = (SaveFileDialogEvents *)this;
	if (self->pfdc) {
		set_details_text(self->pfdc, TEXT_SELECT_FILE, L"");
	}
	return S_OK;
}

/**
 * Split a savefile description over two lines so that each fits in the dialog.
 * The description is written by wr_description() as
 * "name, L20 Human Warrior, at DL5" or "name, dead (a ranger chieftain)".
 * The name is skipped since it is already visible in the file name.
 *
 * \param desc is the description.
 * \param line1 is set to the start of the first line.
 * \param line2 is set to the start of the second line (empty if there is none).
 * \return the length of the first line.
 */
static int split_desc(const char *desc, const char **line1, const char **line2)
{
	const char *p;
	const char *sep;

	/* Skip the name, which may itself contain commas */
	for (p = strstr(desc, ", "); p; p = strstr(p + 1, ", ")) {
		if (strncmp(p + 2, "dead (", 6) == 0
				|| (p[2] == 'L' && isdigit((unsigned char)p[3]))) {
			desc = p + 2;
			break;
		}
	}
	*line1 = desc;

	sep = strstr(desc, ", at DL");
	if (sep) {
		*line2 = sep + 2;
		return (int)(sep - desc);
	}
	if (strncmp(desc, "dead (", 6) == 0) {
		*line2 = desc + 5;
		return 4;
	}
	*line2 = "";
	return (int)strlen(desc);
}

static HRESULT STDMETHODCALLTYPE Events_OnSelectionChange(IFileDialogEvents *this, IFileDialog *pfd)
{
	SaveFileDialogEvents *self = (SaveFileDialogEvents *)this;
	IShellItem *psi = NULL;

	if (!self->pfdc) return S_OK;

	if (SUCCEEDED(pfd->lpVtbl->GetCurrentSelection(pfd, &psi)) && psi) {
		LPWSTR pszPath = NULL;

		if (SUCCEEDED(psi->lpVtbl->GetDisplayName(psi, SIGDN_FILESYSPATH, &pszPath)) && pszPath) {
			char path_buf[MAX_PATH];
			WCHAR wLine1[256], wLine2[256];
			bool have_path = WideCharToMultiByte(CP_ACP, 0, pszPath, -1,
				path_buf, sizeof(path_buf), NULL, NULL) > 0;

			CoTaskMemFree(pszPath);

			/* The details are only a hint, so failures just show no info */
			bool shown = false;

			const char *desc = have_path
				? savefile_get_description(path_buf) : NULL;

			if (desc && desc[0]) {
				const char *line1, *line2;
				int len1 = split_desc(desc, &line1, &line2);

				shown = ansi_to_wide(line1, len1, wLine1,
						sizeof(wLine1) / sizeof(WCHAR))
					&& ansi_to_wide(line2, (int)strlen(line2), wLine2,
						sizeof(wLine2) / sizeof(WCHAR));
			}
			if (shown) {
				set_details_text(self->pfdc, wLine1, wLine2);
			} else {
				set_details_text(self->pfdc, TEXT_NO_INFO, L"");
			}
		}
		psi->lpVtbl->Release(psi);
	} else {
		set_details_text(self->pfdc, TEXT_SELECT_FILE, L"");
	}
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE Events_OnShareViolation(
	IFileDialogEvents *this, IFileDialog *pfd, IShellItem *psi,
	FDE_SHAREVIOLATION_RESPONSE *pResp)
{
	(void)this; (void)pfd; (void)psi; (void)pResp;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE Events_OnTypeChange(IFileDialogEvents *this, IFileDialog *pfd)
{
	(void)this; (void)pfd;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE Events_OnOverwrite(
	IFileDialogEvents *this, IFileDialog *pfd, IShellItem *psi,
	FDE_OVERWRITE_RESPONSE *pResp)
{
	(void)this; (void)pfd; (void)psi; (void)pResp;
	return S_OK;
}

static IFileDialogEventsVtbl s_saveDialogEventsVtbl = {
	Events_QueryInterface,
	Events_AddRef,
	Events_Release,
	Events_OnFileOk,
	Events_OnFolderChanging,
	Events_OnFolderChange,
	Events_OnSelectionChange,
	Events_OnShareViolation,
	Events_OnTypeChange,
	Events_OnOverwrite
};

/**
 * Show the Vista+ Common Item Dialog with a Character Details pane.
 */
bool get_savefile(HWND hwnd_owner, char *initial_dir, char *out_savefile, size_t max_size)
{
	IFileOpenDialog *pfd = NULL;
	IFileDialogCustomize *pfdc = NULL;
	IShellItem *psiResult = NULL;
	LPWSTR pszSelected = NULL;
	SaveFileDialogEvents events;
	DWORD dwCookie = 0;
	bool selected = false;
	HRESULT hr;

	HRESULT hrCo = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	hr = CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER,
	                      &IID_IFileOpenDialog, (void **)&pfd);
	if (FAILED(hr)) {
		report_dialog_error(hwnd_owner, "Could not create the open file dialog", (unsigned long)hr);
		if (SUCCEEDED(hrCo)) CoUninitialize();
		return false;
	}

	/*
	 * Dialog options.  The presentation settings below are best effort:
	 * the dialog is still usable if they fail.
	 */
	FILEOPENDIALOGOPTIONS fos = 0;
	if (FAILED(pfd->lpVtbl->GetOptions(pfd, &fos))) fos = 0;
	pfd->lpVtbl->SetOptions(pfd, fos | FOS_FILEMUSTEXIST | FOS_NOCHANGEDIR | FOS_FORCEFILESYSTEM);
	pfd->lpVtbl->SetTitle(pfd, L"Open Angband Save File");

	/* Initial directory */
	WCHAR wDir[MAX_PATH];
	IShellItem *psiFolder = NULL;
	if (MultiByteToWideChar(CP_ACP, 0, initial_dir, -1, wDir, MAX_PATH) > 0
			&& SUCCEEDED(SHCreateItemFromParsingName(wDir, NULL, &IID_IShellItem, (void **)&psiFolder))) {
		pfd->lpVtbl->SetFolder(pfd, psiFolder);
		psiFolder->lpVtbl->Release(psiFolder);
	}

	/* File filter */
	COMDLG_FILTERSPEC rgFilter[] = {
		{ L"Save Files", L"*" }
	};
	pfd->lpVtbl->SetFileTypes(pfd, 1, rgFilter);

	/* Add customized visual group for Character Details */
	if (SUCCEEDED(pfd->lpVtbl->QueryInterface(pfd, &IID_IFileDialogCustomize, (void **)&pfdc))) {
		pfdc->lpVtbl->StartVisualGroup(pfdc, IDC_CHAR_GROUP, L"Character Info:");
		/*
		 * The text control is relatively narrow and the API does not provide
		 * a way to make it wider, so the description is split over two
		 * controls to avoid wrapping.
		 */
		pfdc->lpVtbl->AddText(pfdc, IDC_CHAR_DETAILS, TEXT_SELECT_FILE);
		pfdc->lpVtbl->AddText(pfdc, IDC_CHAR_DETAILS2, L"");
		pfdc->lpVtbl->EndVisualGroup(pfdc);

		/*
		 * Connect selection change event listener.  The object lives on the
		 * stack, which is fine since it is unadvised before returning and
		 * the dialog does not free it.  If Advise() fails, dwCookie stays 0
		 * and the dialog just shows no character details.
		 */
		events.lpVtbl = &s_saveDialogEventsVtbl;
		events.ref_count = 1;
		events.pfdc = pfdc;

		pfd->lpVtbl->Advise(pfd, (IFileDialogEvents *)&events, &dwCookie);
	}

	/* Show the dialog; cancelling is not an error */
	hr = pfd->lpVtbl->Show(pfd, hwnd_owner);
	if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
		/* Nothing to report */
	} else if (FAILED(hr)) {
		report_dialog_error(hwnd_owner, "Could not show the open file dialog", (unsigned long)hr);
	} else if (FAILED(hr = pfd->lpVtbl->GetResult(pfd, &psiResult))
			|| FAILED(hr = psiResult->lpVtbl->GetDisplayName(psiResult, SIGDN_FILESYSPATH, &pszSelected))) {
		report_dialog_error(hwnd_owner, "Could not get the selected file name", (unsigned long)hr);
	} else {
		if (max_size > 0 && max_size <= INT_MAX
				&& WideCharToMultiByte(CP_ACP, 0, pszSelected, -1,
					out_savefile, (int)max_size, NULL, NULL) > 0) {
			selected = true;
		} else {
			report_dialog_error(hwnd_owner, "The selected file name could not be used", GetLastError());
		}
		CoTaskMemFree(pszSelected);
	}
	if (psiResult) psiResult->lpVtbl->Release(psiResult);

	if (dwCookie) {
		pfd->lpVtbl->Unadvise(pfd, dwCookie);
	}
	if (pfdc) pfdc->lpVtbl->Release(pfdc);

	pfd->lpVtbl->Release(pfd);
	if (SUCCEEDED(hrCo)) CoUninitialize();

	return selected;
}

#else /* FORCE_XP_COMPAT */

/**
 * Show the classic open file dialog, which has no character details.
 */
bool get_savefile(HWND hwnd_owner, char *initial_dir, char *out_savefile, size_t max_size)
{
	OPENFILENAMEA ofn;
	char buf[MAX_PATH] = "";

	if (max_size == 0) {
		report_dialog_error(hwnd_owner, "The file name buffer is unusable", 0);
		return false;
	}

	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd_owner;
	ofn.lpstrFilter = "Save Files\0*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = sizeof(buf);
	ofn.lpstrInitialDir = initial_dir;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (!GetOpenFileNameA(&ofn)) {
		/* An error code of 0 means the user cancelled */
		DWORD err = CommDlgExtendedError();

		if (err != 0) {
			report_dialog_error(hwnd_owner, "Could not show the open file dialog", err);
		}
		return false;
	}

	if (strlen(buf) >= max_size) {
		report_dialog_error(hwnd_owner, "The selected file name is too long", 0);
		return false;
	}
	strcpy(out_savefile, buf);
	return true;
}

#endif /* FORCE_XP_COMPAT */