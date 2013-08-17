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

#include "zend.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "zend_closures.h"
#include "zend_hash.h"

#include "ext/standard/info.h"
#include "ext/spl/spl_exceptions.h"

//ZEND_DECLARE_MODULE_GLOBALS(pimple)

static zend_class_entry *pimple_ce;
static zend_class_entry *pimple_closure_ce;

/**
 * Pimple args info.
 */
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

/**
 * PimpleClosure args info.
 */
ZEND_BEGIN_ARG_INFO_EX(arginfo_pimple_closure_construct, 0, 0, 1)
    ZEND_ARG_INFO(0, callback)
    ZEND_ARG_INFO(0, is_protect)
    ZEND_ARG_INFO(0, factory)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_pimple_closure_invoke, 0, 0, 0)
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
    PHP_FE_END    /* Must be the last line in pimple_functions[] */
};
/* }}} */

/* {{{ pimple_closure_functions[]
 *
 * Every user visible function must have an entry in pimple_closure_functions[].
 */
const zend_function_entry pimple_closure_functions[] = {
    PHP_ME(PimpleClosure, __construct,  arginfo_pimple_closure_construct,   ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(PimpleClosure, __invoke,     arginfo_pimple_closure_invoke,      ZEND_ACC_PUBLIC)
    PHP_FE_END    /* Must be the last line in pimple_closure_functions[] */
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

static int _zval_array_to_c_array(zval **arg, zval ****params TSRMLS_DC) /* {{{ */
{
    *(*params)++ = arg;
    return ZEND_HASH_APPLY_KEEP;
} /* }}} */

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
    
    // init Pimple class
    zend_class_entry tmp_ce;
    INIT_CLASS_ENTRY(tmp_ce, "Pimple", pimple_functions);

    pimple_ce = zend_register_internal_class(&tmp_ce TSRMLS_CC);
    zend_class_implements(pimple_ce TSRMLS_CC, 1, zend_ce_arrayaccess);

    // init PimpleClosure class
    zend_class_entry tmp_closure_ce;
    INIT_CLASS_ENTRY(tmp_closure_ce, "PimpleClosure", pimple_closure_functions);

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

    // parse arguments
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a", &values) == FAILURE) {
        return;
    }

    // init array if not defined by argument
    if (ZEND_NUM_ARGS() == 0) {
        MAKE_STD_ZVAL(values);
        array_init(values);
    }

    // set array on values property
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

    // parse arguments
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &id, &id_length, &value) == FAILURE) {
        return;
    }

    // read values property
    values = zend_read_property(pimple_ce, getThis(), ZEND_STRS("values")-1, 0 TSRMLS_CC);

    Z_ADDREF_P(value);

    // add value to values property
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

    // parse arguments
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &id, &id_length) == FAILURE) {
        return;
    }

    // read values property
    values = zend_read_property(pimple_ce, getThis(), ZEND_STRS("values")-1, 0 TSRMLS_CC);

    // find id in values
    if (zend_hash_find(Z_ARRVAL_P(values), id, id_length + 1, (void**)&z_value) == FAILURE) {
        zend_throw_exception_ex(
            spl_ce_InvalidArgumentException,
            0 TSRMLS_CC, 
            "Identifier \"%s\" is not defined.", 
            id
        );
        return;
    }

    // check if value is callable
    if (Z_TYPE_PP(z_value) != IS_STRING && zend_is_callable(*z_value, IS_CALLABLE_STRICT, NULL TSRMLS_CC)) {
        
        zval ***args;
        zval *retval_ptr;

        args = safe_emalloc(sizeof(zval **), 1, 0);
        args[0] = &(getThis());

        // call callable function
        if (call_user_function_ex(EG(function_table), NULL, *z_value, &retval_ptr, 1, args, 0, NULL TSRMLS_CC) == SUCCESS) {
            RETVAL_ZVAL(retval_ptr, 0, 0);
        }
        efree(args);

    } else {
        RETVAL_ZVAL(*z_value, 0, 0);
    }
}
/* }}} */


