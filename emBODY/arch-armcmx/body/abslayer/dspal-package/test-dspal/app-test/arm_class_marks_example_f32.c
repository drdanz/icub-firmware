/* ---------------------------------------------------------------------- 
* Copyright (C) 2010 ARM Limited. All rights reserved.   
*  
* $Date:        29. November 2010  
* $Revision: 	V1.0.3 
*  
* Project: 	    CMSIS DSP Library  
* Title:        arm_class_marks_example_f32.c		  
*  
* Description:	Example code to calculate Minimum, Maximum 
*               Mean, std and variance of marks obtained in a class 
* 
* Target Processor: Cortex-M4/Cortex-M3
*
* Version 1.0.3 2010/11/29 
*    Re-organized the CMSIS folders and updated documentation. 
*  
* Version 1.0.1 2010/10/05 KK 
*    Production release and review comments incorporated.  
*
* Version 1.0.0 2010/09/20 KK
*    Production release and review comments incorporated.
* ------------------------------------------------------------------- */ 
 
/** 
 * @ingroup groupExamples 
 */ 
 
/**    
 * @defgroup ClassMarks Class Marks Example  
 *
 * \par Description: 
 * \par
 * Demonstrates the use the Maximum, Minimum, Mean, Standard Deviation, Variance
 * and Matrix functions to calculate statistical values of marks obtained in a class.
 *
 * \note This example also demonstrates the usage of static initialization.
 *  
 * \par Variables Description:
 * \par
 * \li \c testMarks_f32 points to the marks scored by 20 students in 4 subjects
 * \li \c max_marks     Maximum of all marks 
 * \li \c min_marks     Minimum of all marks 
 * \li \c mean          Mean of all marks
 * \li \c var           Variance of the marks
 * \li \c std           Standard deviation of the marks 
 * \li \c numStudents   Total number of students in the class
 *
 * \par CMSIS DSP Software Library Functions Used:
 * \par
 * - arm_mat_init_f32()
 * - arm_mat_mult_f32()
 * - arm_max_f32()
 * - arm_min_f32()
 * - arm_mean_f32()
 * - arm_std_f32()
 * - arm_var_f32()
 * 
 * <b> Refer  </b> 
 * \link arm_class_marks_example_f32.c \endlink
 * 
 */ 
 
 
/** \example arm_class_marks_example_f32.c 
  */  
//#include "arm_math.h" 

#include "dspal.h"
 
#define USE_STATIC_INIT 
 
 /* ---------------------------------------------------------------------- 
** Global defines  
** ------------------------------------------------------------------- */ 
 
#define TEST_LENGTH_SAMPLES 	(20*4) 
 
/* ---------------------------------------------------------------------- 
** List of Marks scored by 20 students for 4 subjects 
** ------------------------------------------------------------------- */  
const dspal_f32_t testMarks_f32[TEST_LENGTH_SAMPLES] =  
{    
	42.000000,	37.000000,	81.000000,	28.000000,	 
	83.000000,	72.000000,	36.000000,	38.000000,	 
	32.000000,	51.000000,	63.000000,	64.000000,	 
	97.000000,	82.000000,	95.000000,	90.000000,	 
	66.000000,	51.000000,	54.000000,	42.000000,	 
	67.000000,	56.000000,	45.000000,	57.000000,	 
	67.000000,	69.000000,	35.000000,	52.000000,	 
	29.000000,	81.000000,	58.000000,	47.000000,	 
	38.000000,	76.000000,	100.000000,	29.000000,	 
	33.000000,	47.000000,	29.000000,	50.000000,	 
	34.000000,	41.000000,	61.000000,	46.000000,	 
	52.000000,	50.000000,	48.000000,	36.000000,	 
	47.000000,	55.000000,	44.000000,	40.000000,	 
	100.000000,	94.000000,	84.000000,	37.000000,	 
	32.000000,	71.000000,	47.000000,	77.000000,	 
	31.000000,	50.000000,	49.000000,	35.000000,	 
	63.000000,	67.000000,	40.000000,	31.000000,	 
	29.000000,	68.000000,	61.000000,	38.000000,	 
	31.000000,	28.000000,	28.000000,	76.000000,	 
	55.000000,	33.000000,	29.000000,	39.000000 
};  
 
 
/* ---------------------------------------------------------------------- 
* Number of subjects X 1  
* ------------------------------------------------------------------- */  
const dspal_f32_t testUnity_f32[4] =  
{    
	1.000,  1.000, 	1.000,  1.000 
}; 
 
 
/* ---------------------------------------------------------------------- 
** f32 Output buffer 
** ------------------------------------------------------------------- */  
static dspal_f32_t testOutput[TEST_LENGTH_SAMPLES]; 
 
 
/* ------------------------------------------------------------------ 
* Global defines  
*------------------------------------------------------------------- */ 
#define 	NUMSTUDENTS  20 
#define     NUMSUBJECTS  4 
 
