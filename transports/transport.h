/*! \file   transport.h
 * \author Lorenzo Miniero <lorenzo@meetecho.com>
 * \copyright GNU General Public License v3
 * \brief  Modular Janus API transports
 * \details  This header contains the definition of the callbacks both
 * the gateway and all the transports need to implement to interact with
 * each other. The structures to make the communication possible are
 * defined here as well.
 * 
 * In particular, the gateway implements the \c janus_transport_callbacks
 * interface. This means that, as a transport plugin, you can use the
 * methods it exposes to contact the gateway, e.g., in order to notify
 * an incoming message. In particular, the methods the gateway exposes
 * to transport plugins are:
 * 
 * - \c incoming_request(): to notify an incoming JSON message/event 
 * from one of the transport clients.
 * 
 * On the other hand, a transport plugin that wants to register at the
 * gateway needs to implement the \c janus_transport interface. Besides,
 * as a transport plugin is a shared object, and as such external to the
 * gateway itself, in order to be dynamically loaded at startup it needs
 * to implement the \c create_t() hook as well, that should return a
 * pointer to the plugin instance. This is an example of such a step:
 * 
\verbatim
static janus_transport mytransport = {
	[..]
};

janus_transport *create(void) {
	JANUS_LOG(LOG_VERB, , "%s created!\n", MY_TRANSPORT_NAME);
	return &mytransport;
}
\endverbatim
 * 
 * This will make sure that your transport plugin is loaded at startup
 * by the gateway, if it is deployed in the proper folder.
 * 
 * As anticipated and described in the above example, a transport plugin
 * must basically be an instance of the \c janus_transport type. As such,
 * it must implement the following methods and callbacks for the gateway:
 * 
 * - \c init(): this is called by the gateway as soon as your transport
 * plugin is started; this is where you should setup your transport plugin
 * (e.g., static stuff and reading the configuration file);
 * - \c destroy(): on the other hand, this is called by the gateway when it
 * is shutting down, and your transport plugin should too;
 * - \c get_api_compatibility(): this method MUST return JANUS_TRANSPORT_API_VERSION;
 * - \c get_version(): this method should return a numeric version identifier (e.g., 3);
 * - \c get_version_string(): this method should return a verbose version identifier (e.g., "v1.0.1");
 * - \c get_description(): this method should return a verbose description of your transport plugin (e.g., "This is my avian carrier transport plugin for the Janus API");
 * - \c get_name(): this method should return a short display name for your transport plugin (e.g., "My Amazing Transport");
 * - \c get_package(): this method should return a unique package identifier for your transport plugin (e.g., "janus.transport.mytransport");
 * - \c is_janus_api_enabled(): this method should return TRUE if Janus API can be used with this transport, and support has been enabled by the user;
 * - \c is_admin_api_enabled(): this method should return TRUE if Admin API can be used with this transport, and support has been enabled by the user;
 * - \c send_message(): this method asks the transport to send a message (be it a response or an event) to a client on the specified transport;
 * - \c session_created(): this method notifies the transport that a Janus session has been created by one of its requests;
 * - \c session_over(): this method notifies the transport that one of its Janus sessionss is now over, whether because of a timeour or not.
 * 
 * All the above methods and callbacks are mandatory: the Janus core will
 * reject a transport plugin that doesn't implement any of the
 * mandatory callbacks.
 * 
 * The gateway \c janus_transport_callbacks interface is provided to a
 * transport plugin, together with the path to the configurations files
 * folder, in the \c init() method. This path can be used to read and
 * parse a configuration file for the transport plugin: the transport
 * plugins we made available out of the box use the package name as a
 * name for the file (e.g., \c janus.transport.http.cfg for the HTTP/HTTPS
 * transport plugin), but you're free to use a different one, as long
 * as it doesn't collide with existing ones. Besides, the existing transport
 * plugins use the same INI format for configuration files the gateway
 * uses (relying on the \c janus_config helpers for the purpose) but
 * again, if you prefer a different format (XML, JSON, etc.) that's up to you. 
 * 
 * \ingroup transportapi
 * \ref transportapi
 */

#ifndef _JANUS_TRANSPORT_H
#define _JANUS_TRANSPORT_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <inttypes.h>

#include <glib.h>
#include <jansson.h>


/*! \brief Version of the API, to match the one transport plugins were compiled against */
#define JANUS_TRANSPORT_API_VERSION	6

/*! \brief Initialization of all transport plugin properties to NULL
 * 
 * \note All transport plugins MUST add this as the FIRST line when initializing
 * their transport plugin structure, e.g.:
 * 
\verbatim
static janus_transport janus_http_transport plugin =
	{
		JANUS_TRANSPORT_INIT,
		
		.init = janus_http_init,
		[..]
\endverbatim
 * */
#define JANUS_TRANSPORT_INIT(...) {		\
		.init = NULL,					\
		.destroy = NULL,				\
		.get_api_compatibility = NULL,	\
		.get_version = NULL,			\
		.get_version_string = NULL,		\
		.get_description = NULL,		\
		.get_name = NULL,				\
		.get_author = NULL,				\
		.get_package = NULL,			\
		.is_janus_api_enabled = NULL,	\
		.is_admin_api_enabled = NULL,	\
		.send_message = NULL,			\
		.session_created = NULL,		\
		.session_over = NULL,			\
		## __VA_ARGS__ }


/*! \brief Callbacks to contact the gateway */
typedef struct janus_transport_callbacks janus_transport_callbacks;
/*! \brief The transport plugin session and callbacks interface */
typedef struct janus_transport janus_transport;


