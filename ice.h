/*! \file    ice.h
 * \author   Lorenzo Miniero <lorenzo@meetecho.com>
 * \copyright GNU General Public License v3
 * \brief    ICE/STUN/TURN processing (headers)
 * \details  Implementation (based on libnice) of the ICE process. The
 * code handles the whole ICE process, from the gathering of candidates
 * to the final setup of a virtual channel RTP and RTCP can be transported
 * on. Incoming RTP and RTCP packets from peers are relayed to the associated
 * plugins by means of the incoming_rtp and incoming_rtcp callbacks. Packets
 * to be sent to peers are relayed by peers invoking the relay_rtp and
 * relay_rtcp gateway callbacks instead. 
 * 该代码处理整个ICE过程，从候选者的收集到RTP虚拟通道的最终设置和RTCP可以被传输。
 * 通过incoming_rtp和incoming_rtcp回调的方式，将来自对端的传入RTP和RTCP数据包转发给相关的插件。
 * 发送给对端的数据包由调用relay_rtp和relay_rtcp网关回调的对等点转发。
 * 
 * ICE(交互式连接建立---Interactive Connectivity Establishment)
 * 
 * \ingroup protocols
 * \ref protocols
 */
 
#ifndef _JANUS_ICE_H
#define _JANUS_ICE_H

#include <glib.h>
#include <agent.h>

#include "sdp.h"
#include "dtls.h"
#include "sctp.h"
#include "rtcp.h"
#include "text2pcap.h"
#include "utils.h"
#include "plugins/plugin.h"


/*! \brief ICE stuff initialization (libnice库初始化工作,启动了janus_ice_handles_watchdog线程)
 * @param[in] ice_lite  ICE Lite模式是否被支持
 * @param[in] ice_tcp   ICE TCP模式是否被支持(only libnice >= 0.1.8, currently broken)
 * @param[in] full_trickle  full-trickle是否必须被使用 (instead of half-trickle)
 * @param[in] ipv6 		IPv6候选地址是否被协商
 * @param[in] rtp_min_port  RTP/RTCP使用的最小端口
 * @param[in] rtp_max_port 	RTP/RTCP使用的最大端口*/
void janus_ice_init(gboolean ice_lite, gboolean ice_tcp, gboolean full_trickle, gboolean ipv6, uint16_t rtp_min_port, uint16_t rtp_max_port);

/*! \brief ICE stuff de-initialization (libnice清理工作) */
void janus_ice_deinit(void);

/*! \brief Method to force Janus to use a STUN server when gathering candidates
 * @param[in] stun_server STUN server address to use
 * @param[in] stun_port STUN port to use
 * @returns 0 in case of success, a negative integer on errors */
int janus_ice_set_stun_server(gchar *stun_server, uint16_t stun_port);

/*! \brief 强制Janus在收集候选地址使用TURN服务
 * @param[in] turn_server TURN server address to use
 * @param[in] turn_port TURN port to use
 * @param[in] turn_type Relay type (udp, tcp or tls)
 * @param[in] turn_user TURN username, if needed
 * @param[in] turn_pwd TURN password, if needed
 * @returns 0 in case of success, a negative integer on errors */
int janus_ice_set_turn_server(gchar *turn_server, uint16_t turn_port, gchar *turn_type, gchar *turn_user, gchar *turn_pwd);

/*! \brief 强制要求JJanus在收集候选地址通过Rest API获取TURN服务地址.
 * The TURN REST API takes precedence over any static credential passed via janus_ice_set_turn_server
 * @note Requires libcurl to be available, and a working TURN REST API backend (see turnrest.h)
 * @param[in] api_server TURN REST API backend (NULL to disable the API)
 * @param[in] api_key API key to use, if required
 * @param[in] api_method HTTP method to use (POST by default)
 * @returns 0 in case of success, a negative integer on errors */
int janus_ice_set_turn_rest_api(gchar *api_server, gchar *api_key, gchar *api_method);

/*! \brief 获取STUN服务地址(全局变量janus_stun_server)
 * @returns The currently used STUN server IP address, if available, or NULL if not */
char *janus_ice_get_stun_server(void);

/*! \brief 获取STUN服务端口(全局变量janus_stun_port)
 * @returns The currently used STUN server port, if available, or 0 if not */
