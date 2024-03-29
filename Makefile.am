ACLOCAL_AMFLAGS = -I m4

# FIXME: These flags should be produced in configure.ac using AX_APPEND_COMPILE_FLAGS
AM_CFLAGS = \
	-Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations \
	-Wunused -fstrict-aliasing -Wextra -Wformat=2 -Winit-self -Winline \
	-Wpacked -Wpointer-arith -Wmissing-format-attribute -Wmissing-noreturn \
	-Wnested-externs -Wstrict-prototypes -Wunsafe-loop-optimizations \
	-Wwrite-strings -Wno-missing-field-initializers -Wno-unused-parameter \
	-Wcast-align -Wformat-nonliteral -Wformat-security -Wswitch-default \
	-Wmissing-include-dirs -Wno-aggregate-return -Wunused-but-set-variable \
	-Warray-bounds -Wold-style-definition -Wsign-compare -Wlarger-than=65537
AM_CFLAGS += -Wno-undef  # sofiasip is not -Wundef--safe
AM_CFLAGS += -Wno-redundant-decls  # sofiasip also contains redundant declarations
AM_CFLAGS += -Wno-override-init  # we need this for JANUS_PLUGIN_INIT and JANUS_TRANSPORT_INIT and JANUS_EVENTHANDLER_INIT
# FIXME: These should be enabled once the code is safe for them. That requires
# some fairly big refactoring though, which can wait.
# AM_CFLAGS += -Wshadow -Wstrict-aliasing=2

AM_CFLAGS += -fstack-protector-all -g -ggdb -fPIC -rdynamic -pthread

# FIXME: make docs work with distcheck
DISTCHECK_CONFIGURE_FLAGS = --disable-docs --enable-post-processing

EXTRA_DIST = $(NULL)
CLEANFILES = $(NULL)

bin_PROGRAMS = janus

headerdir = $(includedir)/janus
header_HEADERS = apierror.h config.h log.h debug.h mutex.h record.h \
	rtcp.h rtp.h rtpsrtp.h sdp-utils.h ip-utils.h utils.h text2pcap.h

pluginsheaderdir = $(includedir)/janus/plugins
pluginsheader_HEADERS = plugins/plugin.h

transportsheaderdir = $(includedir)/janus/transports
transportsheader_HEADERS = transports/transport.h

eventsheaderdir = $(includedir)/janus/events
eventsheader_HEADERS = events/eventhandler.h

confdir = $(sysconfdir)/janus
conf_DATA = conf/janus.cfg.sample

certdir = $(datadir)/janus/certs
cert_DATA = certs/mycert.key certs/mycert.pem

plugindir = $(libdir)/janus/plugins
plugin_LTLIBRARIES = $(NULL)

transportdir = $(libdir)/janus/transports
transport_LTLIBRARIES = $(NULL)

eventdir = $(libdir)/janus/events
event_LTLIBRARIES = $(NULL)

SUBDIRS = html
html_DATA = README.md

streamdir = $(datadir)/janus/streams
stream_DATA = $(NULL)

recordingsdir = $(datadir)/janus/recordings
recordings_DATA = $(NULL)

demosdir = $(datadir)/janus/demos
demos_DATA = $(NULL)

jsmodulesdir = $(datadir)/janus/javascript
jsmodules_DATA = html/janus.js

%.sample: %.sample.in
	$(MKDIR_P) $(@D)
	$(AM_V_GEN) sed -e "\
	    s|[@]confdir[@]|$(confdir)|;\
	    s|[@]certdir[@]|$(certdir)|;\
	    s|[@]plugindir[@]|$(plugindir)|;\
	    s|[@]transportdir[@]|$(transportdir)|;\
	    s|[@]eventdir[@]|$(eventdir)|;\
	    s|[@]recordingsdir[@]|$(recordingsdir)|;\
	    s|[@]demosdir[@]|$(demosdir)|;\
	    s|[@]streamdir[@]|$(streamdir)|" \
	$< > $@ || rm $@

