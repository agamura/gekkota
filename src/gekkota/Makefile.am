INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/gekkota

AM_CFLAGS = \
	-std=c99 \
	-pedantic

if CRC32_ENABLED
gekkota_headers_crc32 = \
	gekkota_crc32.h
gekkota_sources_crc32 = \
	gekkota_crc32.c
endif

if !HAVE_NATIVE_IDN
gekkota_headers_idn = \
	gekkota_idn.h
gekkota_sources_unix_idn = \
	gekkota_idn_unix.c
endif

gekkota_headers_public = \
	gekkota.h \
	gekkota_bit.h \
	gekkota_buffer.h \
	gekkota_dns.h \
	gekkota_errors.h \
	gekkota_event.h \
	gekkota_memory.h \
	gekkota_ipaddress.h \
	gekkota_ipendpoint.h \
	gekkota_iphostentry.h \
	gekkota_list.h \
	gekkota_module.h \
	gekkota_networkinterface.h \
	gekkota_packet.h \
	gekkota_platform.h \
	gekkota_socket.h \
	gekkota_string.h \
	gekkota_time.h \
	gekkota_types.h \
	gekkota_xudp.h \
	gekkota_xudpclient.h

gekkota_headers = \
	$(gekkota_headers_public) \
	$(gekkota_headers_crc32) \
	$(gekkota_headers_idn) \
	gekkota_event_internal.h \
	gekkota_internal.h \
	gekkota_ipaddress_internal.h \
	gekkota_ipendpoint_internal.h \
	gekkota_iphostentry_internal.h \
	gekkota_lzf.h \
	gekkota_networkinterface_internal.h \
	gekkota_packet_internal.h \
	gekkota_platform_internal.h \
	gekkota_socket_internal.h \
	gekkota_string_internal.h \
	gekkota_utils.h \
	gekkota_xudp_internal.h \
	gekkota_xudpclient_internal.h

gekkota_sources_unix = \
	$(gekkota_sources_unix_idn) \
	gekkota_ipaddress_unix.c \
	gekkota_iphostentry_unix.c \
	gekkota_module_unix.c \
	gekkota_networkinterface_unix.c \
	gekkota_platform_unix.c \
	gekkota_socket_unix.c \
	gekkota_string_unix.c \
	gekkota_time_unix.c \
	gekkota_unix.c

gekkota_sources_win32 = \
	gekkota_idn_win32.c \
	gekkota_ipaddress_win32.c \
	gekkota_iphostentry_win32.c \
	gekkota_module_win32.c \
	gekkota_networkinterface_win32.c \
	gekkota_platform_win32.c \
	gekkota_socket_win32.c \
	gekkota_string_win32.c \
	gekkota_time_win32.c \
	gekkota_win32.c

gekkota_sources = \
	$(gekkota_sources_unix) \
	$(gekkota_sources_crc32) \
	gekkota.c \
	gekkota_buffer.c \
	gekkota_dns.c \
	gekkota_event.c \
	gekkota_memory.c \
	gekkota_ipaddress.c \
	gekkota_ipendpoint.c \
	gekkota_iphostentry.c \
	gekkota_list.c \
	gekkota_lzf.c \
	gekkota_packet.c \
	gekkota_platform.c \
	gekkota_socket.c \
	gekkota_string.c \
	gekkota_time.c \
	gekkota_xudp.c \
	gekkota_xudpclient.c

gekkotaincludedir = $(includedir)/gekkota
gekkotainclude_HEADERS = $(gekkota_headers_public)

libgekkota_la_SOURCES = \
	$(gekkota_headers) \
	$(gekkota_sources)

lib_LTLIBRARIES = libgekkota.la

EXTRA_DIST = \
	$(gekkota_sources_win32) \
	resource.h \
	gekkota.rc \
	gekkota.aps \
	gekkota.sln \
	gekkota.vcproj