uint16_t janus_ice_get_stun_port(void);

/*! \brief 获取TURN服务地址(全局变量janus_turn_server)
 * @returns The currently used TURN server IP address, if available, or NULL if not */
char *janus_ice_get_turn_server(void);

/*! \brief 获取TURN服务端口(全局变量janus_turn_port)
 * @returns The currently used TURN server port, if available, or 0 if not */
uint16_t janus_ice_get_turn_port(void);

/*! \brief Method to get the specified TURN REST API backend, if any
 * @returns The currently specified  TURN REST API backend, if available, or NULL if not */
char *janus_ice_get_turn_rest_api(void);

/*! \brief Helper method to force Janus to overwrite all host candidates with the public IP */
void janus_ice_enable_nat_1_1(void);

/*! \brief Method to add an interface/IP to the enforce list for ICE (that is, only gather candidates from these and ignore the others)
 * \note This method is especially useful to speed up the ICE gathering process on the gateway: in fact,
 * if you know in advance which interface must be used (e.g., the main interface connected to the internet),
 * adding it to the enforce list will prevent libnice from gathering candidates from other interfaces.
 * If you're interested in excluding interfaces explicitly, instead, check janus_ice_ignore_interface.
 * @param[in] ip Interface/IP to enforce (e.g., 192.168. or eth0) */
void janus_ice_enforce_interface(const char *ip);

/*! \brief Method to check whether an interface is currently in the enforce list for ICE (that is, won't have candidates)
 * @param[in] ip Interface/IP to check (e.g., 192.168.244.1 or eth1)
 * @returns true if the interface/IP is in the enforce list, false otherwise */
gboolean janus_ice_is_enforced(const char *ip);

/*! \brief Method to add an interface/IP to the ignore list for ICE (that is, don't gather candidates)
 * \note This method is especially useful to speed up the ICE gathering process on the gateway: in fact,
 * if you know in advance an interface is not going to be used (e.g., one of those created by VMware),
 * adding it to the ignore list will prevent libnice from gathering a candidate for it.
 * Unlike the enforce list, the ignore list also accepts IP addresses, partial or complete.
 * If you're interested in only using specific interfaces, instead, check janus_ice_enforce_interface.
 * @param[in] ip Interface/IP to ignore (e.g., 192.168. or eth1) */
void janus_ice_ignore_interface(const char *ip);

/*! \brief Method to check whether an interface/IP is currently in the ignore list for ICE (that is, won't have candidates)
 * @param[in] ip Interface/IP to check (e.g., 192.168.244.1 or eth1)
 * @returns true if the interface/IP is in the ignore list, false otherwise */
gboolean janus_ice_is_ignored(const char *ip);

/*! \brief Method to check whether ICE Lite mode is enabled or not (still WIP)
 * @returns true if ICE-TCP support is enabled/supported, false otherwise */
gboolean janus_ice_is_ice_lite_enabled(void);

/*! \brief Method to check whether ICE-TCP support is enabled/supported or not (still WIP)
 * @returns true if ICE-TCP support is enabled/supported, false otherwise */
gboolean janus_ice_is_ice_tcp_enabled(void);

/*! \brief Method to check whether full-trickle support is enabled or not
 * @returns true if full-trickle support is enabled, false otherwise */
gboolean janus_ice_is_full_trickle_enabled(void);

/*! \brief Method to check whether IPv6 candidates are enabled/supported or not (still WIP)
 * @returns true if IPv6 candidates are enabled/supported, false otherwise */
gboolean janus_ice_is_ipv6_enabled(void);

/*! \brief Method to modify the max NACK value (i.e., the number of packets per handle to store for retransmissions)
 * @param[in] mnq The new max NACK value */
void janus_set_max_nack_queue(uint mnq);

/*! \brief Method to get the current max NACK value (i.e., the number of packets per handle to store for retransmissions)
 * @returns The current max NACK value */
uint janus_get_max_nack_queue(void);

/*! \brief Method to modify the no-media event timer (i.e., the number of seconds where no media arrives before Janus notifies this)
 * @param[in] timer The new timer value, in seconds */
void janus_set_no_media_timer(uint timer);

/*! \brief Method to get the current no-media event timer (see above)
 * @returns The current no-media event timer */
uint janus_get_no_media_timer(void);