EXTRA_DIST += conf/janus.cfg.sample.in
CLEANFILES += conf/janus.cfg.sample

##
# Janus
##

janus_SOURCES = \
	apierror.c \
	apierror.h \
	auth.c \
	auth.h \
	cmdline.c \
	cmdline.h \
	config.c \
	config.h \
	debug.h \
	dtls.c \
	dtls.h \
	dtls-bio.c \
	dtls-bio.h \
	events.c \
	events.h \
	ice.c \
	ice.h \
	janus.c \
	janus.h \
	log.c \
	log.h \
	mutex.h \
	record.c \
	record.h \
	rtcp.c \
	rtcp.h \
	rtp.c \
	rtp.h \
	rtpsrtp.h \
	sctp.c \
	sctp.h \
	sdp.c \
	sdp.h \
	sdp-utils.c \
	sdp-utils.h \
	ip-utils.c \
	ip-utils.h \
	turnrest.c \
	turnrest.h \
	utils.c \
	utils.h \
	version.c \
	version.h \
	text2pcap.c \
	text2pcap.h \
	plugins/plugin.c \
	plugins/plugin.h \
	transports/transport.h \
	events/eventhandler.h \
	$(NULL)

janus_CFLAGS = \
	$(AM_CFLAGS) \
	$(JANUS_CFLAGS) \
	-DPLUGINDIR=\"$(plugindir)\" \
	-DTRANSPORTDIR=\"$(transportdir)\" \
	-DEVENTDIR=\"$(eventdir)\" \
	-DCONFDIR=\"$(confdir)\" \
	$(BORINGSSL_CFLAGS) \
	$(NULL)

janus_LDADD = \
	$(BORINGSSL_LIBS) \
	$(JANUS_LIBS) \
	$(JANUS_MANUAL_LIBS) \
	$(NULL)

man1_MANS = janus.1

BUILT_SOURCES = cmdline.c cmdline.h version.c

cmdline.c: janus.c
	gengetopt --set-package="janus" --set-version="$(VERSION)" < janus.ggo

directory = .git
dir_target = $(directory)-$(wildcard $(directory))
dir_present = $(directory)-$(directory)
dir_absent = $(directory)-

if WITH_SOURCE_DATE_EPOCH
build_date = $(shell LC_ALL=C date --utc --date="@$(SOURCE_DATE_EPOCH)")
else
build_date = $(shell date)
endif

version.c: FORCE | $(dir_target)
	echo "$(build_date)" | awk 'BEGIN {} {print "const char *janus_build_git_time = \""$$0"\";"} END {} ' >> version.c
	echo "$(JANUS_VERSION)" | awk 'BEGIN {} {print "int janus_version = "$$0";"} END {} ' >> version.c
	echo "$(JANUS_VERSION_STRING)" | awk 'BEGIN {} {print "const char *janus_version_string = \""$$0"\";"} END {} ' >> version.c

$(dir_present):
	`which git` rev-parse HEAD | awk 'BEGIN {print "#include \"version.h\""} {print "const char *janus_build_git_sha = \"" $$0"\";"} END {}' > version.c

$(dir_absent):
	echo "not-a-git-repo" | awk 'BEGIN {print "#include \"version.h\""} {print "const char *janus_build_git_sha = \"" $$0"\";"} END {}' > version.c

.PHONY: FORCE
FORCE:

EXTRA_DIST += janus.ggo
CLEANFILES += cmdline.c cmdline.h version.c

##
# Transports
##

transports_cflags = \
	$(AM_CFLAGS) \
	$(TRANSPORTS_CFLAGS) \
	$(NULL)

transports_libadd = \
	$(TRANSPORTS_LIBS) \
	$(NULL)

