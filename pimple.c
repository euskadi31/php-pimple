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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "php_pimple.h"

#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "zend_closures.h"

#include "ext/standard/info.h"
#include "ext/spl/spl_exceptions.h"

#include "pimple_closure.h"

//ZEND_DECLARE_MODULE_GLOBALS(pimple)


static zend_class_entry *pimple_ce;
static zend_class_entry *pimple_closure_ce;


ZEND_BEGIN_ARG_INFO(arginfo_pimple_construct, 0)
    ZEND_ARG_INFO(0, values)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_pimple_offsetset, 0)
    ZEND_ARG_INFO(0, id)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_pimple_offsetget, 0)
    ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_pimple_offsetexists, 0)
    ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_pimple_offsetunset, 0)
    ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_pimple_share, 0)
    ZEND_ARG_INFO(0, callable)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_pimple_protect, 0)
    ZEND_ARG_INFO(0, callable)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_pimple_raw, 0)
    ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_pimple_extend, 0)
    ZEND_ARG_INFO(0, id)
    ZEND_ARG_INFO(0, callable)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_pimple_keys, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_pimple_closure_invoke, 0)
    ZEND_ARG_INFO(0, c)
ZEND_END_ARG_INFO()


/* {{{ pimple_functions[]
 *
 * Every user visible function must have an entry in pimple_functions[].
 */
