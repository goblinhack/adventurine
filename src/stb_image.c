/* stbi-1.18 - public domain JPEG/PNG reader - http://nothings.org/stb_image.c
                      when you control the images you're loading

   QUICK NOTES:
      Primarily of interest to game developers and other people who can
          avoid problematic images and only need the trivial interface

      JPEG baseline (no JPEG progressive, no oddball channel decimations)
      PNG 8-bit only
      BMP non-1bpp, non-RLE
      TGA (not sure what subset, if a subset)
      PSD (composited view only, no extra channels)
      HDR (radiance rgbE format)
      writes BMP,TGA (define STBI_NO_WRITE to remove code)
      decoded from memory or through stdio FILE (define STBI_NO_STDIO to remove code)
      supports installable dequantizing-IDCT, YCbCr-to-RGB conversion (define STBI_SIMD)
        
   TODO:
      stbi_info_*
  
   history:
      1.18   fix a threading bug (local mutable static)
      1.17   support interlaced PNG
      1.16   major bugfix - convert_format converted one too many pixels
      1.15   initialize some fields for thread safety
      1.14   fix threadsafe conversion bug; header-file-only version (#define STBI_HEADER_FILE_ONLY before including)
      1.13   threadsafe
      1.12   const qualifiers in the API
      1.11   Support installable IDCT, colorspace conversion routines
      1.10   Fixes for 64-bit (don't use "unsigned long")
             optimized upsampling by Fabian "ryg" Giesen
      1.09   Fix format-conversion for PSD code (bad global variables!)
      1.08   Thatcher Ulrich's PSD code integrated by Nicolas Schulz
      1.07   attempt to fix C++ warning/errors again
      1.06   attempt to fix C++ warning/errors again
      1.05   fix TGA loading to return correct *comp and use good luminance calc
      1.04   default float alpha is 1, not 255; use 'void *' for stbi_image_free
      1.03   bugfixes to STBI_NO_STDIO, STBI_NO_HDR
      1.02   support for (subset of) HDR files, float interface for preferred access to them
      1.01   fix bug: possible bug in handling right-side up bmps... not sure
             fix bug: the stbi_bmp_load() and stbi_tga_load() functions didn't work at all
      1.00   interface to zlib that skips zlib header
      0.99   correct handling of alpha in palette
      0.98   TGA loader by lonesock; dynamically add loaders (untested)
      0.97   jpeg errors on too large a file; also catch another malloc failure
      0.96   fix detection of invalid v value - particleman@mollyrocket forum
      0.95   during header scan, seek to markers in case of padding
      0.94   STBI_NO_STDIO to disable stdio usage; rename all #defines the same
      0.93   handle jpegtran output; verbose errors
      0.92   read 4,8,16,24,32-bit BMP files of several formats
      0.91   output 24-bit Windows 3.0 BMP files
      0.90   fix a few more warnings; bump version number to approach 1.0
      0.61   bugfixes due to Marc LeBlanc, Christopher Lloyd
      0.60   fix compiling as c++
      0.59   fix warnings: merge Dave Moore's -Wall fixes
      0.58   fix bug: zlib uncompressed mode len/nlen was wrong endian
      0.57   fix bug: jpg last huffman symbol before marker was >9 bits but less
                      than 16 available
      0.56   fix bug: zlib uncompressed mode len vs. nlen
      0.55   fix bug: restart_interval not initialized to 0
      0.54   allow 0 for 'int *comp'
      0.53   fix bug in png 3->4; speedup png decoding
      0.52   png handles req_comp=3,4 directly; minor cleanup; jpeg comments
      0.51   obey req_comp requests, 1-component jpegs return as 1-component,
             on 'test' only check type, not whether we support this variant
*/

#include "stb_image.h"

#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STBI_INCLUDE_STB_IMAGE_H

////   begin header file  ////////////////////////////////////////////////////
//
// Limitations:
//    - no progressive/interlaced support (jpeg, png)
//    - 8-bit samples only (jpeg, png)
//    - not threadsafe
//    - channel subsampling of at most 2 in each dimension (jpeg)
//    - no delayed line count (jpeg) -- IJG doesn't support either
//
// Basic usage (see HDR discussion below):
//    int x,y,n;
//    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
//    // ... process data if not 0 ... 
//    // ... x = width, y = height, n = # 8-bit components per pixel ...
//    // ... replace '0' with '1'..'4' to force that many components per pixel
//    stbi_image_free(data)
//
// Standard parameters:
//    int *x       -- outputs image width in pixels
//    int *y       -- outputs image height in pixels
//    int *comp    -- outputs # of image components in image file
//    int req_comp -- if non-zero, # of image components requested in result
//
// The return value from an image loader is an 'unsigned char *' which points
// to the pixel data. The pixel data consists of *y scanlines of *x pixels,
// with each pixel consisting of N interleaved 8-bit components; the first
// pixel pointed to is top-left-most in the image. There is no padding between
// image scanlines or between pixels, regardless of format. The number of
// components N is 'req_comp' if req_comp is non-zero, or *comp otherwise.
// If req_comp is non-zero, *comp has the number of components that _would_
// have been output otherwise. E.g. if you set req_comp to 4, you will always
// get RGBA output, but you can check *comp to easily see if it's opaque.
//
// An output image with N components has the following components interleaved
// in this order in each pixel:
//
//     N=#comp     components
//       1           grey
//       2           grey, alpha
//       3           red, green, blue
//       4           red, green, blue, alpha
//
// If image loading fails for any reason, the return value will be 0,
// and *x, *y, *comp will be unchanged. The function stbi_failure_reason()
// can be queried for an extremely brief, end-user unfriendly explanation
// of why the load failed. Define STBI_NO_FAILURE_STRINGS to avoid
// compiling these strings at all, and STBI_FAILURE_USERMSG to get slightly
// more user-friendly ones.
//
// Paletted PNG and BMP images are automatically depalettized.
//
//
// ===========================================================================
//
// HDR image support   (disable by defining STBI_NO_HDR)
//
// stb_image now supports loading HDR images in general, and currently
// the Radiance .HDR file format, although the support is provided
// generically. You can still load any file through the existing interface;
// if you attempt to load an HDR file, it will be automatically remapped to
// LDR, assuming gamma 2.2 and an arbitrary scale factor defaulting to 1;
// both of these constants can be reconfigured through this interface:
//
//     stbi_hdr_to_ldr_gamma(2.2f);
//     stbi_hdr_to_ldr_scale(1.0f);
//
// (note, do not use _inverse_ constants; stbi_image will invert them
// appropriately).
//
// Additionally, there is a new, parallel interface for loading files as
// (linear) floats to preserve the full dynamic range:
//
//    float *data = stbi_loadf(filename, &x, &y, &n, 0);
// 
// If you load LDR images through this interface, those images will
// be promoted to floating point values, run through the inverse of
// constants corresponding to the above:
//
//     stbi_ldr_to_hdr_scale(1.0f);
//     stbi_ldr_to_hdr_gamma(2.2f);
//
// Finally, given a filename (or an open file or memory block--see header
// file for details) containing image data, you can query for the "most
// appropriate" interface to use (that is, whether the image is HDR or
// not), using:
//
//     stbi_is_hdr(char *filename);

#ifndef STBI_NO_STDIO
#endif

#define STBI_VERSION 1

enum
{
   STBI_default = 0, // only used for req_comp

   STBI_grey       = 1,
   STBI_grey_alpha = 2,
   STBI_rgb        = 3,
   STBI_rgb_alpha  = 4,
};

typedef unsigned char stbi_uc;

#ifdef __cplusplus
extern "C" {
#endif

// WRITING API

#if !defined(STBI_NO_WRITE) && !defined(STBI_NO_STDIO)
// write a BMP/TGA file given tightly packed 'comp' channels (no padding, nor bmp-stride-padding)
// (you must include the appropriate extension in the filename).
// returns TRUE on success, FALSE if couldn't open file, error writing file
extern int      stbi_write_bmp       (char const *filename,     int x, int y, int comp, void *data);
extern int      stbi_write_tga       (char const *filename,     int x, int y, int comp, void *data);
#endif

// PRIMARY API - works on images of any type

// load image by filename, open file, or memory buffer
#ifndef STBI_NO_STDIO
extern stbi_uc *stbi_load            (char const *filename,     int *x, int *y, int *comp, int req_comp);
extern stbi_uc *stbi_load_from_file  (FILE *f,                  int *x, int *y, int *comp, int req_comp);
extern int      stbi_info_from_file  (FILE *f,                  int *x, int *y, int *comp);
#endif
extern stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);
// for stbi_load_from_file, file pointer is left pointing immediately after image

#ifndef STBI_NO_HDR
#ifndef STBI_NO_STDIO
extern float *stbi_loadf            (char const *filename,     int *x, int *y, int *comp, int req_comp);
extern float *stbi_loadf_from_file  (FILE *f,                  int *x, int *y, int *comp, int req_comp);
#endif
extern float *stbi_loadf_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);

extern void   stbi_hdr_to_ldr_gamma(float gamma);
extern void   stbi_hdr_to_ldr_scale(float scale);

extern void   stbi_ldr_to_hdr_gamma(float gamma);
extern void   stbi_ldr_to_hdr_scale(float scale);

#endif // STBI_NO_HDR

// get a VERY brief reason for failure
// NOT THREADSAFE
extern char    *stbi_failure_reason  (void); 

// free the loaded image -- this is just free()
extern void     stbi_image_free      (void *retval_from_stbi_load);

// get image dimensions & components without fully decoding
extern int      stbi_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp);
extern int      stbi_is_hdr_from_memory(stbi_uc const *buffer, int len);
#ifndef STBI_NO_STDIO
extern int      stbi_info            (char const *filename,     int *x, int *y, int *comp);
extern int      stbi_is_hdr          (char const *filename);
extern int      stbi_is_hdr_from_file(FILE *f);
#endif

// ZLIB client - used by PNG, available for other purposes

extern char *stbi_zlib_decode_malloc_guesssize(const char *buffer, int len, int initial_size, int *outlen);
extern char *stbi_zlib_decode_malloc(const char *buffer, int len, int *outlen);
extern int   stbi_zlib_decode_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);

extern char *stbi_zlib_decode_noheader_malloc(const char *buffer, int len, int *outlen);
extern int   stbi_zlib_decode_noheader_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);

// TYPE-SPECIFIC ACCESS

// is it a jpeg?
extern int      stbi_jpeg_test_memory     (stbi_uc const *buffer, int len);
extern stbi_uc *stbi_jpeg_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);
extern int      stbi_jpeg_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp);

#ifndef STBI_NO_STDIO
extern stbi_uc *stbi_jpeg_load            (char const *filename,     int *x, int *y, int *comp, int req_comp);
extern int      stbi_jpeg_test_file       (FILE *f);
extern stbi_uc *stbi_jpeg_load_from_file  (FILE *f,                  int *x, int *y, int *comp, int req_comp);

extern int      stbi_jpeg_info            (char const *filename,     int *x, int *y, int *comp);
extern int      stbi_jpeg_info_from_file  (FILE *f,                  int *x, int *y, int *comp);
#endif

// is it a png?
extern int      stbi_png_test_memory      (stbi_uc const *buffer, int len);
extern stbi_uc *stbi_png_load_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);
extern int      stbi_png_info_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp);

#ifndef STBI_NO_STDIO
extern stbi_uc *stbi_png_load             (char const *filename,     int *x, int *y, int *comp, int req_comp);
extern int      stbi_png_info             (char const *filename,     int *x, int *y, int *comp);
extern int      stbi_png_test_file        (FILE *f);
extern stbi_uc *stbi_png_load_from_file   (FILE *f,                  int *x, int *y, int *comp, int req_comp);
extern int      stbi_png_info_from_file   (FILE *f,                  int *x, int *y, int *comp);
#endif

// is it a bmp?
extern int      stbi_bmp_test_memory      (stbi_uc const *buffer, int len);

extern stbi_uc *stbi_bmp_load             (char const *filename,     int *x, int *y, int *comp, int req_comp);
extern stbi_uc *stbi_bmp_load_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);
#ifndef STBI_NO_STDIO
extern int      stbi_bmp_test_file        (FILE *f);
extern stbi_uc *stbi_bmp_load_from_file   (FILE *f,                  int *x, int *y, int *comp, int req_comp);
#endif

// is it a tga?
extern int      stbi_tga_test_memory      (stbi_uc const *buffer, int len);

extern stbi_uc *stbi_tga_load             (char const *filename,     int *x, int *y, int *comp, int req_comp);
extern stbi_uc *stbi_tga_load_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);
#ifndef STBI_NO_STDIO
extern int      stbi_tga_test_file        (FILE *f);
extern stbi_uc *stbi_tga_load_from_file   (FILE *f,                  int *x, int *y, int *comp, int req_comp);
#endif

// is it a psd?
extern int      stbi_psd_test_memory      (stbi_uc const *buffer, int len);

extern stbi_uc *stbi_psd_load             (char const *filename,     int *x, int *y, int *comp, int req_comp);
extern stbi_uc *stbi_psd_load_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);
#ifndef STBI_NO_STDIO
extern int      stbi_psd_test_file        (FILE *f);
extern stbi_uc *stbi_psd_load_from_file   (FILE *f,                  int *x, int *y, int *comp, int req_comp);
#endif

// is it an hdr?
extern int      stbi_hdr_test_memory      (stbi_uc const *buffer, int len);

extern float *  stbi_hdr_load             (char const *filename,     int *x, int *y, int *comp, int req_comp);
extern float *  stbi_hdr_load_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);
#ifndef STBI_NO_STDIO
extern int      stbi_hdr_test_file        (FILE *f);
extern float *  stbi_hdr_load_from_file   (FILE *f,                  int *x, int *y, int *comp, int req_comp);
#endif

// define new loaders
typedef struct
{
   int       (*test_memory)(stbi_uc const *buffer, int len);
   stbi_uc * (*load_from_memory)(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);
   #ifndef STBI_NO_STDIO
   int       (*test_file)(FILE *f);
   stbi_uc * (*load_from_file)(FILE *f, int *x, int *y, int *comp, int req_comp);
   #endif
} stbi_loader;

// register a loader by filling out the above structure (you must defined ALL functions)
// returns 1 if added or already added, 0 if not added (too many loaders)
// NOT THREADSAFE
extern int stbi_register_loader(stbi_loader *loader);

// define faster low-level operations (typically SIMD support)
#if STBI_SIMD
typedef void (*stbi_idct_8x8)(uint8_t *out, int out_stride, short data[64], unsigned short *dequantize);
// compute an integer IDCT on "input"
//     input[x] = data[x] * dequantize[x]
//     write results to 'out': 64 samples, each run of 8 spaced by 'out_stride'
//                             CLAMP results to 0..255
typedef void (*stbi_YCbCr_to_RGB_run)(uint8_t *output, uint8_t const *y, uint8_t const *cb, uint8_t const *cr, int count, int step);
// compute a conversion from YCbCr to RGB
//     'count' pixels
//     write pixels to 'output'; each pixel is 'step' bytes (either 3 or 4; if 4, write '255' as 4th), order R,G,B
//     y: Y input channel
//     cb: Cb input channel; scale/biased to be 0..255
//     cr: Cr input channel; scale/biased to be 0..255

extern void stbi_install_idct(stbi_idct_8x8 func);
extern void stbi_install_YCbCr_to_RGB(stbi_YCbCr_to_RGB_run func);
#endif // STBI_SIMD

#ifdef __cplusplus
}
#endif

//
//
////   end header file   /////////////////////////////////////////////////////
#endif // STBI_INCLUDE_STB_IMAGE_H

#ifndef STBI_HEADER_FILE_ONLY

#ifndef STBI_NO_HDR
#include <math.h>  // ldexp
#endif

#ifndef STBI_NO_STDIO
#include <stdio.h>
#endif
#include <stdlib.h>
#include <memory.h>
#include <stdarg.h>
#include "main.h"

// implementation:
typedef   signed short int16;
typedef   signed int    int32;
typedef unsigned int   uint;

// should produce compiler error if size is wrong
typedef unsigned char validate_uint32_t[sizeof(uint32_t)==4];

#if defined(STBI_NO_STDIO) && !defined(STBI_NO_WRITE)
#define STBI_NO_WRITE
#endif

//////////////////////////////////////////////////////////////////////////////
//
// Generic API that works on all image types
//

// this is not threadsafe
static char *failure_reason;

char *stbi_failure_reason(void)
{
   return failure_reason;
}

#if 0
static int e(const char *str)
{
    if (str) {
        ERR("%s", str);
    }
   failure_reason = str;
   return 0;
}
#endif

#define STBI_NO_FAILURE_STRINGS
#ifdef STBI_NO_FAILURE_STRINGS
   #define e(x,y)  0
#elif defined(STBI_FAILURE_USERMSG)
   #define e(x,y)  e(y)
#else
   #define e(x,y)  e(x)
#endif

#define epf(x,y)   ((float *) (e(x,y)?0:0))
#define epuc(x,y)  ((unsigned char *) (e(x,y)?0:0))

void stbi_image_free(void *retval_from_stbi_load)
{
   free(retval_from_stbi_load);
}

#define MAX_LOADERS  32
stbi_loader *loaders[MAX_LOADERS];
static int max_loaders = 0;

int stbi_register_loader(stbi_loader *loader)
{
   int i;
   for (i=0; i < MAX_LOADERS; ++i) {
      // already present?
      if (loaders[i] == loader)
         return 1;
      // end of the list?
      if (loaders[i] == 0) {
         loaders[i] = loader;
         max_loaders = i+1;
         return 1;
      }
   }
   // no room for it
   return 0;
}

#ifndef STBI_NO_HDR
static float   *ldr_to_hdr(stbi_uc *data, int x, int y, int comp);
static stbi_uc *hdr_to_ldr(float   *data, int x, int y, int comp);
#endif

