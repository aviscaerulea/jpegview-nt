# JPEGView-nt
[![日本語](https://img.shields.io/badge/lang-日本語-red)](README.md)
[![English](https://img.shields.io/badge/lang-English-blue)](README.en.md)
[![Release](https://img.shields.io/github/v/release/aviscaerulea/jpegview-nt)](https://github.com/aviscaerulea/jpegview-nt/releases/latest)
[![License](https://img.shields.io/github/license/aviscaerulea/jpegview-nt)](LICENSE)
[![Build](https://github.com/aviscaerulea/jpegview-nt/actions/workflows/release.yml/badge.svg)](https://github.com/aviscaerulea/jpegview-nt/actions/workflows/release.yml)

JPEGView-nt は、Windows 向けの軽量で高速な画像ビューアです。[sylikc/jpegview](https://github.com/sylikc/jpegview) の `v1.3.46.0` からフォークし、対応形式と表示速度を強化した個人カスタマイズ版です。HEIC/HEIF と SVG の表示に対応し、AVX2 命令とマルチスレッド処理によって大きな画像でも待たされずに閲覧できます。

## 機能

- 対応形式：JPEG、PNG、WebP、HEIC/HEIF、AVIF、JPEG XL、SVG、カメラ RAW など 19 形式を内蔵対応
- 高速表示：SIMD 演算、マルチスレッド処理、先読みキャッシュの組み合わせによる表示の高速化
- 画像調整：シャープネス、色調、コントラスト、回転、遠近補正を表示したまま適用
- スライドショー：フォルダ内の画像を連続再生
- ポータブル動作：インストール不要で、展開したフォルダのまま実行可能
- 多言語 UI：28 言語に対応し、Windows のロケールを自動判定

### 対応画像形式

| 区分 | 形式 |
| --- | --- |
| 一般 | JPEG（jpg, jpeg, jfif）、PNG（APNG を含む）、GIF、BMP、TIFF |
| 新しい形式 | WebP、JPEG XL、HEIF/HEIC、AVIF、QOI |
| ベクタ | SVG、SVGZ |
| その他 | TGA、PSD/PSB |
| カメラ RAW | DNG、CR2/CR3、NEF、ARW、ORF、RW2、RAF ほか |
| WIC 経由 | WDP、HDP、JXR |

カメラ RAW の詳細な対応機種は [LibRaw supported cameras](https://www.libraw.org/supported-cameras) を参照してください。上記以外の形式も、Windows が対応していれば Windows Imaging Component 経由で表示します。

### フォーク元からの変更点

| 分類 | 内容 |
| --- | --- |
| 表示形式 | SVG/SVGZ の表示に対応。HEIC は読み込みに失敗すると Windows 標準の機能で再試行 |
| 高速化 | AVX2 命令の有効化、並列処理の上限を 64 スレッドへ拡大、メモリ確保の高速化 |
| 依存ライブラリ | libjpeg-turbo、libpng、libwebp、libjxl、libheif、LibRaw などを最新版へ更新 |

## インストール

### 動作要件

- Windows 10/11 の 64bit 版
- AVX2 に対応した CPU

### 手順

#### リリースの ZIP から

[リリースページ](https://github.com/aviscaerulea/jpegview-nt/releases/latest)から ZIP をダウンロードします。次に任意の場所へ展開します。展開したフォルダの `JPEGView.exe` をそのまま実行できます。

初期状態では設定を実行ファイルと同じフォルダへ保存するため、書き込みできない `Program Files` 配下には置かないでください。

#### Scoop から

```shell
scoop bucket add aviscaerulea https://github.com/aviscaerulea/scoop-bucket
scoop install jpegview-nt
```

## 使い方

`JPEGView.exe` に画像ファイルをドラッグするか、エクスプローラの「プログラムから開く」で `JPEGView.exe` を選ぶと画像を表示します。表示した画像と同じフォルダ内の画像を、そのまま順に閲覧できます。

主なキー操作は次のとおりです。

| 操作 | 動作 |
| --- | --- |
| ← / → | 前後の画像へ移動 |
| マウスホイール | 前後の画像へ移動 |
| Space | 画面に合わせる表示と等倍表示の切り替え |
| Ctrl + ↑ / ↓ | 拡大、縮小 |
| F11 | 全画面表示の切り替え |
| Ctrl + N | ナビゲーションパネルの表示切り替え |
| Ctrl + C | 画像をクリップボードへコピー |
| Ctrl + S | 画像を保存 |

回転、クロップ、傾き補正などの編集パネルは、画面下部のナビゲーションパネルから開きます。

## 設定

設定は実行ファイルと同じフォルダの `JPEGView.ini` に保存します。各項目には日本語の説明を付けているため、詳細はファイル内のコメントを参照してください。

| 項目 | 内容 |
| --- | --- |
| `StoreToEXEPath` | 設定の保存先。true で実行ファイルと同じフォルダ、false でユーザのアプリデータフォルダ |
| `CPUCoresUsed` | 画像処理に使う CPU コア数。0 で自動検出 |
| `ReadAheadBuffers` | 先読みバッファ数。大きいほど連続閲覧が速くなり、メモリ使用量が増える |
| `Language` | UI の言語。auto で Windows のロケールに追従 |

キーの割り当ては、同じフォルダの `KeyMap.txt` で変更します。

## 制限事項

- 配布は 64bit 版のみで、32bit 版と MSI インストーラは提供しない
- AVX2 に対応しない CPU では起動できない
- 実行ファイルにコード署名がないため、初回起動時に SmartScreen の警告が出る
- 同梱ドキュメント（`doc` フォルダ）はフォーク元のもので、本フォークの変更点は反映していない

## ライセンス

本体は GNU General Public License v2 です。詳細は [LICENSE](LICENSE) を参照してください。

同梱する libheif、libde265、LibRaw は LGPL のため、静的リンクせず DLL として配布しています。利用者はこれらの DLL を差し替えられます。