/*! \brief Method to enable or disable the RFC4588 support negotiation
 * @param[in] enabled The new timer value, in seconds */
void janus_set_rfc4588_enabled(gboolean enabled);

/*! \brief Method to check whether the RFC4588 support is enabled
 * @returns TRUE if it's enabled, FALSE otherwise */
gboolean janus_is_rfc4588_enabled(void);

/*! \brief Method to modify the event handler statistics period (i.e., the number of seconds that should pass before Janus notifies event handlers about media statistics for a PeerConnection)
 * @param[in] timer The new timer value, in seconds */
void janus_ice_set_event_stats_period(int period);

/*! \brief Method to get the current event handler statistics period (see above)
 * @returns The current event handler stats period */
int janus_ice_get_event_stats_period(void);

/*! \brief Method to check whether libnice debugging has been enabled (http://nice.freedesktop.org/libnice/libnice-Debug-messages.html)
 * @returns True if libnice debugging is enabled, FALSE otherwise */
gboolean janus_ice_is_ice_debugging_enabled(void);

/*! \brief Method to enable libnice debugging (http://nice.freedesktop.org/libnice/libnice-Debug-messages.html) */
void janus_ice_debugging_enable(void);

/*! \brief Method to disable libnice debugging (the default) */
void janus_ice_debugging_disable(void);


/*! \brief Helper method to get a string representation of a libnice ICE state
 * @param[in] state The libnice ICE state
 * @returns A string representation of the libnice ICE state */
const gchar *janus_get_ice_state_name(gint state);


/*! \brief Janus ICE会话(Janus ICE handle/session) */
typedef struct janus_ice_handle janus_ice_handle;
/*! \brief Janus ICE stream */
typedef struct janus_ice_stream janus_ice_stream;
/*! \brief Janus ICE component */
typedef struct janus_ice_component janus_ice_component;
/*! \brief Helper to handle pending trickle candidates (e.g., when we're still waiting for an offer) */
typedef struct janus_ice_trickle janus_ice_trickle;

#define JANUS_ICE_HANDLE_WEBRTC_PROCESSING_OFFER	(1 << 0)
#define JANUS_ICE_HANDLE_WEBRTC_START				(1 << 1)
#define JANUS_ICE_HANDLE_WEBRTC_READY				(1 << 2)
#define JANUS_ICE_HANDLE_WEBRTC_STOP				(1 << 3)
#define JANUS_ICE_HANDLE_WEBRTC_ALERT				(1 << 4)
#define JANUS_ICE_HANDLE_WEBRTC_TRICKLE				(1 << 7)
#define JANUS_ICE_HANDLE_WEBRTC_ALL_TRICKLES		(1 << 8)
#define JANUS_ICE_HANDLE_WEBRTC_TRICKLE_SYNCED		(1 << 9)
#define JANUS_ICE_HANDLE_WEBRTC_DATA_CHANNELS		(1 << 10)
#define JANUS_ICE_HANDLE_WEBRTC_CLEANING			(1 << 11)
#define JANUS_ICE_HANDLE_WEBRTC_HAS_AUDIO			(1 << 12)
#define JANUS_ICE_HANDLE_WEBRTC_HAS_VIDEO			(1 << 13)
#define JANUS_ICE_HANDLE_WEBRTC_GOT_OFFER			(1 << 14)
#define JANUS_ICE_HANDLE_WEBRTC_GOT_ANSWER			(1 << 15)
#define JANUS_ICE_HANDLE_WEBRTC_HAS_AGENT			(1 << 16)
#define JANUS_ICE_HANDLE_WEBRTC_ICE_RESTART			(1 << 17)
#define JANUS_ICE_HANDLE_WEBRTC_RESEND_TRICKLES		(1 << 18)
#define JANUS_ICE_HANDLE_WEBRTC_RFC4588_RTX			(1 << 19)


/*! \brief Janus 媒体数据统计结构
 * \note To improve with more stuff */
typedef struct janus_ice_stats_info {
	/*! \brief Packets sent or received */
	guint32 packets;
	/*! \brief Bytes sent or received */
	guint64 bytes;
	/*! \brief Bytes sent or received in the last second */
	guint32 bytes_lastsec, bytes_lastsec_temp;
	/*! \brief Time we last updated the last second counter */
	gint64 updated;
	/*! \brief Whether or not we notified about lastsec issues already */
	gboolean notified_lastsec;
	/*! \brief Number of NACKs sent or received */
	guint32 nacks;
} janus_ice_stats_info;

