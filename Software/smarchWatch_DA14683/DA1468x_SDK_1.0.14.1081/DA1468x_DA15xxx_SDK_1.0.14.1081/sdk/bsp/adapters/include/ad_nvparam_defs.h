/*
 * DO NOT MODIFY THIS FILE!!!
 *
 * NV-Parameters configuration shall be done in platform_nvparam.h!!!
 *
 */

#ifndef AD_NVPARAM_DEFS_H_
#define AD_NVPARAM_DEFS_H_

#ifndef IN_AD_NVPARAM_C

/*
 * If this is included outside of ad_nvparam.c, we just define empty macros so nothing but tag
 * definitions are created from platform_nvparam.h
 */

#define NVPARAM_AREA(NAME, PARTITION, OFFSET)
#define NVPARAM_PARAM(TAG, OFFSET, LENGTH)
#define NVPARAM_VARPARAM(TAG, OFFSET, LENGTH)
#define NVPARAM_AREA_END()

#else

/*
 * If this is included inside of ad_nvparam.c, we will create proper configuration structure.
 */

/*
 * First we create configurations of each area - this will create "area_XXX" symbol for each defined
 * area in platform_nvparam.h. It contains all parameters defined for given area.
 */

#define NVPARAM_AREA(NAME, PARTITION, OFFSET) \
        static const parameter_t area_ ## NAME[] = {
#define NVPARAM_PARAM(TAG, OFFSET, LENGTH) \
                { \
                        .tag = TAG, \
                        .offset = OFFSET, \
                        .length = LENGTH, \
                },
#define NVPARAM_VARPARAM(TAG, OFFSET, LENGTH) \
                { \
                        .tag = TAG, \
                        .flags = FLAG_VARIABLE_LEN, \
                        .offset = OFFSET, \
                        .length = LENGTH, \
                },
#define NVPARAM_AREA_END() \
        };

#undef PLATFORM_NVPARAM_H_
#include <platform_nvparam.h>

/*
 * Next, using the same platform_nvparam.h, we define list of areas. Each has proper attributes set
 * and pointer to corresponding area structure.
 */

#undef NVPARAM_AREA
#undef NVPARAM_PARAM
#undef NVPARAM_VARPARAM
#undef NVPARAM_AREA_END
#define NVPARAM_AREA(NAME, PARTITION, OFFSET) \
        { \
                .name = #NAME, \
                .partition = PARTITION, \
                .offset = OFFSET, \
                .parameters = area_ ## NAME, \
                .num_parameters = sizeof(area_ ## NAME) / sizeof(area_ ## NAME[0]), \
        },
#define NVPARAM_PARAM(TAG, OFFSET, LENGTH)
#define NVPARAM_VARPARAM(TAG, OFFSET, LENGTH)
#define NVPARAM_AREA_END()

static const area_t areas[] = {
#undef PLATFORM_NVPARAM_H_
#include <platform_nvparam.h>
};

#define num_areas (sizeof(areas) / sizeof(areas[0]))

#endif /* IN_AD_NVPARAM_C */

#endif /* AD_NVPARAM_DEFS_H_ */
