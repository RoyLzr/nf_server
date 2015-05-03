#ifndef _NFSERVER_REGISTER_H
#define _NFSERVER_REGISTER_H

#include "nf_server.h"
#include "nf_server_core.h"

//注册内容
extern int lfpool_init(nf_server_t *);
extern int lfpool_run(nf_server_t *);
extern int lfpool_join(nf_server_t *);
extern int lfpool_destroy(nf_server_t *);
extern long long lfpool_get_socknum(nf_server_t *);
extern long long lfpool_get_queuenum(nf_server_t *);
extern int lfpool_pause(nf_server_t *);
extern int lfpool_resume(nf_server_t *);

extern int sapool_init(nf_server_t *);
extern int sapool_run(nf_server_t *);
extern int sapool_listen(nf_server_t *);
extern int sapool_join(nf_server_t *);
extern int sapool_destroy(nf_server_t *);
extern long long sapool_get_socknum(nf_server_t *);
extern long long sapool_get_queuenum(nf_server_t *);
extern int sapool_pause(nf_server_t *);
extern int sapool_resume(nf_server_t *);


//注册表内容格式
typedef int (*init_pool)(nf_server_t *);
typedef int (*run_pool)(nf_server_t *);
typedef int (*listen_pool)(nf_server_t *);
typedef int (*join_pool)(nf_server_t *);
typedef int (*destroy_pool)(nf_server_t *);
typedef long long (*get_pool_socknum)(nf_server_t *);
typedef long long (*get_pool_queuenum)(nf_server_t *);
typedef int (*pause_pool)(nf_server_t *);
typedef int (*resume_pool)(nf_server_t *);

//注册表
struct _pool_t
{
    init_pool init;	//pool的初始化回调函数
    run_pool run;	//pool的运行回调函数
    listen_pool listen;  // 开始监听
    join_pool join;	//pool的join函数
    destroy_pool destroy;
    get_pool_socknum get_socknum;
    get_pool_queuenum get_queuenum;
    pause_pool pause;
    resume_pool resume;
};

//开始注册
static const struct _pool_t g_pool[] = {
	{
		lfpool_init,
		lfpool_run,
		NULL,
		lfpool_join,
		lfpool_destroy,
		lfpool_get_socknum,
		lfpool_get_queuenum,
		lfpool_pause,
		lfpool_resume
	},
	{
		sapool_init,
		sapool_run,
		sapool_listen,
		sapool_join,
		sapool_destroy,
		sapool_get_socknum,
		sapool_get_queuenum,
		sapool_pause,
		sapool_resume
	},
};

#endif