/*! \brief Janus media statistics container
 * \note To improve with more stuff */
typedef struct janus_ice_stats {
	/*! \brief Audio info */
	janus_ice_stats_info audio;
	/*! \brief Video info (considering we may be simulcasting) */
	janus_ice_stats_info video[3];
	/*! \brief Data info */
	janus_ice_stats_info data;
	/*! \brief Last time the slow_link callback (of the plugin) was called */
	gint64 last_slowlink_time;
	/*! \brief Start time of recent NACKs (for slow_link) */
	gint64 sl_nack_period_ts;
	/*! \brief Count of recent NACKs (for slow_link) */
	guint sl_nack_recent_cnt;
} janus_ice_stats;

/*! \brief Quick helper method to notify a WebRTC hangup through the Janus API
 * @param handle janus_ice_handle对象指针
 * @param reason A description of why this happened */
void janus_ice_notify_hangup(janus_ice_handle *handle, const char *reason);


/*! \brief Quick helper method to check if a plugin session associated with a Janus handle is still valid
 * @param plugin_session The janus_plugin_session instance to validate
 * @returns true if the plugin session is valid, false otherwise */
gboolean janus_plugin_session_is_alive(janus_plugin_session *plugin_session);


/*! \brief A helper struct for determining when to send NACKs */
typedef struct janus_seq_info {
	gint64 ts;
	guint16 seq;
	guint16 state;
	struct janus_seq_info *next;
	struct janus_seq_info *prev;
} janus_seq_info;
void janus_seq_list_free(janus_seq_info **head);
enum {
	SEQ_MISSING,
	SEQ_NACKED,
	SEQ_GIVEUP,
	SEQ_RECVED
};


/*! \brief Janus ICE handle */
struct janus_ice_handle {
	/*! \brief janus_session结构，关联传输层对象 */
	void *session;
	/*! \brief 唯一标示*/
	guint64 handle_id;
	/*! \brief Opaque identifier, e.g., to provide inter-handle relationships to external tools */
	char *opaque_id;
	/*! \brief 创建时间 */
	gint64 created;
	/*! \brief 指向插件的指针*/
	void *app;
	/*! \brief janus_plugin_session指针 */
	janus_plugin_session *app_handle;
	/*! \brief Mask of WebRTC-related flags for this handle */
	janus_flags webrtc_flags;
	/*! \brief Number of gathered candidates */
	gint cdone;
	/*! \brief GLib context for libnice */
	GMainContext *icectx;
	/*! \brief GLib loop for libnice */
	GMainLoop *iceloop;
	/*! \brief GLib thread for libnice */
	GThread *icethread;
	/*! \brief libnice ICE agent */
	NiceAgent *agent;
	/*! \brief Monotonic time of when the ICE agent has been created */
	gint64 agent_created;
	/*! \brief ICE role (controlling or controlled) */
	gboolean controlling;
	/*! \brief Audio mid (media ID) */
	gchar *audio_mid;
	/*! \brief Video mid (media ID) */
	gchar *video_mid;
	/*! \brief Data channel mid (media ID) */
	gchar *data_mid;
	/*! \brief Main mid (will be a pointer to one of the above) */
	gchar *stream_mid;
	/*! \brief ICE Stream ID */
	guint stream_id;
	/*! \brief ICE stream */
	janus_ice_stream *stream;
	/*! \brief 呼叫者使用的传输协议 (so that we can match it) */
	gchar *rtp_profile;
	/*! \brief SDP generated locally (just for debugging purposes) janus_plugin_handle_sdp函数中生成 */
	gchar *local_sdp;
	/*! \brief SDP received by the peer (just for debugging purposes) */
	gchar *remote_sdp;
	/*! \brief Reason this handle has been hung up*/
	const gchar *hangup_reason;
	/*! \brief 待决候选人名单 (在offer之前获取) */
	GList *pending_trickles;
	/*! \brief 需要发送的数据包队列 */
	GAsyncQueue *queued_packets;
	/*! \brief GLib thread for sending outgoing packets */
	GThread *send_thread;
	/*! \brief Atomic flag to make sure we only create the thread once */
	volatile gint send_thread_created;
	/*! \brief Count of the recent SRTP replay errors, in order to avoid spamming the logs */
	guint srtp_errors_count;
	/*! \brief Count of the recent SRTP replay errors, in order to avoid spamming the logs */
	gint last_srtp_error;
	/*! \brief Flag to decide whether or not packets need to be dumped to a text2pcap file */
	volatile gint dump_packets;
	/*! \brief In case this session must be saved to text2pcap, the instance to dump packets to */
	janus_text2pcap *text2pcap;
	/*! \brief Mutex to lock/unlock the ICE session */
	janus_mutex mutex;
};