#ifndef STBI_NO_STDIO
unsigned char *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp)
{
   FILE *f = fopen(filename, "rb");
   unsigned char *result;
   if (!f) return epuc((char*)"can't fopen", (char*)"Unable to open file");
   result = stbi_load_from_file(f,x,y,comp,req_comp);
   fclose(f);
   return result;
}

unsigned char *stbi_load_from_file(FILE *f, int *x, int *y, int *comp, int req_comp)
{
   int i;
   if (stbi_jpeg_test_file(f))
      return stbi_jpeg_load_from_file(f,x,y,comp,req_comp);
   if (stbi_png_test_file(f))
      return stbi_png_load_from_file(f,x,y,comp,req_comp);
   if (stbi_bmp_test_file(f))
      return stbi_bmp_load_from_file(f,x,y,comp,req_comp);
   if (stbi_psd_test_file(f))
      return stbi_psd_load_from_file(f,x,y,comp,req_comp);
   #ifndef STBI_NO_HDR
   if (stbi_hdr_test_file(f)) {
      float *hdr = stbi_hdr_load_from_file(f, x,y,comp,req_comp);
      return hdr_to_ldr(hdr, *x, *y, req_comp ? req_comp : *comp);
   }
   #endif
   for (i=0; i < max_loaders; ++i)
      if (loaders[i]->test_file(f))
         return loaders[i]->load_from_file(f,x,y,comp,req_comp);
   // test tga last because it's a crappy test!
   if (stbi_tga_test_file(f))
      return stbi_tga_load_from_file(f,x,y,comp,req_comp);
   return epuc((char*)"unknown image type", (char*)"Image not of any known type, or corrupt");
}
#endif

unsigned char *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   int i;
   if (stbi_jpeg_test_memory(buffer,len))
      return stbi_jpeg_load_from_memory(buffer,len,x,y,comp,req_comp);
   if (stbi_png_test_memory(buffer,len))
      return stbi_png_load_from_memory(buffer,len,x,y,comp,req_comp);
   if (stbi_bmp_test_memory(buffer,len))
      return stbi_bmp_load_from_memory(buffer,len,x,y,comp,req_comp);
   if (stbi_psd_test_memory(buffer,len))
      return stbi_psd_load_from_memory(buffer,len,x,y,comp,req_comp);
   #ifndef STBI_NO_HDR
   if (stbi_hdr_test_memory(buffer, len)) {
      float *hdr = stbi_hdr_load_from_memory(buffer, len,x,y,comp,req_comp);
      return hdr_to_ldr(hdr, *x, *y, req_comp ? req_comp : *comp);
   }
   #endif
   for (i=0; i < max_loaders; ++i)
      if (loaders[i]->test_memory(buffer,len))
         return loaders[i]->load_from_memory(buffer,len,x,y,comp,req_comp);
   // test tga last because it's a crappy test!
   if (stbi_tga_test_memory(buffer,len))
      return stbi_tga_load_from_memory(buffer,len,x,y,comp,req_comp);
   return epuc((char*)"unknown image type", (char*)"Image not of any known type, or corrupt");
}

#ifndef STBI_NO_HDR

#ifndef STBI_NO_STDIO
float *stbi_loadf(char const *filename, int *x, int *y, int *comp, int req_comp)
{
   FILE *f = fopen(filename, "rb");
   float *result;
   if (!f) return epf((char*)"can't fopen", (char*)"Unable to open file");
   result = stbi_loadf_from_file(f,x,y,comp,req_comp);
   fclose(f);
   return result;
}

float *stbi_loadf_from_file(FILE *f, int *x, int *y, int *comp, int req_comp)
{
   unsigned char *data;
   #ifndef STBI_NO_HDR
   if (stbi_hdr_test_file(f))
      return stbi_hdr_load_from_file(f,x,y,comp,req_comp);
   #endif
   data = stbi_load_from_file(f, x, y, comp, req_comp);
   if (data)
      return ldr_to_hdr(data, *x, *y, req_comp ? req_comp : *comp);
   return epf((char*)"unknown image type", (char*)"Image not of any known type, or corrupt");
}
#endif

float *stbi_loadf_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   stbi_uc *data;
   #ifndef STBI_NO_HDR
   if (stbi_hdr_test_memory(buffer, len))
      return stbi_hdr_load_from_memory(buffer, len,x,y,comp,req_comp);
   #endif
   data = stbi_load_from_memory(buffer, len, x, y, comp, req_comp);
   if (data)
      return ldr_to_hdr(data, *x, *y, req_comp ? req_comp : *comp);
   return epf((char*)"unknown image type",(char*) "Image not of any known type, or corrupt");
}
#endif

// these is-hdr-or-not is defined independent of whether STBI_NO_HDR is
// defined, for API simplicity; if STBI_NO_HDR is defined, it always
// reports false!

int stbi_is_hdr_from_memory(stbi_uc const *buffer, int len)
{
   #ifndef STBI_NO_HDR
   return stbi_hdr_test_memory(buffer, len);
   #else
   return 0;
   #endif
}

#ifndef STBI_NO_STDIO
extern int      stbi_is_hdr          (char const *filename)
{
   FILE *f = fopen(filename, "rb");
   int result=0;
   if (f) {
      result = stbi_is_hdr_from_file(f);
      fclose(f);
   }
   return result;
}

extern int      stbi_is_hdr_from_file(FILE *f)
{
   #ifndef STBI_NO_HDR
   return stbi_hdr_test_file(f);
   #else
   return 0;
   #endif
}

#endif

// @TODO: get image dimensions & components without fully decoding
#ifndef STBI_NO_STDIO
extern int      stbi_info            (char const *filename,           int *x, int *y, int *comp);
extern int      stbi_info_from_file  (FILE *f,                  int *x, int *y, int *comp);
#endif
extern int      stbi_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp);

#ifndef STBI_NO_HDR
static float h2l_gamma_i=1.0f/2.2f, h2l_scale_i=1.0f;
static float l2h_gamma=2.2f, l2h_scale=1.0f;

void   stbi_hdr_to_ldr_gamma(float gamma) { h2l_gamma_i = 1/gamma; }
void   stbi_hdr_to_ldr_scale(float scale) { h2l_scale_i = 1/scale; }

void   stbi_ldr_to_hdr_gamma(float gamma) { l2h_gamma = gamma; }
void   stbi_ldr_to_hdr_scale(float scale) { l2h_scale = scale; }
#endif


//////////////////////////////////////////////////////////////////////////////
//
// Common code used by all image loaders
//

enum
{
   SCAN_load=0,
   SCAN_type,
   SCAN_header,
};

typedef struct
{
   uint32_t img_x, img_y;
   int img_n, img_out_n;

   #ifndef STBI_NO_STDIO
   FILE  *img_file;
   #endif
   uint8_t *img_buffer, *img_buffer_end;
} stbi;

#ifndef STBI_NO_STDIO
static void start_file(stbi *s, FILE *f)
{
   s->img_file = f;
}
#endif

static void start_mem(stbi *s, uint8_t const *buffer, int len)
{
#ifndef STBI_NO_STDIO
   s->img_file = 0;
#endif
   s->img_buffer = (uint8_t *) buffer;
   s->img_buffer_end = (uint8_t *) buffer+len;
}

inline static int get8(stbi *s)
{
#ifndef STBI_NO_STDIO
   if (s->img_file) {
      int c = fgetc(s->img_file);
      return c == EOF ? 0 : c;
   }
#endif
   if (s->img_buffer < s->img_buffer_end)
      return *s->img_buffer++;
   return 0;
}

inline static int at_eof(stbi *s)
{
#ifndef STBI_NO_STDIO
   if (s->img_file)
      return feof(s->img_file);
#endif
   return s->img_buffer >= s->img_buffer_end;   
}

inline static uint8_t get8u(stbi *s)
{
   return (uint8_t) get8(s);
}

static void skip(stbi *s, int n)
{
#ifndef STBI_NO_STDIO
   if (s->img_file)
      fseek(s->img_file, n, SEEK_CUR);
   else
#endif
      s->img_buffer += n;
}

static int get16(stbi *s)
{
   int z = get8(s);
   return (z << 8) + get8(s);
}

static uint32_t get32(stbi *s)
{
   uint32_t z = get16(s);
   return (z << 16) + get16(s);
}

static int get16le(stbi *s)
{
   int z = get8(s);
   return z + (get8(s) << 8);
}

static uint32_t get32le(stbi *s)
{
   uint32_t z = get16le(s);
   return z + (get16le(s) << 16);
}

static void getn(stbi *s, stbi_uc *buffer, int n)
{
#ifndef STBI_NO_STDIO
   if (s->img_file) {
      size_t unused = fread(buffer, 1, n, s->img_file);
      if (!unused) {
        return;
      }
      return;
   }
#endif
   memcpy(buffer, s->img_buffer, n);
   s->img_buffer += n;
}

//////////////////////////////////////////////////////////////////////////////
//
//  generic converter from built-in img_n to req_comp
//    individual types do this automatically as much as possible (e.g. jpeg
//    does all cases internally since it needs to colorspace convert anyway,
//    and it never has alpha, so very few cases ). png can automatically
//    interleave an alpha=255 channel, but falls back to this for other cases
//
//  assume data buffer is malloced, so malloc a new one and free that one
//  only failure mode is malloc failing

static uint8_t compute_y(int r, int g, int b)
{
   return (uint8_t) (((r*77) + (g*150) +  (29*b)) >> 8);
}

static unsigned char *convert_format(unsigned char *data, int img_n, int req_comp, uint x, uint y)
{
   int i,j;
   unsigned char *good;

   if (req_comp == img_n) return data;
   ASSERT(req_comp >= 1 && req_comp <= 4);

   good = (unsigned char *) malloc(req_comp * x * y);
   if (good == 0) {
      free(data);
      return epuc((char*)"outofmem",(char*) "Out of memory");
   }

   for (j=0; j < (int) y; ++j) {
      unsigned char *src  = data + j * x * img_n   ;
      unsigned char *dest = good + j * x * req_comp;

      #define COMBO(a,b)  ((a)*8+(b))
      #define CASE(a,b)   case COMBO(a,b): for(i=x-1; i >= 0; --i, src += a, dest += b)
      // convert source image with img_n components to one with req_comp components;
      // avoid switch per pixel, so use switch per scanline and massive macros
      switch(COMBO(img_n, req_comp)) {
         CASE(1,2) dest[0]=src[0], dest[1]=255; break;
         CASE(1,3) dest[0]=dest[1]=dest[2]=src[0]; break;
         CASE(1,4) dest[0]=dest[1]=dest[2]=src[0], dest[3]=255; break;
         CASE(2,1) dest[0]=src[0]; break;
         CASE(2,3) dest[0]=dest[1]=dest[2]=src[0]; break;
         CASE(2,4) dest[0]=dest[1]=dest[2]=src[0], dest[3]=src[1]; break;
         CASE(3,4) dest[0]=src[0],dest[1]=src[1],dest[2]=src[2],dest[3]=255; break;
         CASE(3,1) dest[0]=compute_y(src[0],src[1],src[2]); break;
         CASE(3,2) dest[0]=compute_y(src[0],src[1],src[2]), dest[1] = 255; break;
         CASE(4,1) dest[0]=compute_y(src[0],src[1],src[2]); break;
         CASE(4,2) dest[0]=compute_y(src[0],src[1],src[2]), dest[1] = src[3]; break;
         CASE(4,3) dest[0]=src[0],dest[1]=src[1],dest[2]=src[2]; break;
         default: ASSERT(0);
      }
      #undef CASE
   }

   free(data);
   return good;
}

#ifndef STBI_NO_HDR
static float   *ldr_to_hdr(stbi_uc *data, int x, int y, int comp)
{
   int i,k,n;
   float *output = (float *) malloc(x * y * comp * sizeof(float));
   if (output == 0) { free(data); return epf((char*)"outofmem", (char*)"Out of memory"); }
   // compute number of non-alpha components
   if (comp & 1) n = comp; else n = comp-1;
   for (i=0; i < x*y; ++i) {
      for (k=0; k < n; ++k) {
         output[i*comp + k] = (float) pow(data[i*comp+k]/255.0f, l2h_gamma) * l2h_scale;
      }
      if (k < comp) output[i*comp + k] = data[i*comp+k]/255.0f;
   }
   free(data);
   return output;
}

