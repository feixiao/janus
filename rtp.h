/*! \file    rtp.h
 * \author   Lorenzo Miniero <lorenzo@meetecho.com>
 * \copyright GNU General Public License v3
 * \brief    RTP processing (headers)
 * \details  Implementation of the RTP header. Since the gateway does not
 * much more than relaying frames around, the only thing we're interested
 * in is the RTP header and how to get its payload, and parsing extensions.
 * 
 * \ingroup protocols
 * \ref protocols
 */
 
#ifndef _JANUS_RTP_H
#define _JANUS_RTP_H

#include <arpa/inet.h>
#ifdef __MACH__
#include <machine/endian.h>
#define __BYTE_ORDER BYTE_ORDER
#define __BIG_ENDIAN BIG_ENDIAN
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#else
#include <endian.h>
#endif
#include <inttypes.h>
#include <string.h>
#include <glib.h>

#define RTP_HEADER_SIZE	12

/*! \brief RTP Header (http://tools.ietf.org/html/rfc3550#section-5.1) */
typedef struct rtp_header
{
#if __BYTE_ORDER == __BIG_ENDIAN
	uint16_t version:2;		// 2比特 此域定义了RTP的版本。此协议定义的版本是2
	uint16_t padding:1;		// 1比特 若填料比特被设置，则此包包含一到多个附加在末端的填充比特，填充比特不算作负载的一部分。填充的最后一个字节指明可以忽略多少个填充比特。
	uint16_t extension:1;	// 1比特 若设置扩展比特，固定头(仅)后面跟随一个头扩展。
	uint16_t csrccount:4;	// 4比特 CSRC计数包含了跟在固定头后面CSRC识别符的数目。
	uint16_t markerbit:1;	// 1比特 标志的解释由具体协议规定。
	uint16_t type:7;		// 7比特 此域定义了负载的格式，由具体应用决定其解释
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	uint16_t csrccount:4;
	uint16_t extension:1;
	uint16_t padding:1;
	uint16_t version:2;
	uint16_t type:7;
	uint16_t markerbit:1;
#endif
	uint16_t seq_number;	// 16比特 每发送一个RTP数据包，序列号加1，接收端可以据此检测丢包和重建包序列
	uint32_t timestamp;		// 32比特时间戳反映了RTP数据包中第一个字节的采样时间
	uint32_t ssrc;			// 32比特 用以识别同步源。
	uint32_t csrc[16];		// 0到15项，每项32比特 CSRC列表识别在此包中负载的所有贡献源。
} rtp_header;
typedef rtp_header janus_rtp_header;

/*! \brief RTP packet */
typedef struct janus_rtp_packet {
	char *data;
	gint length;
	gint64 created;
	gint64 last_retransmit;
} janus_rtp_packet;

/*! \brief RTP extension */
typedef struct janus_rtp_header_extension {
	uint16_t type;				// defined by profile
	uint16_t length;			// 16比特的长度域，指示扩展项中32比特字的个数，不包括4个字节扩展头(
} janus_rtp_header_extension;

/*! \brief a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level */
#define JANUS_RTP_EXTMAP_AUDIO_LEVEL		"urn:ietf:params:rtp-hdrext:ssrc-audio-level"   // 指出在rtp头部中加入音量信息，参考 rfc6464
/*! \brief a=extmap:2 urn:ietf:params:rtp-hdrext:toffset */
#define JANUS_RTP_EXTMAP_TOFFSET			"urn:ietf:params:rtp-hdrext:toffset"
/*! \brief a=extmap:3 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time */
#define JANUS_RTP_EXTMAP_ABS_SEND_TIME		"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time"
/*! \brief a=extmap:4 urn:3gpp:video-orientation */
#define JANUS_RTP_EXTMAP_VIDEO_ORIENTATION	"urn:3gpp:video-orientation"
/*! \brief a=extmap:5 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01 */
#define JANUS_RTP_EXTMAP_TRANSPORT_WIDE_CC	"http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01"
/*! \brief a=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay */
#define JANUS_RTP_EXTMAP_PLAYOUT_DELAY		"http://www.webrtc.org/experiments/rtp-hdrext/playout-delay"
/*! \brief a=extmap:3/sendonly urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id */
#define JANUS_RTP_EXTMAP_RTP_STREAM_ID		"urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id"

/*! \brief Helper to quickly access the RTP payload, skipping header and extensions
 * @param[in] buf The packet data
 * @param[in] len The packet data length in bytes
 * @param[out] plen The payload data length in bytes
 * @returns A pointer to where the payload data starts, or NULL otherwise; plen is also set accordingly */
char *janus_rtp_payload(char *buf, int len, int *plen);

/*! \brief Ugly and dirty helper to quickly get the id associated with an RTP extension (extmap) in an SDP
 * @param sdp The SDP to parse
 * @param extension The extension namespace to look for
 * @returns The extension id, if found, -1 otherwise */
int janus_rtp_header_extension_get_id(const char *sdp, const char *extension);

/*! \brief Ugly and dirty helper to quickly get the RTP extension namespace associated with an id (extmap) in an SDP
 * @note This only looks for the extensions we know about, those defined in rtp.h
 * @param sdp The SDP to parse
 * @param id The extension id to look for
 * @returns The extension namespace, if found, NULL otherwise */
const char *janus_rtp_header_extension_get_from_id(const char *sdp, int id);

