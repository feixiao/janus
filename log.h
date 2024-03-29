/*! \file    log.h
 * \author   Jay Ridgeway <jayridge@gmail.com>
 * \copyright GNU General Public License v3
 * \brief    Buffered logging (headers)
 * \details  Implementation of a simple buffered logger designed to remove
 * I/O wait from threads that may be sensitive to such delays. Buffers are
 * saved and reused to reduce allocation calls. The logger output can then
 * be printed to stdout and/or a log file.
 *
 * \ingroup core
 * \ref core
 */

#ifndef _JANUS_LOG_H
#define _JANUS_LOG_H

#include <stdio.h>
#include <glib.h>

/*! \brief Buffered vprintf
* @param[in] format Format string as defined by glib
* \note This output is buffered and may not appear immediately on stdout. */
void janus_vprintf(const char *format, ...) G_GNUC_PRINTF(1, 2);

/*! \brief 日志初始化
*  该函数应该在使用logger功能之前被调用，内存池和日志处理线程将会被创建
* @param daemon Whether the Janus is running as a daemon or not
* @param console Whether the output should be printed on stdout or not
* @param logfile Log file to save the output to, if any
* @returns 0 in case of success, a negative integer otherwise */
int janus_log_init(gboolean daemon, gboolean console, const char *logfile);
/*! \brief Log destruction */
void janus_log_destroy(void);

/*! \brief Method to check whether stdout logging is enabled
 * @returns TRUE if stdout logging is enabled, FALSE otherwise */
gboolean janus_log_is_stdout_enabled(void);
/*! \brief Method to check whether file-based logging is enabled
 * @returns TRUE if file-based logging is enabled, FALSE otherwise */
gboolean janus_log_is_logfile_enabled(void);
/*! \brief Method to get the path to the log file
 * @returns The full path to the log file, or NULL otherwise */
char *janus_log_get_logfile_path(void);

#endif
