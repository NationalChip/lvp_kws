#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "../include/autoconf.h"

#define BOOT_HEADER_SIZE 40

/* 40字节长boot文件头 */
struct boot_header {
	uint16_t chip_id;
	uint8_t  chip_type;
	uint8_t  chip_version;

	uint16_t boot_delay;
	uint8_t  baudrate;
	uint8_t  reserved;

	uint32_t stage1_size;
	uint32_t stage1_baud_rate;
	uint32_t stage2_baud_rate;
	uint32_t stage2_size;
	uint32_t stage2_checksum;
	uint32_t npu_offset;
	uint32_t npu_cmd_weight_size;
	uint32_t npu_data_size;
};

static int32_t is_little_endian(void)
{
	uint32_t a = 0x11223344;
	uint8_t *p = (unsigned char *)&a;

	return (*p == 0x44) ? 1 : 0;
}

static uint32_t switch_endian(uint32_t v)
{
	return (((v >> 0) & 0xff) << 24) | (((v >> 8) & 0xff) << 16) | (((v >> 16) & 0xff) << 8) | (((v >> 24) & 0xff) << 0);
}

static uint32_t be32_to_cpu(uint32_t v)
{
	if (is_little_endian())
		return switch_endian(v);
	else
		return v;
}

static uint32_t cpu_to_be32(uint32_t v)
{
	if (is_little_endian())
		return switch_endian(v);
	else
		return v;
}

#define CMD_MODIFY_STAGE2 0xffff0001

