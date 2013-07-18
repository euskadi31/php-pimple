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

typedef struct _zend_closure {
    zend_object    std;
    zend_function  func;
    zval          *this_ptr;
    HashTable     *debug_info;
} zend_closure;

static zend_class_entry *pimple_ce;
//static zend_closure *pimple_closure_ce;
ZEND_API zend_class_entry *pimple_closure_ce;
static zend_object_handlers pimple_closure_handlers;

static zend_function *zend_closure_get_constructor(zval *object TSRMLS_DC) /* {{{ */
{
    zend_error(E_RECOVERABLE_ERROR, "Instantiation of 'Closure' is not allowed");
    return NULL;
}
/* }}} */

static int zend_closure_compare_objects(zval *o1, zval *o2 TSRMLS_DC) /* {{{ */
{
    return (Z_OBJ_HANDLE_P(o1) != Z_OBJ_HANDLE_P(o2));
}
/* }}} */


static zend_function *zend_closure_get_method(zval **object_ptr, char *method_name, int method_len, const zend_literal *key TSRMLS_DC) /* {{{ */
{
    char *lc_name;
    ALLOCA_FLAG(use_heap)

    lc_name = do_alloca(method_len + 1, use_heap);
    zend_str_tolower_copy(lc_name, method_name, method_len);
    if ((method_len == sizeof(ZEND_INVOKE_FUNC_NAME)-1) &&
        memcmp(lc_name, ZEND_INVOKE_FUNC_NAME, sizeof(ZEND_INVOKE_FUNC_NAME)-1) == 0
    ) {
        free_alloca(lc_name, use_heap);
        return zend_get_closure_invoke_method(*object_ptr TSRMLS_CC);
    }
    free_alloca(lc_name, use_heap);
    return std_object_handlers.get_method(object_ptr, method_name, method_len, key TSRMLS_CC);
}
/* }}} */

static zval *zend_closure_read_property(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC) /* {{{ */
{
    ZEND_CLOSURE_PROPERTY_ERROR();
    Z_ADDREF(EG(uninitialized_zval));
    return &EG(uninitialized_zval);
}
/* }}} */

static void zend_closure_write_property(zval *object, zval *member, zval *value, const zend_literal *key TSRMLS_DC) /* {{{ */
{
    ZEND_CLOSURE_PROPERTY_ERROR();
}
/* }}} */

static zval **zend_closure_get_property_ptr_ptr(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC) /* {{{ */
{
    ZEND_CLOSURE_PROPERTY_ERROR();
    return NULL;
}
/* }}} */

static int zend_closure_has_property(zval *object, zval *member, int has_set_exists, const zend_literal *key TSRMLS_DC) /* {{{ */
{
    if (has_set_exists != 2) {
        ZEND_CLOSURE_PROPERTY_ERROR();
    }
    return 0;
}
/* }}} */

static void zend_closure_unset_property(zval *object, zval *member, const zend_literal *key TSRMLS_DC) /* {{{ */
{
    ZEND_CLOSURE_PROPERTY_ERROR();
}
/* }}} */

static void zend_closure_free_storage(void *object TSRMLS_DC) /* {{{ */
{
    zend_closure *closure = (zend_closure *)object;

    zend_object_std_dtor(&closure->std TSRMLS_CC);

    if (closure->func.type == ZEND_USER_FUNCTION) {
        zend_execute_data *ex = EG(current_execute_data);
        while (ex) {
            if (ex->op_array == &closure->func.op_array) {
                zend_error(E_ERROR, "Cannot destroy active lambda function");
            }
            ex = ex->prev_execute_data;
        }
        destroy_op_array(&closure->func.op_array TSRMLS_CC);
    }

    if (closure->debug_info != NULL) {
        zend_hash_destroy(closure->debug_info);
        efree(closure->debug_info);
    }

    if (closure->this_ptr) {
        zval_ptr_dtor(&closure->this_ptr);
    }

    efree(closure);
}
/* }}} */

static zend_object_value zend_closure_new(zend_class_entry *class_type TSRMLS_DC) /* {{{ */
{
    zend_closure *closure;
    zend_object_value object;

    closure = emalloc(sizeof(zend_closure));
    memset(closure, 0, sizeof(zend_closure));

    zend_object_std_init(&closure->std, class_type TSRMLS_CC);

    object.handle = zend_objects_store_put(closure, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) zend_closure_free_storage, NULL TSRMLS_CC);
    object.handlers = &pimple_closure_handlers;

    return object;
}
/* }}} */

static zend_object_value zend_closure_clone(zval *zobject TSRMLS_DC) /* {{{ */
{
    zend_closure *closure = (zend_closure *)zend_object_store_get_object(zobject TSRMLS_CC);
    zval result;

    zend_create_closure(&result, &closure->func, closure->func.common.scope, closure->this_ptr TSRMLS_CC);
    return Z_OBJVAL(result);
}
/* }}} */


int zend_closure_get_closure(zval *obj, zend_class_entry **ce_ptr, zend_function **fptr_ptr, zval **zobj_ptr TSRMLS_DC) /* {{{ */
{
    zend_closure *closure;

    if (Z_TYPE_P(obj) != IS_OBJECT) {
        return FAILURE;
    }

    closure = (zend_closure *)zend_object_store_get_object(obj TSRMLS_CC);
    *fptr_ptr = &closure->func;

    if (closure->this_ptr) {
        if (zobj_ptr) {
            *zobj_ptr = closure->this_ptr;
        }
        *ce_ptr = Z_OBJCE_P(closure->this_ptr);
    } else {
        if (zobj_ptr) {
            *zobj_ptr = NULL;
        }
        *ce_ptr = closure->func.common.scope;
    }
    return SUCCESS;
}
/* }}} */

