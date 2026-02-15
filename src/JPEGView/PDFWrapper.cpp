#include "stdafx.h"
#include "PDFWrapper.h"
#include "MaxImageDef.h"

#ifndef WINXP

#include <fpdfview.h>

// PDFium API 全体を保護するミューテックス（RAII パターン）
class CPdfiumLock {
public:
	CPdfiumLock() { ::InitializeCriticalSection(&m_cs); }
	~CPdfiumLock() { ::DeleteCriticalSection(&m_cs); }
	void Lock() { ::EnterCriticalSection(&m_cs); }
	void Unlock() { ::LeaveCriticalSection(&m_cs); }
private:
	CRITICAL_SECTION m_cs;
};
static CPdfiumLock s_pdfiumLock;

// 画面サイズに基づく最適 DPI を計算（画面にフィットする DPI）
double PdfReader::CalculateOptimalDPI(double pageWidthPt, double pageHeightPt) {
	// PDF のページサイズは points 単位 (1 point = 1/72 inch)
	double pageWidthInch = pageWidthPt / 72.0;
	double pageHeightInch = pageHeightPt / 72.0;

	// プライマリモニタの画面解像度を取得
	int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

	// ページ全体が画面に収まる DPI を計算
	double dpiX = screenWidth / pageWidthInch;
	double dpiY = screenHeight / pageHeightInch;
	double dpi = min(dpiX, dpiY);

	// 下限 72 DPI（上限なし: MAX_IMAGE_PIXELS で安全を担保）
	dpi = max(72.0, dpi);
	return dpi;
}

void* PdfReader::ReadImage(int& width, int& height, int& bpp,
	bool& outOfMemory, const void* pBuffer, int fileSize)
{
	outOfMemory = false;
	width = height = 0;
	bpp = 4;  // BGRA

	unsigned char* pPixelData = NULL;

	// PDFium API 全体をロック（スレッドセーフティ確保）
	s_pdfiumLock.Lock();

	// PDFium 初期化（lock 内で安全に一度だけ実行）
	static bool initialized = false;
	if (!initialized) {
		FPDF_InitLibrary();
		initialized = true;
	}

	// メモリバッファから PDF をロード
	FPDF_DOCUMENT doc = FPDF_LoadMemDocument(pBuffer, fileSize, NULL);
	if (doc == NULL) {
		s_pdfiumLock.Unlock();
		return NULL;
	}

	// ページ数取得（表紙のみ対応だが取得はする）
	int pageCount = FPDF_GetPageCount(doc);
	if (pageCount < 1) {
		FPDF_CloseDocument(doc);
		s_pdfiumLock.Unlock();
		return NULL;
	}

	// ページ 0（表紙）をロード
	FPDF_PAGE page = FPDF_LoadPage(doc, 0);
	if (page == NULL) {
		FPDF_CloseDocument(doc);
		s_pdfiumLock.Unlock();
		return NULL;
	}

	// ページサイズ取得（points 単位）
	double pageWidthPt = FPDF_GetPageWidth(page);
	double pageHeightPt = FPDF_GetPageHeight(page);

	// 最適 DPI 計算 → ピクセルサイズ決定
	double dpi = CalculateOptimalDPI(pageWidthPt, pageHeightPt);
	width = (int)((pageWidthPt / 72.0) * dpi);
	height = (int)((pageHeightPt / 72.0) * dpi);

	// ピクセル上限チェック
	if (width > MAX_IMAGE_DIMENSION || height > MAX_IMAGE_DIMENSION) {
		FPDF_ClosePage(page);
		FPDF_CloseDocument(doc);
		s_pdfiumLock.Unlock();
		return NULL;
	}
	if (abs((double)width * height) > MAX_IMAGE_PIXELS) {
		outOfMemory = true;
		FPDF_ClosePage(page);
		FPDF_CloseDocument(doc);
		s_pdfiumLock.Unlock();
		return NULL;
	}

	// ビットマップ作成
	FPDF_BITMAP bitmap = FPDFBitmap_Create(width, height, 0);  // 0 = BGRA
	if (bitmap == NULL) {
		outOfMemory = true;
		FPDF_ClosePage(page);
		FPDF_CloseDocument(doc);
		s_pdfiumLock.Unlock();
		return NULL;
	}

	// 白背景填充
	FPDFBitmap_FillRect(bitmap, 0, 0, width, height, 0xFFFFFFFF);

	// レンダリング（品質優先: AA + LCD テキスト最適化）
	int flags = FPDF_ANNOT | FPDF_LCD_TEXT;
	FPDF_RenderPageBitmap(bitmap, page, 0, 0, width, height, 0, flags);

	// ピクセルデータ取得
	void* buffer_ptr = FPDFBitmap_GetBuffer(bitmap);
	int stride = FPDFBitmap_GetStride(bitmap);

	if (buffer_ptr == NULL || stride < width * bpp) {
		FPDFBitmap_Destroy(bitmap);
		FPDF_ClosePage(page);
		FPDF_CloseDocument(doc);
		s_pdfiumLock.Unlock();
		return NULL;
	}

	// ピクセルデータをコピー
	int size = width * bpp * height;
	pPixelData = new(std::nothrow) unsigned char[size];
	if (pPixelData == NULL) {
		outOfMemory = true;
		FPDFBitmap_Destroy(bitmap);
		FPDF_ClosePage(page);
		FPDF_CloseDocument(doc);
		s_pdfiumLock.Unlock();
		return NULL;
	}

	// PDFium の BGRA は JPEGView と同じフォーマット
	// stride が width*bpp より大きい場合は行ごとにコピー
	for (int y = 0; y < height; y++) {
		memcpy(pPixelData + y * width * bpp,
		       (unsigned char*)buffer_ptr + y * stride,
		       width * bpp);
	}

	// リソース解放（ドキュメントは毎回即座にクローズ）
	FPDFBitmap_Destroy(bitmap);
	FPDF_ClosePage(page);
	FPDF_CloseDocument(doc);

	s_pdfiumLock.Unlock();

	return (void*)pPixelData;
}

#endif // WINXP
