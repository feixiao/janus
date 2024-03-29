/*! \file    config.h
 * \author   Lorenzo Miniero <lorenzo@meetecho.com>
 * \copyright GNU General Public License v3
 * \brief    Configuration files parsing (headers)
 * \details  Implementation of a parser of INI configuration files.
 * 
 * \ingroup core
 * \ref core
 */

#ifndef _JANUS_CONFIG_H
#define _JANUS_CONFIG_H

#include <glib.h>


// 配置文件采用kv形式: name=value
/*! \brief Configuration item (name=value) */
typedef struct janus_config_item {
	/*! \brief Name of the item */
	const char *name;
	/*! \brief Value of the item */
	const char *value;
} janus_config_item;

// 分类
/*! \brief Configuration category ([category]) */
typedef struct janus_config_category {
	/*! \brief Name of the category */
	const char *name;
	/*! \brief Linked list of items */
	GList *items;
} janus_config_category;

/*! \brief 配置文件容器 */
typedef struct janus_config {
	/*! \brief Name of the configuration */
	const char *name;
	/*! \brief Linked list of uncategorized items */
	GList *items;
	/*! \brief Linked list of categories category */
	GList *categories;
} janus_config;


/*! \brief 解析INI配置文件的方法(Method to parse an INI configuration file)
 * @param[in] config_file Path to the configuration file
 * @returns A pointer to a valid janus_config instance if successful, NULL otherwise */ 
janus_config *janus_config_parse(const char *config_file);

/*! \brief 创建空的配置文件方法(Method to create a new, empty, configuration)
 * @param[in] name Name to give to the configuration
 * @returns A pointer to a valid janus_config instance if successful, NULL otherwise */ 
janus_config *janus_config_create(const char *name);

/*! \brief 从已解析的配置中获取所有类别的列表，作为一个GLib链表(Get the list of all categories from a parsed configuration as a GLib linked list)
 * @param[in] config The configuration container
 * @returns A pointer to the categories GLib linked list if successful, NULL otherwise */ 
GList *janus_config_get_categories(janus_config *config);

/*! \brief 从经过分析的配置中获得特定名称的类别(Get the category with a specific name from a parsed configuration)
 * @param[in] config The configuration container
 * @param[in] name The name of the category
 * @returns A pointer to the janus_config_category instance if successful, NULL otherwise */ 
janus_config_category *janus_config_get_category(janus_config *config, const char *name);

/*! \brief 将类别中所有项目的列表作为一个GLib链表(Get the list of all items in a category as a GLib linked list)
 * @param[in] category The configuration category
 * @returns A pointer to the items GLib linked list if successful, NULL otherwise */ 
GList *janus_config_get_items(janus_config_category *category);

/*! \brief 从已解析配置的类别中获取特定名称的条目(Get the item with a specific name from a category of a parsed configuration)
 * @param[in] category The configuration category
 * @param[in] name The name of the item
 * @returns A pointer to the janus_config_item instance if successful, NULL otherwise */ 
janus_config_item *janus_config_get_item(janus_config_category *category, const char *name);

/*! \brief 从一个具有特定名称的类别中获得特定名称，并从已解析配置中获得特定名称(Get the item with a specific name from a category with a specific name from a parsed configuration)
 * \note This is the same as janus_config_get_item, but it looks for the janus_config_category for you
 * @param[in] config The configuration container
 * @param[in] category The name of the configuration category
 * @param[in] name The name of the item
 * @returns A pointer to the janus_config_item instance if successful, NULL otherwise */ 
janus_config_item *janus_config_get_item_drilldown(janus_config *config, const char *category, const char *name);

/*! \brief 添加一个带有特定名称的新类别(Add a new category with the specific name)
 * \note If the item already exists in the category, it is NOT overwritten, and the existing instance is returned
 * @param[in] config The configuration container
 * @param[in] category The category to create
 * @returns A pointer to the janus_config_category instance if successful, NULL otherwise */ 
janus_config_category *janus_config_add_category(janus_config *config, const char *category);

/*! \brief 用特定的名称删除现有类别(Remove an existing category with the specific name)
 * \note This will also remove all items from that category
 * @param[in] config The configuration container
 * @param[in] category The category to remove
 * @returns 0 if successful, a negative integer otherwise */ 
int janus_config_remove_category(janus_config *config, const char *category);

/*! \brief 添加一个带有特定名称和值的新条目，如果它不存在，就创建类别(Add a new item with the specific name and value to a category, and create the category if it doesn't exist)
 * \note If the item already exists in the category, its value is overwritten
 * @param[in] config The configuration container
 * @param[in] category The category to add the item to, and to create if it doesn't exist
 * @param[in] name The name of the item
 * @param[in] value The value of the item
 * @returns A pointer to the janus_config_item instance if successful, NULL otherwise */ 
janus_config_item *janus_config_add_item(janus_config *config, const char *category, const char *name, const char *value);

/*! \brief 从一个类别中删除一个已有的项目(Remove an existing item with the specific name from a category)
 * @param[in] config The configuration container
 * @param[in] category The category to remove the item from
 * @param[in] name The name of the item
 * @returns 0 if successful, a negative integer otherwise */ 
int janus_config_remove_item(janus_config *config, const char *category, const char *name);

/*! \brief 在标准输出上打印配置的助手方法(Helper method to print a configuration on the standard output)
 * @param[in] config The configuration to print */
void janus_config_print(janus_config *config);

/*! \brief 将配置保存到文件中的助手方法(Helper method to save a configuration to a file)
 * @param[in] config The configuration to sav
 * @param[in] folder The folder the file should be saved to
 * @param[in] filename The file name, extension included (should be .cfg)
 * @returns 0 if successful, a negative integer otherwise */
int janus_config_save(janus_config *config, const char *folder, const char *filename);

/*! \brief 销毁一个配置容器实例(Destroy a configuration container instance)
 * @param[in] config The configuration to destroy */
void janus_config_destroy(janus_config *config);


#endif