static HashTable *zend_closure_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    zend_closure *closure = (zend_closure *)zend_object_store_get_object(object TSRMLS_CC);
    zval *val;
    struct _zend_arg_info *arg_info = closure->func.common.arg_info;

    *is_temp = 0;

    if (closure->debug_info == NULL) {
        ALLOC_HASHTABLE(closure->debug_info);
        zend_hash_init(closure->debug_info, 1, NULL, ZVAL_PTR_DTOR, 0);
    }
    if (closure->debug_info->nApplyCount == 0) {
        if (closure->func.type == ZEND_USER_FUNCTION && closure->func.op_array.static_variables) {
            HashTable *static_variables = closure->func.op_array.static_variables;
            MAKE_STD_ZVAL(val);
            array_init(val);
            zend_hash_copy(Z_ARRVAL_P(val), static_variables, (copy_ctor_func_t)zval_add_ref, NULL, sizeof(zval*));
            zend_hash_update(closure->debug_info, "static", sizeof("static"), (void *) &val, sizeof(zval *), NULL);
        }

        if (closure->this_ptr) {
            Z_ADDREF_P(closure->this_ptr);
            zend_symtable_update(closure->debug_info, "this", sizeof("this"), (void *) &closure->this_ptr, sizeof(zval *), NULL);
        }

        if (arg_info) {
            zend_uint i, required = closure->func.common.required_num_args;

            MAKE_STD_ZVAL(val);
            array_init(val);

            for (i = 0; i < closure->func.common.num_args; i++) {
                char *name, *info;
                int name_len, info_len;
                if (arg_info->name) {
                    name_len = zend_spprintf(&name, 0, "%s$%s",
                                    arg_info->pass_by_reference ? "&" : "",
                                    arg_info->name);
                } else {
                    name_len = zend_spprintf(&name, 0, "%s$param%d",
                                    arg_info->pass_by_reference ? "&" : "",
                                    i + 1);
                }
                info_len = zend_spprintf(&info, 0, "%s",
                                i >= required ? "<optional>" : "<required>");
                add_assoc_stringl_ex(val, name, name_len + 1, info, info_len, 0);
                efree(name);
                arg_info++;
            }
            zend_hash_update(closure->debug_info, "parameter", sizeof("parameter"), (void *) &val, sizeof(zval *), NULL);
        }
    }

    return closure->debug_info;
}
/* }}} */

static HashTable *zend_closure_get_gc(zval *obj, zval ***table, int *n TSRMLS_DC) /* {{{ */
{
    zend_closure *closure = (zend_closure *)zend_object_store_get_object(obj TSRMLS_CC);

    *table = closure->this_ptr ? &closure->this_ptr : NULL;
    *n = closure->this_ptr ? 1 : 0;
    return (closure->func.type == ZEND_USER_FUNCTION) ?
        closure->func.op_array.static_variables : NULL;
}
/* }}} */

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

    PHP_ME(PimpleClosure, __invoke, arginfo_pimple_closure_invoke, ZEND_ACC_PUBLIC)
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
    pimple_closure_ce->ce_flags |= ZEND_ACC_FINAL_CLASS;
    pimple_closure_ce->create_object = zend_closure_new;
    pimple_closure_ce->serialize = zend_class_serialize_deny;
    pimple_closure_ce->unserialize = zend_class_unserialize_deny;


    memcpy(&pimple_closure_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    pimple_closure_handlers.get_constructor = zend_closure_get_constructor;
    pimple_closure_handlers.get_method = zend_closure_get_method;
    pimple_closure_handlers.write_property = zend_closure_write_property;
    pimple_closure_handlers.read_property = zend_closure_read_property;
    pimple_closure_handlers.get_property_ptr_ptr = zend_closure_get_property_ptr_ptr;
    pimple_closure_handlers.has_property = zend_closure_has_property;
    pimple_closure_handlers.unset_property = zend_closure_unset_property;
    pimple_closure_handlers.compare_objects = zend_closure_compare_objects;
    pimple_closure_handlers.clone_obj = zend_closure_clone;
    pimple_closure_handlers.get_debug_info = zend_closure_get_debug_info;
    pimple_closure_handlers.get_closure = zend_closure_get_closure;
    pimple_closure_handlers.get_gc = zend_closure_get_gc;

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
    zend_function *cptr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zcallable, zend_ce_closure) == FAILURE) {
        return;
    }

    //zend_create_closure(return_value, pimple_closure_ce, pimple_closure_ce, NULL TSRMLS_CC);

    //cptr = 

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



    /*zend_create_closure(
        return_value, 
        &pimple_closure_ce->ptr, 
        NULL, 
        NULL TSRMLS_CC
    );*/
    
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

    char *string_key;
    uint string_key_len;
    ulong num_key;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }
    
    values = zend_read_property(pimple_ce, getThis(), ZEND_STRS("values")-1, 0 TSRMLS_CC);

    /**
     * return array_keys($this->values);
     */
    
    array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL_P(values)));

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
ZEND_METHOD(PimpleClosure, __invoke) {
    RETURN_TRUE;
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