#define float2int(x)   ((int) (x))
static stbi_uc *hdr_to_ldr(float   *data, int x, int y, int comp)
{
   int i,k,n;
   stbi_uc *output = (stbi_uc *) malloc(x * y * comp);
   if (output == 0) { free(data); return epuc((char*)"outofmem", (char*)"Out of memory"); }
   // compute number of non-alpha components
   if (comp & 1) n = comp; else n = comp-1;
   for (i=0; i < x*y; ++i) {
      for (k=0; k < n; ++k) {
         float z = (float) pow(data[i*comp+k]*h2l_scale_i, h2l_gamma_i) * 255 + 0.5f;
         if (z < 0) z = 0;
         if (z > 255) z = 255;
         output[i*comp + k] = float2int(z);
      }
      if (k < comp) {
         float z = data[i*comp+k] * 255 + 0.5f;
         if (z < 0) z = 0;
         if (z > 255) z = 255;
         output[i*comp + k] = float2int(z);
      }
   }
   free(data);
   return output;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//  "baseline" JPEG/JFIF decoder (not actually fully baseline implementation)
//
//    simple implementation
//      - channel subsampling of at most 2 in each dimension
//      - doesn't support delayed output of y-dimension
//      - simple interface (only one output format: 8-bit interleaved RGB)
//      - doesn't try to recover corrupt jpegs
//      - doesn't allow partial loading, loading multiple at once
//      - still fast on x86 (copying globals into locals doesn't help x86)
//      - allocates lots of intermediate memory (full size of all components)
//        - non-interleaved case requires this anyway
//        - allows good upsampling (see next)
//    high-quality
//      - upsampled channels are bilinearly interpolated, even across blocks
//      - quality integer IDCT derived from IJG's 'slow'
//    performance
//      - fast huffman; reasonable integer IDCT
//      - uses a lot of intermediate memory, could cache poorly
//      - load http://nothings.org/remote/anemones.jpg 3 times on 2.8Ghz P4
//          stb_jpeg:   1.34 seconds (MSVC6, default release build)
//          stb_jpeg:   1.06 seconds (MSVC6, processor = Pentium Pro)
//          IJL11.dll:  1.08 seconds (compiled by intel)
//          IJG 1998:   0.98 seconds (MSVC6, makefile provided by IJG)
//          IJG 1998:   0.95 seconds (MSVC6, makefile + proc=PPro)

// huffman decoding acceleration
#define FAST_BITS   9  // larger handles more cases; smaller stomps less cache

typedef struct
{
   uint8_t fast[1 << FAST_BITS];
   // weirdly, repacking this into AoS is a 10% speed loss, instead of a win
   uint16_t code[256];
   uint8_t values[256];
   uint8_t size[257];
   unsigned int maxcode[18];
   int    delta[17];   // old 'firstsymbol' - old 'firstcode'
} huffman;

typedef struct
{
   #if STBI_SIMD
   unsigned short dequant2[4][64];
   #endif
   stbi s;
   huffman huff_dc[4];
   huffman huff_ac[4];
   uint8_t dequant[4][64];

// sizes for components, interleaved MCUs
   int img_h_max, img_v_max;
   int img_mcu_x, img_mcu_y;
   int img_mcu_w, img_mcu_h;

// definition of jpeg image component
   struct
   {
      int id;
      int h,v;
      int tq;
      int hd,ha;
      int dc_pred;

      int x,y,w2,h2;
      uint8_t *data;
      void *raw_data;
      uint8_t *linebuf;
   } img_comp[4];

   uint32_t         code_buffer; // jpeg entropy-coded buffer
   int            code_bits;   // number of valid bits
   unsigned char marker;      // marker seen while filling entropy buffer
   int            nomore;      // flag if we saw a marker so must stop

   int scan_n, order[4];
   int restart_interval, todo;
} jpeg;

static int build_huffman(huffman *h, int *count)
{
   int i,j,k=0,code;
   // build size list for each symbol (from JPEG spec)
   for (i=0; i < 16; ++i)
      for (j=0; j < count[i]; ++j)
         h->size[k++] = (uint8_t) (i+1);
   h->size[k] = 0;

   // compute actual symbols (from jpeg spec)
   code = 0;
   k = 0;
   for(j=1; j <= 16; ++j) {
      // compute delta to add to code to compute symbol id
      h->delta[j] = k - code;
      if (h->size[k] == j) {
         while (h->size[k] == j)
            h->code[k++] = (uint16_t) (code++);
         if (code-1 >= (1 << j)) return e((char*)"bad code lengths",(char*)"Corrupt JPEG");
      }
      // compute largest code + 1 for this size, preshifted as needed later
      h->maxcode[j] = code << (16-j);
      code <<= 1;
   }
   h->maxcode[j] = 0xffffffff;

   // build non-spec acceleration table; 255 is flag for not-accelerated
   memset(h->fast, 255, 1 << FAST_BITS);
   for (i=0; i < k; ++i) {
      int s = h->size[i];
      if (s <= FAST_BITS) {
         int c = h->code[i] << (FAST_BITS-s);
         int m = 1 << (FAST_BITS-s);
         for (j=0; j < m; ++j) {
            h->fast[c+j] = (uint8_t) i;
         }
      }
   }
   return 1;
}

static void grow_buffer_unsafe(jpeg *j)
{
   do {
      int b = j->nomore ? 0 : get8(&j->s);
      if (b == 0xff) {
         int c = get8(&j->s);
         if (c != 0) {
            j->marker = (unsigned char) c;
            j->nomore = 1;
            return;
         }
      }
      j->code_buffer = (j->code_buffer << 8) | b;
      j->code_bits += 8;
   } while (j->code_bits <= 24);
}

// (1 << n) - 1
static uint32_t bmask[17]={0,1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,16383,32767,65535};

// decode a jpeg huffman value from the bitstream
inline static int decode(jpeg *j, huffman *h)
{
   unsigned int temp;
   int c,k;

   if (j->code_bits < 16) grow_buffer_unsafe(j);

   // look at the top FAST_BITS and determine what symbol ID it is,
   // if the code is <= FAST_BITS
   c = (j->code_buffer >> (j->code_bits - FAST_BITS)) & ((1 << FAST_BITS)-1);
   k = h->fast[c];
   if (k < 255) {
      if (h->size[k] > j->code_bits)
         return -1;
      j->code_bits -= h->size[k];
      return h->values[k];
   }

   // naive test is to shift the code_buffer down so k bits are
   // valid, then test against maxcode. To speed this up, we've
   // preshifted maxcode left so that it has (16-k) 0s at the
   // end; in other words, regardless of the number of bits, it
   // wants to be compared against something shifted to have 16;
   // that way we don't need to shift inside the loop.
   if (j->code_bits < 16)
      temp = (j->code_buffer << (16 - j->code_bits)) & 0xffff;
   else
      temp = (j->code_buffer >> (j->code_bits - 16)) & 0xffff;
   for (k=FAST_BITS+1 ; ; ++k)
      if (temp < h->maxcode[k])
         break;
   if (k == 17) {
      // error! code not found
      j->code_bits -= 16;
      return -1;
   }

   if (k > j->code_bits)
      return -1;

   // convert the huffman code to the symbol id
   c = ((j->code_buffer >> (j->code_bits - k)) & bmask[k]) + h->delta[k];
   ASSERT((((j->code_buffer) >> (j->code_bits - h->size[c])) & bmask[h->size[c]]) == h->code[c]);

   // convert the id to a symbol
   j->code_bits -= k;
   return h->values[c];
}

// combined JPEG 'receive' and JPEG 'extend', since baseline
// always extends everything it receives.
inline static int extend_receive(jpeg *j, int n)
{
   unsigned int m = 1 << (n-1);
   unsigned int k;
   if (j->code_bits < n) grow_buffer_unsafe(j);
   k = (j->code_buffer >> (j->code_bits - n)) & bmask[n];
   j->code_bits -= n;
   // the following test is probably a random branch that won't
   // predict well. I tried to table accelerate it but failed.
   // maybe it's compiling as a conditional move?
   if (k < m)
      return (-1 << n) + k + 1;
   else
      return k;
}

// given a value that's at position X in the zigzag stream,
// where does it appear in the 8x8 matrix coded as row-major?
static uint8_t dezigzag[64+15] =
{
    0,  1,  8, 16,  9,  2,  3, 10,
   17, 24, 32, 25, 18, 11,  4,  5,
   12, 19, 26, 33, 40, 48, 41, 34,
   27, 20, 13,  6,  7, 14, 21, 28,
   35, 42, 49, 56, 57, 50, 43, 36,
   29, 22, 15, 23, 30, 37, 44, 51,
   58, 59, 52, 45, 38, 31, 39, 46,
   53, 60, 61, 54, 47, 55, 62, 63,
   // let corrupt input sample past end
   63, 63, 63, 63, 63, 63, 63, 63,
   63, 63, 63, 63, 63, 63, 63
};

// decode one 64-entry block--
static int decode_block(jpeg *j, short data[64], huffman *hdc, huffman *hac, int b)
{
   int diff,dc,k;
   int t = decode(j, hdc);
   if (t < 0) return e((char*)"bad huffman code",(char*)"Corrupt JPEG");

   // 0 all the ac values now so we can do it 32-bits at a time
   memset(data,0,64*sizeof(data[0]));

   diff = t ? extend_receive(j, t) : 0;
   dc = j->img_comp[b].dc_pred + diff;
   j->img_comp[b].dc_pred = dc;
   data[0] = (short) dc;

   // decode AC components, see JPEG spec
   k = 1;
   do {
      int r,s;
      int rs = decode(j, hac);
      if (rs < 0) return e((char*)"bad huffman code",(char*)"Corrupt JPEG");
      s = rs & 15;
      r = rs >> 4;
      if (s == 0) {
         if (rs != 0xf0) break; // end block
         k += 16;
      } else {
         k += r;
         // decode into unzigzag'd location
         data[dezigzag[k++]] = (short) extend_receive(j,s);
      }
   } while (k < 64);
   return 1;
}

// take a -128..127 value and clamp it and convert to 0..255
inline static uint8_t clamp(int x)
{
   x += 128;
   // trick to use a single test to catch both cases
   if ((unsigned int) x > 255) {
      if (x < 0) return 0;
      if (x > 255) return 255;
   }
   return (uint8_t) x;
}

#define f2f(x)  (int) (((x) * 4096 + 0.5))
#define fsh(x)  ((x) << 12)

// derived from jidctint -- DCT_ISLOW
#define IDCT_1D(s0,s1,s2,s3,s4,s5,s6,s7)       \
   int t0,t1,t2,t3,p1,p2,p3,p4,p5,x0,x1,x2,x3; \
   p2 = s2;                                    \
   p3 = s6;                                    \
   p1 = (p2+p3) * f2f(0.5411961f);             \
   t2 = p1 + p3*f2f(-1.847759065f);            \
   t3 = p1 + p2*f2f( 0.765366865f);            \
   p2 = s0;                                    \
   p3 = s4;                                    \
   t0 = fsh(p2+p3);                            \
   t1 = fsh(p2-p3);                            \
   x0 = t0+t3;                                 \
   x3 = t0-t3;                                 \
   x1 = t1+t2;                                 \
   x2 = t1-t2;                                 \
   t0 = s7;                                    \
   t1 = s5;                                    \
   t2 = s3;                                    \
   t3 = s1;                                    \
   p3 = t0+t2;                                 \
   p4 = t1+t3;                                 \
   p1 = t0+t3;                                 \
   p2 = t1+t2;                                 \
   p5 = (p3+p4)*f2f( 1.175875602f);            \
   t0 = t0*f2f( 0.298631336f);                 \
   t1 = t1*f2f( 2.053119869f);                 \
   t2 = t2*f2f( 3.072711026f);                 \
   t3 = t3*f2f( 1.501321110f);                 \
   p1 = p5 + p1*f2f(-0.899976223f);            \
   p2 = p5 + p2*f2f(-2.562915447f);            \
   p3 = p3*f2f(-1.961570560f);                 \
   p4 = p4*f2f(-0.390180644f);                 \
   t3 += p1+p4;                                \
   t2 += p2+p3;                                \
   t1 += p2+p4;                                \
   t0 += p1+p3;

#if !STBI_SIMD
// .344 seconds on 3*anemones.jpg
static void idct_block(uint8_t *out, int out_stride, short data[64], uint8_t *dequantize)
{
   int i,val[64],*v=val;
   uint8_t *o,*dq = dequantize;
   short *d = data;

   // columns
   for (i=0; i < 8; ++i,++d,++dq, ++v) {
      // if all zeroes, shortcut -- this avoids dequantizing 0s and IDCTing
      if (d[ 8]==0 && d[16]==0 && d[24]==0 && d[32]==0
           && d[40]==0 && d[48]==0 && d[56]==0) {
         //    no shortcut                 0     seconds
         //    (1|2|3|4|5|6|7)==0          0     seconds
         //    all separate               -0.047 seconds
         //    1 && 2|3 && 4|5 && 6|7:    -0.047 seconds
         int dcterm = d[0] * dq[0] << 2;
         v[0] = v[8] = v[16] = v[24] = v[32] = v[40] = v[48] = v[56] = dcterm;
      } else {
         IDCT_1D(d[ 0]*dq[ 0],d[ 8]*dq[ 8],d[16]*dq[16],d[24]*dq[24],
                 d[32]*dq[32],d[40]*dq[40],d[48]*dq[48],d[56]*dq[56])
         // constants scaled things up by 1<<12; let's bring them back
         // down, but keep 2 extra bits of precision
         x0 += 512; x1 += 512; x2 += 512; x3 += 512;
         v[ 0] = (x0+t3) >> 10;
         v[56] = (x0-t3) >> 10;
         v[ 8] = (x1+t2) >> 10;
         v[48] = (x1-t2) >> 10;
         v[16] = (x2+t1) >> 10;
         v[40] = (x2-t1) >> 10;
         v[24] = (x3+t0) >> 10;
         v[32] = (x3-t0) >> 10;
      }
   }

   for (i=0, v=val, o=out; i < 8; ++i,v+=8,o+=out_stride) {
      // no fast case since the first 1D IDCT spread components out
      IDCT_1D(v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7])
      // constants scaled things up by 1<<12, plus we had 1<<2 from first
      // loop, plus horizontal and vertical each scale by sqrt(8) so together
      // we've got an extra 1<<3, so 1<<17 total we need to remove.
      x0 += 65536; x1 += 65536; x2 += 65536; x3 += 65536;
      o[0] = clamp((x0+t3) >> 17);
      o[7] = clamp((x0-t3) >> 17);
      o[1] = clamp((x1+t2) >> 17);
      o[6] = clamp((x1-t2) >> 17);
      o[2] = clamp((x2+t1) >> 17);
      o[5] = clamp((x2-t1) >> 17);
      o[3] = clamp((x3+t0) >> 17);
      o[4] = clamp((x3-t0) >> 17);
   }
}
#else
static void idct_block(uint8_t *out, int out_stride, short data[64], unsigned short *dequantize)
{
   int i,val[64],*v=val;
   uint8_t *o;
   unsigned short *dq = dequantize;
   short *d = data;

   // columns
   for (i=0; i < 8; ++i,++d,++dq, ++v) {
      // if all zeroes, shortcut -- this avoids dequantizing 0s and IDCTing
      if (d[ 8]==0 && d[16]==0 && d[24]==0 && d[32]==0
           && d[40]==0 && d[48]==0 && d[56]==0) {
         //    no shortcut                 0     seconds
         //    (1|2|3|4|5|6|7)==0          0     seconds
         //    all separate               -0.047 seconds
         //    1 && 2|3 && 4|5 && 6|7:    -0.047 seconds
         int dcterm = d[0] * dq[0] << 2;
         v[0] = v[8] = v[16] = v[24] = v[32] = v[40] = v[48] = v[56] = dcterm;
      } else {
         IDCT_1D(d[ 0]*dq[ 0],d[ 8]*dq[ 8],d[16]*dq[16],d[24]*dq[24],
                 d[32]*dq[32],d[40]*dq[40],d[48]*dq[48],d[56]*dq[56])
         // constants scaled things up by 1<<12; let's bring them back
         // down, but keep 2 extra bits of precision
         x0 += 512; x1 += 512; x2 += 512; x3 += 512;
         v[ 0] = (x0+t3) >> 10;
         v[56] = (x0-t3) >> 10;
         v[ 8] = (x1+t2) >> 10;
         v[48] = (x1-t2) >> 10;
         v[16] = (x2+t1) >> 10;
         v[40] = (x2-t1) >> 10;
         v[24] = (x3+t0) >> 10;
         v[32] = (x3-t0) >> 10;
      }
   }

   for (i=0, v=val, o=out; i < 8; ++i,v+=8,o+=out_stride) {
      // no fast case since the first 1D IDCT spread components out
      IDCT_1D(v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7])
      // constants scaled things up by 1<<12, plus we had 1<<2 from first
      // loop, plus horizontal and vertical each scale by sqrt(8) so together
      // we've got an extra 1<<3, so 1<<17 total we need to remove.
      x0 += 65536; x1 += 65536; x2 += 65536; x3 += 65536;
      o[0] = clamp((x0+t3) >> 17);
      o[7] = clamp((x0-t3) >> 17);
      o[1] = clamp((x1+t2) >> 17);
      o[6] = clamp((x1-t2) >> 17);
      o[2] = clamp((x2+t1) >> 17);
      o[5] = clamp((x2-t1) >> 17);
      o[3] = clamp((x3+t0) >> 17);
      o[4] = clamp((x3-t0) >> 17);
   }
}
static stbi_idct_8x8 stbi_idct_installed = idct_block;

extern void stbi_install_idct(stbi_idct_8x8 func)
{
   stbi_idct_installed = func;
}
#endif

#define MARKER_none  0xff
// if there's a pending marker from the entropy stream, return that
// otherwise, fetch from the stream and get a marker. if there's no
// marker, return 0xff, which is never a valid marker value
static uint8_t get_marker(jpeg *j)
{
   uint8_t x;
   if (j->marker != MARKER_none) { x = j->marker; j->marker = MARKER_none; return x; }
   x = get8u(&j->s);
   if (x != 0xff) return MARKER_none;
   while (x == 0xff)
      x = get8u(&j->s);
   return x;
}

// in each scan, we'll have scan_n components, and the order
// of the components is specified by order[]
#define RESTART(x)     ((x) >= 0xd0 && (x) <= 0xd7)

// after a restart interval, reset the entropy decoder and
// the dc prediction
static void reset(jpeg *j)
{
   j->code_bits = 0;
   j->code_buffer = 0;
   j->nomore = 0;
   j->img_comp[0].dc_pred = j->img_comp[1].dc_pred = j->img_comp[2].dc_pred = 0;
   j->marker = MARKER_none;
   j->todo = j->restart_interval ? j->restart_interval : 0x7fffffff;
   // no more than 1<<31 MCUs if no restart_interal? that's plenty safe,
   // since we don't even allow 1<<30 pixels
}

static int parse_entropy_coded_data(jpeg *z)
{
   reset(z);
   if (z->scan_n == 1) {
      int i,j;
      #if STBI_SIMD
      __declspec(align(16))
      #endif
      short data[64];
      int n = z->order[0];
      // non-interleaved data, we just need to process one block at a time,
      // in trivial scanline order
      // number of blocks to do just depends on how many actual "pixels" this
      // component has, independent of interleaved MCU blocking and such
      int w = (z->img_comp[n].x+7) >> 3;
      int h = (z->img_comp[n].y+7) >> 3;
      for (j=0; j < h; ++j) {
         for (i=0; i < w; ++i) {
            if (!decode_block(z, data, z->huff_dc+z->img_comp[n].hd, z->huff_ac+z->img_comp[n].ha, n)) return 0;
            #if STBI_SIMD
            stbi_idct_installed(z->img_comp[n].data+z->img_comp[n].w2*j*8+i*8, z->img_comp[n].w2, data, z->dequant2[z->img_comp[n].tq]);
            #else
            idct_block(z->img_comp[n].data+z->img_comp[n].w2*j*8+i*8, z->img_comp[n].w2, data, z->dequant[z->img_comp[n].tq]);
            #endif
            // every data block is an MCU, so countdown the restart interval
            if (--z->todo <= 0) {
               if (z->code_bits < 24) grow_buffer_unsafe(z);
               // if it's NOT a restart, then just bail, so we get corrupt data
               // rather than no data
               if (!RESTART(z->marker)) return 1;
               reset(z);
            }
         }
      }
   } else { // interleaved!
      int i,j,k,x,y;
      short data[64];
      for (j=0; j < z->img_mcu_y; ++j) {
         for (i=0; i < z->img_mcu_x; ++i) {
            // scan an interleaved mcu... process scan_n components in order
            for (k=0; k < z->scan_n; ++k) {
               int n = z->order[k];
               // scan out an mcu's worth of this component; that's just determined
               // by the basic H and V specified for the component
               for (y=0; y < z->img_comp[n].v; ++y) {
                  for (x=0; x < z->img_comp[n].h; ++x) {
                     int x2 = (i*z->img_comp[n].h + x)*8;
                     int y2 = (j*z->img_comp[n].v + y)*8;
                     if (!decode_block(z, data, z->huff_dc+z->img_comp[n].hd, z->huff_ac+z->img_comp[n].ha, n)) return 0;
                     #if STBI_SIMD
                     stbi_idct_installed(z->img_comp[n].data+z->img_comp[n].w2*y2+x2, z->img_comp[n].w2, data, z->dequant2[z->img_comp[n].tq]);
                     #else
                     idct_block(z->img_comp[n].data+z->img_comp[n].w2*y2+x2, z->img_comp[n].w2, data, z->dequant[z->img_comp[n].tq]);
                     #endif
                  }
               }
            }
            // after all interleaved components, that's an interleaved MCU,
            // so now count down the restart interval
            if (--z->todo <= 0) {
               if (z->code_bits < 24) grow_buffer_unsafe(z);
               // if it's NOT a restart, then just bail, so we get corrupt data
               // rather than no data
               if (!RESTART(z->marker)) return 1;
               reset(z);
            }
         }
      }
   }
   return 1;
}

static int process_marker(jpeg *z, int m)
{
   int L;
   switch (m) {
      case MARKER_none: // no marker found
         return e((char*)"expected marker",(char*)"Corrupt JPEG");

      case 0xC2: // SOF - progressive
         return e((char*)"progressive jpeg",(char*)"JPEG format not supported (progressive)");

      case 0xDD: // DRI - specify restart interval
         if (get16(&z->s) != 4) return e((char*)"bad DRI len",(char*)"Corrupt JPEG");
         z->restart_interval = get16(&z->s);
         return 1;

      case 0xDB: // DQT - define quantization table
         L = get16(&z->s)-2;
         while (L > 0) {
            int q = get8(&z->s);
            int p = q >> 4;
            int t = q & 15,i;
            if (p != 0) return e((char*)"bad DQT type",(char*)"Corrupt JPEG");
            if (t > 3) return e((char*)"bad DQT table",(char*)"Corrupt JPEG");
            for (i=0; i < 64; ++i)
               z->dequant[t][dezigzag[i]] = get8u(&z->s);
            #if STBI_SIMD
            for (i=0; i < 64; ++i)
               z->dequant2[t][i] = z->dequant[t][i];
            #endif
            L -= 65;
         }
         return L==0;

      case 0xC4: // DHT - define huffman table
         L = get16(&z->s)-2;
         while (L > 0) {
            uint8_t *v;
            int sizes[16],i,m=0;
            int q = get8(&z->s);
            int tc = q >> 4;
            int th = q & 15;
            if (tc > 1 || th > 3) return e((char*)"bad DHT header",(char*)"Corrupt JPEG");
            for (i=0; i < 16; ++i) {
               sizes[i] = get8(&z->s);
               m += sizes[i];
            }
            L -= 17;
            if (tc == 0) {
               if (!build_huffman(z->huff_dc+th, sizes)) return 0;
               v = z->huff_dc[th].values;
            } else {
               if (!build_huffman(z->huff_ac+th, sizes)) return 0;
               v = z->huff_ac[th].values;
            }
            for (i=0; i < m; ++i)
               v[i] = get8u(&z->s);
            L -= m;
         }
         return L==0;
   }
   // check for comment block or APP blocks
   if ((m >= 0xE0 && m <= 0xEF) || m == 0xFE) {
      skip(&z->s, get16(&z->s)-2);
      return 1;
   }
   return 0;
}

