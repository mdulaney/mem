#include "CUnit/Basic.h"

#include "cpu.h"
#include "ram.h"

int init_cpu_suite(void) {
	return cpu_init(DEFAULT_RAM_SIZE);
}

int clean_cpu_suite(void) {
	return cpu_shutdown();
}

void test_cpu_write(void) {
	unsigned int *write_ptr = (unsigned int *)cpu_get_vm_base();
	*write_ptr = 0x1;
}

int main()
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("cpu", init_cpu_suite, clean_cpu_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
   if ((NULL == CU_add_test(pSuite, "write mem", test_cpu_write)))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}



