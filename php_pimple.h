/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Axel Etcheverry <axel@etcheverry.biz>                        |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_PIMPLE_H
#define PHP_PIMPLE_H

#define PIMPLE_VERSION "1.0"

extern ZEND_API zend_class_entry *pimple_closure_ce;

extern zend_module_entry pimple_module_entry;
#define phpext_pimple_ptr &pimple_module_entry

#ifdef PHP_WIN32
#	define PHP_PIMPLE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_PIMPLE_API __attribute__ ((visibility("default")))
#else
#	define PHP_PIMPLE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(pimple);
PHP_MSHUTDOWN_FUNCTION(pimple);
PHP_RINIT_FUNCTION(pimple);
PHP_RSHUTDOWN_FUNCTION(pimple);
PHP_MINFO_FUNCTION(pimple);

PHP_METHOD(Pimple, __construct);
PHP_METHOD(Pimple, offsetSet);
PHP_METHOD(Pimple, offsetGet);
PHP_METHOD(Pimple, offsetExists);
PHP_METHOD(Pimple, offsetUnset);
PHP_METHOD(Pimple, share);
PHP_METHOD(Pimple, protect);
PHP_METHOD(Pimple, raw);
PHP_METHOD(Pimple, extend);
PHP_METHOD(Pimple, keys);

PHP_METHOD(PimpleClosure, __invoke);


#ifdef ZTS
#define PIMPLE_G(v) TSRMG(pimple_globals_id, zend_pimple_globals *, v)
#else
#define PIMPLE_G(v) (pimple_globals.v)
#endif

#endif	/* PHP_PIMPLE_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
