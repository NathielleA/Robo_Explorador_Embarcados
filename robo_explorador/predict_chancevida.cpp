#include "predict_chancevida.h"

// Implementação das funções
/*
 * File: predict_chancevida_data.c
 *
 * MATLAB Coder version            : 5.2
 * C/C++ source code generated on  : 06-Jun-2025 09:14:57
 */



/*
 * File trailer for predict_chancevida_data.c
 *
 * [EOF]
 */

/*
 * File: predict_chancevida_initialize.c
 *
 * MATLAB Coder version            : 5.2
 * C/C++ source code generated on  : 06-Jun-2025 09:14:57
 */



/* Function Definitions */
/*
 * Arguments    : void
 * Return Type  : void
 */
void predict_chancevida_initialize(void)
{
}

/*
 * File trailer for predict_chancevida_initialize.c
 *
 * [EOF]
 */

/*
 * File: predict_chancevida_terminate.c
 *
 * MATLAB Coder version            : 5.2
 * C/C++ source code generated on  : 06-Jun-2025 09:14:57
 */



/* Function Definitions */
/*
 * Arguments    : void
 * Return Type  : void
 */
void predict_chancevida_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for predict_chancevida_terminate.c
 *
 * [EOF]
 */

/*
 * File: predict_chancevida.c
 *
 * MATLAB Coder version            : 5.2
 * C/C++ source code generated on  : 06-Jun-2025 09:14:57
 */

/* Include Files */
#include "predict_chancevida.h"
#include <math.h>

/* Function Definitions */
/*
 * entrada: vetor 1x4 [temperatura, umidade, gás, luz]
 *
 * Arguments    : const double entrada[4]
 * Return Type  : double
 */
double predict_chancevida(const double entrada[4])
{
  static const double c_a[40] = {
      -0.0812, -0.8504, 2.18,    -0.001,  -0.0162, 0.005,   0.0125,  -1.7949,
      -2.1488, -0.2558, -1.3015, 0.802,   0.0062,  0.0095,  0.0181,  -0.1701,
      -3.0706, 0.4438,  -0.0011, 0.6854,  -2.349,  -0.2009, 0.0031,  0.0146,
      0.0278,  -0.3669, 0.0372,  -1.6067, 0.006,   -1.7261, -0.0827, -0.8443,
      0.0113,  2.1782,  2.3891,  1.4967,  0.0155,  0.8339,  0.004,   0.1246};
  static const double b_a[10] = {0.0467, 0.0426,  -1.0885, 0.6245,  -0.5541,
                                 0.0814, -0.4242, -0.0089, -1.0967, 1.3598};
  static const double bias1[10] = {-2.6087, 1.7565,  -0.5134, 1.6241,  -1.3888,
                                   1.5299,  -0.1221, -1.4091, -0.8787, -3.56};
  double a;
  double unnamed_idx_0;
  double unnamed_idx_1;
  double unnamed_idx_2;
  double unnamed_idx_3;
  int k;
  /*  Parâmetros de normalização */
  /*  Normalização Min-Max para [-1, 1] */
  /*  Pesos e bias da camada oculta */
  /*  Ativação tansig (hiperbólica) */
  /*  Função tansig (tangente hiperbólica) */
  unnamed_idx_0 = 2.0 * (entrada[0] - 5.7937) / 35.5363 - 1.0;
  unnamed_idx_1 = 2.0 * (entrada[1] - 20.5961) / 61.303900000000006 - 1.0;
  unnamed_idx_2 = 2.0 * (entrada[2] - 9.4146) / 208.3854 - 1.0;
  unnamed_idx_3 = 2.0 * (entrada[3] - 160.5827) / 925.91730000000007 - 1.0;
  /*  Camada de saída */
  /*  Desnormalização da saída ([-1, 1] para real) */
  a = 0.0;
  for (k = 0; k < 10; k++) {
    a += b_a[k] *
         (2.0 / (exp(-2.0 *
                     ((((c_a[k] * unnamed_idx_0 + c_a[k + 10] * unnamed_idx_1) +
                        c_a[k + 20] * unnamed_idx_2) +
                       c_a[k + 30] * unnamed_idx_3) +
                      bias1[k])) +
                 1.0) -
          1.0);
  }
  return ((a + -0.5273) + 1.0) / 2.0 * 47.2998 + 27.8333;
}

/*
 * File trailer for predict_chancevida.c
 *
 * [EOF]
 */