if ENABLE_REST
transport_LTLIBRARIES += transports/libjanus_http.la
transports_libjanus_http_la_SOURCES = transports/janus_http.c
transports_libjanus_http_la_CFLAGS = $(transports_cflags) $(MHD_CFLAGS)
transports_libjanus_http_la_LDFLAGS = $(transports_ldflags) $(MHD_LDFLAGS) $(MHD_LIBS)
transports_libjanus_http_la_LIBADD = $(transports_libadd) $(MHD_LDFLAGS)
conf_DATA += conf/janus.transport.http.cfg.sample
EXTRA_DIST += conf/janus.transport.http.cfg.sample.in
CLEANFILES += conf/janus.transport.http.cfg.sample
endif

if ENABLE_WEBSOCKETS
transport_LTLIBRARIES += transports/libjanus_websockets.la
transports_libjanus_websockets_la_SOURCES = transports/janus_websockets.c
transports_libjanus_websockets_la_CFLAGS = $(transports_cflags)
transports_libjanus_websockets_la_LDFLAGS = $(transports_ldflags) $(WS_MANUAL_LIBS)
transports_libjanus_websockets_la_LIBADD = $(transports_libadd)
conf_DATA += conf/janus.transport.websockets.cfg.sample
EXTRA_DIST += conf/janus.transport.websockets.cfg.sample.in
CLEANFILES += conf/janus.transport.websockets.cfg.sample
endif

if ENABLE_RABBITMQ
transport_LTLIBRARIES += transports/libjanus_rabbitmq.la
transports_libjanus_rabbitmq_la_SOURCES = transports/janus_rabbitmq.c
transports_libjanus_rabbitmq_la_CFLAGS = $(transports_cflags)
transports_libjanus_rabbitmq_la_LDFLAGS = $(transports_ldflags) -lrabbitmq
transports_libjanus_rabbitmq_la_LIBADD = $(transports_libadd)
conf_DATA += conf/janus.transport.rabbitmq.cfg.sample
EXTRA_DIST += conf/janus.transport.rabbitmq.cfg.sample
endif

if ENABLE_MQTT
transport_LTLIBRARIES += transports/libjanus_mqtt.la
transports_libjanus_mqtt_la_SOURCES = transports/janus_mqtt.c
transports_libjanus_mqtt_la_CFLAGS = $(transports_cflags)
transports_libjanus_mqtt_la_LDFLAGS = $(transports_ldflags) -lpaho-mqtt3as
transports_libjanus_mqtt_la_LIBADD = $(transports_libadd)
conf_DATA += conf/janus.transport.mqtt.cfg.sample
EXTRA_DIST += conf/janus.transport.mqtt.cfg.sample
endif

if ENABLE_PFUNIX
transport_LTLIBRARIES += transports/libjanus_pfunix.la
transports_libjanus_pfunix_la_SOURCES = transports/janus_pfunix.c
transports_libjanus_pfunix_la_CFLAGS = $(transports_cflags)
transports_libjanus_pfunix_la_LDFLAGS = $(transports_ldflags)
transports_libjanus_pfunix_la_LIBADD = $(transports_libadd)
conf_DATA += conf/janus.transport.pfunix.cfg.sample
EXTRA_DIST += conf/janus.transport.pfunix.cfg.sample
endif

##
# Event handlers
##

events_cflags = \
	$(AM_CFLAGS) \
	$(EVENTS_CFLAGS) \
	$(NULL)

events_libadd = \
	$(EVENTS_LIBS) \
	$(NULL)

if ENABLE_SAMPLEEVH
event_LTLIBRARIES += events/libjanus_sampleevh.la
events_libjanus_sampleevh_la_SOURCES = events/janus_sampleevh.c
events_libjanus_sampleevh_la_CFLAGS = $(events_cflags)
events_libjanus_sampleevh_la_LDFLAGS = $(events_ldflags) -lcurl
events_libjanus_sampleevh_la_LIBADD = $(events_libadd)
conf_DATA += conf/janus.eventhandler.sampleevh.cfg.sample
EXTRA_DIST += conf/janus.eventhandler.sampleevh.cfg.sample
endif

