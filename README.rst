.. image:: https://travis-ci.org/xcir/libvmod-otp.svg?branch=master
    :target: https://travis-ci.org/xcir/libvmod-otp

===================
vmod_otp
===================

-------------------------------
Varnish OTP(HOTP/TOTP) module
-------------------------------

:Author: Shohei Tanaka(@xcir)
:Date: 2017-08-22
:Version: 51.2
:Support Varnish Version: 5.1.x, 5.2.x, 6.0.x 6.1.x, 6.2.x
:Manual section: 3

SYNOPSIS
========

import otp;

DESCRIPTION
===========

Generate one-time password(HOTP/TOTP) token.


ATTENTION
=========

This vmod only works at a 64 bit environment.
Time synchronization is required with ntp etc.

FUNCTIONS
=========

hotp
-----

Prototype
        ::

                hotp(STRING b32_secret, INT count, INT digit=6, ENUM { md4, md5, sha1, sha256, sha512 } alg="sha1")
Return value
	STRING
Description
	Generate HOTP(RFC4226) token.
Example
        ::

                if(req.http.x-example-otp != otp.hotp(
                    b32_secret = "GEZDGNBVGY3TQOJQGEZDGNBVGY3TQOJQ", //base32 encoded secret value
                    count      = 1,                                  //counter value
                    digit      = 6,                                  //number of digits
                    alg        = sha1                                //hash algorithm
                )){
                  return (synth(401));
                }

totp
-----

Prototype
        ::

                totp(STRING b32_secret, INT step=30, INT digit=6, ENUM { md4, md5, sha1, sha256, sha512 } alg="sha1")
Return value
	STRING
Description
	Generate TOTP(RFC6238) token.
	Require time sync.(use ntp...)
Example
        ::

                if(req.http.x-example-otp != otp.totp(
                    b32_secret = "GEZDGNBVGY3TQOJQGEZDGNBVGY3TQOJQ", //base32 encoded secret value
                    step       = 30,                                 //time step in seconds
                    digit      = 6,                                  //number of digits
                    alg        = sha1                                //hash algorithm
                )){
                  return (synth(401));
                }
totp_settime
---------------

Prototype
        ::

                totp_settime(STRING b32_secret, REAL time, INT step=30, INT digit=6, ENUM { md4, md5, sha1, sha256, sha512 } alg="sha1")
Return value
	STRING
Description
	Generate TOTP(RFC6238) token.
	This function used for tests.
Example
        ::

                if(req.http.x-example-otp != otp.totp(
                    b32_secret = "GEZDGNBVGY3TQOJQGEZDGNBVGY3TQOJQ", //base32 encoded secret value
                    time       = 1502380366.0,                       //unixtime
                    step       = 30,                                 //time step in seconds
                    digit      = 6,                                  //number of digits
                    alg        = sha1                                //hash algorithm
                )){
                  return (synth(401));
                }

INSTALLATION
============

The source tree is based on autotools to configure the building, and
does also have the necessary bits in place to do functional unit tests
using the ``varnishtest`` tool.

Building requires the Varnish header files and uses pkg-config to find
the necessary paths.

Usage::

 ./autogen.sh
 ./configure

If you have installed Varnish to a non-standard directory, call
``autogen.sh`` and ``configure`` with ``PKG_CONFIG_PATH`` pointing to
the appropriate path. For instance, when varnishd configure was called
with ``--prefix=$PREFIX``, use

::

 export PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig
 export ACLOCAL_PATH=${PREFIX}/share/aclocal

The module will inherit its prefix from Varnish, unless you specify a
different ``--prefix`` when running the ``configure`` script for this
module.

Make targets:

* make - builds the vmod.
* make install - installs your vmod.
* make check - runs the unit tests in ``src/tests/*.vtc``.
* make distcheck - run check and prepare a tarball of the vmod.

If you build a dist tarball, you don't need any of the autotools or
pkg-config. You can build the module simply by running::

 ./configure
 make

Installation directories
------------------------

By default, the vmod ``configure`` script installs the built vmod in the
directory relevant to the prefix. The vmod installation directory can be
overridden by passing the ``vmoddir`` variable to ``make install``.


COMMON PROBLEMS
===============


* configure: error: Need varnish.m4 -- see README.rst

  Check whether ``PKG_CONFIG_PATH`` and ``ACLOCAL_PATH`` were set correctly
  before calling ``autogen.sh`` and ``configure``

* Incompatibilities with different Varnish Cache versions

  Make sure you build this vmod against its correspondent Varnish Cache version.
  For instance, to build against Varnish Cache 4.1, this vmod must be built from
  branch 4.1.
