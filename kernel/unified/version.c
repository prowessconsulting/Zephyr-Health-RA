/* version.c */

/*
 * Copyright (c) 1997-2010, 2012-2014 Wind River Systems, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include "version.h" /* generated by MAKE, at compile time */

static uint32_t kernel_version = KERNELVERSION;

/**
 *
 * @brief Return the kernel version of the present build
 *
 * The kernel version is a four-byte value, whose format is described in the
 * file "kernel_version.h".
 *
 * @return kernel version
 */
uint32_t sys_kernel_version_get(void)
{
	return kernel_version;
}