if ENABLE_RABBITMQEVH
event_LTLIBRARIES += events/libjanus_rabbitmqevh.la
events_libjanus_rabbitmqevh_la_SOURCES = events/janus_rabbitmqevh.c
events_libjanus_rabbitmqevh_la_CFLAGS = $(events_cflags)
events_libjanus_rabbitmqevh_la_LDFLAGS = $(events_ldflags) -lrabbitmq
events_libjanus_rabbitmqevh_la_LIBADD = $(events_libadd)
conf_DATA += conf/janus.eventhandler.rabbitmqevh.cfg.sample
EXTRA_DIST += conf/janus.eventhandler.rabbitmqevh.cfg.sample
endif

##
# Plugins
##

plugins_cflags = \
	$(AM_CFLAGS) \
	$(PLUGINS_CFLAGS) \
	$(NULL)

plugins_libadd = \
	$(PLUGINS_LIBS) \
	$(NULL)

if ENABLE_PLUGIN_AUDIOBRIDGE
plugin_LTLIBRARIES += plugins/libjanus_audiobridge.la
plugins_libjanus_audiobridge_la_SOURCES = plugins/janus_audiobridge.c
plugins_libjanus_audiobridge_la_CFLAGS = $(plugins_cflags) $(OPUS_CFLAGS)
plugins_libjanus_audiobridge_la_LDFLAGS = $(plugins_ldflags) $(OPUS_LDFLAGS) $(OPUS_LIBS)
plugins_libjanus_audiobridge_la_LIBADD = $(plugins_libadd) $(OPUS_LIBADD)
conf_DATA += conf/janus.plugin.audiobridge.cfg.sample
EXTRA_DIST += conf/janus.plugin.audiobridge.cfg.sample
endif

if ENABLE_PLUGIN_ECHOTEST
plugin_LTLIBRARIES += plugins/libjanus_echotest.la
plugins_libjanus_echotest_la_SOURCES = plugins/janus_echotest.c
plugins_libjanus_echotest_la_CFLAGS = $(plugins_cflags)
plugins_libjanus_echotest_la_LDFLAGS = $(plugins_ldflags)
plugins_libjanus_echotest_la_LIBADD = $(plugins_libadd)
conf_DATA += conf/janus.plugin.echotest.cfg.sample
EXTRA_DIST += conf/janus.plugin.echotest.cfg.sample
endif

if ENABLE_PLUGIN_RECORDPLAY
plugin_LTLIBRARIES += plugins/libjanus_recordplay.la
plugins_libjanus_recordplay_la_SOURCES = plugins/janus_recordplay.c
plugins_libjanus_recordplay_la_CFLAGS = $(plugins_cflags)
plugins_libjanus_recordplay_la_LDFLAGS = $(plugins_ldflags)
plugins_libjanus_recordplay_la_LIBADD = $(plugins_libadd)
conf_DATA += conf/janus.plugin.recordplay.cfg.sample
recordings_DATA += \
	plugins/recordings/1234.nfo \
	plugins/recordings/rec-sample-audio.mjr \
	plugins/recordings/rec-sample-video.mjr
EXTRA_DIST += \
	conf/janus.plugin.recordplay.cfg.sample.in \
	$(recordings_DATA)
CLEANFILES += conf/janus.plugin.recordplay.cfg.sample
endif

if ENABLE_PLUGIN_SIP
plugin_LTLIBRARIES += plugins/libjanus_sip.la
plugins_libjanus_sip_la_SOURCES = plugins/janus_sip.c
plugins_libjanus_sip_la_CFLAGS = $(plugins_cflags) $(SOFIA_CFLAGS)
plugins_libjanus_sip_la_LDFLAGS = $(plugins_ldflags) $(SOFIA_LDFLAGS) $(SOFIA_LIBS)
plugins_libjanus_sip_la_LIBADD = $(plugins_libadd) $(SOFIA_LIBADD)
conf_DATA += conf/janus.plugin.sip.cfg.sample
EXTRA_DIST += conf/janus.plugin.sip.cfg.sample
endif