static int32_t modify_stage2(const char *filename, int stage2_size, int npu_cmd_weight_size, int npu_offset, int npu_data_size)
{
	FILE *fp;
	uint8_t header[40];
	struct boot_header *pheader = (struct boot_header *)header;
	uint32_t stage1_size, checksum;
	struct stat statbuf;
	unsigned char *stage2;
	int32_t i;

	stat(filename, &statbuf);
	//boot_size = statbuf.st_size;
	printf("stage2_size =%d", stage2_size);
	if (stage2_size <= 0) {
		printf("invalid file size, %s!\n", filename);
		return -1;
	}

	fp = fopen(filename, "rb+");
	if (!fp) {
		printf("open file %s error\n", filename);
	}

	if (fread(header, BOOT_HEADER_SIZE, 1, fp) != 1) {
		printf("Read boot header error!\n");
		fclose(fp);
		return -1;
	}

	pheader->stage1_size = cpu_to_be32(0x3000);
	pheader->stage1_baud_rate = cpu_to_be32(2000000);
	pheader->stage2_baud_rate = cpu_to_be32(1500000);

	stage1_size = be32_to_cpu(pheader->stage1_size);
	stage2_size = npu_offset - stage1_size;
	if(npu_cmd_weight_size)
	{
		pheader->npu_offset = cpu_to_be32(npu_offset + BOOT_HEADER_SIZE);
		pheader->npu_cmd_weight_size = cpu_to_be32(npu_cmd_weight_size);
		pheader->npu_data_size = cpu_to_be32(npu_data_size);
	}
	else
	{
		pheader->npu_offset = cpu_to_be32(0);
		pheader->npu_cmd_weight_size = cpu_to_be32(0);
		pheader->npu_data_size = cpu_to_be32(0);
	}

	stage2 = malloc(stage2_size);
	if (!stage2) {
		printf ("malloc failed, size : %d\n", stage2_size);
		fclose(fp);
		return -1;
	}

	fseek(fp, BOOT_HEADER_SIZE + stage1_size, SEEK_SET);

    // printf("pos: %ld\n", ftell(fp));
	if (fread(stage2, stage2_size, 1, fp) != 1) {
		printf("Read stage2 error!\n");
		free(stage2);
		fclose(fp);
		return -1;
	}

	checksum = 0;
	for (i = 0; i < stage2_size; i++)
		checksum += stage2[i];

	// change to big endian
	pheader->stage2_size     = cpu_to_be32(stage2_size);
	pheader->stage2_checksum = cpu_to_be32(checksum);

	rewind(fp);

	// printf("Wirte heSes\n", BOOT_HEADER_SIZE);
	if (fwrite(header, BOOT_HEADER_SIZE, 1, fp) != 1) {
		printf("Wirte boot header error!\n");
		free(stage2);
		fclose(fp);
		return -1;
	}

	free(stage2);
	fclose(fp);
	return 0;
}
int main(int argc, char *argv[])
{

	FILE *fp;
	char StrLine[1024];             //每行最大读取的字符数
	int i = 0;
	int stage2_text_size, stage2_data_size, stage2_total_size, npu_cmd_weight_size = 0;
	unsigned char data[15] = {0};
	int npu_offset, npu_size_elf_offset ,npu_total_size = 0, npu_data_size;

	if((fp = fopen("./stage2_size.txt","r")) == NULL) //判断文件是否存在及可读
	{
	  printf("open stage2_size.txt error!");
	  return -1;
	}
	while (!feof(fp))
	{
	  char *p;
	  p = fgets(StrLine,1024,fp);  //读取一行
	  if(p)
	  {
	  	if(i == 0)
	  	{
	  		memcpy(data, p, 8);
	  		sscanf(data,"%x", &stage2_text_size);
	  		i = 1;
	  		continue;
	  	}
	  	if(i == 1)
	  	{
	  		memcpy(data, p, 8);
	  		sscanf(data,"%x", &stage2_data_size);
	  		break;
	  	}
	  }
	}
	fclose(fp);                     //关闭文件
	memset(data, 0, 15);
	stage2_total_size = stage2_text_size + stage2_data_size;
#ifndef CONFIG_NPU_RUN_IN_FLASH
#if defined(CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION) || defined(CONFIG_LVP_ENABLE_NN_DENOISE)
	i = 0;
	int start_npu_addr, end_npu_addr;
	if((fp = fopen("./npu_cmd_weight_size.txt","r")) == NULL) //判断文件是否存在及可读
	{
	  printf("open npu_cmd_weight_size.txt error!");
	  return -1;
	}
	while (!feof(fp))
	{
	  char *p;
	  p = fgets(StrLine,1024,fp);  //读取一行
	  if(p)
	  {
	  	if(i == 0)
	  	{
	  		memcpy(data, p, 10);
	  		sscanf(data,"%x", &start_npu_addr);
	  		i = 1;
	  		continue;
	  	}
	  	if(i == 1)
	  	{
	  		memcpy(data, p, 10);
	  		sscanf(data,"%x", &end_npu_addr);
	  		break;
	  	}
	  }
	}
	fclose(fp);
	npu_cmd_weight_size = end_npu_addr - start_npu_addr;


	npu_total_size = CONFIG_NPU_TOTAL_SIZE;
	#endif
#endif
	memset(data, 0, 15);
	if((fp = fopen("./npu_offset.txt","r")) == NULL) //判断文件是否存在及可读
	{
	  printf("open npu_offset.txt error!");
	  return -1;
	}
	while (!feof(fp))
	{
		char *p;
		p = fgets(StrLine,1024,fp);  //读取一行
		if(p)
		{
			memcpy(data, p, 10);
			sscanf(data,"%x", &npu_offset);
			break;
		}
	}
	fclose(fp);
	// memset(data, 0, 15);
	// if((fp = fopen("./npu_total_size.txt","r")) == NULL) //判断文件是否存在及可读
	// {
	//   printf("open npu_total_size.txt error!");
	//   return -1;
	// }
	// while (!feof(fp))
	// {
	// 	char *p;
	// 	p = fgets(StrLine,1024,fp);  //读取一行
	// 	if(p)
	// 	{
	// 		memcpy(data, p, 8);
	// 		sscanf(data,"%x", &npu_size_elf_offset);
	// 		printf("npu_size_elf_offset =0x%x\n", npu_size_elf_offset);
	// 		break;
	// 	}
	// }
	// fclose(fp);


	// memset(data, 0, 15);
	// if((fp = fopen("./output/mcu.elf","r")) == NULL) //判断文件是否存在及可读
	// {
	//   printf("open ouput/mcu.elf error!");
	//   return -1;
	// }

	// fseek(fp, npu_size_elf_offset, SEEK_SET);
	// fread(&npu_total_size, 4, 1,fp);
	// fclose(fp);

	npu_data_size = npu_total_size - npu_cmd_weight_size +4; //kws_tmp_content 的4个字节没有算进npu_total_size中
	npu_data_size = (npu_data_size+3)/4*4;
	printf("stage2_total_size=%d npu_cmd_weight_size=%d npu_offset=0x%x npu_data_size=%d\n", 
		stage2_total_size, npu_cmd_weight_size, npu_offset +BOOT_HEADER_SIZE, npu_data_size);

	modify_stage2(argv[1], stage2_total_size, npu_cmd_weight_size, npu_offset, npu_data_size);
	return 0;
}