// after we see SOS
static int process_scan_header(jpeg *z)
{
   int i;
   int Ls = get16(&z->s);
   z->scan_n = get8(&z->s);
   if (z->scan_n < 1 || z->scan_n > 4 || z->scan_n > (int) z->s.img_n) return e((char*)"bad SOS component count",(char*)"Corrupt JPEG");
   if (Ls != 6+2*z->scan_n) return e((char*)"bad SOS len",(char*)"Corrupt JPEG");
   for (i=0; i < z->scan_n; ++i) {
      int id = get8(&z->s), which;
      int q = get8(&z->s);
      for (which = 0; which < z->s.img_n; ++which)
         if (z->img_comp[which].id == id)
            break;
      if (which == z->s.img_n) return 0;
      z->img_comp[which].hd = q >> 4;   if (z->img_comp[which].hd > 3) return e((char*)"bad DC huff",(char*)"Corrupt JPEG");
      z->img_comp[which].ha = q & 15;   if (z->img_comp[which].ha > 3) return e((char*)"bad AC huff",(char*)"Corrupt JPEG");
      z->order[i] = which;
   }
   if (get8(&z->s) != 0) return e((char*)"bad SOS",(char*)"Corrupt JPEG");
   get8(&z->s); // should be 63, but might be 0
   if (get8(&z->s) != 0) return e((char*)"bad SOS",(char*)"Corrupt JPEG");

   return 1;
}

static int process_frame_header(jpeg *z, int scan)
{
   stbi *s = &z->s;
   int Lf,p,i,q, h_max=1,v_max=1,c;
   Lf = get16(s);         if (Lf < 11) return e((char*)"bad SOF len",(char*)"Corrupt JPEG"); // JPEG
   p  = get8(s);          if (p != 8) return e((char*)"only 8-bit",(char*)"JPEG format not supported: 8-bit only"); // JPEG baseline
   s->img_y = get16(s);   if (s->img_y == 0) return e((char*)"no header height", "JPEG format not supported: delayed height"); // Legal, but we don't handle it--but neither does IJG
   s->img_x = get16(s);   if (s->img_x == 0) return e((char*)"0 width",(char*)"Corrupt JPEG"); // JPEG requires
   c = get8(s);
   if (c != 3 && c != 1) return e((char*)"bad component count",(char*)"Corrupt JPEG");    // JFIF requires
   s->img_n = c;
   for (i=0; i < c; ++i) {
      z->img_comp[i].data = 0;
      z->img_comp[i].linebuf = 0;
   }

   if (Lf != 8+3*s->img_n) return e((char*)"bad SOF len",(char*)"Corrupt JPEG");

   for (i=0; i < s->img_n; ++i) {
      z->img_comp[i].id = get8(s);
      if (z->img_comp[i].id != i+1)   // JFIF requires
         if (z->img_comp[i].id != i)  // some version of jpegtran outputs non-JFIF-compliant files!
            return e((char*)"bad component ID",(char*)"Corrupt JPEG");
      q = get8(s);
      z->img_comp[i].h = (q >> 4);  if (!z->img_comp[i].h || z->img_comp[i].h > 4) return e((char*)"bad H",(char*)"Corrupt JPEG");
      z->img_comp[i].v = q & 15;    if (!z->img_comp[i].v || z->img_comp[i].v > 4) return e((char*)"bad V",(char*)"Corrupt JPEG");
      z->img_comp[i].tq = get8(s);  if (z->img_comp[i].tq > 3) return e((char*)"bad TQ",(char*)"Corrupt JPEG");
   }

   if (scan != SCAN_load) return 1;

   if ((1 << 30) / s->img_x / s->img_n < s->img_y) return e((char*)"too large", "Image too large to decode");

   for (i=0; i < s->img_n; ++i) {
      if (z->img_comp[i].h > h_max) h_max = z->img_comp[i].h;
      if (z->img_comp[i].v > v_max) v_max = z->img_comp[i].v;
   }

   // compute interleaved mcu info
   z->img_h_max = h_max;
   z->img_v_max = v_max;
   z->img_mcu_w = h_max * 8;
   z->img_mcu_h = v_max * 8;
   z->img_mcu_x = (s->img_x + z->img_mcu_w-1) / z->img_mcu_w;
   z->img_mcu_y = (s->img_y + z->img_mcu_h-1) / z->img_mcu_h;

   for (i=0; i < s->img_n; ++i) {
      // number of effective pixels (e.g. for non-interleaved MCU)
      z->img_comp[i].x = (s->img_x * z->img_comp[i].h + h_max-1) / h_max;
      z->img_comp[i].y = (s->img_y * z->img_comp[i].v + v_max-1) / v_max;
      // to simplify generation, we'll allocate enough memory to decode
      // the bogus oversized data from using interleaved MCUs and their
      // big blocks (e.g. a 16x16 iMCU on an image of width 33); we won't
      // discard the extra data until colorspace conversion
      z->img_comp[i].w2 = z->img_mcu_x * z->img_comp[i].h * 8;
      z->img_comp[i].h2 = z->img_mcu_y * z->img_comp[i].v * 8;
      z->img_comp[i].raw_data = malloc(z->img_comp[i].w2 * z->img_comp[i].h2+15);
      if (z->img_comp[i].raw_data == 0) {
         for(--i; i >= 0; --i) {
            free(z->img_comp[i].raw_data);
            z->img_comp[i].data = 0;
         }
         return e((char*)"outofmem", "Out of memory");
      }
      // align blocks for installable-idct using mmx/sse
      z->img_comp[i].data = (uint8_t*) (((uintptr_t) z->img_comp[i].raw_data + 15) & ~15);
      z->img_comp[i].linebuf = 0;
   }

   return 1;
}

// use comparisons since in some cases we handle more than one case (e.g. SOF)
#define DNL(x)         ((x) == 0xdc)
#define SOI(x)         ((x) == 0xd8)
#define EOI(x)         ((x) == 0xd9)
#define SOF(x)         ((x) == 0xc0 || (x) == 0xc1)
#define SOS(x)         ((x) == 0xda)

static int decode_jpeg_header(jpeg *z, int scan)
{
   int m;
   z->marker = MARKER_none; // initialize cached marker to empty
   m = get_marker(z);
   if (!SOI(m)) return e((char*)"no SOI",(char*)"Corrupt JPEG");
   if (scan == SCAN_type) return 1;
   m = get_marker(z);
   while (!SOF(m)) {
      if (!process_marker(z,m)) return 0;
      m = get_marker(z);
      while (m == MARKER_none) {
         // some files have extra padding after their blocks, so ok, we'll scan
         if (at_eof(&z->s)) return e((char*)"no SOF", "Corrupt JPEG");
         m = get_marker(z);
      }
   }
   if (!process_frame_header(z, scan)) return 0;
   return 1;
}

static int decode_jpeg_image(jpeg *j)
{
   int m;
   j->restart_interval = 0;
   if (!decode_jpeg_header(j, SCAN_load)) return 0;
   m = get_marker(j);
   while (!EOI(m)) {
      if (SOS(m)) {
         if (!process_scan_header(j)) return 0;
         if (!parse_entropy_coded_data(j)) return 0;
      } else {
         if (!process_marker(j, m)) return 0;
      }
      m = get_marker(j);
   }
   return 1;
}

// static jfif-centered resampling (across block boundaries)

typedef uint8_t *(*resample_row_func)(uint8_t *out, uint8_t *in0, uint8_t *in1,
                                    int w, int hs);

#define div4(x) ((uint8_t) ((x) >> 2))

static uint8_t *resample_row_1(uint8_t *out, uint8_t *in_near, uint8_t *in_far, int w, int hs)
{
   return in_near;
}

static uint8_t* resample_row_v_2(uint8_t *out, uint8_t *in_near, uint8_t *in_far, int w, int hs)
{
   // need to generate two samples vertically for every one in input
   int i;
   for (i=0; i < w; ++i)
      out[i] = div4(3*in_near[i] + in_far[i] + 2);
   return out;
}

static uint8_t*  resample_row_h_2(uint8_t *out, uint8_t *in_near, uint8_t *in_far, int w, int hs)
{
   // need to generate two samples horizontally for every one in input
   int i;
   uint8_t *input = in_near;
   if (w == 1) {
      // if only one sample, can't do any interpolation
      out[0] = out[1] = input[0];
      return out;
   }

   out[0] = input[0];
   out[1] = div4(input[0]*3 + input[1] + 2);
   for (i=1; i < w-1; ++i) {
      int n = 3*input[i]+2;
      out[i*2+0] = div4(n+input[i-1]);
      out[i*2+1] = div4(n+input[i+1]);
   }
   out[i*2+0] = div4(input[w-2]*3 + input[w-1] + 2);
   out[i*2+1] = input[w-1];
   return out;
}

#define div16(x) ((uint8_t) ((x) >> 4))

static uint8_t *resample_row_hv_2(uint8_t *out, uint8_t *in_near, uint8_t *in_far, int w, int hs)
{
   // need to generate 2x2 samples for every one in input
   int i,t0,t1;
   if (w == 1) {
      out[0] = out[1] = div4(3*in_near[0] + in_far[0] + 2);
      return out;
   }

   t1 = 3*in_near[0] + in_far[0];
   out[0] = div4(t1+2);
   for (i=1; i < w; ++i) {
      t0 = t1;
      t1 = 3*in_near[i]+in_far[i];
      out[i*2-1] = div16(3*t0 + t1 + 8);
      out[i*2  ] = div16(3*t1 + t0 + 8);
   }
   out[w*2-1] = div4(t1+2);
   return out;
}

static uint8_t *resample_row_generic(uint8_t *out, uint8_t *in_near, uint8_t *in_far, int w, int hs)
{
   // resample with nearest-neighbor
   int i,j;
   for (i=0; i < w; ++i)
      for (j=0; j < hs; ++j)
         out[i*hs+j] = in_near[i];
   return out;
}

#define float2fixed(x)  ((int) ((x) * 65536 + 0.5))

// 0.38 seconds on 3*anemones.jpg   (0.25 with processor = Pro)
// VC6 without processor=Pro is generating multiple LEAs per multiply!
static void YCbCr_to_RGB_row(uint8_t *out, const uint8_t *y, const uint8_t *pcb, const uint8_t *pcr, int count, int step)
{
   int i;
   for (i=0; i < count; ++i) {
      int y_fixed = (y[i] << 16) + 32768; // rounding
      int r,g,b;
      int cr = pcr[i] - 128;
      int cb = pcb[i] - 128;
      r = y_fixed + cr*float2fixed(1.40200f);
      g = y_fixed - cr*float2fixed(0.71414f) - cb*float2fixed(0.34414f);
      b = y_fixed                            + cb*float2fixed(1.77200f);
      r >>= 16;
      g >>= 16;
      b >>= 16;
      if ((unsigned) r > 255) { if (r < 0) r = 0; else r = 255; }
      if ((unsigned) g > 255) { if (g < 0) g = 0; else g = 255; }
      if ((unsigned) b > 255) { if (b < 0) b = 0; else b = 255; }
      out[0] = (uint8_t)r;
      out[1] = (uint8_t)g;
      out[2] = (uint8_t)b;
      out[3] = 255;
      out += step;
   }
}

#if STBI_SIMD
static stbi_YCbCr_to_RGB_run stbi_YCbCr_installed = YCbCr_to_RGB_row;

void stbi_install_YCbCr_to_RGB(stbi_YCbCr_to_RGB_run func)
{
   stbi_YCbCr_installed = func;
}
#endif


// clean up the temporary component buffers
static void cleanup_jpeg(jpeg *j)
{
   int i;
   for (i=0; i < j->s.img_n; ++i) {
      if (j->img_comp[i].data) {
         free(j->img_comp[i].raw_data);
         j->img_comp[i].data = 0;
      }
      if (j->img_comp[i].linebuf) {
         free(j->img_comp[i].linebuf);
         j->img_comp[i].linebuf = 0;
      }
   }
}

typedef struct
{
   resample_row_func resample;
   uint8_t *line0,*line1;
   int hs,vs;   // expansion factor in each axis
   int w_lores; // horizontal pixels pre-expansion 
   int ystep;   // how far through vertical expansion we are
   int ypos;    // which pre-expansion row we're on
} stbi_resample;

static uint8_t *load_jpeg_image(jpeg *z, int *out_x, int *out_y, int *comp, int req_comp)
{
   int n, decode_n;
   // validate req_comp
   if (req_comp < 0 || req_comp > 4) return epuc((char*)"bad req_comp", "Internal error");
   z->s.img_n = 0;

   // load a jpeg image from whichever source
   if (!decode_jpeg_image(z)) { cleanup_jpeg(z); return 0; }

   // determine actual number of components to generate
   n = req_comp ? req_comp : z->s.img_n;

   if (z->s.img_n == 3 && n < 3)
      decode_n = 1;
   else
      decode_n = z->s.img_n;

   // resample and color-convert
   {
      int k;
      uint i,j;
      uint8_t *output;
      uint8_t *coutput[4] = {0};

      stbi_resample res_comp[4];

      for (k=0; k < decode_n; ++k) {
         stbi_resample *r = &res_comp[k];

         // allocate line buffer big enough for upsampling off the edges
         // with upsample factor of 4
         z->img_comp[k].linebuf = (uint8_t *) malloc(z->s.img_x + 3);
         if (!z->img_comp[k].linebuf) { cleanup_jpeg(z); return epuc((char*)"outofmem", "Out of memory"); }

         r->hs      = z->img_h_max / z->img_comp[k].h;
         r->vs      = z->img_v_max / z->img_comp[k].v;
         r->ystep   = r->vs >> 1;
         r->w_lores = (z->s.img_x + r->hs-1) / r->hs;
         r->ypos    = 0;
         r->line0   = r->line1 = z->img_comp[k].data;

         if      (r->hs == 1 && r->vs == 1) r->resample = resample_row_1;
         else if (r->hs == 1 && r->vs == 2) r->resample = resample_row_v_2;
         else if (r->hs == 2 && r->vs == 1) r->resample = resample_row_h_2;
         else if (r->hs == 2 && r->vs == 2) r->resample = resample_row_hv_2;
         else                               r->resample = resample_row_generic;
      }

      // can't error after this so, this is safe
      output = (uint8_t *) malloc(n * z->s.img_x * z->s.img_y + 1);
      if (!output) { cleanup_jpeg(z); return epuc((char*)"outofmem", "Out of memory"); }

      // now go ahead and resample
      for (j=0; j < z->s.img_y; ++j) {
         uint8_t *out = output + n * z->s.img_x * j;
         for (k=0; k < decode_n; ++k) {
            stbi_resample *r = &res_comp[k];
            int y_bot = r->ystep >= (r->vs >> 1);
            coutput[k] = r->resample(z->img_comp[k].linebuf,
                                     y_bot ? r->line1 : r->line0,
                                     y_bot ? r->line0 : r->line1,
                                     r->w_lores, r->hs);
            if (++r->ystep >= r->vs) {
               r->ystep = 0;
               r->line0 = r->line1;
               if (++r->ypos < z->img_comp[k].y)
                  r->line1 += z->img_comp[k].w2;
            }
         }
         if (n >= 3) {
            uint8_t *y = coutput[0];
            if (z->s.img_n == 3) {
               #if STBI_SIMD
               stbi_YCbCr_installed(out, y, coutput[1], coutput[2], z->s.img_x, n);
               #else
               YCbCr_to_RGB_row(out, y, coutput[1], coutput[2], z->s.img_x, n);
               #endif
            } else
               for (i=0; i < z->s.img_x; ++i) {
                  out[0] = out[1] = out[2] = y[i];
                  out[3] = 255; // not used if n==3
                  out += n;
               }
         } else {
            uint8_t *y = coutput[0];
            if (y) {
                if (n == 1)
                for (i=0; i < z->s.img_x; ++i) out[i] = y[i];
                else
                for (i=0; i < z->s.img_x; ++i) *out++ = y[i], *out++ = 255;
            }
         }
      }
      cleanup_jpeg(z);
      *out_x = z->s.img_x;
      *out_y = z->s.img_y;
      if (comp) *comp  = z->s.img_n; // report original components, not output
      return output;
   }
}

#ifndef STBI_NO_STDIO
unsigned char *stbi_jpeg_load_from_file(FILE *f, int *x, int *y, int *comp, int req_comp)
{
   jpeg j;
   start_file(&j.s, f);
   return load_jpeg_image(&j, x,y,comp,req_comp);
}

unsigned char *stbi_jpeg_load(char const *filename, int *x, int *y, int *comp, int req_comp)
{
   unsigned char *data;
   FILE *f = fopen(filename, "rb");
   if (!f) return 0;
   data = stbi_jpeg_load_from_file(f,x,y,comp,req_comp);
   fclose(f);
   return data;
}
#endif