if ENABLE_PLUGIN_SIPRE
plugin_LTLIBRARIES += plugins/libjanus_sipre.la
plugins_libjanus_sipre_la_SOURCES = plugins/janus_sipre.c
plugins_libjanus_sipre_la_CFLAGS = $(plugins_cflags) $(LIBRE_CFLAGS)
plugins_libjanus_sipre_la_LDFLAGS = $(plugins_ldflags) $(LIBRE_LDFLAGS) $(LIBRE_LIBS)
plugins_libjanus_sipre_la_LIBADD = $(plugins_libadd) $(LIBRE_LIBADD)
conf_DATA += conf/janus.plugin.sipre.cfg.sample
EXTRA_DIST += conf/janus.plugin.sipre.cfg.sample
endif

if ENABLE_PLUGIN_NOSIP
plugin_LTLIBRARIES += plugins/libjanus_nosip.la
plugins_libjanus_nosip_la_SOURCES = plugins/janus_nosip.c
plugins_libjanus_nosip_la_CFLAGS = $(plugins_cflags)
plugins_libjanus_nosip_la_LDFLAGS = $(plugins_ldflags)
plugins_libjanus_nosip_la_LIBADD = $(plugins_libadd)
conf_DATA += conf/janus.plugin.nosip.cfg.sample
EXTRA_DIST += conf/janus.plugin.nosip.cfg.sample
endif

if ENABLE_PLUGIN_STREAMING
plugin_LTLIBRARIES += plugins/libjanus_streaming.la
plugins_libjanus_streaming_la_SOURCES = plugins/janus_streaming.c
plugins_libjanus_streaming_la_CFLAGS = $(plugins_cflags)
plugins_libjanus_streaming_la_LDFLAGS = $(plugins_ldflags)
plugins_libjanus_streaming_la_LIBADD = $(plugins_libadd)
conf_DATA += conf/janus.plugin.streaming.cfg.sample
stream_DATA += \
	plugins/streams/music.mulaw \
	plugins/streams/radio.alaw \
	plugins/streams/test_gstreamer.sh \
	plugins/streams/test_gstreamer_1.sh
EXTRA_DIST += \
	conf/janus.plugin.streaming.cfg.sample.in \
	$(stream_DATA)
CLEANFILES += conf/janus.plugin.streaming.cfg.sample
endif

if ENABLE_PLUGIN_VIDEOCALL
plugin_LTLIBRARIES += plugins/libjanus_videocall.la
plugins_libjanus_videocall_la_SOURCES = plugins/janus_videocall.c
plugins_libjanus_videocall_la_CFLAGS = $(plugins_cflags)
plugins_libjanus_videocall_la_LDFLAGS = $(plugins_ldflags)
plugins_libjanus_videocall_la_LIBADD = $(plugins_libadd)
conf_DATA += conf/janus.plugin.videocall.cfg.sample
EXTRA_DIST += conf/janus.plugin.videocall.cfg.sample
endif

if ENABLE_PLUGIN_VIDEOROOM
plugin_LTLIBRARIES += plugins/libjanus_videoroom.la
plugins_libjanus_videoroom_la_SOURCES = plugins/janus_videoroom.c
plugins_libjanus_videoroom_la_CFLAGS = $(plugins_cflags)
plugins_libjanus_videoroom_la_LDFLAGS = $(plugins_ldflags)
plugins_libjanus_videoroom_la_LIBADD = $(plugins_libadd)
conf_DATA += conf/janus.plugin.videoroom.cfg.sample
EXTRA_DIST += conf/janus.plugin.videoroom.cfg.sample
endif

if ENABLE_PLUGIN_VOICEMAIL
plugin_LTLIBRARIES += plugins/libjanus_voicemail.la
plugins_libjanus_voicemail_la_SOURCES = plugins/janus_voicemail.c
plugins_libjanus_voicemail_la_CFLAGS = $(plugins_cflags)
plugins_libjanus_voicemail_la_LDFLAGS = $(plugins_ldflags) -logg
plugins_libjanus_voicemail_la_LIBADD = $(plugins_libadd)
conf_DATA += conf/janus.plugin.voicemail.cfg.sample
EXTRA_DIST += conf/janus.plugin.voicemail.cfg.sample.in
CLEANFILES += conf/janus.plugin.voicemail.cfg.sample
endif