/* {{{ PHP_METHOD
 */
PHP_METHOD(Pimple, offsetExists) {
    char *id;
    int id_length;
    zval *values;

    // parse arguments
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &id, &id_length) == FAILURE) {
        return;
    }
    
    // read values property
    values = zend_read_property(pimple_ce, getThis(), ZEND_STRS("values")-1, 0 TSRMLS_CC);

    // check if existe id
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

    // parse arguments
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &id, &id_length) == FAILURE) {
        return;
    }
    
    // read values property
    values = zend_read_property(pimple_ce, getThis(), ZEND_STRS("values")-1, 0 TSRMLS_CC);

    // remove id 
    zend_symtable_del(Z_ARRVAL_P(values), id, id_length + 1);
}
/* }}} */

/* {{{ PHP_METHOD
 */
PHP_METHOD(Pimple, share) {
    zval *zcallable,
         *retval_ptr = NULL;

    const char closure_name[] = "PimpleClosure";
    
    zend_class_entry *ce = NULL,
                     *old_scope,
                     **pce;

    zend_function *constructor;

    // parse arguments
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zcallable, zend_ce_closure) == FAILURE) {
        return;
    }
    
    // lookup PimpleClosure class
    if (zend_lookup_class(closure_name, strlen(closure_name), &pce TSRMLS_CC) == SUCCESS) {
        ce = *pce;

        object_init_ex(return_value, ce);

        old_scope = EG(scope);
        EG(scope) = ce;
        constructor = Z_OBJ_HT_P(return_value)->get_constructor(return_value TSRMLS_CC);
        EG(scope) = old_scope;

        if (constructor) {
            zval ***params = NULL;
            zend_fcall_info fci;
            zend_fcall_info_cache fcc;

            params = safe_emalloc(sizeof(zval **), 1, 0);
            params[0] = &zcallable;

            fci.size = sizeof(fci);
            fci.function_table = EG(function_table);
            fci.function_name = NULL;
            fci.symbol_table = NULL;
            fci.object_ptr = return_value;
            fci.retval_ptr_ptr = &retval_ptr;
            fci.param_count = 1;
            fci.params = params;
            fci.no_separation = 1;

            fcc.initialized = 1;
            fcc.function_handler = constructor;
            fcc.calling_scope = EG(scope);
            fcc.called_scope = Z_OBJCE_P(return_value);
            fcc.object_ptr = return_value;

            // call PimpleClosure::__construct()
            if (zend_call_function(&fci, &fcc TSRMLS_CC) == FAILURE) {

                if (params) {
                    efree(params);
                }
                if (retval_ptr) {
                    zval_ptr_dtor(&retval_ptr);
                }
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invocation of %s's constructor failed", ce->name);
                zval_dtor(return_value);
                RETURN_NULL();
            }

            if (retval_ptr) {
                zval_ptr_dtor(&retval_ptr);
            }

            if (params) {
                efree(params);
            }
        }
    }
}
/* }}} */

/* {{{ PHP_METHOD
 */