const zend_function_entry pimple_functions[] = {
    PHP_ME(Pimple, __construct,     arginfo_pimple_construct,       ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(Pimple, offsetSet,       arginfo_pimple_offsetset,       ZEND_ACC_PUBLIC)
    PHP_ME(Pimple, offsetGet,       arginfo_pimple_offsetget,       ZEND_ACC_PUBLIC)
    PHP_ME(Pimple, offsetExists,    arginfo_pimple_offsetexists,    ZEND_ACC_PUBLIC)
    PHP_ME(Pimple, offsetUnset,     arginfo_pimple_offsetunset,     ZEND_ACC_PUBLIC)
    PHP_ME(Pimple, share,           arginfo_pimple_share,           ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(Pimple, protect,         arginfo_pimple_protect,         ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(Pimple, raw,             arginfo_pimple_raw,             ZEND_ACC_PUBLIC)
    PHP_ME(Pimple, extend,          arginfo_pimple_extend,          ZEND_ACC_PUBLIC)
    PHP_ME(Pimple, keys,            arginfo_pimple_keys,            ZEND_ACC_PUBLIC)

    PHP_ME(PimpleClosure, __invoke, arginfo_pimple_offsetset, ZEND_ACC_PUBLIC)
    PHP_FE_END    /* Must be the last line in pimple_functions[] */
};
/* }}} */



/* {{{ pimple_module_entry
 */
zend_module_entry pimple_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "pimple",
    pimple_functions,
    PHP_MINIT(pimple),
    PHP_MSHUTDOWN(pimple),
    PHP_RINIT(pimple),        /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(pimple),    /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(pimple),
#if ZEND_MODULE_API_NO >= 20010901
    PIMPLE_VERSION, /* Replace with version number for your extension */
#endif
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PIMPLE
ZEND_GET_MODULE(pimple)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("pimple.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_pimple_globals, pimple_globals)
    STD_PHP_INI_ENTRY("pimple.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_pimple_globals, pimple_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_pimple_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_pimple_init_globals(zend_pimple_globals *pimple_globals)
{
    pimple_globals->global_value = 0;
    pimple_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(pimple)
{
    /* If you have INI entries, uncomment these lines 
    REGISTER_INI_ENTRIES();
    */
    zend_class_entry tmp_ce;
    INIT_CLASS_ENTRY(tmp_ce, "Pimple", pimple_functions);

    pimple_ce = zend_register_internal_class(&tmp_ce TSRMLS_CC);
    zend_class_implements(pimple_ce TSRMLS_CC, 1, zend_ce_arrayaccess);


    zend_class_entry tmp_closure_ce;
    INIT_CLASS_ENTRY(tmp_closure_ce, "PimpleClosure", pimple_functions);
    pimple_closure_ce = zend_register_internal_class(&tmp_closure_ce TSRMLS_CC);

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(pimple)
{
    /* uncomment this line if you have INI entries
    UNREGISTER_INI_ENTRIES();
    */
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(pimple)
{
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(pimple)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(pimple)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "pimple support", "enabled");
    php_info_print_table_end();

    /* Remove comments if you have entries in php.ini
    DISPLAY_INI_ENTRIES();
    */
}
/* }}} */


/* {{{ PHP_METHOD
 */
PHP_METHOD(Pimple, __construct) {
    zval *values = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a", &values) == FAILURE) {
        return;
    }

    if (ZEND_NUM_ARGS() == 0) {
        MAKE_STD_ZVAL(values);
        array_init(values);
    }

    zend_update_property(pimple_ce, getThis(), ZEND_STRS("values")-1, values TSRMLS_CC);
}
/* }}} */

/* {{{ PHP_METHOD
 */
PHP_METHOD(Pimple, offsetSet) {
    char *id;
    int id_length;
    zval *value,
         *values;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &id, &id_length, &value) == FAILURE) {
        return;
    }

    values = zend_read_property(pimple_ce, getThis(), ZEND_STRS("values")-1, 0 TSRMLS_CC);

    Z_ADDREF_P(value);
    zend_symtable_update(Z_ARRVAL_P(values), id, id_length + 1, &value, sizeof(value), NULL);

}
/* }}} */


/* {{{ PHP_METHOD
 */
PHP_METHOD(Pimple, offsetGet) {
    char *id;
    int id_length;
    zval *values,
         **z_value;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &id, &id_length) == FAILURE) {
        return;
    }

    values = zend_read_property(pimple_ce, getThis(), ZEND_STRS("values")-1, 0 TSRMLS_CC);

    if (zend_hash_find(Z_ARRVAL_P(values), id, id_length + 1, (void**)&z_value) == FAILURE) {
        zend_throw_exception_ex(
            spl_ce_InvalidArgumentException,
            0 TSRMLS_CC, 
            "Identifier \"%s\" is not defined.", 
            id
        );
        return;
    }

    if (Z_TYPE_PP(z_value) != IS_STRING && zend_is_callable(*z_value, 0, NULL TSRMLS_CC)) {
        
        zval ***args;
        zval *retval_ptr;

        args = safe_emalloc(sizeof(zval **), 1, 0);
        args[0] = &(getThis());

        if (call_user_function_ex(EG(function_table), NULL, *z_value, &retval_ptr, 1, args, 0, NULL TSRMLS_CC) == SUCCESS) {
            RETVAL_ZVAL(retval_ptr, 1, 0);
        }
        efree(args);

    } else {
        RETVAL_ZVAL(*z_value, 1, 0);
    }
}
/* }}} */


/* {{{ PHP_METHOD
 */
PHP_METHOD(Pimple, offsetExists) {
    char *id;
    int id_length;
    zval *values;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &id, &id_length) == FAILURE) {
        return;
    }
    
    values = zend_read_property(pimple_ce, getThis(), ZEND_STRS("values")-1, 0 TSRMLS_CC);

    if (zend_symtable_exists(Z_ARRVAL_P(values), id, id_length + 1)) {
        RETURN_TRUE;
    }
    
    RETURN_FALSE;
}
/* }}} */


/* {{{ PHP_METHOD
 */
PHP_METHOD(Pimple, offsetUnset) {
    char *id;
    int id_length;
    zval *values;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &id, &id_length) == FAILURE) {
        return;
    }
    
    values = zend_read_property(pimple_ce, getThis(), ZEND_STRS("values")-1, 0 TSRMLS_CC);

    zend_symtable_del(Z_ARRVAL_P(values), id, id_length + 1);
}
/* }}} */

