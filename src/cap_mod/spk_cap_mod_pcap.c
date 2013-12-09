#include <pcap.h>
#include <spark.h>
#include <cap_adpt.h>

static void *spk_pcap_open ( void *ctx, const char *dev ) ;
static void  spk_pcap_close ( void *ctx ) ;
static int   spk_pcap_get_pkts ( void *ctx, struct spark_packet *pkt ) ;

void cap_mod_register ( void *ctx )
{
	struct spk_cap_adpt_func func_list ;
	func_list.open     = spk_pcap_open ;
	func_list.close    = spk_pcap_close ;
	func_list.get_pkts = spk_pcap_get_pkts ;
	spk_cap_adpt_reg ( ctx, &func_list ) ;
}

/* every module must have this function, return module name */
char *cap_mod_getname(void)
{
	return "PCAP";
}

static void *spk_pcap_open ( void *ctx, const char *dev )
{
	int err = 0 ;
	pcap_t *pcap_ctx = NULL ;
    char    pcap_errbuf[PCAP_ERRBUF_SIZE] ;

	pcap_ctx = pcap_open_live ( dev, BUFSIZ, 1, 1000, pcap_errbuf ) ;
	if ( pcap_ctx == NULL ) {
		fprintf ( stderr, "pcap_open : pcap_open_live : %s\n", pcap_errbuf ) ;
		err = 1 ;
		goto err ;
	}

	return (void *)pcap_ctx ;
err : 
	return (void *)pcap_ctx ;
}

static void spk_pcap_close ( void *ctx )
{
	pcap_t *pcap_ctx = NULL ;
	pcap_ctx = ( pcap_t * )spk_cap_adpt_get_dev ( ctx ) ;

	if ( pcap_ctx )
		pcap_close ( pcap_ctx ) ;
}

static int spk_pcap_get_pkts ( void *ctx, struct spark_packet *pkt )
{
	int ret = 0 ;
	pcap_t *pcap_ctx = NULL ;
	struct pcap_pkthdr *pkt_head = NULL ;
	const u_char *pkt_data = NULL ;

	pcap_ctx = ( pcap_t * )spk_cap_adpt_get_dev ( ctx ) ;

	ret = pcap_next_ex ( pcap_ctx, &pkt_head, &pkt_data ) ;

	pkt->len        = pkt_head->len ;
	pkt->caplen     = pkt_head->caplen ;
	pkt->ts.tv_sec  = pkt_head->ts.tv_sec ;
	pkt->ts.tv_usec = pkt_head->ts.tv_usec ;

	pkt->data       = (uint8_t *)pkt_data ;

	return ret ;
}