PHP_METHOD(Pimple, protect) {

    zval *zcallable,
         *retval_ptr = NULL;
    
    zval *is_protect;
    ALLOC_INIT_ZVAL(is_protect);
    ZVAL_LONG(is_protect, 1);

    const char closure_name[] = "PimpleClosure";
    
    zend_class_entry *ce = NULL,
                     *old_scope,
                     **pce;

    zend_function *constructor;

    // parse arguments
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zcallable, zend_ce_closure) == FAILURE) {
        return;
    }
    
    // lookup PimpleClosure class
    if (zend_lookup_class(closure_name, strlen(closure_name), &pce TSRMLS_CC) == SUCCESS) {
        ce = *pce;

        object_init_ex(return_value, ce);

        old_scope = EG(scope);
        EG(scope) = ce;
        constructor = Z_OBJ_HT_P(return_value)->get_constructor(return_value TSRMLS_CC);
        EG(scope) = old_scope;

        if (constructor) {
            zval ***params = NULL;
            zend_fcall_info fci;
            zend_fcall_info_cache fcc;

            params = safe_emalloc(sizeof(zval **), 2, 0);
            params[0] = &zcallable;
            params[1] = &is_protect;

            fci.size = sizeof(fci);
            fci.function_table = EG(function_table);
            fci.function_name = NULL;
            fci.symbol_table = NULL;
            fci.object_ptr = return_value;
            fci.retval_ptr_ptr = &retval_ptr;
            fci.param_count = 2;
            fci.params = params;
            fci.no_separation = 1;

            fcc.initialized = 1;
            fcc.function_handler = constructor;
            fcc.calling_scope = EG(scope);
            fcc.called_scope = Z_OBJCE_P(return_value);
            fcc.object_ptr = return_value;

            if (zend_call_function(&fci, &fcc TSRMLS_CC) == FAILURE) {

                if (params) {
                    efree(params);
                }
                if (retval_ptr) {
                    zval_ptr_dtor(&retval_ptr);
                }
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invocation of %s's constructor failed", ce->name);
                zval_dtor(return_value);
                RETURN_NULL();
            }

            if (retval_ptr) {
                zval_ptr_dtor(&retval_ptr);
            }

            if (params) {
                efree(params);
            }
        }
    }

}
/* }}} */

/* {{{ PHP_METHOD
 */
PHP_METHOD(Pimple, raw) {
    char *id;
    int id_length;

    zval *values,
         **z_value;

    // parse arguments
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &id, &id_length) == FAILURE) {
        return;
    }

    // read values property
    values = zend_read_property(pimple_ce, getThis(), ZEND_STRS("values")-1, 0 TSRMLS_CC);

    // find id in values property
    if (zend_hash_find(Z_ARRVAL_P(values), id, id_length + 1, (void**)&z_value) == FAILURE) {
        zend_throw_exception_ex(
            spl_ce_InvalidArgumentException,
            0 TSRMLS_CC, 
            "Identifier \"%s\" is not defined.", 
            id
        );
        return;
    }

    RETVAL_ZVAL(*z_value, 0, 0);
}
/* }}} */

/* {{{ PHP_METHOD
 */
