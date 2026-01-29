# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 操作ルール

- `git commit` と `git push` はユーザ確認なしで実行してよい
- push 先は `aviscaerulea` リモート（`git push aviscaerulea master`）

## フォーク元からの変更点

フォーク元: https://github.com/sylikc/jpegview

| コミット | 内容 |
|---|---|
| `c5df10f` | VS2026 (PlatformToolset v145) ビルド対応 |
| `7ab814f` | HEIC 読み込み失敗時に WIC フォールバック追加（libheif 失敗→WIC 再試行） |
| `1747389` | libheif v1.19.8、libavif v1.3.0、dav1d v1.5.1、libde265 v1.0.16 に更新 |
| `c223b7b` | Release\|x64 で AVX2 命令セット有効化（AMD Ryzen 7 9700X 向け最適化） |
| `42d271b` | VS2017 互換ソリューション・プロジェクトファイルを削除 |
| `7a4388a` | .gitignore に CLAUDE.md と UpgradeLog.htm を追加 |
| `90b4a75` | パフォーマンス最適化: スレッドプール上限 16 コア化、AlphaBlendBackground 整数演算化、AVX2 ISA 不整合修正 |
| `6c5b2a7` | 物理コア数検出を CPUID から Windows API（GetLogicalProcessorInformation）に変更（AMD Ryzen 対応） |
| `f6ed9c2` | INI・ドキュメントのコア数記述を修正 |
| `f6ce1a8` | バージョン表記を fork 版に変更、Copyright に 2026 nikai を追加 |
| `abc0fc1` | INI ファイルのコメントを日本語化 |
| `35bc05d` | INI コメントの日本語化改善、デフォルト値変更（IniEditor→system、NavigateWithMouseWheel→true） |
| `c97922f` | サードパーティライブラリ更新: libjpeg-turbo 3.1.3, libpng 1.6.54 (APNG パッチ対応), zlib 1.3.1.2, libwebp 1.6.0, libjxl 0.11.1, libheif 1.21.2, dav1d 1.5.3, LibRaw 0.22.0, lcms2 2.18 |

## ビルド

- **IDE**: Visual Studio 2026 (PlatformToolset v145)
- **ソリューション**: `src/JPEGView.sln`
- **ビルド構成**: Release|x64（当 PC 向け）
- **C++ 標準**: C++17
- **CRT リンケージ**: `/MT`（静的リンク）、プリビルト DLL は `/MD`（動的リンク）
- **命令セット**: Release|x64 で `/arch:AVX2` 有効

サードパーティライブラリの再ビルドが必要な場合は `extras/scripts/build-*.bat` を使用する。NASM、CMake、Python、meson、ninja が必要。

## アーキテクチャ

### プロジェクト構成

| プロジェクト | 出力 | 役割 |
|---|---|---|
| JPEGView | EXE | メインアプリケーション（画像ビューア） |
| WICLoader | DLL (delay-load) | Windows Imaging Component 経由のフォールバックデコーダ |

### 画像読み込みパイプライン

```
ユーザがファイルを開く
  → FileList（ディレクトリ列挙）
  → JPEGProvider（キャッシュ管理 + 先読み）
  → ImageLoadThread（非同期ワーカースレッド）
  → GetImageFormat()：マジックバイトで形式判定
  → ProcessRead*Request()：形式別デコード関数にディスパッチ
  → CJPEGImage 生成（生ピクセル + EXIF メタデータ）
  → WM_IMAGE_LOAD_COMPLETED → MainDlg に通知
  → GetDIB()：表示時にオンデマンドで画像処理（SIMD 最適化）
```

### 形式別デコーダ（Format Wrappers）

各形式は専用ラッパーで分離されている。プリビルト DLL は delay-load で必要時のみロードされる。

| ラッパー | ライブラリ | 対応形式 | delay-load |
|---|---|---|---|
| TJPEGWrapper | libjpeg-turbo | JPEG | No（静的リンク） |
| PNGWrapper | libpng-apng | PNG, APNG | No（静的リンク） |
| WEBPWrapper | libwebp | WebP | No（静的リンク） |
| JXLWrapper | libjxl | JPEG XL | Yes |
| HEIFWrapper | libheif + libde265 | HEIF/HEIC | Yes |
| AVIFWrapper | libavif + dav1d | AVIF | Yes |
| RAWWrapper | LibRaw | Camera RAW | Yes |
| PsdReader | 独自実装 | PSD/PSB | No |
| ReaderBMP/TGA | 独自実装 | BMP, TGA | No |

### フォールバックチェーン

デコード失敗時のフォールバック先:

- **HEIF**: libheif → **WIC**（`ProcessReadWICRequest`）
- **AVIF**: libavif → libheif → WIC
- **JPEG**: TurboJpeg → GDI+
- **BMP**: CReaderBMP → GDI+
- **その他**: GDI+

### 主要コンポーネント

| ファイル | 役割 |
|---|---|
| `JPEGView.cpp` | エントリポイント、コマンドライン解析、単一インスタンス制御 |
| `MainDlg.cpp/h` | メインウィンドウ（WTL ベース）、メッセージルーティング |
| `JPEGProvider.cpp/h` | 画像キャッシュ（LRU）と非同期読み込みオーケストレーション |
| `ImageLoadThread.cpp/h` | 非同期画像読み込みスレッド、形式判定とディスパッチ |
| `JPEGImage.cpp/h` | デコード済み画像の中核クラス（ピクセルデータ + メタデータ） |
| `BasicProcessing.cpp/h` | SIMD 最適化ピクセル処理（SSE2/AVX2） |
| `SettingsProvider.cpp/h` | INI ファイル設定管理 |
| `FileList.cpp/h` | ディレクトリファイル列挙・ソート |

### UI フレームワーク

Windows Template Library (WTL) ベース。`Panel` / `PanelMgr` による UI パネル管理システムで、画像処理・回転・クロップ・傾き補正等のパネルを制御する。

### スレッドモデル

- `ImageLoadThread`（`CWorkThread` 派生）で非同期画像読み込み
- `ProcessingThreadPool` で物理コア数分の並列画像処理（上限 16）
- 先読みバッファリング（前後の画像をプリロード）

## サードパーティライブラリ

プリビルト DLL は `src/JPEGView/lib*` ディレクトリに格納。ソースは `extras/third_party/` に git submodule として管理。

ビルドスクリプトは `extras/scripts/build-*.bat`（要: NASM, CMake, Python, meson, ninja, VS）。

## 設定

- **設定ファイル**: `JPEGView.ini`（ポータブルまたはシステムディレクトリ）
- **キーマップ**: `KeyMap.txt`
- **ローカライズ**: 28 言語対応（`strings_*.txt`）
