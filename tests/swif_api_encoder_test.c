#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include "../src/swif_api.h"

#define ARBITRARY_UINT32 0x12345678
#define ARBITRARY_PTR ((void *) ARBITRARY_UINT32)

swif_encoder_t dummy_encoder;
void *func_args[5];

bool set_callback_functions;
bool set_parameters;
bool get_parameters;
bool build_repair_symbol;
bool reset_coding_window;
bool add_source_symbol_to_coding_window;
bool remove_source_symbol_from_coding_window;
bool get_coding_window_information;
bool set_coding_coefs_tab;
bool get_coding_coefs_tab;
bool generate_coding_coefs;


void store_args(int count, ...) {
    va_list list;
    va_start(list, count);
    for (int i = 0 ; i < count ; i++) func_args[i] = va_arg(list, void *);
    va_end(list);
}

void check_correct_args(int count, ...) {
    va_list list;
    va_start(list, count);
    char str[250];
    for (int i = 0 ; i < count ; i++) {
        CU_ASSERT_EQUAL(va_arg(list, void *), func_args[i]);
//        if (va_arg(list, void *) != func_args[i]) {
//
//            sprintf(str, "the function has been called with the wrong argument at index %d", i);
//            CU_FAIL(the function has been called with the wrong argument at index %d);
//        }
    }
    va_end(list);
}



swif_status_t   dummy_encoder_set_callback_functions (
        swif_encoder_t*        enc,
        void (*source_symbol_removed_from_coding_window_callback) (
                void*   context,
                esi_t   old_symbol_esi),
        void* context_4_callback)
{
    store_args(3, enc, source_symbol_removed_from_coding_window_callback, context_4_callback);
    set_callback_functions = true;
    return ARBITRARY_UINT32;
}



swif_status_t   dummy_encoder_set_parameters  (
        swif_encoder_t* enc,
        uint32_t        type,
        uint32_t        length,
        void*           value)
{
    store_args(4, enc, type, length, value);
    set_parameters = true;
    return ARBITRARY_UINT32;
}


swif_status_t   dummy_encoder_get_parameters  (
        swif_encoder_t* enc,
        uint32_t        type,
        uint32_t        length,
        void*           value)
{
    store_args(4, enc, type, length, value);
    get_parameters = true;
    return ARBITRARY_UINT32;
}


swif_status_t   dummy_build_repair_symbol (
        swif_encoder_t* generic_encoder,
        void*           new_buf)
{
    store_args(2, generic_encoder, new_buf);
    build_repair_symbol = true;
    return ARBITRARY_UINT32;
}

swif_status_t   dummy_encoder_reset_coding_window (swif_encoder_t*  enc)
{
    store_args(1, enc);
    reset_coding_window = true;
    return ARBITRARY_UINT32;
}

swif_status_t   dummy_encoder_add_source_symbol_to_coding_window (
        swif_encoder_t* enc,
        void*           new_src_symbol_buf,
        esi_t           new_src_symbol_esi)
{
    store_args(3, enc, new_src_symbol_buf, new_src_symbol_esi);
    add_source_symbol_to_coding_window = true;
    return ARBITRARY_UINT32;
}

swif_status_t   dummy_encoder_remove_source_symbol_from_coding_window (
        swif_encoder_t* enc,
        esi_t           old_src_symbol_esi)
{
    store_args(2, enc, old_src_symbol_esi);
    remove_source_symbol_from_coding_window = true;
    return ARBITRARY_UINT32;
}

swif_status_t   dummy_encoder_get_coding_window_information (
        swif_encoder_t* enc,
        esi_t*          first,
        esi_t*          last,
        uint32_t*       nss)
{
    store_args(4, enc, first, last, nss);
    get_coding_window_information = true;
    return ARBITRARY_UINT32;
}


swif_status_t   dummy_encoder_set_coding_coefs_tab (
        swif_encoder_t* enc,
        void*           coding_coefs_tab,
        uint32_t        nb_coefs_in_tab)
{
    store_args(3, enc, coding_coefs_tab, nb_coefs_in_tab);
    set_coding_coefs_tab = true;
    return ARBITRARY_UINT32;
}


swif_status_t   dummy_encoder_generate_coding_coefs (
        swif_encoder_t* enc,
        uint32_t        key,
        uint32_t        add_param)
{
    store_args(3, enc, key, add_param);
    generate_coding_coefs = true;
    return ARBITRARY_UINT32;
}