if ENABLE_PLUGIN_TEXTROOM
plugin_LTLIBRARIES += plugins/libjanus_textroom.la
plugins_libjanus_textroom_la_SOURCES = plugins/janus_textroom.c
plugins_libjanus_textroom_la_CFLAGS = $(plugins_cflags)
plugins_libjanus_textroom_la_LDFLAGS = $(plugins_ldflags)
plugins_libjanus_textroom_la_LIBADD = $(plugins_libadd)
conf_DATA += conf/janus.plugin.textroom.cfg.sample
EXTRA_DIST += conf/janus.plugin.textroom.cfg.sample
endif

##
# Post-processing
##

if ENABLE_POST_PROCESSING
bin_PROGRAMS += janus-pp-rec
man1_MANS += postprocessing/janus-pp-rec.1

janus_pp_rec_SOURCES = \
	postprocessing/pp-g711.c \
	postprocessing/pp-g711.h \
	postprocessing/pp-g722.c \
	postprocessing/pp-g722.h \
	postprocessing/pp-h264.c \
	postprocessing/pp-h264.h \
	postprocessing/pp-opus.c \
	postprocessing/pp-opus.h \
	postprocessing/pp-opus-silence.h \
	postprocessing/pp-rtp.h \
	postprocessing/pp-srt.c \
	postprocessing/pp-srt.h \
	postprocessing/pp-webm.c \
	postprocessing/pp-webm.h \
	postprocessing/janus-pp-rec.c \
	log.c \
	version.c \
	$(NULL)

janus_pp_rec_CFLAGS = \
	$(AM_CFLAGS) \
	$(POST_PROCESSING_CFLAGS) \
	$(NULL)

janus_pp_rec_LDADD = \
	$(POST_PROCESSING_LIBS) \
	$(NULL)
endif

##
# Docs
##

if ENABLE_DOCS
SUBDIRS += docs
endif

##
# JavaScript module flavours for janus.js
##

if ENABLE_JAVASCRIPT_ES_MODULE
jsmodules_DATA += npm/bundles/janus.es.js
EXTRA_DIST += npm/bundles/janus.es.js
CLEANFILES += npm/bundles/janus.es.js
endif

if ENABLE_JAVASCRIPT_UMD_MODULE
jsmodules_DATA += npm/bundles/janus.umd.js
EXTRA_DIST += npm/bundles/janus.umd.js
CLEANFILES += npm/bundles/janus.umd.js
endif

if ENABLE_JAVASCRIPT_IIFE_MODULE
jsmodules_DATA += npm/bundles/janus.iife.js
EXTRA_DIST += npm/bundles/janus.iife.js
CLEANFILES += npm/bundles/janus.iife.js
endif

if ENABLE_JAVASCRIPT_COMMON_JS_MODULE
jsmodules_DATA += npm/bundles/janus.cjs.js
EXTRA_DIST += npm/bundles/janus.cjs.js
CLEANFILES += npm/bundles/janus.cjs.js
endif

if ENABLE_JAVASCRIPT_MODULES

npm/node_modules/rollup/bin/rollup: npm/package.json
	cd npm && $(NPM) install && touch node_modules/rollup/bin/rollup

npm/bundles/janus.%.js: html/janus.js npm/node_modules/rollup/bin/rollup npm/rollup.config.js npm/module.js
	cd npm && $(NPM) run rollup -- --o $(patsubst npm/%,%,$@) --f $*

endif

##
# Configuration
##

configs:
	$(MKDIR_P) $(DESTDIR)$(confdir)
	$(foreach config,$(conf_DATA),cp "$(CURDIR)/$(config)" "$(DESTDIR)$(confdir)/$(notdir $(basename $(config) .sample))";)

##
# Extra cleanup
##

clean-local:
	-rm -f docs/doxygen_sqlite3.db