unsigned char *stbi_jpeg_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   jpeg j;
   start_mem(&j.s, buffer,len);
   return load_jpeg_image(&j, x,y,comp,req_comp);
}

#ifndef STBI_NO_STDIO
int stbi_jpeg_test_file(FILE *f)
{
   int n,r;
   jpeg j;
   n = (int)ftell(f);
   start_file(&j.s, f);
   r = decode_jpeg_header(&j, SCAN_type);
   fseek(f,n,SEEK_SET);
   return r;
}
#endif

int stbi_jpeg_test_memory(stbi_uc const *buffer, int len)
{
   jpeg j;
   start_mem(&j.s, buffer,len);
   return decode_jpeg_header(&j, SCAN_type);
}

// @TODO:
#ifndef STBI_NO_STDIO
extern int      stbi_jpeg_info            (char const *filename,           int *x, int *y, int *comp);
extern int      stbi_jpeg_info_from_file  (FILE *f,                  int *x, int *y, int *comp);
#endif
extern int      stbi_jpeg_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp);

// public domain zlib decode    v0.2  Sean Barrett 2006-11-18
//    simple implementation
//      - all input must be provided in an upfront buffer
//      - all output is written to a single output buffer (can malloc/realloc)
//    performance
//      - fast huffman

// fast-way is faster to check than jpeg huffman, but slow way is slower
#define ZFAST_BITS  9 // accelerate all cases in default tables
#define ZFAST_MASK  ((1 << ZFAST_BITS) - 1)

// zlib-style huffman encoding
// (jpegs packs from left, zlib from right, so can't share code)
typedef struct
{
   uint16_t fast[1 << ZFAST_BITS];
   uint16_t firstcode[16];
   int maxcode[17];
   uint16_t firstsymbol[16];
   uint8_t size[288];
   uint16_t value[288]; 
} zhuffman;

inline static int bitreverse16(int n)
{
  n = ((n & 0xAAAA) >>  1) | ((n & 0x5555) << 1);
  n = ((n & 0xCCCC) >>  2) | ((n & 0x3333) << 2);
  n = ((n & 0xF0F0) >>  4) | ((n & 0x0F0F) << 4);
  n = ((n & 0xFF00) >>  8) | ((n & 0x00FF) << 8);
  return n;
}

inline static int bit_reverse(int v, int bits)
{
   ASSERT(bits <= 16);
   // to bit reverse n bits, reverse 16 and shift
   // e.g. 11 bits, bit reverse and shift away 5
   return bitreverse16(v) >> (16-bits);
}

static int zbuild_huffman(zhuffman *z, uint8_t *sizelist, int num)
{
   int i,k=0;
   int code, next_code[16], sizes[17];

   // DEFLATE spec for generating codes
   memset(sizes, 0, sizeof(sizes));
   memset(z->fast, 255, sizeof(z->fast));
   for (i=0; i < num; ++i) 
      ++sizes[sizelist[i]];
   sizes[0] = 0;
   for (i=1; i < 16; ++i)
      ASSERT(sizes[i] <= (1 << i));
   code = 0;
   for (i=1; i < 16; ++i) {
      next_code[i] = code;
      z->firstcode[i] = (uint16_t) code;
      z->firstsymbol[i] = (uint16_t) k;
      code = (code + sizes[i]);
      if (sizes[i])
         if (code-1 >= (1 << i)) return e((char*)"bad codelengths",(char*)"Corrupt JPEG");
      z->maxcode[i] = code << (16-i); // preshift for inner loop
      code <<= 1;
      k += sizes[i];
   }
   z->maxcode[16] = 0x10000; // sentinel
   for (i=0; i < num; ++i) {
      int s = sizelist[i];
      if (s) {
         int c = next_code[s] - z->firstcode[s] + z->firstsymbol[s];
         z->size[c] = (uint8_t)s;
         z->value[c] = (uint16_t)i;
         if (s <= ZFAST_BITS) {
            int k = bit_reverse(next_code[s],s);
            while (k < (1 << ZFAST_BITS)) {
               z->fast[k] = (uint16_t) c;
               k += (1 << s);
            }
         }
         ++next_code[s];
      }
   }
   return 1;
}

// zlib-from-memory implementation for PNG reading
//    because PNG allows splitting the zlib stream arbitrarily,
//    and it's annoying structurally to have PNG call ZLIB call PNG,
//    we require PNG read all the IDATs and combine them into a single
//    memory buffer

typedef struct
{
   uint8_t *zbuffer, *zbuffer_end;
   int num_bits;
   uint32_t code_buffer;

   char *zout;
   char *zout_start;
   char *zout_end;
   int   z_expandable;

   zhuffman z_length, z_distance;
} zbuf;

inline static int zget8(zbuf *z)
{
   if (z->zbuffer >= z->zbuffer_end) return 0;
   return *z->zbuffer++;
}

static void fill_bits(zbuf *z)
{
   do {
      ASSERT(z->code_buffer < (1U << z->num_bits));
      z->code_buffer |= zget8(z) << z->num_bits;
      z->num_bits += 8;
   } while (z->num_bits <= 24);
}

inline static unsigned int zreceive(zbuf *z, int n)
{
   unsigned int k;
   if (z->num_bits < n) fill_bits(z);
   k = z->code_buffer & ((1 << n) - 1);
   z->code_buffer >>= n;
   z->num_bits -= n;
   return k;   
}

inline static int zhuffman_decode(zbuf *a, zhuffman *z)
{
   int b,s,k;
   if (a->num_bits < 16) fill_bits(a);
   b = z->fast[a->code_buffer & ZFAST_MASK];
   if (b < 0xffff) {
      s = z->size[b];
      a->code_buffer >>= s;
      a->num_bits -= s;
      return z->value[b];
   }

   // not resolved by fast table, so compute it the slow way
   // use jpeg approach, which requires MSbits at top
   k = bit_reverse(a->code_buffer, 16);
   for (s=ZFAST_BITS+1; ; ++s)
      if (k < z->maxcode[s])
         break;
   if (s == 16) return -1; // invalid code!
   // code size is s, so:
   b = (k >> (16-s)) - z->firstcode[s] + z->firstsymbol[s];
   ASSERT(z->size[b] == s);
   a->code_buffer >>= s;
   a->num_bits -= s;
   return z->value[b];
}

static int expand(zbuf *z, int n)  // need to make room for n bytes
{
   char *q;
   int cur, limit;
   if (!z->z_expandable) return e((char*)"output buffer limit",(char*)"Corrupt PNG");
   cur   = (int) (z->zout     - z->zout_start);
   limit = (int) (z->zout_end - z->zout_start);
   while (cur + n > limit)
      limit *= 2;
   q = (char *) realloc(z->zout_start, limit);
   if (q == 0) return e((char*)"outofmem", "Out of memory");
   z->zout_start = q;
   z->zout       = q + cur;
   z->zout_end   = q + limit;
   return 1;
}

static int length_base[31] = {
   3,4,5,6,7,8,9,10,11,13,
   15,17,19,23,27,31,35,43,51,59,
   67,83,99,115,131,163,195,227,258,0,0 };

static int length_extra[31]= 
{ 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0 };

static int dist_base[32] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,
257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0,0};

