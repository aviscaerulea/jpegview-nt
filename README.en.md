# JPEGView-nt
[![日本語](https://img.shields.io/badge/lang-日本語-red)](README.md)
[![English](https://img.shields.io/badge/lang-English-blue)](README.en.md)
[![Release](https://img.shields.io/github/v/release/aviscaerulea/jpegview-nt)](https://github.com/aviscaerulea/jpegview-nt/releases/latest)
[![License](https://img.shields.io/github/license/aviscaerulea/jpegview-nt)](LICENSE)
[![Build](https://github.com/aviscaerulea/jpegview-nt/actions/workflows/release.yml/badge.svg)](https://github.com/aviscaerulea/jpegview-nt/actions/workflows/release.yml)

JPEGView-nt is a lean and fast image viewer for Windows. It is a personally customized fork of [sylikc/jpegview](https://github.com/sylikc/jpegview) v1.3.46.0, with expanded format support and improved display speed. It can display HEIC/HEIF and SVG, and AVX2 instructions combined with multithreaded processing keep even large images responsive.

## Features

- Formats: 19 built-in including JPEG, PNG, WebP, HEIC/HEIF, AVIF, JPEG XL, SVG, and camera RAW
- Fast display: SIMD arithmetic, multithreaded processing, and a read-ahead cache working together
- Image adjustment: sharpness, color balance, contrast, rotation, and perspective while viewing
- Slideshow: plays the images in a folder one after another
- Portable operation: no installation required; run it straight from the extracted folder
- Multilingual UI: 28 languages, auto-detected from the Windows locale

### Supported Image Formats

| Category | Formats |
| --- | --- |
| Common | JPEG (jpg, jpeg, jfif), PNG (including APNG), GIF, BMP, TIFF |
| Modern | WebP, JPEG XL, HEIF/HEIC, AVIF, QOI |
| Vector | SVG, SVGZ |
| Other | TGA, PSD/PSB |
| Camera RAW | DNG, CR2/CR3, NEF, ARW, ORF, RW2, RAF, and more |
| Via WIC | WDP, HDP, JXR |

For the full list of supported camera models, see [LibRaw supported cameras](https://www.libraw.org/supported-cameras). Formats not listed above are still displayed through Windows Imaging Component when Windows itself supports them.

### Changes from the Upstream Project

| Category | Details |
| --- | --- |
| Formats | Added SVG/SVGZ display. HEIC falls back to the built-in Windows decoder when decoding fails |
| Performance | Enabled AVX2 instructions, raised the parallel processing limit to 64 threads, and sped up memory allocation |
| Dependencies | Updated libjpeg-turbo, libpng, libwebp, libjxl, libheif, LibRaw, and others to their latest versions |

## Installation

### Requirements

- Windows 10/11, 64-bit
- A CPU with AVX2 support

### Steps

#### From the release ZIP

Download the ZIP from the [releases page](https://github.com/aviscaerulea/jpegview-nt/releases/latest) and extract it anywhere. Run `JPEGView.exe` from the extracted folder as-is.

By default the settings are saved next to the executable, so do not place the folder under `Program Files`, where writing is not permitted.

#### From Scoop

```shell
scoop bucket add aviscaerulea https://github.com/aviscaerulea/scoop-bucket
scoop install jpegview-nt
```

## Usage

Drag an image file onto `JPEGView.exe`, or pick `JPEGView.exe` from the Explorer "Open with" menu. You can then browse through the other images in the same folder in order.

The main key bindings are as follows.

| Input | Action |
| --- | --- |
| ← / → | Move to the previous or next image |
| Mouse wheel | Move to the previous or next image |
| Space | Toggle between fit-to-screen and actual size |
| Ctrl + ↑ / ↓ | Zoom in, zoom out |
| F11 | Toggle full screen |
| Ctrl + N | Toggle the navigation panel |
| Ctrl + C | Copy the image to the clipboard |
| Ctrl + S | Save the image |

Editing panels such as rotation, cropping, and perspective correction are opened from the navigation panel at the bottom of the window.

## Configuration

Settings are stored in `JPEGView.ini` next to the executable. Every entry carries an inline comment, so refer to the file itself for details.

| Key | Description |
| --- | --- |
| `StoreToEXEPath` | Where settings are saved. true for the folder holding the executable, false for the user application data folder |
| `CPUCoresUsed` | Number of CPU cores used for image processing. 0 auto-detects |
| `ReadAheadBuffers` | Number of read-ahead buffers. Larger values speed up sequential browsing and use more memory |
| `Language` | UI language. auto follows the Windows locale |

Key bindings are changed in `KeyMap.txt`, located in the same folder.

## Limitations

- Only a 64-bit build is distributed; no 32-bit build and no MSI installer
- The application will not start on a CPU without AVX2 support
- The executable is not code-signed, so SmartScreen shows a warning on first launch
- The bundled documentation (`doc` folder) is from upstream and does not cover this fork's changes

## License

The application itself is licensed under the GNU General Public License v2. See [LICENSE](LICENSE) for details.

The bundled libheif, libde265, and LibRaw are LGPL-licensed, so they are shipped as DLLs rather than statically linked. Users are free to replace those DLLs.