/*! \brief Janus ICE stream */
struct janus_ice_stream {
	/*! \brief 指向所属的janus_ice_handle对象 */
	janus_ice_handle *handle;
	/*! \brief libnice ICE stream ID */
	guint stream_id;
	/*! \brief Whether this stream is ready to be used */
	gint cdone:1;
	/*! \brief Audio SSRC of the gateway for this stream */
	guint32 audio_ssrc;
	/*! \brief Video SSRC of the gateway for this stream */
	guint32 video_ssrc;
	/*! \brief Video retransmission SSRC of the peer for this stream */
	guint32 video_ssrc_rtx;
	/*! \brief Audio SSRC of the peer for this stream */
	guint32 audio_ssrc_peer, audio_ssrc_peer_new, audio_ssrc_peer_orig;
	/*! \brief Video SSRC(s) of the peer for this stream (may be simulcasting) */
	guint32 video_ssrc_peer[3], video_ssrc_peer_new[3], video_ssrc_peer_orig[3];
	/*! \brief Video retransmissions SSRC(s) of the peer for this stream */
	guint32 video_ssrc_peer_rtx[3], video_ssrc_peer_rtx_new[3], video_ssrc_peer_rtx_orig[3];
	/*! \brief Array of RTP Stream IDs (for Firefox simulcasting, if enabled) */
	char *rid[3];
	/*! \brief RTP switching context(s) in case of renegotiations (audio+video and/or simulcast) */
	janus_rtp_switching_context rtp_ctx[3];
	/*! \brief List of payload types we can expect for audio */
	GList *audio_payload_types;
	/*! \brief List of payload types we can expect for video */
	GList *video_payload_types;
	/*! \brief Mapping of rtx payload types to actual media-related packet types */
	GHashTable *rtx_payload_types;
	/*! \brief RTP payload types of this stream */
	gint audio_payload_type, video_payload_type, video_rtx_payload_type;
	/*! \brief Codecs used by this stream */
	char *audio_codec, *video_codec;
	/*! \brief 函数指针用于判断包是否属于关键桢 (depends on negotiated codec) */
	gboolean (* video_is_keyframe)(char* buffer, int len);
	/*! \brief Media direction */
	gboolean audio_send, audio_recv, video_send, video_recv;
	/*! \brief RTCP context for the audio stream */
	janus_rtcp_context *audio_rtcp_ctx;
	/*! \brief RTCP context(s) for the video stream (may be simulcasting) */
	janus_rtcp_context *video_rtcp_ctx[3];
	/*! \brief Map(s) of the NACKed packets (to track retransmissions and avoid duplicates) */
	GHashTable *rtx_nacked[3];
	/*! \brief First received audio NTP timestamp */
	gint64 audio_first_ntp_ts;
	/*! \brief First received audio RTP timestamp */
	guint32 audio_first_rtp_ts;
	/*! \brief First received video NTP timestamp (for all simulcast video streams) */
	gint64 video_first_ntp_ts[3];
	/*! \brief First received video NTP RTP timestamp (for all simulcast video streams) */
	guint32 video_first_rtp_ts[3];
	/*! \brief Last sent audio RTP timestamp */
	guint32 audio_last_ts;
	/*! \brief Last sent video RTP timestamp */
	guint32 video_last_ts;
	/*! \brief  Wether we do transport wide cc for video */
	gboolean do_transport_wide_cc;
	/*! \brief Transport wide cc rtp ext ID */
	guint transport_wide_cc_ext_id;
	/*! \brief Last received transport wide seq num */
	guint32 transport_wide_cc_last_seq_num;
	/*! \brief Last transport wide seq num sent on feedback */
	guint32 transport_wide_cc_last_feedback_seq_num;
	/*! \brief Transport wide cc transport seq num wrap cycles */
	guint16 transport_wide_cc_cycles;
	/*! \brief Transport wide cc rtp ext ID */
	guint transport_wide_cc_feedback_count;
	/*! \brief GLib list of transport wide cc stats in reverse received order */
	GSList *transport_wide_received_seq_nums;
	/*! \brief DTLS role of the gateway for this stream */
	janus_dtls_role dtls_role;
	/*! \brief 对端使用的散列算法 SHA-256等 */
	gchar *remote_hashing;
	/*! \brief 在SDP中解析的对端证书的散列值 */
	gchar *remote_fingerprint;
	/*! \brief 在STUN连通性检查中组成用户名的片断 */
	gchar *ruser;
	/*! \brief 提供用于保护STUN连通性检查的密码 */
	gchar *rpass;
	/*! \brief GLib hash table of components (IDs are the keys) */
	GHashTable *components;
	/*! \brief ICE component */
	janus_ice_component *component;
	/*! \brief Helper flag to avoid flooding the console with the same error all over again */
	gboolean noerrorlog;
	/*! \brief Mutex to lock/unlock this stream */
	janus_mutex mutex;
};