swif_status_t   dummy_encoder_get_coding_coefs_tab (
        swif_encoder_t* enc,
        void**          coding_coefs_tab,
        uint32_t*       nb_coefs_in_tab)
{
    store_args(3, enc, coding_coefs_tab, nb_coefs_in_tab);
    get_coding_coefs_tab = true;
    return ARBITRARY_UINT32;
}



int init_test(void)
{
    memset(&dummy_encoder, 0, sizeof(swif_encoder_t));
    memset(func_args, 0, sizeof(func_args));

    dummy_encoder.generate_coding_coefs = dummy_encoder_generate_coding_coefs;
    dummy_encoder.get_coding_window_information = dummy_encoder_get_coding_window_information;
    dummy_encoder.get_coding_coefs_tab = dummy_encoder_get_coding_coefs_tab;
    dummy_encoder.get_parameters = dummy_encoder_get_parameters;
    dummy_encoder.set_parameters = dummy_encoder_set_parameters;
    dummy_encoder.set_callback_functions = dummy_encoder_set_callback_functions;
    dummy_encoder.set_coding_coefs_tab = dummy_encoder_set_coding_coefs_tab;
    dummy_encoder.remove_source_symbol_from_coding_window = dummy_encoder_remove_source_symbol_from_coding_window;
    dummy_encoder.add_source_symbol_to_coding_window = dummy_encoder_add_source_symbol_to_coding_window;
    dummy_encoder.build_repair_symbol = dummy_build_repair_symbol;
    dummy_encoder.reset_coding_window = dummy_encoder_reset_coding_window;


    set_callback_functions = false;
    set_parameters = false;
    get_parameters = false;
    build_repair_symbol = false;
    reset_coding_window = false;
    add_source_symbol_to_coding_window = false;
    remove_source_symbol_from_coding_window = false;
    get_coding_window_information = false;
    set_coding_coefs_tab = false;
    get_coding_coefs_tab = false;
    generate_coding_coefs = false;

}



void test_generate_coding_coefs(void)
{
    init_test();
    swif_status_t ret = swif_encoder_generate_coding_coefs(&dummy_encoder, ARBITRARY_UINT32, ARBITRARY_UINT32+1);
    check_correct_args(3, &dummy_encoder, ARBITRARY_UINT32, ARBITRARY_UINT32+1);
    CU_ASSERT_TRUE(generate_coding_coefs);
    CU_ASSERT_EQUAL(ret, ARBITRARY_UINT32);
}

void test_get_coding_window_information(void)
{
    init_test();
    swif_status_t ret = swif_encoder_get_coding_window_information(&dummy_encoder, ARBITRARY_PTR, ARBITRARY_PTR+1, ARBITRARY_PTR+2);
    check_correct_args(4, &dummy_encoder, ARBITRARY_PTR, ARBITRARY_PTR+1, ARBITRARY_PTR+2);
    CU_ASSERT_TRUE(get_coding_window_information);
    CU_ASSERT_EQUAL(ret, ARBITRARY_UINT32);
}


void test_get_coding_coefs_tab(void)
{
    init_test();
    swif_status_t ret = swif_encoder_get_coding_coefs_tab(&dummy_encoder, ARBITRARY_PTR, ARBITRARY_PTR+1);
    check_correct_args(3, &dummy_encoder, ARBITRARY_PTR, ARBITRARY_PTR+1);
    CU_ASSERT_TRUE(get_coding_coefs_tab);
    CU_ASSERT_EQUAL(ret, ARBITRARY_UINT32);
}


void test_get_parameters(void)
{
    init_test();
    swif_status_t ret = swif_encoder_get_parameters(&dummy_encoder, ARBITRARY_UINT32, ARBITRARY_UINT32+1, ARBITRARY_PTR+2);
    check_correct_args(4, &dummy_encoder, ARBITRARY_UINT32, ARBITRARY_UINT32+1, ARBITRARY_PTR+2);
    CU_ASSERT_TRUE(get_parameters);
    CU_ASSERT_EQUAL(ret, ARBITRARY_UINT32);
}


void test_set_parameters(void)
{
    init_test();
    swif_status_t ret = swif_encoder_set_parameters(&dummy_encoder, ARBITRARY_UINT32, ARBITRARY_UINT32+1, ARBITRARY_PTR+2);
    check_correct_args(4, &dummy_encoder, ARBITRARY_UINT32, ARBITRARY_UINT32+1, ARBITRARY_PTR+2);
    CU_ASSERT_TRUE(set_parameters);
    CU_ASSERT_EQUAL(ret, ARBITRARY_UINT32);
}