static int dist_extra[32] =
{ 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

static int parse_huffman_block(zbuf *a)
{
   for(;;) {
      int z = zhuffman_decode(a, &a->z_length);
      if (z < 256) {
         if (z < 0) return e((char*)"bad huffman code",(char*)"Corrupt PNG"); // error in huffman codes
         if (a->zout >= a->zout_end) if (!expand(a, 1)) return 0;
         *a->zout++ = (char) z;
      } else {
         uint8_t *p;
         int len,dist;
         if (z == 256) return 1;
         z -= 257;
         len = length_base[z];
         if (length_extra[z]) len += zreceive(a, length_extra[z]);
         z = zhuffman_decode(a, &a->z_distance);
         if (z < 0) return e((char*)"bad huffman code",(char*)"Corrupt PNG");
         dist = dist_base[z];
         if (dist_extra[z]) dist += zreceive(a, dist_extra[z]);
         if (a->zout - a->zout_start < dist) return e((char*)"bad dist",(char*)"Corrupt PNG");
         if (a->zout + len > a->zout_end) if (!expand(a, len)) return 0;
         p = (uint8_t *) (a->zout - dist);
         while (len--)
            *a->zout++ = *p++;
      }
   }
}

static int compute_huffman_codes(zbuf *a)
{
   static uint8_t length_dezigzag[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
   zhuffman z_codelength;
   uint8_t lencodes[286+32+137];//padding for maximum single op
   uint8_t codelength_sizes[19];
   int i,n;

   int hlit  = zreceive(a,5) + 257;
   int hdist = zreceive(a,5) + 1;
   int hclen = zreceive(a,4) + 4;

   memset(codelength_sizes, 0, sizeof(codelength_sizes));
   for (i=0; i < hclen; ++i) {
      int s = zreceive(a,3);
      codelength_sizes[length_dezigzag[i]] = (uint8_t) s;
   }
   if (!zbuild_huffman(&z_codelength, codelength_sizes, 19)) return 0;

   n = 0;
   while (n < hlit + hdist) {
      int c = zhuffman_decode(a, &z_codelength);
      ASSERT(c >= 0 && c < 19);
      if (c < 16)
         lencodes[n++] = (uint8_t) c;
      else if (c == 16) {
         c = zreceive(a,2)+3;
         memset(lencodes+n, lencodes[n-1], c);
         n += c;
      } else if (c == 17) {
         c = zreceive(a,3)+3;
         memset(lencodes+n, 0, c);
         n += c;
      } else {
         ASSERT(c == 18);
         c = zreceive(a,7)+11;
         memset(lencodes+n, 0, c);
         n += c;
      }
   }
   if (n != hlit+hdist) return e((char*)"bad codelengths",(char*)"Corrupt PNG");
   if (!zbuild_huffman(&a->z_length, lencodes, hlit)) return 0;
   if (!zbuild_huffman(&a->z_distance, lencodes+hlit, hdist)) return 0;
   return 1;
}

static int parse_uncompressed_block(zbuf *a)
{
   uint8_t header[4];
   int len,nlen,k;
   if (a->num_bits & 7)
      zreceive(a, a->num_bits & 7); // discard
   // drain the bit-packed data into header
   k = 0;
   while (a->num_bits > 0) {
      header[k++] = (uint8_t) (a->code_buffer & 255); // wtf this warns?
      a->code_buffer >>= 8;
      a->num_bits -= 8;
   }
   ASSERT(a->num_bits == 0);
   // now fill header the normal way
   while (k < 4)
      header[k++] = (uint8_t) zget8(a);
   len  = header[1] * 256 + header[0];
   nlen = header[3] * 256 + header[2];
   if (nlen != (len ^ 0xffff)) return e((char*)"zlib corrupt",(char*)"Corrupt PNG");
   if (a->zbuffer + len > a->zbuffer_end) return e((char*)"read past buffer",(char*)"Corrupt PNG");
   if (a->zout + len > a->zout_end)
      if (!expand(a, len)) return 0;
   memcpy(a->zout, a->zbuffer, len);
   a->zbuffer += len;
   a->zout += len;
   return 1;
}

static int parse_zlib_header(zbuf *a)
{
   int cmf   = zget8(a);
   int cm    = cmf & 15;
   /* int cinfo = cmf >> 4; */
   int flg   = zget8(a);
   if ((cmf*256+flg) % 31 != 0) return e((char*)"bad zlib header",(char*)"Corrupt PNG"); // zlib spec
   if (flg & 32) return e((char*)"no preset dict",(char*)"Corrupt PNG"); // preset dictionary not allowed in png
   if (cm != 8) return e((char*)"bad compression",(char*)"Corrupt PNG"); // DEFLATE required for png
   // window = 1 << (8 + cinfo)... but who cares, we fully buffer output
   return 1;
}

// @TODO: should statically initialize these for optimal thread safety
static uint8_t default_length[288], default_distance[32];
static void init_defaults(void)
{
   int i;   // use <= to match clearly with spec
   for (i=0; i <= 143; ++i)     default_length[i]   = 8;
   for (   ; i <= 255; ++i)     default_length[i]   = 9;
   for (   ; i <= 279; ++i)     default_length[i]   = 7;
   for (   ; i <= 287; ++i)     default_length[i]   = 8;

   for (i=0; i <=  31; ++i)     default_distance[i] = 5;
}

int stbi_png_partial; // a quick hack to only allow decoding some of a PNG... I should implement real streaming support instead
static int parse_zlib(zbuf *a, int parse_header)
{
   int final, type;
   if (parse_header)
      if (!parse_zlib_header(a)) 
      {
          ERR("decompress");
              return 0;
              }
   a->num_bits = 0;
   a->code_buffer = 0;
   do {
      final = zreceive(a,1);
      type = zreceive(a,2);
      if (type == 0) {
         if (!parse_uncompressed_block(a)) 
         {
         ERR("decompress");
             return 0;
             }
      } else if (type == 3) {
         
      {
      ERR("decompress");
          return 0;
          }
      } else {
         if (type == 1) {
            // use fixed code lengths
            if (!default_distance[31]) init_defaults();
            if (!zbuild_huffman(&a->z_length  , default_length  , 288)) 
            {
            ERR("decompress");
                return 0;
                }
            if (!zbuild_huffman(&a->z_distance, default_distance,  32)) 
            {
            ERR("decompress");
                return 0;
                }
         } else {
            if (!compute_huffman_codes(a)) 
            {
            ERR("decompress");
                return 0;
                }
         }
         if (!parse_huffman_block(a)) 
         {
         ERR("decompress");
             return 0;
             }
      }
      if (stbi_png_partial && a->zout - a->zout_start > 65536)
         break;
   } while (!final);
   return 1;
}

static int do_zlib(zbuf *a, char *obuf, int olen, int exp, int parse_header)
{
   a->zout_start = obuf;
   a->zout       = obuf;
   a->zout_end   = obuf + olen;
   a->z_expandable = exp;

   return parse_zlib(a, parse_header);
}

char *stbi_zlib_decode_malloc_guesssize(const char *buffer, int len, int initial_size, int *outlen)
{
   zbuf a;
   char *p = (char *) malloc(initial_size);
   if (p == 0) return 0;
   a.zbuffer = (uint8_t *) buffer;
   a.zbuffer_end = (uint8_t *) buffer + len;
   if (do_zlib(&a, p, initial_size, 1, 1)) {
      if (outlen) *outlen = (int) (a.zout - a.zout_start);
      return a.zout_start;
   } else {
      free(a.zout_start);
      return 0;
   }
}

char *stbi_zlib_decode_malloc(char const *buffer, int len, int *outlen)
{
   return stbi_zlib_decode_malloc_guesssize(buffer, len, 16384, outlen);
}

int stbi_zlib_decode_buffer(char *obuffer, int olen, char const *ibuffer, int ilen)
{
   zbuf a;
   a.zbuffer = (uint8_t *) ibuffer;
   a.zbuffer_end = (uint8_t *) ibuffer + ilen;
   if (do_zlib(&a, obuffer, olen, 0, 1))
      return (int) (a.zout - a.zout_start);
   else
      return -1;
}

char *stbi_zlib_decode_noheader_malloc(char const *buffer, int len, int *outlen)
{
   zbuf a;
   char *p = (char *) malloc(16384);
   if (p == 0) return 0;
   a.zbuffer = (uint8_t *) buffer;
   a.zbuffer_end = (uint8_t *) buffer+len;
   if (do_zlib(&a, p, 16384, 1, 0)) {
      if (outlen) *outlen = (int) (a.zout - a.zout_start);
      return a.zout_start;
   } else {
      free(a.zout_start);
      return 0;
   }
}

int stbi_zlib_decode_noheader_buffer(char *obuffer, int olen, const char *ibuffer, int ilen)
{
   zbuf a;
   a.zbuffer = (uint8_t *) ibuffer;
   a.zbuffer_end = (uint8_t *) ibuffer + ilen;
   if (do_zlib(&a, obuffer, olen, 0, 0))
      return (int) (a.zout - a.zout_start);
   else
      return -1;
}

// public domain "baseline" PNG decoder   v0.10  Sean Barrett 2006-11-18
//    simple implementation
//      - only 8-bit samples
//      - no CRC checking
//      - allocates lots of intermediate memory
//        - avoids problem of streaming data between subsystems
//        - avoids explicit window management
//    performance
//      - uses stb_zlib, a PD zlib implementation with fast huffman decoding


typedef struct
{
   uint32_t length;
   uint32_t type;
} chunk;

#define PNG_TYPE(a,b,c,d)  (((a) << 24) + ((b) << 16) + ((c) << 8) + (d))

static chunk get_chunk_header(stbi *s)
{
   chunk c;
   c.length = get32(s);
   c.type   = get32(s);
   return c;
}

static int check_png_header(stbi *s)
{
   static uint8_t png_sig[8] = { 137,80,78,71,13,10,26,10 };
   int i;
   for (i=0; i < 8; ++i)
      if (get8(s) != png_sig[i]) return e((char*)"bad png sig",(char*)"Not a PNG");
   return 1;
}

typedef struct
{
   stbi s;
   uint8_t *idata, *expanded, *out;
} png;


enum {
   F_none=0, F_sub=1, F_up=2, F_avg=3, F_paeth=4,
   F_avg_first, F_paeth_first,
};

static uint8_t first_row_filter[5] =
{
   F_none, F_sub, F_none, F_avg_first, F_paeth_first
};

static int paeth(int a, int b, int c)
{
   int p = a + b - c;
   int pa = abs(p-a);
   int pb = abs(p-b);
   int pc = abs(p-c);
   if (pa <= pb && pa <= pc) return a;
   if (pb <= pc) return b;
   return c;
}

// create the png data from post-deflated data
static int create_png_image_raw(png *a, uint8_t *raw, uint32_t raw_len, int out_n, uint32_t x, uint32_t y)
{
   stbi *s = &a->s;
   uint32_t i,j,stride = x*out_n;
   int k;
   int img_n = s->img_n; // copy it into a local for later
   ASSERT(out_n == s->img_n || out_n == s->img_n+1);
   if (stbi_png_partial) y = 1;
   a->out = (uint8_t *) malloc(x * y * out_n);
   if (!a->out) return e((char*)"outofmem", "Out of memory");
   if (!stbi_png_partial) {
      if (s->img_x == x && s->img_y == y) {
         if (raw_len != (img_n * x + 1) * y) return e((char*)"not enough pixels",(char*)"Corrupt PNG");
      } else { // interlaced:
         if (raw_len < (img_n * x + 1) * y) return e((char*)"not enough pixels",(char*)"Corrupt PNG");
      }
   }
   for (j=0; j < y; ++j) {
      uint8_t *cur = a->out + stride*j;
      uint8_t *prior = cur - stride;
      int filter = *raw++;
      if (filter > 4) return e((char*)"invalid filter",(char*)"Corrupt PNG");
      // if first row, use special filter that doesn't sample previous row
      if (j == 0) filter = first_row_filter[filter];
      // handle first pixel explicitly
      for (k=0; k < img_n; ++k) {
         switch(filter) {
            case F_none       : cur[k] = raw[k]; break;
            case F_sub        : cur[k] = raw[k]; break;
            case F_up         : cur[k] = raw[k] + prior[k]; break;
            case F_avg        : cur[k] = raw[k] + (prior[k]>>1); break;
            case F_paeth      : cur[k] = (uint8_t) (raw[k] + paeth(0,prior[k],0)); break;
            case F_avg_first  : cur[k] = raw[k]; break;
            case F_paeth_first: cur[k] = raw[k]; break;
         }
      }
      if (img_n != out_n) cur[img_n] = 255;
      raw += img_n;
      cur += out_n;
      prior += out_n;
      // this is a little gross, so that we don't switch per-pixel or per-component
      if (img_n == out_n) {
         #define CASE(f) \
             case f:     \
                for (i=x-1; i >= 1; --i, raw+=img_n,cur+=img_n,prior+=img_n) \
                   for (k=0; k < img_n; ++k)
         switch(filter) {
            CASE(F_none)  cur[k] = raw[k]; break;
            CASE(F_sub)   cur[k] = raw[k] + cur[k-img_n]; break;
            CASE(F_up)    cur[k] = raw[k] + prior[k]; break;
            CASE(F_avg)   cur[k] = raw[k] + ((prior[k] + cur[k-img_n])>>1); break;
            CASE(F_paeth)  cur[k] = (uint8_t) (raw[k] + paeth(cur[k-img_n],prior[k],prior[k-img_n])); break;
            CASE(F_avg_first)    cur[k] = raw[k] + (cur[k-img_n] >> 1); break;
            CASE(F_paeth_first)  cur[k] = (uint8_t) (raw[k] + paeth(cur[k-img_n],0,0)); break;
         }
         #undef CASE
      } else {
         ASSERT(img_n+1 == out_n);
         #define CASE(f) \
             case f:     \
                for (i=x-1; i >= 1; --i, cur[img_n]=255,raw+=img_n,cur+=out_n,prior+=out_n) \
                   for (k=0; k < img_n; ++k)
         switch(filter) {
            CASE(F_none)  cur[k] = raw[k]; break;
            CASE(F_sub)   cur[k] = raw[k] + cur[k-out_n]; break;
            CASE(F_up)    cur[k] = raw[k] + prior[k]; break;
            CASE(F_avg)   cur[k] = raw[k] + ((prior[k] + cur[k-out_n])>>1); break;
            CASE(F_paeth)  cur[k] = (uint8_t) (raw[k] + paeth(cur[k-out_n],prior[k],prior[k-out_n])); break;
            CASE(F_avg_first)    cur[k] = raw[k] + (cur[k-out_n] >> 1); break;
            CASE(F_paeth_first)  cur[k] = (uint8_t) (raw[k] + paeth(cur[k-out_n],0,0)); break;
         }
         #undef CASE
      }
   }
   return 1;
}

static int create_png_image(png *a, uint8_t *raw, uint32_t raw_len, int out_n, int interlaced)
{
   uint8_t *final;
   int p;
   int save;
   if (!interlaced)
      return create_png_image_raw(a, raw, raw_len, out_n, a->s.img_x, a->s.img_y);
   save = stbi_png_partial;
   stbi_png_partial = 0;

   // de-interlacing
   final = (uint8_t *) malloc(a->s.img_x * a->s.img_y * out_n);
   for (p=0; p < 7; ++p) {
      int xorig[] = { 0,4,0,2,0,1,0 };
      int yorig[] = { 0,0,4,0,2,0,1 };
      int xspc[]  = { 8,8,4,4,2,2,1 };
      int yspc[]  = { 8,8,8,4,4,2,2 };
      int i,j,x,y;
      // pass1_x[4] = 0, pass1_x[5] = 1, pass1_x[12] = 1
      x = (a->s.img_x - xorig[p] + xspc[p]-1) / xspc[p];
      y = (a->s.img_y - yorig[p] + yspc[p]-1) / yspc[p];
      if (x && y) {
         if (!create_png_image_raw(a, raw, raw_len, out_n, x, y)) {
            free(final);
            return 0;
         }
         for (j=0; j < y; ++j)
            for (i=0; i < x; ++i)
               memcpy(final + (j*yspc[p]+yorig[p])*a->s.img_x*out_n + (i*xspc[p]+xorig[p])*out_n,
                      a->out + (j*x+i)*out_n, out_n);
         free(a->out);
         raw += (x*out_n+1)*y;
         raw_len -= (x*out_n+1)*y;
      }
   }
   a->out = final;

   stbi_png_partial = save;
   return 1;
}

static int compute_transparency(png *z, uint8_t tc[3], int out_n)
{
   stbi *s = &z->s;
   uint32_t i, pixel_count = s->img_x * s->img_y;
   uint8_t *p = z->out;

   // compute color-based transparency, assuming we've
   // already got 255 as the alpha value in the output
   ASSERT(out_n == 2 || out_n == 4);

   if (out_n == 2) {
      for (i=0; i < pixel_count; ++i) {
         p[1] = (p[0] == tc[0] ? 0 : 255);
         p += 2;
      }
   } else {
      for (i=0; i < pixel_count; ++i) {
         if (p[0] == tc[0] && p[1] == tc[1] && p[2] == tc[2])
            p[3] = 0;
         p += 4;
      }
   }
   return 1;
}

static int expand_palette(png *a, uint8_t *palette, int len, int pal_img_n)
{
   uint32_t i, pixel_count = a->s.img_x * a->s.img_y;
   uint8_t *p, *temp_out, *orig = a->out;

   p = (uint8_t *) malloc(pixel_count * pal_img_n);
   if (p == 0) return e((char*)"outofmem", "Out of memory");

   // between here and free(out) below, exitting would leak
   temp_out = p;

   if (pal_img_n == 3) {
      for (i=0; i < pixel_count; ++i) {
         int n = orig[i]*4;
         p[0] = palette[n  ];
         p[1] = palette[n+1];
         p[2] = palette[n+2];
         p += 3;
      }
   } else {
      for (i=0; i < pixel_count; ++i) {
         int n = orig[i]*4;
         p[0] = palette[n  ];
         p[1] = palette[n+1];
         p[2] = palette[n+2];
         p[3] = palette[n+3];
         p += 4;
      }
   }
   free(a->out);
   a->out = temp_out;
   return 1;
}

static int parse_png_file(png *z, int scan, int req_comp)
{
   uint8_t palette[1024], pal_img_n=0;
   uint8_t has_trans=0, tc[3];
   uint32_t ioff=0, idata_limit=0, i, pal_len=0;
   int first=1,k,interlace=0;
   stbi *s = &z->s;

   if (!check_png_header(s)) return 0;

   if (scan == SCAN_type) return 1;

   for(;;first=0) {
      chunk c = get_chunk_header(s);
      if (first && c.type != PNG_TYPE('I','H','D','R'))
         return e((char*)"first not IHDR",(char*)"Corrupt PNG");
      switch (c.type) {
         case PNG_TYPE('I','H','D','R'): {
            int depth,color,comp,filter;
            if (!first) return e((char*)"multiple IHDR",(char*)"Corrupt PNG");
            if (c.length != 13) return e((char*)"bad IHDR len",(char*)"Corrupt PNG");
            s->img_x = get32(s); if (s->img_x > (1 << 24)) return e((char*)"too large",(char*)"Very large image (corrupt?)");
            s->img_y = get32(s); if (s->img_y > (1 << 24)) return e((char*)"too large",(char*)"Very large image (corrupt?)");
            depth = get8(s);  if (depth != 8)        return e((char*)"8bit only",(char*)"PNG not supported: 8-bit only");
            color = get8(s);  if (color > 6)         return e((char*)"bad ctype",(char*)"Corrupt PNG");
            if (color == 3) pal_img_n = 3; else if (color & 1) return e((char*)"bad ctype",(char*)"Corrupt PNG");
            comp  = get8(s);  if (comp) return e((char*)"bad comp method",(char*)"Corrupt PNG");
            filter= get8(s);  if (filter) return e((char*)"bad filter method",(char*)"Corrupt PNG");
            interlace = get8(s); if (interlace>1) return e((char*)"bad interlace method",(char*)"Corrupt PNG");
            if (!s->img_x || !s->img_y) return e((char*)"0-pixel image",(char*)"Corrupt PNG");
            if (!pal_img_n) {
               s->img_n = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0);
               if ((1 << 30) / s->img_x / s->img_n < s->img_y) return e((char*)"too large", "Image too large to decode");
               if (scan == SCAN_header) return 1;
            } else {
               // if paletted, then pal_n is our final components, and
               // img_n is # components to decompress/filter.
               s->img_n = 1;
               if ((1 << 30) / s->img_x / 4 < s->img_y) return e((char*)"too large",(char*)"Corrupt PNG");
               // if SCAN_header, have to scan to see if we have a tRNS
            }
            break;
         }

         case PNG_TYPE('P','L','T','E'):  {
            if (c.length > 256*3) return e((char*)"invalid PLTE",(char*)"Corrupt PNG");
            pal_len = c.length / 3;
            if (pal_len * 3 != c.length) return e((char*)"invalid PLTE",(char*)"Corrupt PNG");
            for (i=0; i < pal_len; ++i) {
               palette[i*4+0] = get8u(s);
               palette[i*4+1] = get8u(s);
               palette[i*4+2] = get8u(s);
               palette[i*4+3] = 255;
            }
            break;
         }

         case PNG_TYPE('t','R','N','S'): {
            if (z->idata) return e((char*)"tRNS after IDAT",(char*)"Corrupt PNG");
            if (pal_img_n) {
               if (scan == SCAN_header) { s->img_n = 4; return 1; }
               if (pal_len == 0) return e((char*)"tRNS before PLTE",(char*)"Corrupt PNG");
               if (c.length > pal_len) return e((char*)"bad tRNS len",(char*)"Corrupt PNG");
               pal_img_n = 4;
               for (i=0; i < c.length; ++i)
                  palette[i*4+3] = get8u(s);
            } else {
               if (!(s->img_n & 1)) return e((char*)"tRNS with alpha",(char*)"Corrupt PNG");
               if (c.length != (uint32_t) s->img_n*2) return e((char*)"bad tRNS len",(char*)"Corrupt PNG");
               has_trans = 1;
               for (k=0; k < s->img_n; ++k)
                  tc[k] = (uint8_t) get16(s); // non 8-bit images will be larger
            }
            break;
         }

         case PNG_TYPE('I','D','A','T'): {
            if (pal_img_n && !pal_len) return e((char*)"no PLTE",(char*)"Corrupt PNG");
            if (scan == SCAN_header) { s->img_n = pal_img_n; return 1; }
            if (ioff + c.length > idata_limit) {
               uint8_t *p;
               if (idata_limit == 0) idata_limit = c.length > 4096 ? c.length : 4096;
               while (ioff + c.length > idata_limit)
                  idata_limit *= 2;
               p = (uint8_t *) realloc(z->idata, idata_limit); if (p == 0) return e((char*)"outofmem", "Out of memory");
               z->idata = p;
            }
            #ifndef STBI_NO_STDIO
            if (s->img_file)
            {
               if (fread(z->idata+ioff,1,c.length,s->img_file) != c.length) return e((char*)"outofdata",(char*)"Corrupt PNG");
            }
            else
            #endif
            {
               memcpy(z->idata+ioff, s->img_buffer, c.length);
               s->img_buffer += c.length;
            }
            ioff += c.length;
            break;
         }

         case PNG_TYPE('I','E','N','D'): {
            uint32_t raw_len;
            if (scan != SCAN_load) return 1;
            if (z->idata == 0) return e((char*)"no IDAT",(char*)"Corrupt PNG");
            z->expanded = (uint8_t *) stbi_zlib_decode_malloc((char *) z->idata, ioff, (int *) &raw_len);
            if (z->expanded == 0) return 0; // zlib should set error
            free(z->idata); z->idata = 0;
            if ((req_comp == s->img_n+1 && req_comp != 3 && !pal_img_n) || has_trans)
               s->img_out_n = s->img_n+1;
            else
               s->img_out_n = s->img_n;
            if (!create_png_image(z, z->expanded, raw_len, s->img_out_n, interlace)) return 0;
            if (has_trans)
               if (!compute_transparency(z, tc, s->img_out_n)) return 0;
            if (pal_img_n) {
               // pal_img_n == 3 or 4
               s->img_n = pal_img_n; // record the actual colors we had
               s->img_out_n = pal_img_n;
               if (req_comp >= 3) s->img_out_n = req_comp;
               if (!expand_palette(z, palette, pal_len, s->img_out_n))
                  return 0;
            }
            free(z->expanded); z->expanded = 0;
            return 1;
         }

         default:
            // if critical, fail
            if ((c.type & (1 << 29)) == 0) {
               #ifndef STBI_NO_FAILURE_STRINGS
               // not threadsafe
               static char invalid_chunk[] = "chunk not known";
               invalid_chunk[0] = (uint8_t) (c.type >> 24);
               invalid_chunk[1] = (uint8_t) (c.type >> 16);
               invalid_chunk[2] = (uint8_t) (c.type >>  8);
               invalid_chunk[3] = (uint8_t) (c.type >>  0);
               #endif
               return e(invalid_chunk, "PNG not supported: unknown chunk type");
            }
            skip(s, c.length);
            break;
      }
      // end of chunk, read and skip CRC
      get32(s);
   }
}

static unsigned char *do_png(png *p, int *x, int *y, int *n, int req_comp)
{
   unsigned char *result=0;
   p->expanded = 0;
   p->idata = 0;
   p->out = 0;
   if (req_comp < 0 || req_comp > 4) return epuc((char*)"bad req_comp", "Internal error");
   if (parse_png_file(p, SCAN_load, req_comp)) {
      result = p->out;
      p->out = 0;
      if (req_comp && req_comp != p->s.img_out_n) {
         result = convert_format(result, p->s.img_out_n, req_comp, p->s.img_x, p->s.img_y);
         p->s.img_out_n = req_comp;
         if (result == 0) return result;
      }
      *x = p->s.img_x;
      *y = p->s.img_y;
      if (n) *n = p->s.img_n;
   }
   free(p->out);      p->out      = 0;
   free(p->expanded); p->expanded = 0;
   free(p->idata);    p->idata    = 0;

   return result;
}

#ifndef STBI_NO_STDIO
unsigned char *stbi_png_load_from_file(FILE *f, int *x, int *y, int *comp, int req_comp)
{
   png p;
   start_file(&p.s, f);
   return do_png(&p, x,y,comp,req_comp);
}

unsigned char *stbi_png_load(char const *filename, int *x, int *y, int *comp, int req_comp)
{
   unsigned char *data;
   FILE *f = fopen(filename, "rb");
   if (!f) return 0;
   data = stbi_png_load_from_file(f,x,y,comp,req_comp);
   fclose(f);
   return data;
}
#endif

unsigned char *stbi_png_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   png p;
   start_mem(&p.s, buffer,len);
   return do_png(&p, x,y,comp,req_comp);
}

#ifndef STBI_NO_STDIO
int stbi_png_test_file(FILE *f)
{
   png p;
   int n,r;
   n = (int)ftell(f);
   start_file(&p.s, f);
   r = parse_png_file(&p, SCAN_type,STBI_default);
   fseek(f,n,SEEK_SET);
   return r;
}
#endif

int stbi_png_test_memory(stbi_uc const *buffer, int len)
{
   png p;
   start_mem(&p.s, buffer, len);
   return parse_png_file(&p, SCAN_type,STBI_default);
}

// TODO: load header from png
#ifndef STBI_NO_STDIO
int      stbi_png_info             (char const *filename,           int *x, int *y, int *comp)
{
   png p;
   FILE *f = fopen(filename, "rb");
   if (!f) return 0;
   start_file(&p.s, f);
   if (parse_png_file(&p, SCAN_header, 0)) {
      if(x) *x = p.s.img_x;
      if(y) *y = p.s.img_y;
      if (comp) *comp = p.s.img_n;
      fclose(f);
      return 1;
   }
   fclose(f);
   return 0;
}

extern int      stbi_png_info_from_file   (FILE *f,                  int *x, int *y, int *comp);
#endif
extern int      stbi_png_info_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp);

// Microsoft/Windows BMP image

static int bmp_test(stbi *s)
{
   int sz;
   if (get8(s) != 'B') return 0;
   if (get8(s) != 'M') return 0;
   get32le(s); // discard filesize
   get16le(s); // discard reserved
   get16le(s); // discard reserved
   get32le(s); // discard data offset
   sz = get32le(s);
   if (sz == 12 || sz == 40 || sz == 56 || sz == 108) return 1;
   return 0;
}