// 结构体 https://janus.conf.meetecho.com/docs/structjanus__ice__handle.html
#define LAST_SEQS_MAX_LEN 160
/*! \brief Janus ICE component.   */
struct janus_ice_component {
	/*! \brief 指向所属的janus_ice_stream对象 */
	janus_ice_stream *stream;			
	/*! \brief libnice ICE stream ID */
	guint stream_id;
	/*! \brief libnice ICE component ID */
	guint component_id;
	/*! \brief libnice ICE component state */
	guint state;
	/*! \brief Monotonic time of when this component has successfully connected */
	gint64 component_connected;
	/*! \brief GLib list of libnice remote candidates for this component */
	GSList *candidates;
	/*! \brief 本端的候选地址列表*/
	GSList *local_candidates;
	/*! \brief 对端的候选地址列表(字符串的形式) */
	GSList *remote_candidates;
	/*! \brief String representation of the selected pair as notified by libnice (foundations) */
	gchar *selected_pair;
	/*! \brief Whether the setup of remote candidates for this component has started or not */
	gboolean process_started;
	/*! \brief Timer to check when we should consider ICE as failed */
	GSource *icestate_source;
	/*! \brief Time of when we first detected an ICE failed (we'll need this for the timer above) */
	gint64 icefailed_detected;
	/*! \brief Re-transmission timer for DTLS */
	GSource *dtlsrt_source;
	/*! \brief DTLS-SRTP stack */
	janus_dtls_srtp *dtls;
	/*! \brief Whether we should do NACKs (in or out) for audio */
	gboolean do_audio_nacks;
	/*! \brief Whether we should do NACKs (in or out) for video */
	gboolean do_video_nacks;
	/*! \brief List of previously sent janus_rtp_packet RTP packets, in case we receive NACKs */
	GQueue *audio_retransmit_buffer, *video_retransmit_buffer;
	/*! \brief HashTable of retransmittable sequence numbers, in case we receive NACKs */
	GHashTable *audio_retransmit_seqs, *video_retransmit_seqs;
	/*! \brief Current sequence number for the RFC4588 rtx SSRC session */
	guint16 rtx_seq_number;
	/*! \brief Last time a log message about sending retransmits was printed */
	gint64 retransmit_log_ts;
	/*! \brief Number of retransmitted packets since last log message */
	guint retransmit_recent_cnt;
	/*! \brief Last time a log message about sending NACKs was printed */
	gint64 nack_sent_log_ts;
	/*! \brief Number of NACKs sent since last log message */
	guint nack_sent_recent_cnt;
	/*! \brief List of recently received audio sequence numbers (as a support to NACK generation) */
	janus_seq_info *last_seqs_audio;
	/*! \brief List of recently received video sequence numbers (as a support to NACK generation, for each simulcast SSRC) */
	janus_seq_info *last_seqs_video[3];
	/*! \brief Stats for incoming data (audio/video/data) */
	janus_ice_stats in_stats;
	/*! \brief Stats for outgoing data (audio/video/data) */
	janus_ice_stats out_stats;
	/*! \brief Helper flag to avoid flooding the console with the same error all over again */
	gboolean noerrorlog;
	/*! \brief Mutex to lock/unlock this component */
	janus_mutex mutex;
};