void test_set_callback_functions(void)
{
    init_test();
    swif_status_t ret = swif_encoder_set_callback_functions(&dummy_encoder, ARBITRARY_PTR, ARBITRARY_PTR+1);
    check_correct_args(3, &dummy_encoder, ARBITRARY_PTR, ARBITRARY_PTR+1);
    CU_ASSERT_TRUE(set_callback_functions);
    CU_ASSERT_EQUAL(ret, ARBITRARY_UINT32);
}

void test_set_coding_coefs_tab(void)
{
    init_test();
    swif_status_t ret = swif_encoder_set_coding_coefs_tab(&dummy_encoder, ARBITRARY_PTR, ARBITRARY_UINT32+1);
    check_correct_args(3, &dummy_encoder, ARBITRARY_PTR, ARBITRARY_UINT32+1);
    CU_ASSERT_TRUE(set_coding_coefs_tab);
    CU_ASSERT_EQUAL(ret, ARBITRARY_UINT32);
}

void test_remove_source_symbol_from_coding_window(void)
{
    init_test();
    swif_status_t ret = swif_encoder_remove_source_symbol_from_coding_window(&dummy_encoder, ARBITRARY_UINT32);
    check_correct_args(2, &dummy_encoder, ARBITRARY_UINT32);
    CU_ASSERT_TRUE(remove_source_symbol_from_coding_window);
    CU_ASSERT_EQUAL(ret, ARBITRARY_UINT32);
}

void test_add_source_symbol_to_coding_window(void)
{
    init_test();
    swif_status_t ret = swif_encoder_add_source_symbol_to_coding_window(&dummy_encoder, ARBITRARY_PTR, ARBITRARY_UINT32+1);
    check_correct_args(3, &dummy_encoder, ARBITRARY_PTR, ARBITRARY_UINT32+1);
    CU_ASSERT_TRUE(add_source_symbol_to_coding_window);
    CU_ASSERT_EQUAL(ret, ARBITRARY_UINT32);
}

void test_build_repair_symbol(void)
{
    init_test();
    swif_status_t ret = swif_build_repair_symbol(&dummy_encoder, ARBITRARY_PTR);
    check_correct_args(2, &dummy_encoder, ARBITRARY_PTR);
    CU_ASSERT_TRUE(build_repair_symbol);
    CU_ASSERT_EQUAL(ret, ARBITRARY_UINT32);
}


void test_reset_coding_window(void)
{
    init_test();
    swif_status_t ret = swif_encoder_reset_coding_window(&dummy_encoder);
    check_correct_args(1, &dummy_encoder);
    CU_ASSERT_TRUE(reset_coding_window);
    CU_ASSERT_EQUAL(ret, ARBITRARY_UINT32);
}




/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main()
{
    CU_pSuite pSuite = NULL;

    CU_basic_set_mode(CU_BRM_VERBOSE);
    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    pSuite = CU_add_suite("Suite_1", NULL, NULL);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* add the tests to the suite */
    /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
    if ((NULL == CU_add_test(pSuite, "test of generate_coding_coefs()", test_generate_coding_coefs)) ||
        (NULL == CU_add_test(pSuite, "test of get_coding_window_information()", test_get_coding_window_information)) ||
        (NULL == CU_add_test(pSuite, "test of get_coding_coefs_tab()", test_get_coding_coefs_tab)) ||
        (NULL == CU_add_test(pSuite, "test of get_parameters()", test_get_parameters)) ||
        (NULL == CU_add_test(pSuite, "test of set_parameters()", test_set_parameters)) ||
        (NULL == CU_add_test(pSuite, "test of set_callback_functions()", test_set_callback_functions)) ||
        (NULL == CU_add_test(pSuite, "test of set_coding_coefs_tab()", test_set_coding_coefs_tab)) ||
        (NULL == CU_add_test(pSuite, "test of remove_source_symbol_from_coding_window()", test_remove_source_symbol_from_coding_window)) ||
        (NULL == CU_add_test(pSuite, "test of add_source_symbol_to_coding_window()", test_add_source_symbol_to_coding_window)) ||
        (NULL == CU_add_test(pSuite, "test of build_repair_symbol()", test_build_repair_symbol)) ||
        (NULL == CU_add_test(pSuite, "test of reset_coding_window()", test_reset_coding_window)))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