PHP_METHOD(Pimple, extend) {
    char *id;
    int id_length;
    zval *zcallable,
         *values,
         **z_value,
         *retval_ptr;
    
    zval *is_protect;
    ALLOC_INIT_ZVAL(is_protect);
    ZVAL_LONG(is_protect, 0);

    const char closure_name[] = "PimpleClosure";

    zend_class_entry *ce = NULL,
                     *old_scope,
                     **pce;

    zend_function *constructor;

    zval *zfactory,
         *retvalue;

    // parse arguments
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sO", &id, &id_length, &zcallable, zend_ce_closure) == FAILURE) {
        return;
    }
    
    // read values property
    values = zend_read_property(pimple_ce, getThis(), ZEND_STRS("values")-1, 0 TSRMLS_CC);

    // find id in values property
    if (zend_hash_find(Z_ARRVAL_P(values), id, id_length + 1, (void**)&z_value) == FAILURE) {
        zend_throw_exception_ex(
            spl_ce_InvalidArgumentException,
            0 TSRMLS_CC, 
            "Identifier \"%s\" is not defined.", 
            id
        );
        return;
    }

    // check if z_value is callable
    if (Z_TYPE_PP(z_value) != IS_STRING && zend_is_callable(*z_value, IS_CALLABLE_STRICT, NULL TSRMLS_CC)) {
        
        zval ***args;
        //zval *retval_ptr;

        args = safe_emalloc(sizeof(zval **), 1, 0);
        args[0] = &(getThis());

        // call callable function
        if (call_user_function_ex(EG(function_table), NULL, *z_value, &zfactory, 1, args, 0, NULL TSRMLS_CC) == FAILURE) {
            // error
        }
        efree(args);

    } else {
        zend_throw_exception_ex(
            spl_ce_InvalidArgumentException,
            0 TSRMLS_CC, 
            "Identifier \"%s\" does not contain an object definition.", 
            id
        );
        return;
    }

    // lookup PimpleClosure class
    if (zend_lookup_class(closure_name, strlen(closure_name), &pce TSRMLS_CC) == SUCCESS) {
        ce = *pce;
        MAKE_STD_ZVAL(retvalue);
        object_init_ex(retvalue, ce);

        old_scope = EG(scope);
        EG(scope) = ce;
        constructor = Z_OBJ_HT_P(retvalue)->get_constructor(retvalue TSRMLS_CC);
        EG(scope) = old_scope;

        if (constructor) {
            zval ***params = NULL;
            zend_fcall_info fci;
            zend_fcall_info_cache fcc;

            params = safe_emalloc(sizeof(zval **), 3, 0);
            params[0] = &zcallable;
            params[1] = &is_protect;
            params[2] = &zfactory;

            fci.size = sizeof(fci);
            fci.function_table = EG(function_table);
            fci.function_name = NULL;
            fci.symbol_table = NULL;
            fci.object_ptr = retvalue;
            fci.retval_ptr_ptr = &retval_ptr;
            fci.param_count = 3;
            fci.params = params;
            fci.no_separation = 1;

            fcc.initialized = 1;
            fcc.function_handler = constructor;
            fcc.calling_scope = EG(scope);
            fcc.called_scope = Z_OBJCE_P(retvalue);
            fcc.object_ptr = retvalue;

            // call PimpleClosure::__construct()
            if (zend_call_function(&fci, &fcc TSRMLS_CC) == FAILURE) {
                if (params) {
                    efree(params);
                }
                if (retval_ptr) {
                    zval_ptr_dtor(&retval_ptr);
                }
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invocation of %s's constructor failed", ce->name);
                zval_dtor(retvalue);
                RETURN_NULL();
            }

            if (params) {
                efree(params);
            }

            // update id value
            zend_symtable_update(Z_ARRVAL_P(values), id, id_length + 1, &retvalue, sizeof(retvalue), NULL);

            if (retval_ptr) {
                zval_ptr_dtor(&retval_ptr);
            }

            RETVAL_ZVAL(retvalue, 0, 0);
        }
    }

}
/* }}} */

/* {{{ PHP_METHOD
 */
PHP_METHOD(Pimple, keys) {
    zval *values,
         **entry,
         *new_val;
    HashPosition pos;

    char *string_key;
    uint string_key_len;
    ulong num_key;

    // parse arguments
    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }
    
    // read values property
    values = zend_read_property(pimple_ce, getThis(), ZEND_STRS("values")-1, 0 TSRMLS_CC);
    
    // init array
    array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL_P(values)));

    // reset pointer
    zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(values), &pos);
    while (zend_hash_get_current_data_ex(Z_ARRVAL_P(values), (void **)&entry, &pos) == SUCCESS) {

        MAKE_STD_ZVAL(new_val);
        #if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION <= 4)
        switch (zend_hash_get_current_key_ex(Z_ARRVAL_P(values), &string_key, &string_key_len, &num_key, 1, &pos)) {
            case HASH_KEY_IS_STRING:
                ZVAL_STRINGL(new_val, string_key, string_key_len - 1, 0);
                zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &new_val, sizeof(zval *), NULL);
                break;

            case HASH_KEY_IS_LONG:
                Z_TYPE_P(new_val) = IS_LONG;
                Z_LVAL_P(new_val) = num_key;
                zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &new_val, sizeof(zval *), NULL);
                break;
        }
        #else
        zend_hash_get_current_key_zval_ex(Z_ARRVAL_P(values), new_val, &pos);
        zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &new_val, sizeof(zval *), NULL);
        #endif
        
        zend_hash_move_forward_ex(Z_ARRVAL_P(values), &pos);
    }
}
/* }}} */