/*! \brief Helper to handle pending trickle candidates (e.g., when we're still waiting for an offer) */
struct janus_ice_trickle {
	/*! \brief Janus ICE handle this trickle candidate belongs to */
	janus_ice_handle *handle;
	/*! \brief Monotonic time of when this trickle candidate has been received */
	gint64 received;
	/*! \brief Janus API transaction ID of the original trickle request */
	char *transaction;
	/*! \brief JSON object of the trickle candidate(s) */
	json_t *candidate;
};

/** @name Janus ICE trickle candidates methods
 */
///@{
/*! \brief Helper method to allocate a janus_ice_trickle instance
 * @param[in] handle The Janus ICE handle this trickle candidate belongs to
 * @param[in] transaction The Janus API ID of the original trickle request
 * @param[in] candidate The trickle candidate, as a Jansson object
 * @returns a pointer to the new instance, if successful, NULL otherwise */
janus_ice_trickle *janus_ice_trickle_new(janus_ice_handle *handle, const char *transaction, json_t *candidate);

/*! \brief 解析单个候选者的方法
 * @param[in] handle 候选者所属的janus_ice_handle对象
 * @param[in] candidate 需要解析的候选者
 * @param[in,out] error Error string describing the failure, if any
 * @returns 0 in case of success, any code from apierror.h in case of failure */
gint janus_ice_trickle_parse(janus_ice_handle *handle, json_t *candidate, const char **error);

/*! \brief Helper method to destroy a janus_ice_trickle instance
 * @param[in] trickle The janus_ice_trickle instance to destroy */
void janus_ice_trickle_destroy(janus_ice_trickle *trickle);
///@}


/** @name Janus ICE 处理方法
 */
///@{
/*! \brief 创建新的ICE实例
 * @param[in] gateway_session ICE实例所属的Session对象指针
 * @param[in] opaque_id The opaque identifier provided by the creator, if any (optional)
 * @returns The created Janus ICE handle if successful, NULL otherwise */
janus_ice_handle *janus_ice_handle_create(void *gateway_session, const char *opaque_id);

/*! \brief Method to find an existing Janus ICE handle from its ID
 * @param[in] gateway_session ICE实例所属的Session对象指针
 * @param[in] handle_id The Janus ICE handle ID
 * @returns The created Janus ICE handle if successful, NULL otherwise */
janus_ice_handle *janus_ice_handle_find(void *gateway_session, guint64 handle_id);

/*! \brief ICE对象与网关实例关联
 * \details 这个方法非常重要，他允许插件给Webrtc对象交换数据
 * @param[in] handle_id The Janus ICE handle ID
 * @param[in] plugin 需要关联的插件指针
 * @returns 0 in case of success, a negative integer otherwise */
gint janus_ice_handle_attach_plugin(void *gateway_session, guint64 handle_id, janus_plugin *plugin);

/*! \brief 销毁ICE实例
 * @param[in] gateway_session ICE实例所属的Session对象指针
 * @param[in] handle_id 需要销毁的ICE实例ID
 * @returns 0 in case of success, a negative integer otherwise */
gint janus_ice_handle_destroy(void *gateway_session, guint64 handle_id);

/*! \brief Method to actually free the resources allocated by a Janus ICE handle
 * @param[in] handle The Janus ICE handle instance to free */
void janus_ice_free(janus_ice_handle *handle);

/*! \brief 挂断ICE实例创建的WebRTC PeerConnection
 * @param[in] handle 管理WebRTC PeerConnection的ICE实例
 * @param[in] reason A description of why this happened */
void janus_ice_webrtc_hangup(janus_ice_handle *handle, const char *reason);

/*! \brief 释放ICE实例创建有关WebRTC PeerConnection的资源
 * @param[in] handle The Janus ICE handle instance managing the WebRTC resources to free */
void janus_ice_webrtc_free(janus_ice_handle *handle);

/*! \brief 释放ICE实例创建ICE stream实例
 * @param[in] stream The Janus ICE stream instance to free */
