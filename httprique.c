#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <event.h>
#include <evhttp.h>
#include <signal.h>
#include <getopt.h>
#include "pqnode.h"
#include "priorque.h"

static struct priq_base *base;

/* 网络初始化 */
static int net_init()
{
#ifdef _WIN32
    WSADATA wsaData;
    DWORD Ret;
    if ((Ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0 ) {
        return -1;
    }
#endif
    return 0;
}

/* 处理模块 */
void priorq_handler(struct evhttp_request *req, void *arg)
{
	struct evbuffer *buf;
	char *decode_uri;
	char *client_ip;
	unsigned short uport;
	struct evkeyvalq priorq_http_query;
	const char *priorq_input_name = NULL;
	const char *priorq_input_charset = NULL;
	const char *priorq_input_opt = NULL;
	const char *priorq_input_data = NULL;
	const char *priorq_input_pos_tmp = NULL;
	const char *priorq_input_num_tmp = NULL;
	const char *priorq_input_prior_tmp = NULL;
	const char *priorq_input_seq_tmp = NULL;

	int priorq_input_pos = 0;
	int priorq_input_prior = 0;
	int priorq_input_seq = 0;
	int priorq_input_num = 0;

	struct evhttp_connection* http_connect = NULL;
	buf = evbuffer_new();
	/* 获取客户端信息 */
	http_connect = evhttp_request_get_connection(req);
	/* 分析URL参数 */
	decode_uri = strdup((char*) evhttp_request_uri(req));
	
	evhttp_parse_query(decode_uri, &priorq_http_query);
	free(decode_uri);
	/* 接收GET表单参数 */
	priorq_input_name = evhttp_find_header (&priorq_http_query, "name"); /* 队列名称 */
	priorq_input_charset = evhttp_find_header (&priorq_http_query, "charset"); /* 操作类别 */
	priorq_input_opt = evhttp_find_header (&priorq_http_query, "opt"); /* 操作类别 */
	priorq_input_data = evhttp_find_header (&priorq_http_query, "data"); /* 操作类别 */
	priorq_input_pos_tmp = evhttp_find_header (&priorq_http_query, "pos"); /* 队列位置点 字符型 */
	priorq_input_num_tmp = evhttp_find_header (&priorq_http_query, "num"); /* 队列总长度 字符型 */
	priorq_input_prior_tmp = evhttp_find_header(&priorq_http_query, "prior");  /* 队列优先级 */
	priorq_input_seq_tmp = evhttp_find_header(&priorq_http_query, "seq");

	if (priorq_input_pos_tmp != NULL) {
		priorq_input_pos = atoi(priorq_input_pos_tmp); /* 队列位置点 */
	}
	if (priorq_input_prior_tmp != NULL) {
		priorq_input_prior = atoi(priorq_input_prior_tmp); /* 队列优先级 */
	}
	if (priorq_input_seq_tmp != NULL) {
		priorq_input_seq = atoi(priorq_input_seq_tmp); /* 队列序号 */
	}
	if (priorq_input_num_tmp != NULL) {
		priorq_input_num = atoi(priorq_input_num_tmp); /* 队列总长度 */
	}		

	/* 返回给用户的Header头信息 */
	if (priorq_input_charset != NULL && strlen(priorq_input_charset) <= 40) {
		char *content_type = (char *)malloc(64);
		memset(content_type, '\0', 64);
		sprintf(content_type, "text/plain; charset=%s", priorq_input_charset);
		evhttp_add_header(req->output_headers, "Content-Type", content_type);
		free(content_type);
	} else {
		evhttp_add_header(req->output_headers, "Content-Type", "text/plain");
	}
	evhttp_add_header(req->output_headers, "Connection", "keep-alive");
	evhttp_add_header(req->output_headers, "Cache-Control", "no-cache");

	/* 参数是否存在判断 */
	if (priorq_input_name != NULL && priorq_input_opt != NULL && strlen(priorq_input_name) <= MAX_QNAME_LEN) {
		/* 入队列 */
		if (strcmp(priorq_input_opt, "put") == 0) {
			if (priorq_input_data != NULL && strlen(priorq_input_data) < MAX_DATA_SIZE && priorq_input_prior >= 1) {
				int seq; 
				char header[32];
				struct stKey key;
				key.prior = priorq_input_prior;

				seq = priq_put(base, priorq_input_name, key, priorq_input_data);
				if (seq > 0)
				{
					memset(header, '\0', sizeof(header));
					sprintf(header, "%d", seq);
					evhttp_add_header(req->output_headers, "Pos", header);
					evbuffer_add_printf(buf, "%s:%d", "PQ_PUT_OK", seq);
				}
				else
				{
					evbuffer_add_printf(buf, "%s", "PQ_PUT_END");
				}
			} 
			else {
				evbuffer_add_printf(buf, "%s", "PQ_PUT_ERROR");
			}
		}
		/* 出队列 */
		else if (strcmp(priorq_input_opt, "get") == 0 && priorq_input_prior >= 1 && priorq_input_seq >= 1) {
			struct stKey key;
			char *priorq_output_value = NULL;
			key.prior = priorq_input_prior;
			key.seq = priorq_input_seq;
			if (priq_get(base, priorq_input_name, key, &priorq_output_value) == 0){
				evbuffer_add_printf(buf, "%s", priorq_output_value);
			} 
			else {
				evbuffer_add_printf(buf, "%s", "PQ_NOT_EXIST");
			}
			if (priorq_output_value != NULL){
				free(priorq_output_value);
			}
		}
		/* 查看队列状态 */
		else if (strcmp(priorq_input_opt, "status") == 0) {
			int curlength;
			priq_info(base, priorq_input_name, &curlength);
			evbuffer_add_printf(buf, "{\"name\":\"%s\",\"curlength\":%d}", priorq_input_name, curlength);
		}
		/* 查看单条队列内容 */
		else if (strcmp(priorq_input_opt, "view") == 0 && priorq_input_pos >= 1) {
			char *priorq_output_value = NULL;
			int prior = -1;
			int seq = -1;
			if(prig_view(base, priorq_input_name, priorq_input_pos, &prior, &seq, &priorq_output_value) == 0)
			{
				evbuffer_add_printf(buf, "{\"prior\":%d,\"seq\":%d,\"data\":%s}", prior, seq, priorq_output_value);
				free(priorq_output_value);
			}
			else {
				evbuffer_add_printf(buf, "%s", "PQ_NOT_EXIST");
			}
		}
		/* 获取节点在队列中位置 */
		else if (strcmp(priorq_input_opt, "getpos") == 0)
		{
			if (priorq_input_prior >= 1 && priorq_input_seq >= 1) {
				int pos = priq_getpos(base, priorq_input_name, priorq_input_prior, priorq_input_seq);
				evbuffer_add_printf(buf, "%d", pos);
			} else {
				evbuffer_add_printf(buf, "%s", "PQ_GETPOS_ERROR");
			}
		}
		/* 重置队列 */
		else if (strcmp(priorq_input_opt, "reset") == 0)
		{
		     
		}
		/* 数据持久化到磁盘 */
		else if (strcmp(priorq_input_opt, "persist") == 0)
		{
			if (priq_persist(base) == 0)  {
                evbuffer_add_printf(buf, "%s", "PQ_PERSIST_OK");
			} else {
                evbuffer_add_printf(buf, "%s", "PQ_PERSIST_FAIL");
			}
		}
	}
	evhttp_send_reply(req, HTTP_OK, "OK", buf);

	evhttp_clear_headers(&priorq_http_query);
	evbuffer_free(buf);
}

int main(int argc, char *argv[])
{
    int c;
    char *priorq_settings_listen = "0.0.0.0";
    int priorq_settings_port = 8080;
    int priorq_settings_timeout = 60;
    int priorq_settings_daemon = 0;

    struct evhttp *httpd;
    while ((c = getopt(argc, argv, "p:t:d")) != -1) {
        switch (c) {
            case 'p':
                priorq_settings_port = atoi(optarg);
                break;
            case 'd':
                priorq_settings_daemon = 1;
                break;
            default:
                return 1;
        }
    }

    if (priorq_settings_daemon)
    {
        pid_t pid;

        pid = fork();
        if (pid < 0) {
            exit(EXIT_FAILURE);
        }
        if (pid > 0) {
            exit(EXIT_SUCCESS);
        }
    }

    net_init();
    event_init();
	base = priq_init("priqueue.db");
    httpd = evhttp_start(priorq_settings_listen, priorq_settings_port);
    if (httpd == NULL) {
        fprintf(stderr, "Error: Unable to listen on %s:%d\n\n", priorq_settings_listen, priorq_settings_port);		
        exit(1);		
    }
    evhttp_set_timeout(httpd, priorq_settings_timeout);
    evhttp_set_gencb(httpd, priorq_handler, NULL);

    event_dispatch();
    evhttp_free(httpd);
    return 0;
}
