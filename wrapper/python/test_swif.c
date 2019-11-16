
#include<stdlib.h>
#include<stdio.h>

#include "swif_full_symbol.h"
#include "swif_full_symbol_impl.h"



void test_set(void)
{
    uint8_t data1[4] = {0x11, 0x22, 0x33, 0x44};
    uint8_t data2[4] = {0xa0, 0xb0, 0xc0, 0xd0};

    swif_full_symbol_t* symbol1 = full_symbol_create_from_source(1, data1, sizeof(data1));
    swif_full_symbol_t* symbol2 = full_symbol_create_from_source(3, data2, sizeof(data2));


    swif_full_symbol_set_t *set = full_symbol_set_alloc();

    full_symbol_set_add(set, symbol1);
    full_symbol_set_add(set, symbol2);

    full_symbol_free(symbol1);
    full_symbol_free(symbol2);

    full_symbol_set_dump(set, stdout);
    full_symbol_set_free(set);
}
void test_set2(void)
{
    uint8_t data1[4] = {0x01, 0x01, 0x01, 0x01};
    uint8_t data2[4] = { 0x01, 0x02};
    uint8_t data3[4] = { 0x01, 0x02, 0x00};
   /* uint8_t symbol_id_table = [1,0,0,0];
full_symbol_create(symbol_id_table, 1, len(symbol_id_table),data1, len(data1)); 
    swif_full_symbol_t* symbol1 = full_symbol_create(1, data1, sizeof(data1));  */
    swif_full_symbol_t* symbol1 = full_symbol_create_from_source(1, data1, sizeof(data1));
    swif_full_symbol_t* symbol2 = full_symbol_create_from_source(3, data2, sizeof(data2));
    swif_full_symbol_t* new_symbol = full_symbol_create_from_source(1, data3, sizeof(data3));


    swif_full_symbol_set_t *set = full_symbol_set_alloc();

    full_symbol_set_add(set, symbol1);
    full_symbol_set_add(set, symbol2);
    swif_full_symbol_t *symbol_cloned=full_symbol_clone(symbol2);
    full_symbol_set_add_as_pivot(set,new_symbol);
    full_symbol_set_dump(set, stdout);
    full_symbol_free(symbol1);
    full_symbol_free(symbol2);
    full_symbol_free(symbol_cloned);
    full_symbol_free(new_symbol);

    full_symbol_set_dump(set, stdout);
    full_symbol_set_free(set);

}
void test_add_with_elimination(void)
{
    swif_full_symbol_set_t *set1 = full_symbol_set_alloc();
     uint8_t data1[4] = {0x01, 0x00, 0x00};
    uint8_t data2[4] = { 0x00, 0x01, 0x00};
    swif_full_symbol_t* symbol1 = full_symbol_create_from_source(1, data1, sizeof(data1));
    swif_full_symbol_t* symbol2 = full_symbol_create_from_source(3, data2, sizeof(data2));
    
    swif_full_symbol_t *v1 = full_symbol_clone(symbol1);
    swif_full_symbol_t *v2 = full_symbol_add(symbol1,symbol2);

    full_symbol_dump(symbol1,stdout);
    full_symbol_dump(symbol2,stdout);
    full_symbol_dump(v1,stdout);
    full_symbol_dump(v2,stdout);
    full_symbol_add_with_elimination(set1, v1);
    full_symbol_set_dump(set1,stdout);
    full_symbol_add_with_elimination(set1, v2);
    full_symbol_set_dump(set1,stdout);
    full_symbol_free(symbol1);
    full_symbol_free(symbol2);
    full_symbol_free(v1);
    full_symbol_free(v2);
    full_symbol_set_free(set1);

}
void test_clone(void)
{
     uint8_t data1[4] = {0x01, 0x00, 0x00};
    swif_full_symbol_t* symbol1 = full_symbol_create_from_source(1, data1, sizeof(data1));
    swif_full_symbol_t *v1 = full_symbol_clone(symbol1);
    full_symbol_free(v1);
    full_symbol_free(symbol1);

}

void test_rref_mat(void)
{
    uint8_t data[7][11] = {{   0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
			   {   0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
			   {  68, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
			   { 215, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
			   {   0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
			   { 153, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
			   {  69, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

    swif_full_symbol_set_t *set = full_symbol_set_alloc();
    uint8_t content;
    for (int i=0; i<7; i++) {
	swif_full_symbol_t *symbol
	    = full_symbol_create(data[i], 0, 11, &content, 0);
	full_symbol_dump(symbol, stdout);
	full_symbol_add_with_elimination(set, symbol);
	full_symbol_free(symbol);
    }
    full_symbol_set_dump(set, stdout);
    full_symbol_set_free(set);
}

int main(int argc, char **argv)
{
    //test_set();
    //test_set2();
    //test_add_with_elimination();
    //test_clone();
    test_rref_mat();
    exit(0);
}
