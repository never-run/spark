#include <spark.h>
#include <cap_adpt.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

struct spk_cap_adpt_ctx {
#define CAPTURE_NAME_SIZE 20
	int8_t  name [ CAPTURE_NAME_SIZE + 1 ] ;
	void   *dev_ctx ;
	struct  spk_cap_adpt_func func ;
} ;

typedef void (*spk_cap_mod_reg_func_t)(void *ctx) ;
typedef char *(*spk_cap_mod_getname_func_t)(void);

/* scandir filter, only care about .so file */
static int so_filter(const struct dirent *de)
{
	if (!strcmp(rindex(de->d_name, '.'), ".so"))
		return 1;
	else
		return 0;
}

static void *get_exact_lib(const char *path, struct dirent **namelist, int nr, const char *mod_name)
{
	void *lib = NULL;
	spk_cap_mod_getname_func_t mod_getname = NULL;
#define MAX_PATH 256
	char absolute_path[MAX_PATH] = {0};
	while (nr--) {
		strncpy(absolute_path, path, MAX_PATH);
		strncat(absolute_path, namelist[nr]->d_name, MAX_PATH);
		lib = dlopen(absolute_path, RTLD_NOW | RTLD_GLOBAL );
		if (lib == NULL) {
			fprintf ( stderr, "dlopen : %s\n", dlerror() ) ;
			continue;
		}
		mod_getname = dlsym(lib, "cap_mod_getname");
		if ( mod_getname == NULL ) {
			fprintf ( stderr, "dlsym cap_mod_getname : %s\n", dlerror() ) ;
			dlclose(lib);
			continue;
		}
		if (!strcmp(mod_getname(), mod_name))
			return lib;
		else
			dlclose(lib);
	}
	return NULL;
}

static inline void free_namelist(struct dirent **namelist, int n)
{
	while (n--)
		free(namelist[n]);
	free(namelist);
}

void *spk_cap_adpt_load (const char *mod_path, const char *mod_name)
{
	int err = 0 ;
	void *lib = NULL ;
	spk_cap_mod_reg_func_t mod_register = NULL ;

	struct spk_cap_adpt_ctx *adpt_ctx = NULL ;

	struct dirent **namelist;
	int nr, n;
	if (mod_path == NULL)
		return NULL;
	n = nr = scandir(mod_path, &namelist, so_filter, NULL);
	if (nr < 0) {
		perror("scandir");
		return NULL;
	} else {
		lib = get_exact_lib(mod_path, namelist, nr, mod_name);
		if (lib == NULL) {
			err = 1;
			goto err;
		}
	}

	mod_register = dlsym( lib, "cap_mod_register" ) ;
	if ( mod_register == NULL ) {
		err = 2 ;
		fprintf ( stderr, "dlsym capture_mod_register : %s\n", dlerror() ) ;
		goto err ;
	}
	
	adpt_ctx = ( struct spk_cap_adpt_ctx * ) malloc ( sizeof ( struct spk_cap_adpt_ctx ) ) ;
	if ( adpt_ctx == NULL ) {
		err = 3 ;
		fprintf ( stderr, "malloc capture_adapter_ctx error\n" ) ;
		goto err ;
	}
	memset ( adpt_ctx, 0, sizeof ( struct spk_cap_adpt_ctx ) ) ;

	mod_register ( (void *)adpt_ctx ) ;
	free_namelist(namelist, n);

	return (void *)adpt_ctx ;
err : 
	switch (err) {
		case 3 : 
		case 2 : 
			if ( lib ) 	dlclose ( lib ) ;
		case 1 : 
			free_namelist(namelist, n);
			break ;
	}
	return NULL ;
}  

void spk_cap_adpt_reg ( void *ctx, struct spk_cap_adpt_func *func_list )
{
	struct spk_cap_adpt_ctx *adpt_ctx = NULL ;

	adpt_ctx = ( struct spk_cap_adpt_ctx * )ctx ;
	adpt_ctx->func.open     = func_list->open ;
	adpt_ctx->func.close    = func_list->close ;
	adpt_ctx->func.get_pkts = func_list->get_pkts ;
}

void *spk_cap_adpt_get_dev ( void *ctx )
{
	struct spk_cap_adpt_ctx *adpt_ctx = NULL ;
	adpt_ctx = ( struct spk_cap_adpt_ctx * )ctx ;
	return adpt_ctx->dev_ctx ;
}

int spk_cap_adpt_open ( void *ctx, char *dev ) 
{
	struct spk_cap_adpt_ctx *adpt_ctx = NULL ;
	adpt_ctx = ( struct spk_cap_adpt_ctx * )ctx ;

	adpt_ctx->dev_ctx = ( void * )adpt_ctx->func.open ( ctx, dev ) ;
	if ( adpt_ctx->dev_ctx == NULL )
		return -1 ;
	return 0 ;
}

void spk_cap_adpt_close ( void *ctx ) 
{
	struct spk_cap_adpt_ctx *adpt_ctx = NULL ;
	adpt_ctx = ( struct spk_cap_adpt_ctx * )ctx ;

	adpt_ctx->func.close ( ctx ) ;

	if ( ctx )
		free ( ctx ) ;
}

int spk_cap_adpt_get_pkts ( void *ctx, struct spark_packet *pkts )
{
	struct spk_cap_adpt_ctx *adpt_ctx = NULL ;

	adpt_ctx = ( struct spk_cap_adpt_ctx * )ctx ;
	return adpt_ctx->func.get_pkts ( ctx, pkts ) ;
}