/*! \brief 传输插件需要实现的接口(The transport plugin session and callbacks interface) */
struct janus_transport {
	/*! \brief 传输插件的构造函数
	 * @param[in] callback 网关的回调函数
	 * @param[in] config_path 配置文件路径获取传输插件的信息
	 * @returns 0表示成功，其他参考错误码 */
	int (* const init)(janus_transport_callbacks *callback, const char *config_path);

	/*! \brief 传输插件的析构函数 */
	void (* const destroy)(void);

	/*! \brief 获取插件的版本信息
	 *  \note 全部的插件必须全部实现该方法并且返回JANUS_TRANSPORT_API_VERSION */
	int (* const get_api_compatibility)(void);
	/*! \brief 获取传输插件的数字版本 */
	int (* const get_version)(void);
	/*! \brief 获取传输插件的字符串版本 */
	const char *(* const get_version_string)(void);
	/*! \brief 获取传输插件的描述信息 */
	const char *(* const get_description)(void);
	/*! \brief 获取传输插件的名字信息 */
	const char *(* const get_name)(void);
	/*! \brief 获取传输插件的作者信息 */
	const char *(* const get_author)(void);
	/*! \brief 获取传输插件的包信息 (用于web应用程序的引用) */
	const char *(* const get_package)(void);


	/*! \brief 检测Janus API 是否被支持*/
	gboolean (* const is_janus_api_enabled)(void);

	/*! \brief 检测Admin API 是否被支持 */
	gboolean (* const is_admin_api_enabled)(void);

	/*! \brief 使用传输会话(transport session)给客户端发送信息。
	 * \note 传输插件需要释放该消息的资源，除非收到成功的消息否则不能保证消息被送达。
	 * @param[in] transport 指向transport session实例的指针
	 * @param[in] request_id 如果这是对之前请求的响应,request_id不能为NULL
	 * @param[in] admin 是否为an admin API 或者 a Janus API 信息
	 * @param[in] message json_t对象存储的消息内容
	 * @returns 0表示成功 */
	int (* const send_message)(void *transport, void *request_id, gboolean admin, json_t *message);

	/*! \brief 主程(janus.c)通知传输插件新的会话已经被建立
	 * @param[in] transport 指向transport session实例的指针
	 * @param[in] session_id 如果传输关心id，会话id会被创建 */
	void (* const session_created)(void *transport, guint64 session_id);

	/*! \brief 通知传输插件会话超时
	 * @param[in] 指向transport session实例的指针
	 * @param[in] 如果传输关心id，该会话id会被关闭
	 * @param[in] 会话关闭的原因是否是超时 */
	void (* const session_over)(void *transport, guint64 session_id, gboolean timeout);

};

/*! \brief 传输插件调用的网关函数(Callbacks to contact the gateway)*/
struct janus_transport_callbacks {
		/*! \brief 回调函数通知有新的请求
	 * @param[in] plugin 
	 * @param[in] transport 指向transport session实例的指针用于获取事件
	 * @param[in] request_id Opaque pointer to a transport plugin specific value that identifies this request, so that an incoming response coming later can be matched
	 * @param[in] admin 是否为an admin API 或者 a Janus API 请求
	 * @param[in] message json_t对象存储的消息内容 */
	void (* const incoming_request)(janus_transport *plugin, void *transport, void *request_id, gboolean admin, json_t *message, json_error_t *error);
	
	/*! \brief 回调函数通知已经存在的传输通道离开
	 * @param[in] handle The transport session that went away
	 * @param[in] transport 指向离开会话的实例指针 */
	void (* const transport_gone)(janus_transport *plugin, void *transport);

	/*! \brief 回调函数检测核心代码，是否API秘钥必须被提供
	 * @returns TRUE if an API secret is needed, FALSE otherwise */
	gboolean (* const is_api_secret_needed)(janus_transport *plugin);
	/*! \brief Callback to check with the core if a provided API secret is valid
	 * \note This callback should only be needed when, for any reason, the transport needs to
	 * validate requests directly, as in general requests will be validated by the core itself.
	 * It is the case, for instance, of HTTP long polls to get session events, as those never
	 * pass through the core and so need to be validated by the transport plugin on its behalf.
	 * @param[in] apisecret The API secret to validate
	 * @returns TRUE if the API secret is correct, FALSE otherwise */
	gboolean (* const is_api_secret_valid)(janus_transport *plugin, const char *apisecret);
	/*! \brief Callback to check with the core if an authentication token is needed
	 * @returns TRUE if an auth token is needed, FALSE otherwise */
	gboolean (* const is_auth_token_needed)(janus_transport *plugin);
	/*! \brief Callback to check with the core if a provided authentication token is valid
	 * \note This callback should only be needed when, for any reason, the transport needs to
	 * validate requests directly, as in general requests will be validated by the core itself.
	 * It is the case, for instance, of HTTP long polls to get session events, as those never
	 * pass through the core and so need to be validated by the transport plugin on its behalf.
	 * @param[in] token The auth token to validate
	 * @returns TRUE if the auth token is valid, FALSE otherwise */
	gboolean (* const is_auth_token_valid)(janus_transport *plugin, const char *token);

	/*! \brief Callback to check whether the event handlers mechanism is enabled
	 * @returns TRUE if it is, FALSE if it isn't (which means notify_event should NOT be called) */
	gboolean (* const events_is_enabled)(void);
	/*! \brief Callback to notify an event to the registered and subscribed event handlers
	 * \note Don't unref the event object, the core will do that for you
	 * @param[in] plugin The transport originating the event
	 * @param[in] event The event to notify as a Jansson json_t object */
	void (* const notify_event)(janus_transport *plugin, void *transport, json_t *event);
};

/*! \brief The hook that transport plugins need to implement to be created from the gateway */
typedef janus_transport* create_t(void);

#endif
