/*******************************************************************************
 *
 * TNeoKernel: real-time kernel initially based on TNKernel
 *
 *    TNKernel:                  copyright © 2004, 2013 Yuri Tiomkin.
 *    PIC32-specific routines:   copyright © 2013, 2014 Anders Montonen.
 *    TNeoKernel:                copyright © 2014       Dmitry Frank.
 *
 *    TNeoKernel was born as a thorough review and re-implementation of
 *    TNKernel. The new kernel has well-formed code, inherited bugs are fixed
 *    as well as new features being added, and it is tested carefully with
 *    unit-tests.
 *
 *    API is changed somewhat, so it's not 100% compatible with TNKernel,
 *    hence the new name: TNeoKernel.
 *
 *    Permission to use, copy, modify, and distribute this software in source
 *    and binary forms and its documentation for any purpose and without fee
 *    is hereby granted, provided that the above copyright notice appear
 *    in all copies and that both that copyright notice and this permission
 *    notice appear in supporting documentation.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE DMITRY FRANK AND CONTRIBUTORS "AS IS"
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DMITRY FRANK OR CONTRIBUTORS BE
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *    THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

/**
 * \file
 *
 * Dispatch configuration: set predefined options, include user-provided cfg
 * file as well as default cfg file.
 *
 */

#ifndef _TN_CFG_DISPATCH_H
#define _TN_CFG_DISPATCH_H


//-- Configuration constants
/**
 * In this case, you should use macro like this: `#TN_MAKE_ALIG(struct my_struct)`.
 * This way is used in the majority of TNKernel ports. (actually, in all ports
 * except the one by AlexB)
 */
#define  TN_API_MAKE_ALIG_ARG__TYPE       1

/**
 * In this case, you should use macro like this: `#TN_MAKE_ALIG(sizeof(struct
 * my_struct))`. This way is stated in TNKernel docs and used in the port for
 * dsPIC/PIC24/PIC32 by AlexB.
 */
#define  TN_API_MAKE_ALIG_ARG__SIZE       2


//--- As a starting point, you might want to copy tn_cfg_default.h -> tn_cfg.h,
//    and then edit it if you want to change default configuration.
//    NOTE: the file tn_cfg.h is specified in .hgignore file, in order to not include
//    project-specific configuration in the common TNKernel repository.
#include "tn_cfg.h"

//--- default cfg file is included too, so that you are free to not set
//    all available options in your tn_cfg.h file.
#include "tn_cfg_default.h"


#if defined (__XC16__)
#  if TN_P24_SYS_IPL >= 7
#     error TN_P24_SYS_IPL must be less than 7
#  endif
#  if TN_P24_SYS_IPL <= 0
#     error TN_P24_SYS_IPL must be more than 0
#  endif
#endif

#endif // _TN_CFG_DISPATCH_H