/*! \brief Helper to parse a ssrc-audio-level RTP extension (https://tools.ietf.org/html/rfc6464)
 * @param[in] buf The packet data
 * @param[in] len The packet data length in bytes
 * @param[in] id The extension ID to look for
 * @param[out] level The level value in dBov (0=max, 127=min)
 * @returns 0 if found, -1 otherwise */
int janus_rtp_header_extension_parse_audio_level(char *buf, int len, int id, int *level);

/*! \brief Helper to parse a video-orientation RTP extension (http://www.3gpp.org/ftp/Specs/html-info/26114.htm)
 * @param[in] buf The packet data
 * @param[in] len The packet data length in bytes
 * @param[in] id The extension ID to look for
 * @param[out] c The value of the Camera (C) bit
 * @param[out] f The value of the Flip (F) bit
 * @param[out] r1 The value of the first Rotation (R1) bit
 * @param[out] r0 The value of the second Rotation (R0) bit
 * @returns 0 if found, -1 otherwise */
int janus_rtp_header_extension_parse_video_orientation(char *buf, int len, int id,
	gboolean *c, gboolean *f, gboolean *r1, gboolean *r0);

/*! \brief Helper to parse a playout-delay RTP extension (https://webrtc.org/experiments/rtp-hdrext/playout-delay)
 * @param[in] buf The packet data
 * @param[in] len The packet data length in bytes
 * @param[in] id The extension ID to look for
 * @param[out] min_delay The minimum delay value
 * @param[out] max_delay The maximum delay value
 * @returns 0 if found, -1 otherwise */
int janus_rtp_header_extension_parse_playout_delay(char *buf, int len, int id,
	uint16_t *min_delay, uint16_t *max_delay);

/*! \brief 帮助解析rtp-stream-id RTP扩展 (https://tools.ietf.org/html/draft-ietf-avtext-rid-09)
 * @param[in] buf The packet data
 * @param[in] len The packet data length in bytes
 * @param[in] id The extension ID to look for
 * @param[out] sdes_item Buffer where the RTP stream ID will be written
 * @param[in] sdes_len Size of the input/output buffer
 * @returns 0 if found, -1 otherwise */
int janus_rtp_header_extension_parse_rtp_stream_id(char *buf, int len, int id,
	char *sdes_item, int sdes_len);

/*! \brief Helper to parse a rtp-stream-id RTP extension (https://tools.ietf.org/html/draft-ietf-avtext-rid-09)
 * @param[in] buf The packet data
 * @param[in] len The packet data length in bytes
 * @param[in] id The extension ID to look for
 * @param[out] transport wide sequence number
 * @returns 0 if found, -1 otherwise */
int janus_rtp_header_extension_parse_transport_wide_cc(char *buf, int len, int id,
	uint16_t *transSeqNum);

/*! \brief RTP context, in order to make sure SSRC changes result in coherent seq/ts increases */
typedef struct janus_rtp_switching_context {
	uint32_t a_last_ssrc, a_last_ts, a_base_ts, a_base_ts_prev, a_prev_ts, a_target_ts, a_start_ts,
			v_last_ssrc, v_last_ts, v_base_ts, v_base_ts_prev, v_prev_ts, v_target_ts, v_start_ts;
	uint16_t a_last_seq, a_prev_seq, a_base_seq, a_base_seq_prev,
			v_last_seq, v_prev_seq, v_base_seq, v_base_seq_prev;
	gboolean a_seq_reset, a_new_ssrc,
			v_seq_reset, v_new_ssrc;
	gint16 a_seq_offset,
			v_seq_offset;
	gint32 a_prev_delay, a_active_delay, a_ts_offset,
			v_prev_delay, v_active_delay, v_ts_offset;
	gint64 a_last_time, a_reference_time, a_start_time,
			v_last_time, v_reference_time, v_start_time;
} janus_rtp_switching_context;

/*! \brief Set (or reset) the context fields to their default values
 * @param[in] context The context to (re)set */
void janus_rtp_switching_context_reset(janus_rtp_switching_context *context);

/*! \brief Use the context info to update the RTP header of a packet, if needed
 * @param[in] header The RTP header to update
 * @param[in] context The context to use as a reference
 * @param[in] video Whether this is an audio or a video packet
 * @param[in] step \b deprecated The expected timestamp step */
void janus_rtp_header_update(janus_rtp_header *header, janus_rtp_switching_context *context, gboolean video, int step);

#define RTP_AUDIO_SKEW_TH_MS 40
#define RTP_VIDEO_SKEW_TH_MS 40
#define SKEW_DETECTION_WAIT_TIME_SECS 15

/*! \brief Use the context info to compensate for audio source skew, if needed
 * @param[in] header The RTP header to update
 * @param[in] context The context to use as a reference
 * @param[in] now \b The packet arrival monotonic time
 * @returns 0 if no compensation is needed, -N if a N packets drop must be performed, N if a N sequence numbers jump has been performed */
int janus_rtp_skew_compensate_audio(janus_rtp_header *header, janus_rtp_switching_context *context, gint64 now);
/*! \brief Use the context info to compensate for video source skew, if needed
 * @param[in] header The RTP header to update
 * @param[in] context The context to use as a reference
 * @param[in] now \b The packet arrival monotonic time
 * @returns 0 if no compensation is needed, -N if a N packets drop must be performed, N if a N sequence numbers jump has been performed */
int janus_rtp_skew_compensate_video(janus_rtp_header *header, janus_rtp_switching_context *context, gint64 now);

#endif