/* ------------------------------------------------------------------ 
* Global variables  
*------------------------------------------------------------------- */ 
 
uint32_t  	numStudents = 20; 
uint32_t  	numSubjects = 4;  
dspal_f32_t	max_marks, min_marks, mean, std, var; 
uint32_t 	student_num;    
 
/* ---------------------------------------------------------------------------------- 
* Main f32 test function.  It returns maximum marks secured and student number 
* ------------------------------------------------------------------------------- */ 
 
extern void test_class_marks_main(void) 
{ 



    








 
#ifndef  USE_STATIC_INIT 
 
  	dspal_matrix_f32_t srcA; 
  	dspal_matrix_f32_t srcB; 
  	dspal_matrix_f32_t dstC;  
 
	/* Input and output matrices initializations */  
	arm_mat_init_f32(&srcA, numStudents, numSubjects, (dspal_f32_t *)testMarks_f32);  
	arm_mat_init_f32(&srcB, numSubjects, 1, (dspal_f32_t *)testUnity_f32);  
	arm_mat_init_f32(&dstC, numStudents, 1, testOutput);  
 
#else 
 
	/* Static Initializations of Input and output matrix sizes and array */ 
	dspal_matrix_f32_t srcA = {NUMSTUDENTS, NUMSUBJECTS, (dspal_f32_t *)testMarks_f32}; 
	dspal_matrix_f32_t srcB = {NUMSUBJECTS, 1, (dspal_f32_t *)testUnity_f32}; 
	dspal_matrix_f32_t dstC = {NUMSTUDENTS, 1, testOutput}; 
 
#endif 
 
	 
	/* ---------------------------------------------------------------------- 
	*Call the Matrix multiplication process function   
	* ------------------------------------------------------------------- */ 
	dspal_matrix_mult_f32(&srcA, &srcB, &dstC); 
	 
	/* ---------------------------------------------------------------------- 
	** Call the Max function to calculate max marks among numStudents 
	** ------------------------------------------------------------------- */ 
	dspal_statistics_max_f32(testOutput, numStudents, &max_marks, &student_num);  
 
	/* ---------------------------------------------------------------------- 
	** Call the Min function to calculate min marks among numStudents 
	** ------------------------------------------------------------------- */ 
	dspal_statistics_min_f32(testOutput, numStudents, &min_marks, &student_num);  
 
	/* ---------------------------------------------------------------------- 
	** Call the Mean function to calculate mean 
	** ------------------------------------------------------------------- */ 
	dspal_statistics_mean_f32(testOutput, numStudents, &mean); 
 
	/* ---------------------------------------------------------------------- 
	** Call the std function to calculate standard deviation 
	** ------------------------------------------------------------------- */ 
	dspal_statistics_std_f32(testOutput, numStudents, &std); 
 
	/* ---------------------------------------------------------------------- 
	** Call the var function to calculate variance 
	** ------------------------------------------------------------------- */ 
	dspal_statistics_var_f32(testOutput, numStudents, &var); 
 
} 
 
 
