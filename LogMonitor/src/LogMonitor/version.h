//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
//

#ifndef _VERSION_H_
#define _VERSION_H_

#define LM_MAJORNUMBER          2
#define LM_MINORNUMBER          0
#define LM_BUILDNUMBER          0
#ifndef LM_BUILDMINORVERSION
// removed in support of semantic versioning - https://semver.org
// major.minor.patch
// #define LM_BUILDMINORVERSION    0
#endif

#endif
