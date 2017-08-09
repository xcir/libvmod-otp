AM_CFLAGS = $(VARNISHAPI_CFLAGS) -Wall -Werror
AM_LDFLAGS = $(VARNISHAPI_LIBS) $(VMOD_LDFLAGS) -lmhash

vmod_LTLIBRARIES = libvmod_otp.la

libvmod_otp_la_SOURCES = \
	vmod_otp.c

nodist_libvmod_otp_la_SOURCES = \
	vcc_otp_if.c \
	vcc_otp_if.h

dist_man_MANS = vmod_otp.3

@BUILD_VMOD_OTP@

AM_TESTS_ENVIRONMENT = \
	PATH="$(VMOD_TEST_PATH)" \
	LD_LIBRARY_PATH="$(VARNISH_LIBRARY_PATH)"
TEST_EXTENSIONS = .vtc
VTC_LOG_COMPILER = varnishtest -v
AM_VTC_LOG_FLAGS = -Dvmod_otp="$(VMOD_OTP)"

TESTS = @VMOD_TESTS@

EXTRA_DIST = \
	vmod_otp.vcc \
	$(VMOD_TESTS)