#ifndef STBI_NO_STDIO
int      stbi_bmp_test_file        (FILE *f)
{
   stbi s;
   int r,n = (int)ftell(f);
   start_file(&s,f);
   r = bmp_test(&s);
   fseek(f,n,SEEK_SET);
   return r;
}
#endif

int      stbi_bmp_test_memory      (stbi_uc const *buffer, int len)
{
   stbi s;
   start_mem(&s, buffer, len);
   return bmp_test(&s);
}

// returns 0..31 for the highest set bit
static int high_bit(unsigned int z)
{
   int n=0;
   if (z == 0) return -1;
   if (z >= 0x10000) n += 16, z >>= 16;
   if (z >= 0x00100) n +=  8, z >>=  8;
   if (z >= 0x00010) n +=  4, z >>=  4;
   if (z >= 0x00004) n +=  2, z >>=  2;
   if (z >= 0x00002) n +=  1, z >>=  1;
   return n;
}

static int bitcount(unsigned int a)
{
   a = (a & 0x55555555) + ((a >>  1) & 0x55555555); // max 2
   a = (a & 0x33333333) + ((a >>  2) & 0x33333333); // max 4
   a = (a + (a >> 4)) & 0x0f0f0f0f; // max 8 per 4, now 8 bits
   a = (a + (a >> 8)); // max 16 per 8 bits
   a = (a + (a >> 16)); // max 32 per 8 bits
   return a & 0xff;
}

static int shiftsigned(int v, int shift, int bits)
{
   int result;
   int z=0;

   if (shift < 0) v <<= -shift;
   else v >>= shift;
   result = v;

   z = bits;
   while (z < 8) {
      result += v >> z;
      z += bits;
   }
   return result;
}

static stbi_uc *bmp_load(stbi *s, int *x, int *y, int *comp, int req_comp)
{
   uint8_t *out;
   unsigned int mr=0,mg=0,mb=0,ma=0;
   stbi_uc pal[256][4];
   int psize=0,i,j,compress=0,width;
   int bpp, flip_horizically, pad, target, offset, hsz;
   if (get8(s) != 'B' || get8(s) != 'M') return epuc((char*)"not BMP", "Corrupt BMP");
   get32le(s); // discard filesize
   get16le(s); // discard reserved
   get16le(s); // discard reserved
   offset = get32le(s);
   hsz = get32le(s);
   if (hsz != 12 && hsz != 40 && hsz != 56 && hsz != 108) return epuc((char*)"unknown BMP", "BMP type not supported: unknown");
   failure_reason = (char*)"bad BMP";
   if (hsz == 12) {
      s->img_x = get16le(s);
      s->img_y = get16le(s);
   } else {
      s->img_x = get32le(s);
      s->img_y = get32le(s);
   }
   if (get16le(s) != 1) return 0;
   bpp = get16le(s);
   if (bpp == 1) return epuc((char*)"monochrome", "BMP type not supported: 1-bit");
   flip_horizically = ((int) s->img_y) > 0;
   s->img_y = abs((int) s->img_y);
   if (hsz == 12) {
      if (bpp < 24)
         psize = (offset - 14 - 24) / 3;
   } else {
      compress = get32le(s);
      if (compress == 1 || compress == 2) return epuc((char*)"BMP RLE", "BMP type not supported: RLE");
      get32le(s); // discard sizeof
      get32le(s); // discard hres
      get32le(s); // discard vres
      get32le(s); // discard colorsused
      get32le(s); // discard max important
      if (hsz == 40 || hsz == 56) {
         if (hsz == 56) {
            get32le(s);
            get32le(s);
            get32le(s);
            get32le(s);
         }
         if (bpp == 16 || bpp == 32) {
            if (compress == 0) {
               if (bpp == 32) {
                  mr = 0xff << 16;
                  mg = 0xff <<  8;
                  mb = 0xff <<  0;
                  ma = 0xff << 24;
//                  fake_a = 1; // @TODO: check for cases like alpha value is 
//                  all 0 and switch it to 255
               } else {
                  mr = 31 << 10;
                  mg = 31 <<  5;
                  mb = 31 <<  0;
               }
            } else if (compress == 3) {
               mr = get32le(s);
               mg = get32le(s);
               mb = get32le(s);
               // not documented, but generated by photoshop and handled by mspaint
               if (mr == mg && mg == mb) {
                  // ?!?!?
                  return 0;
               }
            } else
               return 0;
         }
      } else {
         ASSERT(hsz == 108);
         mr = get32le(s);
         mg = get32le(s);
         mb = get32le(s);
         ma = get32le(s);
         get32le(s); // discard color space
         for (i=0; i < 12; ++i)
            get32le(s); // discard color space parameters
      }
      if (bpp < 16)
         psize = (offset - 14 - hsz) >> 2;
   }
   s->img_n = ma ? 4 : 3;
   if (req_comp && req_comp >= 3) // we can directly decode 3 or 4
      target = req_comp;
   else
      target = s->img_n; // if they want monochrome, we'll post-convert
   out = (stbi_uc *) malloc(target * s->img_x * s->img_y);
   if (!out) return epuc((char*)"outofmem", "Out of memory");
   if (bpp < 16) {
      int z=0;
      if (psize == 0 || psize > 256) { free(out); return epuc((char*)"invalid", "Corrupt BMP"); }
      for (i=0; i < psize; ++i) {
         pal[i][2] = get8(s);
         pal[i][1] = get8(s);
         pal[i][0] = get8(s);
         if (hsz != 12) get8(s);
         pal[i][3] = 255;
      }
      skip(s, offset - 14 - hsz - psize * (hsz == 12 ? 3 : 4));
      if (bpp == 4) width = (s->img_x + 1) >> 1;
      else if (bpp == 8) width = s->img_x;
      else { free(out); return epuc((char*)"bad bpp", "Corrupt BMP"); }
      pad = (-width)&3;
      for (j=0; j < (int) s->img_y; ++j) {
         for (i=0; i < (int) s->img_x; i += 2) {
            int v=get8(s),v2=0;
            if (bpp == 4) {
               v2 = v & 15;
               v >>= 4;
            }
            out[z++] = pal[v][0];
            out[z++] = pal[v][1];
            out[z++] = pal[v][2];
            if (target == 4) out[z++] = 255;
            if (i+1 == (int) s->img_x) break;
            v = (bpp == 8) ? get8(s) : v2;
            out[z++] = pal[v][0];
            out[z++] = pal[v][1];
            out[z++] = pal[v][2];
            if (target == 4) out[z++] = 255;
         }
         skip(s, pad);
      }
   } else {
      int rshift=0,gshift=0,bshift=0,ashift=0,rcount=0,gcount=0,bcount=0,acount=0;
      int z = 0;
      int easy=0;
      skip(s, offset - 14 - hsz);
      if (bpp == 24) width = 3 * s->img_x;
      else if (bpp == 16) width = 2*s->img_x;
      else /* bpp = 32 and pad = 0 */ width=0;
      pad = (-width) & 3;
      if (bpp == 24) {
         easy = 1;
      } else if (bpp == 32) {
         if (mb == 0xff && mg == 0xff00 && mr == 0xff000000 && ma == 0xff000000)
            easy = 2;
      }
      if (!easy) {
         if (!mr || !mg || !mb) return epuc((char*)"bad masks", "Corrupt BMP");
         // right shift amt to put high bit in position #7
         rshift = high_bit(mr)-7; rcount = bitcount(mr);
         gshift = high_bit(mg)-7; gcount = bitcount(mr);
         bshift = high_bit(mb)-7; bcount = bitcount(mr);
         ashift = high_bit(ma)-7; acount = bitcount(mr);
      }
      for (j=0; j < (int) s->img_y; ++j) {
         if (easy) {
            for (i=0; i < (int) s->img_x; ++i) {
               int a;
               out[z+2] = get8(s);
               out[z+1] = get8(s);
               out[z+0] = get8(s);
               z += 3;
               a = (easy == 2 ? get8(s) : 255);
               if (target == 4) out[z++] = a;
            }
         } else {
            for (i=0; i < (int) s->img_x; ++i) {
               uint32_t v = (bpp == 16 ? get16le(s) : get32le(s));
               int a;
               out[z++] = shiftsigned(v & mr, rshift, rcount);
               out[z++] = shiftsigned(v & mg, gshift, gcount);
               out[z++] = shiftsigned(v & mb, bshift, bcount);
               a = (ma ? shiftsigned(v & ma, ashift, acount) : 255);
               if (target == 4) out[z++] = a; 
            }
         }
         skip(s, pad);
      }
   }
   if (flip_horizically) {
      stbi_uc t;
      for (j=0; j < (int) s->img_y>>1; ++j) {
         stbi_uc *p1 = out +      j     *s->img_x*target;
         stbi_uc *p2 = out + (s->img_y-1-j)*s->img_x*target;
         for (i=0; i < (int) s->img_x*target; ++i) {
            t = p1[i], p1[i] = p2[i], p2[i] = t;
         }
      }
   }

   if (req_comp && req_comp != target) {
      out = convert_format(out, target, req_comp, s->img_x, s->img_y);
      if (out == 0) return out; // convert_format frees input on failure
   }

   *x = s->img_x;
   *y = s->img_y;
   if (comp) *comp = target;
   return out;
}

#ifndef STBI_NO_STDIO
stbi_uc *stbi_bmp_load             (char const *filename,           int *x, int *y, int *comp, int req_comp)
{
   stbi_uc *data;
   FILE *f = fopen(filename, "rb");
   if (!f) return 0;
   data = stbi_bmp_load_from_file(f, x,y,comp,req_comp);
   fclose(f);
   return data;
}

stbi_uc *stbi_bmp_load_from_file   (FILE *f,                  int *x, int *y, int *comp, int req_comp)
{
   stbi s;
   start_file(&s, f);
   return bmp_load(&s, x,y,comp,req_comp);
}
#endif

stbi_uc *stbi_bmp_load_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   stbi s;
   start_mem(&s, buffer, len);
   return bmp_load(&s, x,y,comp,req_comp);
}

// Targa Truevision - TGA
// by Jonathan Dummer

static int tga_test(stbi *s)
{
	int sz;
	get8u(s);		//	discard Offset
	sz = get8u(s);	//	color type
	if( sz > 1 ) return 0;	//	only RGB or indexed allowed
	sz = get8u(s);	//	image type
	if( (sz != 1) && (sz != 2) && (sz != 3) && (sz != 9) && (sz != 10) && (sz != 11) ) return 0;	//	only RGB or grey allowed, +/- RLE
	get16(s);		//	discard palette start
	get16(s);		//	discard palette length
	get8(s);			//	discard bits per palette color entry
	get16(s);		//	discard x origin
	get16(s);		//	discard y origin
	if( get16(s) < 1 ) return 0;		//	test width
	if( get16(s) < 1 ) return 0;		//	test height
	sz = get8(s);	//	bits per pixel
	if( (sz != 8) && (sz != 16) && (sz != 24) && (sz != 32) ) return 0;	//	only RGB or RGBA or grey allowed
	return 1;		//	seems to have passed everything
}

#ifndef STBI_NO_STDIO
int      stbi_tga_test_file        (FILE *f)
{
   stbi s;
   int r,n = (int)ftell(f);
   start_file(&s, f);
   r = tga_test(&s);
   fseek(f,n,SEEK_SET);
   return r;
}
#endif

int      stbi_tga_test_memory      (stbi_uc const *buffer, int len)
{
   stbi s;
   start_mem(&s, buffer, len);
   return tga_test(&s);
}

int tga_x_origin;
int tga_y_origin;
int tga_palette_start;
int tga_palette_len;
int tga_palette_bits;

static stbi_uc *tga_load(stbi *s, int *x, int *y, int *comp, int req_comp)
{
	//	read in the TGA header stuff
	int tga_offset = get8u(s);
	int tga_indexed = get8u(s);
	int tga_image_type = get8u(s);
	int tga_is_RLE = 0;
	tga_palette_start = get16le(s);
	tga_palette_len = get16le(s);
	tga_palette_bits = get8u(s);
	tga_x_origin = get16le(s);
	tga_y_origin = get16le(s);
	int tga_width = get16le(s);
	int tga_height = get16le(s);
	int tga_bits_per_pixel = get8u(s);
	int tga_inverted = get8u(s);
	//	image data
	unsigned char *tga_data;
	unsigned char *tga_palette = 0;
	int i, j;
	unsigned char raw_data[4] = {0};
	unsigned char trans_data[4] = {0};
	int RLE_count = 0;
	int RLE_repeating = 0;
	int read_next_pixel = 1;
	//	do a tiny bit of precessing
	if( tga_image_type >= 8 )
	{
		tga_image_type -= 8;
		tga_is_RLE = 1;
	}
	/* int tga_alpha_bits = tga_inverted & 15; */
	tga_inverted = 1 - ((tga_inverted >> 5) & 1);

	//	error check
	if( //(tga_indexed) ||
		(tga_width < 1) || (tga_height < 1) ||
		(tga_image_type < 1) || (tga_image_type > 3) ||
		((tga_bits_per_pixel != 8) && (tga_bits_per_pixel != 16) &&
		(tga_bits_per_pixel != 24) && (tga_bits_per_pixel != 32))
		)
	{
		return 0;
	}

	//	If I'm paletted, then I'll use the number of bits from the palette
	if( tga_indexed )
	{
		tga_bits_per_pixel = tga_palette_bits;
	}

	//	tga info
	*x = tga_width;
	*y = tga_height;
	if( (req_comp < 1) || (req_comp > 4) )
	{
		//	just use whatever the file was
		req_comp = tga_bits_per_pixel / 8;
		*comp = req_comp;
	} else
	{
		//	force a new number of components
		*comp = tga_bits_per_pixel/8;
	}
	tga_data = (unsigned char*)malloc( tga_width * tga_height * req_comp );

	//	skip to the data's starting position (offset usually = 0)
	skip(s, tga_offset );
	//	do I need to load a palette?
	if( tga_indexed )
	{
		//	any data to skip? (offset usually = 0)
		skip(s, tga_palette_start );
		//	load the palette
		tga_palette = (unsigned char*)malloc( tga_palette_len * tga_palette_bits / 8 );
		getn(s, tga_palette, tga_palette_len * tga_palette_bits / 8 );
	}
	//	load the data
	for( i = 0; i < tga_width * tga_height; ++i )
	{
		//	if I'm in RLE mode, do I need to get a RLE chunk?
		if( tga_is_RLE )
		{
			if( RLE_count == 0 )
			{
				//	yep, get the next byte as a RLE command
				int RLE_cmd = get8u(s);
				RLE_count = 1 + (RLE_cmd & 127);
				RLE_repeating = RLE_cmd >> 7;
				read_next_pixel = 1;
			} else if( !RLE_repeating )
			{
				read_next_pixel = 1;
			}
		} else
		{
			read_next_pixel = 1;
		}
		//	OK, if I need to read a pixel, do it now
		if( read_next_pixel )
		{
			//	load however much data we did have
			if( tga_indexed )
			{
				//	read in 1 byte, then perform the lookup
				int pal_idx = get8u(s);
				if( pal_idx >= tga_palette_len )
				{
					//	invalid index
					pal_idx = 0;
				}
				pal_idx *= tga_bits_per_pixel / 8;
				for( j = 0; j*8 < tga_bits_per_pixel; ++j )
				{
					raw_data[j] = tga_palette[pal_idx+j];
				}
			} else
			{
				//	read in the data raw
				for( j = 0; j*8 < tga_bits_per_pixel; ++j )
				{
					raw_data[j] = get8u(s);
				}
			}
			//	convert raw to the intermediate format
			switch( tga_bits_per_pixel )
			{
			case 8:
				//	Luminous => RGBA
				trans_data[0] = raw_data[0];
				trans_data[1] = raw_data[0];
				trans_data[2] = raw_data[0];
				trans_data[3] = 255;
				break;
			case 16:
				//	Luminous,Alpha => RGBA
				trans_data[0] = raw_data[0];
				trans_data[1] = raw_data[0];
				trans_data[2] = raw_data[0];
				trans_data[3] = raw_data[1];
				break;
			case 24:
				//	BGR => RGBA
				trans_data[0] = raw_data[2];
				trans_data[1] = raw_data[1];
				trans_data[2] = raw_data[0];
				trans_data[3] = 255;
				break;
			case 32:
				//	BGRA => RGBA
				trans_data[0] = raw_data[2];
				trans_data[1] = raw_data[1];
				trans_data[2] = raw_data[0];
				trans_data[3] = raw_data[3];
				break;
			}
			//	clear the reading flag for the next pixel
			read_next_pixel = 0;
		} // end of reading a pixel
		//	convert to final format
		switch( req_comp )
		{
		case 1:
			//	RGBA => Luminance
			tga_data[i*req_comp+0] = compute_y(trans_data[0],trans_data[1],trans_data[2]);
			break;
		case 2:
			//	RGBA => Luminance,Alpha
			tga_data[i*req_comp+0] = compute_y(trans_data[0],trans_data[1],trans_data[2]);
			tga_data[i*req_comp+1] = trans_data[3];
			break;
		case 3:
			//	RGBA => RGB
			tga_data[i*req_comp+0] = trans_data[0];
			tga_data[i*req_comp+1] = trans_data[1];
			tga_data[i*req_comp+2] = trans_data[2];
			break;
		case 4:
			//	RGBA => RGBA
			tga_data[i*req_comp+0] = trans_data[0];
			tga_data[i*req_comp+1] = trans_data[1];
			tga_data[i*req_comp+2] = trans_data[2];
			tga_data[i*req_comp+3] = trans_data[3];
			break;
		}
		//	in case we're in RLE mode, keep counting down
		--RLE_count;
	}
	//	do I need to invert the image?
	if( tga_inverted )
	{
		for( j = 0; j*2 < tga_height; ++j )
		{
			int index1 = j * tga_width * req_comp;
			int index2 = (tga_height - 1 - j) * tga_width * req_comp;
			for( i = tga_width * req_comp; i > 0; --i )
			{
				unsigned char temp = tga_data[index1];
				tga_data[index1] = tga_data[index2];
				tga_data[index2] = temp;
				++index1;
				++index2;
			}
		}
	}
	//	clear my palette, if I had one
	if( tga_palette != 0 )
	{
		free( tga_palette );
	}
	//	the things I do to get rid of an error message, and yet keep
	//	Microsoft's C compilers happy... [8^(
	tga_palette_start = tga_palette_len = tga_palette_bits =
			tga_x_origin = tga_y_origin = 0;
	//	OK, done
	return tga_data;
}