/* {{{ PHP_METHOD
 */
PHP_METHOD(Pimple, share) {
    zval *zcallable;
    //zend_closure *closure;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zcallable, zend_ce_closure) == FAILURE) {
        return;
    }

    /**
     * return function ($c) use ($callable) {
     *     static $object;
     * 
     *     if (null === $object) {
     *         $object = $callable($c);
     *     }
     *
     *     return $object;
     * };
     */

    //zend_create_closure(return_value, );
    
}
/* }}} */

/* {{{ PHP_METHOD
 */
PHP_METHOD(Pimple, protect) {
    zval *zcallable;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zcallable, zend_ce_closure) == FAILURE) {
        return;
    }

    /**
     * return function ($c) use ($callable) {
     *     return $callable;
     * };
     */
}
/* }}} */

/* {{{ PHP_METHOD
 */
PHP_METHOD(Pimple, raw) {
    char *id;
    int id_length;

    zval *values,
         **z_value;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &id, &id_length) == FAILURE) {
        return;
    }

    values = zend_read_property(pimple_ce, getThis(), ZEND_STRS("values")-1, 0 TSRMLS_CC);

    if (zend_hash_find(Z_ARRVAL_P(values), id, id_length + 1, (void**)&z_value) == FAILURE) {
        zend_throw_exception_ex(
            spl_ce_InvalidArgumentException,
            0 TSRMLS_CC, 
            "Identifier \"%s\" is not defined.", 
            id
        );
        return;
    }

    RETVAL_ZVAL(*z_value, 1, 0);
}
/* }}} */

/* {{{ PHP_METHOD
 */
PHP_METHOD(Pimple, extend) {
    char *id;
    int id_length;
    zval *zcallable,
         *values,
         **z_value;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sO", &id, &id_length, &zcallable, zend_ce_closure) == FAILURE) {
        return;
    }
    
    values = zend_read_property(pimple_ce, getThis(), ZEND_STRS("values")-1, 0 TSRMLS_CC);

    if (zend_hash_find(Z_ARRVAL_P(values), id, id_length + 1, (void**)&z_value) == FAILURE) {
        zend_throw_exception_ex(
            spl_ce_InvalidArgumentException,
            0 TSRMLS_CC, 
            "Identifier \"%s\" is not defined.", 
            id
        );
        return;
    }

    if (!zend_is_callable(*z_value, 0, NULL TSRMLS_CC)) {
        zend_throw_exception_ex(
            spl_ce_InvalidArgumentException,
            0 TSRMLS_CC, 
            "Identifier \"%s\" does not contain an object definition.", 
            id
        );
        return;
    }

    /**
     * return $this->values[$id] = function ($c) use ($callable, $factory) {
     *     return $callable($factory($c), $c);
     * };
     */
}
/* }}} */

/* {{{ PHP_METHOD
 */
PHP_METHOD(Pimple, keys) {
    zval *values,
         **entry,
         *new_val;

    HashPosition pos;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }
    
    values = zend_read_property(pimple_ce, getThis(), ZEND_STRS("values")-1, 0 TSRMLS_CC);

    /**
     * return array_keys($this->values);
     */
    
    //php_printf("Count: %d\n", zend_hash_num_elements(Z_ARRVAL_P(values)));

    array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL_P(values)));

    zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(values), &pos);
    /*while (zend_hash_get_current_data_ex(Z_ARRVAL_P(values), (void **)&entry, &pos) == SUCCESS) {

        MAKE_STD_ZVAL(new_val);
        zend_hash_get_current_key_zval_ex(Z_ARRVAL_P(values), &new_val, &pos);
        zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &new_val, sizeof(zval *), NULL);
        
        zend_hash_move_forward_ex(Z_ARRVAL_P(values), &pos);
    }*/
}
/* }}} */


/* {{{ PHP_METHOD
 */
PHP_METHOD(PimpleClosure, __invoke) {

}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
