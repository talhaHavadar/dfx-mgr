/*
 * Copyright (c) 2019, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#include <acapd/accel.h>
#include <acapd/assert.h>
#include <acapd/device.h>
#include <acapd/print.h>
#include <acapd/shell.h>
#include <errno.h>
#include <dirent.h>
#include <ftw.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <jsmn.h>

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

int parseAccelJson(acapd_accel_t *accel, const char *filename)
{
	FILE *fptr;
	long numBytes;
	jsmn_parser parser;
	jsmntok_t token[128];
	int ret,i;
	char *jsonData;
	char *dma_ops;
	acapd_device_t *dma_dev;
	uint32_t buff_size = 0;

	dma_dev = (acapd_device_t *)calloc(1, sizeof(*dma_dev));
	dma_ops = (char *)calloc(64, sizeof(char));
	
	if (dma_dev == NULL) {
		acapd_perror("%s: failed to alloc mem for dma dev.\n", __func__);
		return -EINVAL;
	}


	fptr = fopen(filename, "r");
	if (fptr == NULL)
		return -1;
	fseek(fptr, 0L, SEEK_END);
	numBytes = ftell(fptr);
	fseek(fptr, 0L, SEEK_SET);

	jsonData = (char *)calloc(numBytes, sizeof(char));
	if (jsonData == NULL)
		return -1;
	fread(jsonData, sizeof(char), numBytes, fptr);
	fclose(fptr);
	acapd_praw("jsonData read:\n %s\n",jsonData);

	jsmn_init(&parser);
	ret = jsmn_parse(&parser, jsonData, numBytes, token, sizeof(token)/sizeof(token[0]));
	if (ret < 0){
		acapd_praw("Failed to parse JSON: %d\n", ret);
	}

	for(i=1; i < ret; i++){
		if (token[i].type == JSMN_OBJECT)
			continue;
		if (jsoneq(jsonData, &token[i],"accel_devices") == 0){
			int j;
			int numDevices = token[i+1].size;
			acapd_device_t *devs;
			devs = (acapd_device_t *)calloc(numDevices, sizeof(*devs));
			if (devs == NULL) {
				acapd_perror("%s: Failed to alloc mem for accel devs.\n", __func__);
				ret = -ENOMEM;
				goto error;
			}
			i+=2;

			accel->num_ip_devs = numDevices;
			for(j=0; j < numDevices; j++){
				if (jsoneq(jsonData, &token[i+j+1],"dev_name") == 0)
					devs[j].dev_name = strndup(jsonData+token[i+j+2].start, token[i+j+2].end - token[i+j+2].start);
				if (jsoneq(jsonData, &token[i+j+3],"reg_base") == 0)
					devs[j].reg_pa = (uint64_t)strtoll(strndup(jsonData+token[i+j+4].start, token[i+j+4].end - token[i+j+4].start), NULL, 16);
				if (jsoneq(jsonData, &token[i+j+5],"reg_size") == 0)
					devs[j].reg_size = (size_t)strtoll(strndup(jsonData+token[i+j+6].start, token[i+j+6].end - token[i+j+6].start), NULL, 16);
				i+=6;
			}
			accel->ip_dev = devs;
		}
		if (jsoneq(jsonData, &token[i],"accel_reg_base") == 0){}
		if (jsoneq(jsonData, &token[i],"accel_reg_size") == 0){}
		if (jsoneq(jsonData, &token[i],"sizeInKB")== 0){}
		if (jsoneq(jsonData, &token[i],"sharedMemType") == 0){}
		if (jsoneq(jsonData, &token[i],"dma_dev_name") == 0)
			dma_dev->dev_name = strndup(jsonData+token[i+1].start, token[i+1].end - token[i+1].start);
		if (jsoneq(jsonData, &token[i],"dma_driver") == 0)
			dma_dev->driver = strndup(jsonData+token[i+1].start, token[i+1].end - token[i+1].start);
		if (jsoneq(jsonData, &token[i],"dma_reg_base") == 0){}
		if (jsoneq(jsonData, &token[i],"iommu_group") == 0)
			dma_dev->iommu_group = atoi(strndup(jsonData+token[i+1].start, token[i+1].end - token[i+1].start));
		if (jsoneq(jsonData, &token[i],"max_buf_size") == 0)
			buff_size = atoi(strndup(jsonData+token[i+1].start, token[i+1].end - token[i+1].start));
		if (jsoneq(jsonData, &token[i],"Bus") == 0){}
		if (jsoneq(jsonData, &token[i],"HWType") == 0)
			dma_ops = strndup(jsonData+token[i+1].start, token[i+1].end - token[i+1].start);
		if (jsoneq(jsonData, &token[i],"dataMoverCacheCoherent") == 0){}
		if (jsoneq(jsonData, &token[i],"dataMoverVirtualAddress") == 0){}
		if (jsoneq(jsonData, &token[i],"dataMoverChnls") == 0){
			int j;
			int numChnls = token[i+1].size;
			acapd_chnl_t *chnls;

			chnls = (acapd_chnl_t *)calloc(numChnls, sizeof(*chnls));
			if (chnls == NULL) {
				acapd_perror("%s: failed to alloc mem for chnls.\n", __func__);
				ret = -ENOMEM;
				goto error;
			}

			i+=2;
			for(j=0; j < numChnls; j++){
				if (jsoneq(jsonData, &token[i+j+1],"chnl_id") == 0){
					chnls[j].chnl_id = atoi(strndup(jsonData+token[i+j+2].start, token[i+j+2].end - token[i+j+2].start));
				}
				if (jsoneq(jsonData, &token[i+j+3],"chnl_dir") == 0){
					char *dir = strndup(jsonData+token[i+j+4].start, token[i+j+4].end - token[i+j+4].start);
					if (!strcmp(dir,"ACAPD_DMA_DEV_R"))
						chnls[j].dir = ACAPD_DMA_DEV_R;
					else if (!strcmp(dir,"ACAPD_DMA_DEV_W"))
						chnls[j].dir = ACAPD_DMA_DEV_W;
					else if (!strcmp(dir,"ACAPD_DMA_DEV_RW"))
						chnls[j].dir = ACAPD_DMA_DEV_RW;
				}
				chnls[j].dev = dma_dev;
				chnls[j].max_buf_size = buff_size;
				if (!strcmp(dma_ops,"axidma"))
					chnls[j].ops = &axidma_vfio_dma_ops;
				else if (!strcmp(dma_ops,"mcdma"))
					chnls[j].ops = &mcdma_vfio_dma_ops;
				i+=4;//move token to point to next channel in array
			}
			accel->num_chnls = numChnls;
			accel->chnls = chnls;
		}
		if (jsoneq(jsonData, &token[i],"AccelHandshakeType") == 0){}
		if (jsoneq(jsonData, &token[i],"fallbackBehaviour") == 0){}
	}
	return 0;
error:
	if (dma_dev != NULL) {
		free(dma_dev);
	}
	if (accel->ip_dev != NULL) {
		free(accel->ip_dev);
		accel->ip_dev = NULL;
		accel->num_ip_devs = 0;
	}
	if (accel->chnls != NULL) {
		free(accel->chnls);
		accel->chnls = NULL;
		accel->num_chnls = 0;
	}
	return ret;
}

int parseShellJson(acapd_shell_t *shell, const char *filename)
{
	FILE *fptr;
	long numBytes;
	jsmn_parser parser;
	jsmntok_t token[128];
	int ret,i;
	char *jsonData;
	acapd_device_t *dev;
	acapd_device_t *clk_dev;
	
	printf("Parse shell.json\n");
	acapd_assert(shell != NULL);
	acapd_assert(filename != NULL);
	fptr = fopen(filename, "r");
	if (fptr == NULL)
		return -1;
	fseek(fptr, 0L, SEEK_END);
	numBytes = ftell(fptr);
	fseek(fptr, 0L, SEEK_SET);

	jsonData = (char *)calloc(numBytes, sizeof(char));
	if (jsonData == NULL)
		return -1;
	fread(jsonData, sizeof(char), numBytes, fptr);
	fclose(fptr);
	acapd_debug("jsonData read:\n %s\n",jsonData);

	jsmn_init(&parser);
	ret = jsmn_parse(&parser, jsonData, numBytes, token, sizeof(token)/sizeof(token[0]));
	if (ret < 0){
		acapd_perror("Failed to parse JSON: %d\n", ret);
	}

	dev = &shell->dev;
	clk_dev = &shell->clock_dev;

	for(i=1; i < ret; i++){
		if (token[i].type == JSMN_OBJECT)
			continue;
		if (jsoneq(jsonData, &token[i],"device_name") == 0)
			dev->dev_name = strndup(jsonData+token[i+1].start, token[i+1].end - token[i+1].start);
		if (jsoneq(jsonData, &token[i],"shell_type") == 0) {
		}
		if (jsoneq(jsonData, &token[i],"reg_base")== 0)
			dev->reg_pa = (uint64_t)strtol(strndup(jsonData+token[i+1].start, token[i+1].end - token[i+1].start), NULL, 16);
		if (jsoneq(jsonData, &token[i],"reg_size") == 0){}
			dev->reg_size = (size_t)strtol(strndup(jsonData+token[i+1].start, token[i+1].end - token[i+1].start), NULL, 16);
		if (jsoneq(jsonData, &token[i],"clock_device_name") == 0)
			clk_dev->dev_name = strndup(jsonData+token[i+1].start, token[i+1].end - token[i+1].start);
		if (jsoneq(jsonData, &token[i],"clock_reg_base")== 0)
			clk_dev->reg_pa = (uint64_t)strtol(strndup(jsonData+token[i+1].start, token[i+1].end - token[i+1].start), NULL, 16);
		if (jsoneq(jsonData, &token[i],"clock_reg_size") == 0){}
			clk_dev->reg_size = (size_t)strtol(strndup(jsonData+token[i+1].start, token[i+1].end - token[i+1].start), NULL, 16);
		if (jsoneq(jsonData, &token[i],"isolation_slots") == 0){
			int j;
			if(token[i+1].type != JSMN_ARRAY) {
				acapd_perror("shell.json slots expects an array \n");
				continue;
			}
			int numSlots = token[i+1].size;
			acapd_shell_regs_t *slot_regs;

			slot_regs = (acapd_shell_regs_t *)calloc(numSlots, sizeof(*slot_regs));
			if (slot_regs == NULL) {
				acapd_perror("%s: failed to alloc mem for slot regs.\n", __func__);
				ret = -ENOMEM;
			}
			i+=3;
			for(j=0; j < numSlots; j++){
				if (jsoneq(jsonData, &token[i+j],"offset") == 0){
					int k;
					if(token[i+j+1].type != JSMN_ARRAY) {
						acapd_perror("shell.json offset expects an array \n");
						continue;
					}
					slot_regs[j].offset = (uint32_t *)calloc(token[i+j+1].size, sizeof(uint32_t *));
					for (k = 0; k < token[i+j+1].size; k++){
						slot_regs[j].offset[k] = (uint32_t)strtol(strndup(jsonData+token[i+j+k+2].start, token[i+j+k+2].end - token[i+j+k+2].start), NULL, 16);
					}
					i += token[i+j+1].size;//increment by number of elements in offset array
				}
				if (jsoneq(jsonData, &token[i+j+2],"values") == 0){
					int k;
					if(token[i+j+3].type != JSMN_ARRAY) {
						acapd_perror("shell.json values expects an array \n");
						continue;
					}
					slot_regs[j].values = (uint32_t *)calloc(token[i+j+3].size, sizeof(uint32_t *));
					for (k = 0; k < token[i+j+3].size; k++){
						slot_regs[j].values[k] = (uint32_t)strtol(strndup(jsonData+token[i+j+k+4].start, token[i+j+k+4].end - token[i+j+k+4].start), NULL, 16);
					}
					i += token[i+j+3].size;//increment by number of elements in offset array
				}
				i+=4;//increment to point to next slot
			}
			shell->num_slots = numSlots;
			shell->slot_regs = slot_regs;
			//shell->active_slots = (acapd_accel_t **)calloc(shell->num_slots, sizeof(acapd_accel_t *));
		}
	}
	return 0;
}