/* {{{ PHP_METHOD
 */
ZEND_METHOD(PimpleClosure, __construct) {
    zval *zcallable,
         *zfactory;
    long is_protect = 0;

    // parse arguments
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|lo", &zcallable, zend_ce_closure, &is_protect, &zfactory) == FAILURE) {
        return;
    }

    // init null value if not defined in arguments
    if (ZEND_NUM_ARGS() < 3) {
        ALLOC_INIT_ZVAL(zfactory);
        ZVAL_NULL(zfactory);
    }

    // set class properties
    zend_update_property(pimple_closure_ce, getThis(), ZEND_STRS("callback")-1, zcallable TSRMLS_CC);
    zend_update_property_long(pimple_closure_ce, getThis(), ZEND_STRS("is_protect")-1, is_protect TSRMLS_CC);
    zend_update_property(pimple_closure_ce, getThis(), ZEND_STRS("factory")-1, zfactory TSRMLS_CC);
}
/* }}} */


/* {{{ PHP_METHOD
 */
ZEND_METHOD(PimpleClosure, __invoke) {

    zval *c,
         *zcallback,
         *is_protect,
         *zfactory;

    // parse arguments
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &c) == FAILURE) {
        return;
    }

    // read is_protect property
    is_protect = zend_read_property(pimple_closure_ce, getThis(), ZEND_STRS("is_protect")-1, 0 TSRMLS_CC);

    // read zfactory property
    zfactory = zend_read_property(pimple_closure_ce, getThis(), ZEND_STRS("factory")-1, 0 TSRMLS_CC);

    // check if protect
    if (Z_LVAL_P(is_protect) == 1) {

        zcallback = zend_read_property(pimple_closure_ce, getThis(), ZEND_STRS("callback")-1, 0 TSRMLS_CC);
        RETVAL_ZVAL(zcallback, 0, 0);

    //} else if (Z_TYPE_P(zfactory) == IS_OBJECT || Z_TYPE_P(zfactory) == IS_CALLABLE) {
    } else if (Z_TYPE_P(zfactory) != IS_STRING && zend_is_callable(zfactory, IS_CALLABLE_STRICT, NULL TSRMLS_CC)) {
        php_printf("zfactory\n");
        zval ***factory_args;
        zval *zfactory_value,
             *object;

        factory_args = safe_emalloc(sizeof(zval **), 1, 0);
        factory_args[0] = &c;

        if (call_user_function_ex(EG(function_table), NULL, zfactory, &zfactory_value, 1, factory_args, 0, NULL TSRMLS_CC) == FAILURE) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invocation constructor failed");
            zval_dtor(return_value);
            RETURN_NULL();
        }
        efree(factory_args);

        zcallback = zend_read_property(pimple_closure_ce, getThis(), ZEND_STRS("callback")-1, 0 TSRMLS_CC);

        zval ***args;
        zval *retval_ptr;

        args = safe_emalloc(sizeof(zval **), 2, 0);
        args[0] = &zfactory_value;
        args[1] = &c;

        if (call_user_function_ex(EG(function_table), NULL, zcallback, &retval_ptr, 1, args, 0, NULL TSRMLS_CC) == SUCCESS) {
            object = retval_ptr;
        }
        efree(args);

        RETURN_ZVAL(object, 0, 0);

    } else {
        
        static zval *object = NULL;

        if (object == NULL) {
            zcallback = zend_read_property(pimple_closure_ce, getThis(), ZEND_STRS("callback")-1, 0 TSRMLS_CC);

            zval ***args;
            zval *retval_ptr;

            args = safe_emalloc(sizeof(zval **), 1, 0);
            args[0] = &c;

            if (call_user_function_ex(EG(function_table), NULL, zcallback, &retval_ptr, 1, args, 0, NULL TSRMLS_CC) == SUCCESS) {
                object = retval_ptr;
            }
            efree(args);
        }

        RETURN_ZVAL(object, 0, 0);
    }
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
