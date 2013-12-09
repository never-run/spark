#include <getopt.h>
#include <spark.h>
#include <cap_adpt.h>

struct options {
	char *device ;
} ;

static const char *opt_str = "i:h?";

static const struct option opt_long [] = {
	{ "iface", required_argument, NULL, 'i' },
	{ "help" , no_argument,       NULL, 'h' },
	{  NULL  , no_argument,       NULL, 0 }
};

struct options g_options ;

void usage ( void )
{
	printf ( "spark_demo -i <interface>\n" ) ;
}

int get_options ( int argc, char *argv[], struct options *options )
{
	int ret = -1 ;
	int opt = 0 ;
	int opt_index ;

	opt = getopt_long( argc, argv, opt_str, opt_long, &opt_index );
	while( opt != -1 ) {
		switch( opt ) {
			case 'i':
				options->device = optarg ;
				ret = 1 ;
				break;
			case 'h':
			case '?':
				usage();
				ret = 0 ;
				break;
			default:
				break;
		}
		opt = getopt_long( argc, argv, opt_str, opt_long, &opt_index );
	}

	return ret ;
}

int main ( int argc, char *argv[] )
{

	int ret = 0 ;
	void *cap_adpt_ctx = NULL ;
	struct spark_packet pkt ;

	ret = get_options ( argc, argv, &g_options ) ;
	if ( ret == 0 )
		goto main_err ;
	if ( ret < 0 ) {
		usage () ;
		goto main_err ;
	}

/* we can get these configures in the configure file later */
#define MOD_PATH "../mod/"
#define MOD_NAME "PCAP"
	cap_adpt_ctx = spk_cap_adpt_load (MOD_PATH, MOD_NAME) ;
	if (cap_adpt_ctx == NULL) {
		fprintf(stderr, "get adapter context failed\n");
		goto main_err;
	}
	spk_cap_adpt_open ( cap_adpt_ctx, g_options.device ) ;

	while (1) {
		ret = spk_cap_adpt_get_pkts ( cap_adpt_ctx, &pkt ) ;

		if ( ret == 1 ) {
			printf ( "[len : %5d][cap len : %5d][time : %10lu : %10lu]\n", 
					pkt.len, 
					pkt.caplen, 
					pkt.ts.tv_sec, 
					pkt.ts.tv_usec) ;
		} else if ( ret ==  0 ) {
			printf ( "timeout\n" ) ;
		} else if ( ret == -1 ) {
			fprintf ( stderr, "error\n" ) ;
		} else if ( ret == -2 ) {
			fprintf ( stderr, "error\n" ) ;
		}
	}

	spk_cap_adpt_close ( cap_adpt_ctx ) ;
main_err : 
	return 0 ;
}

