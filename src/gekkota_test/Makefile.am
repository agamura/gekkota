INCLUDES = \
	-UGEKKOTA_BUILDING_LIB \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/gekkota \
	-I$(top_srcdir)/src/gekkota_test

AM_CFLAGS = \
	-std=c99 \
	-pedantic

LDADD = \
	../gekkota/libgekkota.la

gekkota_test_client_headers =

gekkota_test_server_headers =

gekkota_test_client_sources = \
	gekkota_test_client.c

gekkota_test_server_sources = \
	gekkota_test_server.c

gekkota_test_client_SOURCES = \
	$(gekkota_test_client_headers) \
	$(gekkota_test_client_sources)

gekkota_test_server_SOURCES = \
	$(gekkota_test_server_headers) \
	$(gekkota_test_server_sources)

bin_PROGRAMS = gekkota_test_client gekkota_test_server

EXTRA_DIST = \
	gekkota_test.sln \
	gekkota_test.vcproj
