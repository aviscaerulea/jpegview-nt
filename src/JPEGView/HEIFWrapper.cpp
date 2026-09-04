#include "stdafx.h"

#include "HEIFWrapper.h"
#include "MaxImageDef.h"
#include "ICCProfileTransform.h"
#include "SettingsProvider.h"

#include <memory>
#include <vector>

// libheif is called through its C API: the C++ wrapper (heif_cxx.h) lost set_max_decoding_threads
// in v1.23 and gives no access to the raw heif_context.
// Errors are still thrown as heif::Error so the caller falls back to WIC as before.
namespace {

struct HeifContextDeleter { void operator()(heif_context* p) const { heif_context_free(p); } };
struct HeifHandleDeleter { void operator()(heif_image_handle* p) const { heif_image_handle_release(p); } };
struct HeifImageDeleter { void operator()(heif_image* p) const { heif_image_release(p); } };

void ThrowOnError(const heif_error& err)
{
	if (err.code != heif_error_Ok)
		throw heif::Error(err);
}

}

void * HeifReader::ReadImage(int &width,
					   int &height,
					   int &nchannels,
					   int &frame_count,
					   void* &exif_chunk,
					   bool &outOfMemory,
					   int frame_index,
					   const void *buffer,
					   int sizebytes)
{
	outOfMemory = false;
	width = height = 0;
	nchannels = 4;

	unsigned char* pPixelData = NULL;
	exif_chunk = NULL;

	std::unique_ptr<heif_context, HeifContextDeleter> context(heif_context_alloc());
	if (context == NULL) {
		outOfMemory = true;
		return NULL;
	}
	ThrowOnError(heif_context_read_from_memory_without_copy(context.get(), buffer, sizebytes, NULL));
	// Parallel HEIC tile decoding: use the CPUCoresUsed INI setting like the AVIF decoder
	heif_context_set_max_decoding_threads(context.get(), CSettingsProvider::This().NumberOfCoresToUse());
	frame_count = heif_context_get_number_of_top_level_images(context.get());
	std::vector<heif_item_id> item_ids(frame_count);
	heif_context_get_list_of_top_level_image_IDs(context.get(), item_ids.data(), frame_count);
	heif_item_id item_id = item_ids.at(frame_index);

	heif_image_handle* raw_handle = NULL;
	ThrowOnError(heif_context_get_image_handle(context.get(), item_id, &raw_handle));
	std::unique_ptr<heif_image_handle, HeifHandleDeleter> handle(raw_handle);

	heif_image* raw_image = NULL;
	ThrowOnError(heif_decode_image(handle.get(), &raw_image, heif_colorspace_RGB, heif_chroma_interleaved_RGBA, NULL));
	std::unique_ptr<heif_image, HeifImageDeleter> image(raw_image);

	int stride;
	const uint8_t* data = heif_image_get_plane_readonly(image.get(), heif_channel_interleaved, &stride);
	if (data == NULL)
		return NULL;
	width = heif_image_get_width(image.get(), heif_channel_interleaved);
	height = heif_image_get_height(image.get(), heif_channel_interleaved);

	if (width > MAX_IMAGE_DIMENSION || height > MAX_IMAGE_DIMENSION)
		return NULL;
	if (abs((double)width * height) > MAX_IMAGE_PIXELS) {
		outOfMemory = true;
		return NULL;
	}
	if (width < 1 || height < 1 || width * nchannels > stride)
		return NULL;

	size_t size = (size_t)width * nchannels * height;
	pPixelData = new(std::nothrow) unsigned char[size];
	if (pPixelData == NULL) {
		outOfMemory = true;
		return NULL;
	}
	std::vector<uint8_t> iccp(heif_image_get_raw_color_profile_size(image.get()));
	if (!iccp.empty())
		heif_image_get_raw_color_profile(image.get(), iccp.data());
	void* transform = ICCProfileTransform::CreateTransform(iccp.data(), iccp.size(), ICCProfileTransform::FORMAT_RGBA);
	size_t i, j;
	if (!ICCProfileTransform::DoTransform(transform, data, pPixelData, width, height, stride)) {
		unsigned int* o = (unsigned int*)pPixelData;
		for (i = 0; i < height; i++) {
			unsigned int* p = (unsigned int*)(data + i * stride);
			for (j = 0; j < width; j++) {
				// RGBA -> BGRA conversion
				*o = _rotr(_byteswap_ulong(*p), 8);
				p++;
				o++;
			}
		}
	}
	ICCProfileTransform::DeleteTransform(transform);

	// Use only the first Exif block; a metadata read failure does not discard the decoded image
	heif_item_id exif_id;
	if (heif_image_handle_get_list_of_metadata_block_IDs(handle.get(), "Exif", &exif_id, 1) == 1) {
		size_t exif_size = heif_image_handle_get_metadata_size(handle.get(), exif_id);
		// 65538 magic number comes from investigations by qbnu
		// see https://github.com/sylikc/jpegview/pull/213#pullrequestreview-1494451359 for more details
		/*
		* These libraries all have their own ideas about where to start the Exif data from.
		* JPEG Exif blocks are in the format
		  FF E1 SS SS 45 78 69 66 00 00 [data]

		  The SS SS is a big-endian unsigned short representing the size of everything after FF E1.

		  libjxl gives 00 00 00 00 [data]
		  libheif gives 00 00 00 00 45 78 69 66 00 00 [data]
		  libavif, libwebp and libpng give [data], so they have different limits for size.

		  If you want I can change it to 65536 + an offset and add notes explaining why.
		*/
		if (exif_size > 8 && exif_size < 65538) {
			std::vector<uint8_t> exif(exif_size);
			if (heif_image_handle_get_metadata(handle.get(), exif_id, exif.data()).code == heif_error_Ok) {
				exif_chunk = malloc(exif_size);
				if (exif_chunk != NULL) {
					memcpy(exif_chunk, exif.data(), exif_size);
					*((unsigned short*)exif_chunk) = _byteswap_ushort(0xFFE1);
					*((unsigned short*)exif_chunk + 1) = _byteswap_ushort(exif_size - 2);
				}
			}
		}
	}

	return (void*)pPixelData;
}
