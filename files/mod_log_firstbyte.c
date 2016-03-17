/*
 * https://code.google.com/p/mod-log-firstbyte/source/browse/trunk/mod_log_firstbyte.c
 *
 * Modified to provide the data in ms which is much more readable microseconds don't provide any value /FS
 *
 * mod_log_firstbyte.c
 * Version: 1.01
 *
 * Copyright (C) 2004, Matthew Lloyd (http://matthewlloyd.net)
 *
 *
 * The argument to LogFormat and CustomLog is a string, which can include
 * literal characters copied into the log files, and '%' directives as
 * follows:
 *
 * %...F:  time between request being read and the first byte being served
 *
 */

#include "apr_strings.h"
#include "apr_lib.h"
#include "apr_hash.h"
#include "apr_optional.h"

#define APR_WANT_STRFUNC
#include "apr_want.h"

#include "ap_config.h"
#include "mod_log_config.h"
#include "httpd.h"
#include "http_core.h"
#include "http_config.h"
#include "http_connection.h"
#include "http_protocol.h"

module AP_MODULE_DECLARE_DATA log_firstbyte_module;

static const char firstbyte_filter_name[] = "LOG_FIRSTBYTE";

/*
 Per-connection data:
 */

typedef struct firstbyte_config_t {
    apr_time_t req_read_time;
    apr_time_t first_out_time;
    int first_out;
} firstbyte_config_t;

/*
 CustomLog items:
 */

static const char *log_firstbyte_time(request_rec *r, char *a)
{
    firstbyte_config_t *cf = ap_get_module_config(r->connection->conn_config,
                                              &log_firstbyte_module);

    return apr_psprintf(r->pool, "%" APR_TIME_T_FMT,
		((cf->first_out_time - cf->req_read_time) / 1000));
}

// Called when Apache outputs data:
static apr_status_t firstbyte_out_filter(ap_filter_t *f,
                                     apr_bucket_brigade *bb) {
    firstbyte_config_t *cf = ap_get_module_config(f->c->conn_config, &log_firstbyte_module);

	apr_bucket *b = APR_BRIGADE_LAST(bb);
    /* End of data, make sure we flush */
    if (APR_BUCKET_IS_EOS(b)) {
        APR_BRIGADE_INSERT_TAIL(bb,
                                apr_bucket_flush_create(f->c->bucket_alloc));
        APR_BUCKET_REMOVE(b);
        apr_bucket_destroy(b);
    }

	if (cf->first_out==1) {
		cf->first_out_time = apr_time_now();
		cf->first_out = 0;
	}

    return ap_pass_brigade(f->next, bb);
}

/*
 Hook to Apache2
 */

static int firstbyte_pre_conn(conn_rec *c, void *csd) {
    firstbyte_config_t *cf = apr_pcalloc(c->pool, sizeof(*cf));

    ap_set_module_config(c->conn_config, &log_firstbyte_module, cf);
    ap_add_output_filter(firstbyte_filter_name, NULL, NULL, c);

    return OK;
}

static int firstbyte_pre_config(apr_pool_t *p, apr_pool_t *plog, apr_pool_t *ptemp)
{
    static APR_OPTIONAL_FN_TYPE(ap_register_log_handler) *log_pfn_register;

    log_pfn_register = APR_RETRIEVE_OPTIONAL_FN(ap_register_log_handler);

    if (log_pfn_register) {
        log_pfn_register(p, "F", log_firstbyte_time, 0);
    }

    return OK;
}

static int firstbyte_post_req(request_rec *r) {
    firstbyte_config_t *cf = ap_get_module_config(r->connection->conn_config, &log_firstbyte_module);

    cf->req_read_time = apr_time_now();
    cf->first_out = 1;

    return OK;
}

static void register_hooks(apr_pool_t *p)
{
    static const char *pre[] = { "mod_log_config.c", NULL };

    ap_hook_pre_connection(firstbyte_pre_conn, NULL, NULL, APR_HOOK_MIDDLE);
    ap_hook_pre_config(firstbyte_pre_config, NULL, NULL, APR_HOOK_REALLY_FIRST);
    ap_hook_post_read_request(firstbyte_post_req, NULL, NULL, APR_HOOK_LAST);

    ap_register_output_filter(firstbyte_filter_name, firstbyte_out_filter, NULL,
								AP_FTYPE_NETWORK - 1);
}

module AP_MODULE_DECLARE_DATA log_firstbyte_module =
{
    STANDARD20_MODULE_STUFF,
    NULL,                       /* create per-dir config */
    NULL,                       /* merge per-dir config */
    NULL,                       /* server config */
    NULL,                       /* merge server config */
    NULL,                       /* command apr_table_t */
    register_hooks              /* register hooks */
};