/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                          License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#ifndef OPENCV_VIDEOIO_HPP
#define OPENCV_VIDEOIO_HPP

#include "opencv2/core/core.hpp"

/**
  @defgroup videoio Video I/O

  @brief Read and write video or images sequence with OpenCV

  ### See also:
  - @ref videoio_overview
  - Tutorials: @ref tutorial_table_of_content_videoio
  @{
    @defgroup videoio_flags_base Flags for video I/O
    @defgroup videoio_flags_others Additional flags for video I/O API backends
    @defgroup videoio_c C API for video I/O
    @defgroup videoio_ios iOS glue for video I/O
    @defgroup videoio_winrt WinRT glue for video I/O
    @defgroup videoio_registry Query I/O API backends registry
  @}
*/

////////////////////////////////// video io /////////////////////////////////

//typedef struct CvCapture CvCapture;
//typedef struct CvVideoWriter CvVideoWriter;

namespace cv
{

enum VideoCaptureProperties {
       CAP_PROP_POS_MSEC       =0, //!< Current position of the video file in milliseconds.
       CAP_PROP_POS_FRAMES     =1, //!< 0-based index of the frame to be decoded/captured next.
       CAP_PROP_POS_AVI_RATIO  =2, //!< Relative position of the video file: 0=start of the film, 1=end of the film.
       CAP_PROP_FRAME_WIDTH    =3, //!< Width of the frames in the video stream.
       CAP_PROP_FRAME_HEIGHT   =4, //!< Height of the frames in the video stream.
       CAP_PROP_FPS            =5, //!< Frame rate.
       CAP_PROP_FOURCC         =6, //!< 4-character code of codec. see VideoWriter::fourcc .
       CAP_PROP_FRAME_COUNT    =7, //!< Number of frames in the video file.
       CAP_PROP_FORMAT         =8, //!< Format of the %Mat objects (see Mat::type()) returned by VideoCapture::retrieve().
                                   //!< Set value -1 to fetch undecoded RAW video streams (as Mat 8UC1).
       CAP_PROP_MODE           =9, //!< Backend-specific value indicating the current capture mode.
       CAP_PROP_BRIGHTNESS    =10, //!< Brightness of the image (only for those cameras that support).
       CAP_PROP_CONTRAST      =11, //!< Contrast of the image (only for cameras).
       CAP_PROP_SATURATION    =12, //!< Saturation of the image (only for cameras).
       CAP_PROP_HUE           =13, //!< Hue of the image (only for cameras).
       CAP_PROP_GAIN          =14, //!< Gain of the image (only for those cameras that support).
       CAP_PROP_EXPOSURE      =15, //!< Exposure (only for those cameras that support).
       CAP_PROP_CONVERT_RGB   =16, //!< Boolean flags indicating whether images should be converted to RGB. <br/>
                                   //!< *GStreamer note*: The flag is ignored in case if custom pipeline is used. It's user responsibility to interpret pipeline output.
       CAP_PROP_WHITE_BALANCE_BLUE_U =17, //!< Currently unsupported.
       CAP_PROP_RECTIFICATION =18, //!< Rectification flag for stereo cameras (note: only supported by DC1394 v 2.x backend currently).
       CAP_PROP_MONOCHROME    =19,
       CAP_PROP_SHARPNESS     =20,
       CAP_PROP_AUTO_EXPOSURE =21, //!< DC1394: exposure control done by camera, user can adjust reference level using this feature.
       CAP_PROP_GAMMA         =22,
       CAP_PROP_TEMPERATURE   =23,
       CAP_PROP_TRIGGER       =24,
       CAP_PROP_TRIGGER_DELAY =25,
       CAP_PROP_WHITE_BALANCE_RED_V =26,
       CAP_PROP_ZOOM          =27,
       CAP_PROP_FOCUS         =28,
       CAP_PROP_GUID          =29,
       CAP_PROP_ISO_SPEED     =30,
       CAP_PROP_BACKLIGHT     =32,
       CAP_PROP_PAN           =33,
       CAP_PROP_TILT          =34,
       CAP_PROP_ROLL          =35,
       CAP_PROP_IRIS          =36,
       CAP_PROP_SETTINGS      =37, //!< Pop up video/camera filter dialog (note: only supported by DSHOW backend currently. The property value is ignored)
       CAP_PROP_BUFFERSIZE    =38,
       CAP_PROP_AUTOFOCUS     =39,
       CAP_PROP_SAR_NUM       =40, //!< Sample aspect ratio: num/den (num)
       CAP_PROP_SAR_DEN       =41, //!< Sample aspect ratio: num/den (den)
       CAP_PROP_BACKEND       =42, //!< Current backend (enum VideoCaptureAPIs). Read-only property
       CAP_PROP_CHANNEL       =43, //!< Video input or Channel Number (only for those cameras that support)
       CAP_PROP_AUTO_WB       =44, //!< enable/ disable auto white-balance
       CAP_PROP_WB_TEMPERATURE=45, //!< white-balance color temperature
       CAP_PROP_CODEC_PIXEL_FORMAT =46,    //!< (read-only) codec's pixel format. 4-character code - see VideoWriter::fourcc . Subset of [AV_PIX_FMT_*](https://github.com/FFmpeg/FFmpeg/blob/master/libavcodec/raw.c) or -1 if unknown
       CAP_PROP_BITRATE       =47, //!< (read-only) Video bitrate in kbits/s
#ifndef CV_DOXYGEN
       CV__CAP_PROP_LATEST
#endif
     };


} // cv

#endif //OPENCV_VIDEOIO_HPP
