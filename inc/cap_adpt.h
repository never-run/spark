#ifndef __CAP_ADPT_H__
#define __CAP_ADPT_H__

#include <stdint.h>

struct spark_packet
{
	uint32_t        len ;
	uint32_t        caplen ;
	struct timeval  ts ;
	uint8_t        *data ;
} ;

typedef void *(*cap_adpt_open_t)(void *ctx, const char *dev);
typedef void  (*cap_adpt_close_t)(void *ctx);
typedef int   (*cap_adpt_get_pkts_t)(void *ctx, struct spark_packet *pkt);

struct spk_cap_adpt_func
{
	cap_adpt_open_t     open ;
	cap_adpt_close_t    close ;
	cap_adpt_get_pkts_t get_pkts ;
} ;

void *spk_cap_adpt_load (const char *mod_path, const char *mod_name) ;
void  spk_cap_adpt_reg ( void *ctx, struct spk_cap_adpt_func *func_list ) ;

void *spk_cap_adpt_get_dev ( void *ctx ) ;

int   spk_cap_adpt_open ( void *ctx, char *dev ) ;
void  spk_cap_adpt_close ( void *ctx ) ;
int   spk_cap_adpt_get_pkts ( void *ctx, struct spark_packet *pkt ) ;

#endif