#ifndef STBI_NO_STDIO
stbi_uc *stbi_tga_load             (char const *filename,           int *x, int *y, int *comp, int req_comp)
{
   stbi_uc *data;
   FILE *f = fopen(filename, "rb");
   if (!f) return 0;
   data = stbi_tga_load_from_file(f, x,y,comp,req_comp);
   fclose(f);
   return data;
}

stbi_uc *stbi_tga_load_from_file   (FILE *f,                  int *x, int *y, int *comp, int req_comp)
{
   stbi s;
   start_file(&s, f);
   return tga_load(&s, x,y,comp,req_comp);
}
#endif

stbi_uc *stbi_tga_load_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   stbi s;
   start_mem(&s, buffer, len);
   return tga_load(&s, x,y,comp,req_comp);
}


// *************************************************************************************************
// Photoshop PSD loader -- PD by Thatcher Ulrich, integration by Nicholas Schulz, tweaked by STB

static int psd_test(stbi *s)
{
	if (get32(s) != 0x38425053) return 0;	// "8BPS"
	else return 1;
}

#ifndef STBI_NO_STDIO
int stbi_psd_test_file(FILE *f)
{
   stbi s;
   int r,n = (int)ftell(f);
   start_file(&s, f);
   r = psd_test(&s);
   fseek(f,n,SEEK_SET);
   return r;
}
#endif

int stbi_psd_test_memory(stbi_uc const *buffer, int len)
{
   stbi s;
   start_mem(&s, buffer, len);
   return psd_test(&s);
}

static stbi_uc *psd_load(stbi *s, int *x, int *y, int *comp, int req_comp)
{
	int	pixelCount;
	int channelCount, compression;
	int channel, i, count, len;
   int w,h;
   uint8_t *out;

	// Check identifier
	if (get32(s) != 0x38425053)	// "8BPS"
		return epuc((char*)"not PSD", "Corrupt PSD image");

	// Check file type version.
	if (get16(s) != 1)
		return epuc((char*)"wrong version", "Unsupported version of PSD image");

	// Skip 6 reserved bytes.
	skip(s, 6 );

	// Read the number of channels (R, G, B, A, etc).
	channelCount = get16(s);
	if (channelCount < 0 || channelCount > 16)
		return epuc((char*)"wrong channel count", "Unsupported number of channels in PSD image");

	// Read the rows and columns of the image.
   h = get32(s);
   w = get32(s);
	
	// Make sure the depth is 8 bits.
	if (get16(s) != 8)
		return epuc((char*)"unsupported bit depth", "PSD bit depth is not 8 bit");

	// Make sure the color mode is RGB.
	// Valid options are:
	//   0: Bitmap
	//   1: Grayscale
	//   2: Indexed color
	//   3: RGB color
	//   4: CMYK color
	//   7: Multichannel
	//   8: Duotone
	//   9: Lab color
	if (get16(s) != 3)
		return epuc((char*)"wrong color format", "PSD is not in RGB color format");

	// Skip the Mode Data.  (It's the palette for indexed color; other info for other modes.)
	skip(s,get32(s) );

	// Skip the image resources.  (resolution, pen tool paths, etc)
	skip(s, get32(s) );

	// Skip the reserved data.
	skip(s, get32(s) );

	// Find out if the data is compressed.
	// Known values:
	//   0: no compression
	//   1: RLE compressed
	compression = get16(s);
	if (compression > 1)
		return epuc((char*)"bad compression", "PSD has an unknown compression format");

	// Create the destination image.
	out = (stbi_uc *) malloc(4 * w*h);
	if (!out) return epuc((char*)"outofmem", "Out of memory");
   pixelCount = w*h;

	// Initialize the data to zero.
	//memset( out, 0, pixelCount * 4 );
	
	// Finally, the image data.
	if (compression) {
		// RLE as used by .PSD and .TIFF
		// Loop until you get the number of unpacked bytes you are expecting:
		//     Read the next source byte into n.
		//     If n is between 0 and 127 inclusive, copy the next n+1 bytes literally.
		//     Else if n is between -127 and -1 inclusive, copy the next byte -n+1 times.
		//     Else if n is 128, noop.
		// Endloop

		// The RLE-compressed data is preceeded by a 2-byte data count for each row in the data,
		// which we're going to just skip.
		skip(s, h * channelCount * 2 );

		// Read the RLE data by channel.
		for (channel = 0; channel < 4; channel++) {
			uint8_t *p;
			
         p = out+channel;
			if (channel >= channelCount) {
				// Fill this channel with default data.
				for (i = 0; i < pixelCount; i++) *p = (channel == 3 ? 255 : 0), p += 4;
			} else {
				// Read the RLE data.
				count = 0;
				while (count < pixelCount) {
					len = get8(s);
					if (len == 128) {
						// No-op.
					} else if (len < 128) {
						// Copy next len+1 bytes literally.
						len++;
						count += len;
						while (len) {
							*p = get8(s);
                     p += 4;
							len--;
						}
					} else if (len > 128) {
						uint32_t	val;
						// Next -len+1 bytes in the dest are replicated from next source byte.
						// (Interpret len as a negative 8-bit int.)
						len ^= 0x0FF;
						len += 2;
                  val = get8(s);
						count += len;
						while (len) {
							*p = val;
                     p += 4;
							len--;
						}
					}
				}
			}
		}
		
	} else {
		// We're at the raw image data.  It's each channel in order (Red, Green, Blue, Alpha, ...)
		// where each channel consists of an 8-bit value for each pixel in the image.
		
		// Read the data by channel.
		for (channel = 0; channel < 4; channel++) {
			uint8_t *p;
			
         p = out + channel;
			if (channel > channelCount) {
				// Fill this channel with default data.
				for (i = 0; i < pixelCount; i++) *p = channel == 3 ? 255 : 0, p += 4;
			} else {
				// Read the data.
				for (i = 0; i < pixelCount; i++)
					*p = get8(s), p += 4;
			}
		}
	}

	if (req_comp && req_comp != 4) {
		out = convert_format(out, 4, req_comp, w, h);
		if (out == 0) return out; // convert_format frees input on failure
	}

	if (comp) *comp = channelCount;
	*y = h;
	*x = w;
	
	return out;
}

#ifndef STBI_NO_STDIO
stbi_uc *stbi_psd_load(char const *filename, int *x, int *y, int *comp, int req_comp)
{
   stbi_uc *data;
   FILE *f = fopen(filename, "rb");
   if (!f) return 0;
   data = stbi_psd_load_from_file(f, x,y,comp,req_comp);
   fclose(f);
   return data;
}

stbi_uc *stbi_psd_load_from_file(FILE *f, int *x, int *y, int *comp, int req_comp)
{
   stbi s;
   start_file(&s, f);
   return psd_load(&s, x,y,comp,req_comp);
}
#endif

stbi_uc *stbi_psd_load_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   stbi s;
   start_mem(&s, buffer, len);
   return psd_load(&s, x,y,comp,req_comp);
}


// *************************************************************************************************
// Radiance RGBE HDR loader
// originally by Nicolas Schulz
#ifndef STBI_NO_HDR
static int hdr_test(stbi *s)
{
   char *signature = (char*)"#?RADIANCE\n";
   int i;
   for (i=0; signature[i]; ++i)
      if (get8(s) != signature[i])
         return 0;
	return 1;
}

int stbi_hdr_test_memory(stbi_uc const *buffer, int len)
{
   stbi s;
	start_mem(&s, buffer, len);
	return hdr_test(&s);
}

#ifndef STBI_NO_STDIO
int stbi_hdr_test_file(FILE *f)
{
   stbi s;
   memset(&s, 0, sizeof(s));
   int r,n = (int)ftell(f);
   start_file(&s, f);
   r = hdr_test(&s);
   fseek(f,n,SEEK_SET);
   return r;
}
#endif

#define HDR_BUFLEN  1024
static char *hdr_gettoken(stbi *z, char *buffer)
{
   int len=0;
	char c = '\0';

   c = get8(z);

	while (!at_eof(z) && c != '\n') {
		buffer[len++] = c;
      if (len == HDR_BUFLEN-1) {
         // flush to end of line
         while (!at_eof(z) && get8(z) != '\n')
            ;
         break;
      }
      c = get8(z);
	}

   buffer[len] = 0;
	return buffer;
}

static void hdr_convert(float *output, stbi_uc *input, int req_comp)
{
	if( input[3] != 0 ) {
      float f1;
		// Exponent
		f1 = (float) ldexp(1.0f, input[3] - (int)(128 + 8));
      if (req_comp <= 2)
         output[0] = (input[0] + input[1] + input[2]) * f1 / 3;
      else {
         output[0] = input[0] * f1;
         output[1] = input[1] * f1;
         output[2] = input[2] * f1;
      }
      if (req_comp == 2) output[1] = 1;
      if (req_comp == 4) output[3] = 1;
	} else {
      switch (req_comp) {
         case 4: output[3] = 1; /* fallthrough */
         case 3: output[0] = output[1] = output[2] = 0;
                 break;
         case 2: output[1] = 1; /* fallthrough */
         case 1: output[0] = 0;
                 break;
      }
	}
}


static float *hdr_load(stbi *s, int *x, int *y, int *comp, int req_comp)
{
   char buffer[HDR_BUFLEN];
	char *token;
	int valid = 0;
	int width, height;
   stbi_uc *scanline;
	float *hdr_data;
	int len;
	unsigned char count, value;
	int i, j, k, c1,c2, z;


	// Check identifier
	if (strcmp(hdr_gettoken(s,buffer), "#?RADIANCE") != 0)
		return epf((char*)"not HDR", (char*)"Corrupt HDR image");
	
	// Parse header
	while(1) {
		token = hdr_gettoken(s,buffer);
      if (token[0] == 0) break;
		if (strcmp(token, "FORMAT=32-bit_rle_rgbe") == 0) valid = 1;
   }

	if (!valid)    return epf((char*)"unsupported format", (char*)"Unsupported HDR format");

   // Parse width and height
   // can't use sscanf() if we're not using stdio!
   token = hdr_gettoken(s,buffer);
   if (strncmp(token, "-Y ", 3))  return epf((char*)"unsupported data layout", (char*)"Unsupported HDR format");
   token += 3;
   height = (int)strtol(token, &token, 10);
   while (*token == ' ') ++token;
   if (strncmp(token, "+X ", 3))  return epf((char*)"unsupported data layout", (char*)"Unsupported HDR format");
   token += 3;
   width = (int)strtol(token, 0, 10);

	*x = width;
	*y = height;

   *comp = 3;
	if (req_comp == 0) req_comp = 3;

	// Read data
	hdr_data = (float *) malloc(height * width * req_comp * sizeof(float));

	// Load image data
   // image data is stored as some number of sca
	if( width < 8 || width >= 32768) {
		// Read flat data
      for (j=0; j < height; ++j) {
         for (i=0; i < width; ++i) {
            stbi_uc rgbe[4];
           main_decode_loop:
            getn(s, rgbe, 4);
            hdr_convert(hdr_data + j * width * req_comp + i * req_comp, rgbe, req_comp);
         }
      }
	} else {
		// Read RLE-encoded data
		scanline = 0;

		for (j = 0; j < height; ++j) {
         c1 = get8(s);
         c2 = get8(s);
         len = get8(s);
         if (c1 != 2 || c2 != 2 || (len & 0x80)) {
            // not run-length encoded, so we have to actually use THIS data as a decoded
            // pixel (note this can't be a valid pixel--one of RGB must be >= 128)
            stbi_uc rgbe[4] = { (stbi_uc) c1,(stbi_uc) c2,(stbi_uc) len, (stbi_uc) get8(s) };
            hdr_convert(hdr_data, rgbe, req_comp);
            i = 1;
            j = 0;
            free(scanline);
            goto main_decode_loop; // yes, this is fucking insane; blame the fucking insane format
         }
         len <<= 8;
         len |= get8(s);
         if (len != width) { free(hdr_data); free(scanline); return epf((char*)"invalid decoded scanline length", (char*)"corrupt HDR"); }
         if (scanline == 0) scanline = (stbi_uc *) malloc(width * 4);
				
			for (k = 0; k < 4; ++k) {
				i = 0;
				while (i < width) {
					count = get8(s);
					if (count > 128) {
						// Run
						value = get8(s);
                  count -= 128;
						for (z = 0; z < count; ++z)
							scanline[i++ * 4 + k] = value;
					} else {
						// Dump
						for (z = 0; z < count; ++z)
							scanline[i++ * 4 + k] = get8(s);
					}
				}
			}
         for (i=0; i < width; ++i)
            hdr_convert(hdr_data+(j*width + i)*req_comp, scanline + i*4, req_comp);
		}
      free(scanline);
	}

   return hdr_data;
}

#ifndef STBI_NO_STDIO
float *stbi_hdr_load_from_file(FILE *f, int *x, int *y, int *comp, int req_comp)
{
   stbi s;
   start_file(&s,f);
   return hdr_load(&s,x,y,comp,req_comp);
}
#endif

float *stbi_hdr_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   stbi s;
   start_mem(&s,buffer, len);
   return hdr_load(&s,x,y,comp,req_comp);
}

#endif // STBI_NO_HDR

/////////////////////// write image ///////////////////////

#ifndef STBI_NO_WRITE

static void write8(FILE *f, int x) { uint8_t z = (uint8_t) x; fwrite(&z,1,1,f); }

static void writefv(FILE *f, char *fmt, va_list v)
{
   while (*fmt) {
      switch (*fmt++) {
         case ' ': break;
         case '1': { uint8_t x = va_arg(v, int); write8(f,x); break; }
         case '2': { int16 x = va_arg(v, int); write8(f,x); write8(f,x>>8); break; }
         case '4': { int32 x = va_arg(v, int); write8(f,x); write8(f,x>>8); write8(f,x>>16); write8(f,x>>24); break; }
         default:
            ASSERT(0);
            va_end(v);
            return;
      }
   }
}

static void writef(FILE *f, char *fmt, ...)
{
   va_list v;
   va_start(v, fmt);
   writefv(f,fmt,v);
   va_end(v);
}

static void write_pixels(FILE *f, int rgb_dir, int vdir, int x, int y, int comp, void *data, int write_alpha, int scanline_pad)
{
   uint8_t bg[3] = { 255, 0, 255}, px[3];
   uint32_t zero = 0;
   int i,j,k, j_end;

   if (vdir < 0) 
      j_end = -1, j = y-1;
   else
      j_end =  y, j = 0;

   for (; j != j_end; j += vdir) {
      for (i=0; i < x; ++i) {
         uint8_t *d = (uint8_t *) data + (j*x+i)*comp;
         if (write_alpha < 0)
            fwrite(&d[comp-1], 1, 1, f);
         switch (comp) {
            case 1:
            case 2: writef(f, (char*)"111", d[0],d[0],d[0]);
                    break;
            case 4:
               if (!write_alpha) {
                  for (k=0; k < 3; ++k)
                     px[k] = bg[k] + ((d[k] - bg[k]) * d[3])/255;
                  writef(f, (char*)"111", px[1-rgb_dir],px[1],px[1+rgb_dir]);
                  break;
               }
               /* FALLTHROUGH */
            case 3:
               writef(f,(char*) "111", d[1-rgb_dir],d[1],d[1+rgb_dir]);
               break;
         }
         if (write_alpha > 0)
            fwrite(&d[comp-1], 1, 1, f);
      }
      fwrite(&zero,scanline_pad,1,f);
   }
}

static int outfile(char const *filename, int rgb_dir, int vdir, int x, int y, int comp, void *data, int alpha, int pad, char *fmt, ...)
{
   FILE *f = fopen(filename, "wb");
   if (f) {
      va_list v;
      va_start(v, fmt);
      writefv(f, fmt, v);
      va_end(v);
      write_pixels(f,rgb_dir,vdir,x,y,comp,data,alpha,pad);
      fclose(f);
   }
   return f != 0;
}

int stbi_write_bmp(char const *filename, int x, int y, int comp, void *data)
{
   int pad = (-x*3) & 3;
   return outfile(filename,-1,-1,x,y,comp,data,0,pad,
           (char*)"11 4 22 4" "4 44 22 444444",
           'B', 'M', 14+40+(x*3+pad)*y, 0,0, 14+40,  // file header
            40, x,y, 1,24, 0,0,0,0,0,0);             // bitmap header
}

int stbi_write_tga(char const *filename, int x, int y, int comp, void *data)
{
   int has_alpha = !(comp & 1);
   return outfile(filename, -1,-1, x, y, comp, data, has_alpha, 0,
                  (char*)"111 221 2222 11", 0,0,2, 0,0,0, 0,0,x,y, 24+8*has_alpha, 8*has_alpha);
}

// any other image formats that do interleaved rgb data?
//    PNG: requires adler32,crc32 -- significant amount of code
//    PSD: no, channels output separately
//    TIFF: no, stripwise-interleaved... i think

#endif // STBI_NO_WRITE

#endif // STBI_HEADER_FILE_ONLY