void janus_ice_stream_free(janus_ice_stream *stream);

/*! \brief 释放ICE实例创建janus_ice_component
 * @param[in] component The Janus ICE component instance to free */
void janus_ice_component_free(janus_ice_component *component);
///@}


/** @name Janus ICE 媒体转发回调函数
 */
///@{ 
/*! \brief 网关RTP回调，当一个插件有一个RTP包发送给一个对端时调用
 * @param[in] handle ICE实例
 * @param[in] video 是否为视频帧
 * @param[in] buf The packet data (buffer)
 * @param[in] len The buffer lenght */
void janus_ice_relay_rtp(janus_ice_handle *handle, int video, char *buf, int len);

/*! \brief 网关RTCP回调，当一个插件有一个RTCP包发送给一个对端时调用
 * @param[in] handle  handle ICE实例
 * @param[in] video 是否为视频流相关的
 * @param[in] buf The message data (buffer)
 * @param[in] len The buffer lenght */
void janus_ice_relay_rtcp(janus_ice_handle *handle, int video, char *buf, int len);

/*! \brief 网关SCTP/DataChannel回调，当一个插件有一个RTP包发送给一个对端时调用
 * @param[in] handle The Janus ICE handle associated with the peer
 * @param[in] buf The message data (buffer)
 * @param[in] len The buffer lenght */
void janus_ice_relay_data(janus_ice_handle *handle, char *buf, int len);

/*! \brief Plugin SCTP/DataChannel callback, called by the SCTP stack when when there's data for a plugin
 * @param[in] handle The Janus ICE handle associated with the peer
 * @param[in] buffer The message data (buffer)
 * @param[in] length The buffer lenght */
void janus_ice_incoming_data(janus_ice_handle *handle, char *buffer, int length);
///@}


/** @name Janus ICE handle helpers
 */
///@{
/*! \brief Janus ICE handle thread */
void *janus_ice_thread(void *data);

/*! \brief Janus ICE thread for sending outgoing packets */
void *janus_ice_send_thread(void *data);

/*! \brief 设置本地ICE候选人的方法 (initialization and gathering) 创建线程
 * @param[in] handle 指向ICE实例的指针 
 * @param[in] offer 指示是OFFER还是ANSWER
 * @param[in] audio 是否开启音频
 * @param[in] video 是否开启视频
 * @param[in] data  SCTP data channels是否被支持
 * @param[in] trickle Whether ICE trickling is supported or not
 * @returns 0 in case of success, a negative integer otherwise */
int janus_ice_setup_local(janus_ice_handle *handle, int offer, int audio, int video, int data, int trickle);

/*! \brief 在SDP对象中添加本地候选项的方法
 * @param[in] handle 指向ICE实例的指针
 * @param[in] mline SDP的m-line对象用于添加本地候选
 * @param[in] stream_id The stream ID of the candidate to add to the SDP
 * @param[in] component_id The component ID of the candidate to add to the SDP */
void janus_ice_candidates_to_sdp(janus_ice_handle *handle, janus_sdp_mline *mline, guint stream_id, guint component_id);

/*! \brief 用于处理对端候选者和开启连通性检测的方法
 * @param[in] handle 指向ICE实例的指针
 * @param[in] stream_id The stream ID of the candidate to add to the SDP
 * @param[in] component_id The component ID of the candidate to add to the SDP */
void janus_ice_setup_remote_candidates(janus_ice_handle *handle, guint stream_id, guint component_id);

/*! \brief Callback to be notified when the DTLS handshake for a specific component has been completed
 * \details This method also decides when to notify attached plugins about the availability of a reliable PeerConnection
 * @param[in] handle The Janus ICE handle this callback refers to
 * @param[in] component The Janus ICE component that is now ready to be used */
void janus_ice_dtls_handshake_done(janus_ice_handle *handle, janus_ice_component *component);

/*! \brief Method to restart ICE and the connectivity checks
 * @param[in] handle The Janus ICE handle this method refers to */
void janus_ice_restart(janus_ice_handle *handle);

/*! \brief Method to resend all the existing candidates via trickle (e.g., after an ICE restart)
 * @param[in] handle The Janus ICE handle this method refers to */
void janus_ice_resend_trickles(janus_ice_handle *handle);

///@}

#endif
