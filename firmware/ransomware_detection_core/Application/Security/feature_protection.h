/**
 * @file    feature_protection.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef APPLICATION_SECURITY_FEATURE_PROTECTION_H_
#define APPLICATION_SECURITY_FEATURE_PROTECTION_H_

#include "dsp.h"

void FS_LowLevelProtect(DSP_Features_t *features);
void FS_DebugRestore(DSP_Features_t *features);

#endif

