/*! \file    auth.h
 * \author   Lorenzo Miniero <lorenzo@meetecho.com>
 * \copyright GNU General Public License v3
 * \brief    Requests authentication (headers)
 * \details  Implementation of a simple mechanism for authenticating
 * requests. If enabled (it's disabled by default), the Janus admin API
 * can be used to specify valid tokens; each request must then contain
 * a valid token string, or otherwise the request is rejected with an
 * error. Whether tokens should be shared across users or not is
 * completely up to the controlling application: these tokens are
 * completely opaque to Janus, and treated as strings, which means
 * Janus will only check if the token exists or not when asked.
 * 
 * \ingroup core
 * \ref core
 */
 
#ifndef _JANUS_AUTH_H
#define _JANUS_AUTH_H

#include <glib.h>

#include "plugins/plugin.h"

/*! \brief 初始化基于令牌身份验证的方法(Method to initializing the token based authentication)
 * @param[in] enabled : 身份验证机制是否应该启用(Whether the authentication mechanism should be enabled or not)
 * @param[in] secret : 验证已签署的令牌，或使用NULL来存储令牌(the secret to validate signed tokens against, or NULL to use stored tokens) */
void janus_auth_init(gboolean enabled, const char *secret);

/*! \brief 检查该机制是否启用(Method to check whether the mechanism is enabled or not) */
gboolean janus_auth_is_enabled(void);

/*! \brief 检查是否在存储令牌模式机制中(Method to check whether the mechanism is in stored-token mode or not) */
gboolean janus_auth_is_stored_mode(void);

/*! \brief 重新初始化机制(Method to de-initialize the mechanism) */
void janus_auth_deinit(void);

/*! \brief 检查签名令牌是否有效的方法(Method to check whether a signed token is valid)
 * @param[in] token 令牌验证(The token to validate)
 * @param[in] realm 令牌领域(The token realm) 
 * @returns 签名是有效的，没有过期则返回true(TRUE if the signature is valid and not expired, FALSE otherwise) */
gboolean janus_auth_check_signature(const char *token, const char *realm);

/*! \brief 验证签名令牌是否包含描述符的方法(Method to verify a signed token contains a descriptor)
 * @param[in] token 令牌验证(The token to validate)
 * @param[in] realm 令牌领域(The token realm) 
 * @param[in] desc 要搜索的描述符(The descriptor to search for)
 * @returns 签名是有效的，没有过期,同时包含了描述符,则返回true(TRUE if the token is valid, not expired and contains the descriptor, FALSE otherwise) */
gboolean janus_auth_check_signature_contains(const char *token, const char *realm, const char *desc);

/*! \brief 添加一个新的有效令牌进行身份验证的方法(Method to add a new valid token for authenticating)
 * @param[in] token 新的有效的令牌(The new valid token)
 * @returns TRUE if the operation was successful, FALSE otherwise */
gboolean janus_auth_add_token(const char *token);

/*! \brief 检查所提供的令牌是否有效的方法(Method to check whether a provided token is valid or not)
 * \note verifies both token signatures and against stored tokens
 * @param[in] token 令牌验证(The token to validate)
 * @returns TRUE if the token is valid, FALSE otherwise */
gboolean janus_auth_check_token(const char *token);


/*! \brief 返回令牌列表的方法(Method to return a list of the tokens)
 * \note It's the caller responsibility to free the list and its values
 * @returns A pointer to a GList instance containing the tokens */
GList *janus_auth_list_tokens(void);

/*! \brief 使现有令牌无效的方法(Method to invalidate an existing token)
 * @param[in] token The valid to invalidate
 * @returns TRUE if the operation was successful, FALSE otherwise */
gboolean janus_auth_remove_token(const char *token);

/*! \brief 允许令牌使用插件的方法(Method to allow a token to use a plugin)
 * @param[in] token The token that can now access this plugin
 * @param[in] plugin Opaque pointer to the janus_plugin instance this token can access
 * @returns TRUE if the operation was successful, FALSE otherwise */
gboolean janus_auth_allow_plugin(const char *token, janus_plugin *plugin);

/*! \brief 检查所提供的令牌是否可以访问指定的插件的方法(Method to check whether a provided token can access a specified plugin)
 * \note verifies both token signatures and against stored tokens
 * @param[in] token The token to check
 * @param[in] plugin The plugin to check as an opaque pointer to a janus_plugin instance
 * @returns TRUE if the token is allowed to access the plugin, FALSE otherwise */
gboolean janus_auth_check_plugin(const char *token, janus_plugin *plugin);

/*! \brief 返回一个特定令牌可以访问的插件列表的方法(Method to return a list of the plugins a specific token has access to)
 * \note It's the caller responsibility to free the list (but NOT the values)
 * @param[in] token The token to get the list for
 * @returns A pointer to a GList instance containing the liist */
GList *janus_auth_list_plugins(const char *token);

/*! \brief 不允许令牌使用插件(Method to disallow a token to use a plugin)
 * @param[in] token The token this operation refers to
 * @param[in] plugin Opaque pointer to the janus_plugin instance this token can not access anymore
 * @returns TRUE if the operation was successful, FALSE otherwise */
gboolean janus_auth_disallow_plugin(const char *token, janus_plugin *plugin);

#endif
