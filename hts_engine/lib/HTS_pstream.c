/* ----------------------------------------------------------------- */
/*           The HMM-Based Speech Synthesis Engine "hts_engine API"  */
/*           developed by HTS Working Group                          */
/*           http://hts-engine.sourceforge.net/                      */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2001-2015  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/*                2001-2008  Tokyo Institute of Technology           */
/*                           Interdisciplinary Graduate School of    */
/*                           Science and Engineering                 */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the HTS working group nor the names of its  */
/*   contributors may be used to endorse or promote products derived */
/*   from this software without specific prior written permission.   */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#ifndef HTS_PSTREAM_C
#define HTS_PSTREAM_C

#ifdef __cplusplus
#define HTS_PSTREAM_C_START extern "C" {
#define HTS_PSTREAM_C_END   }
#else
#define HTS_PSTREAM_C_START
#define HTS_PSTREAM_C_END
#endif                          /* __CPLUSPLUS */

HTS_PSTREAM_C_START;

#include <math.h>               /* for sqrt() */

/* hts_engine libraries */
#include "HTS_hidden.h"

/* HTS_finv: calculate 1.0/variance function */
static float HTS_finv(const float x)
{
   if (x >= INFTY2)
      return 0.0;
   if (x <= -INFTY2)
      return 0.0;
   if (x <= INVINF2 && x >= 0)
      return INFTY;
   if (x >= -INVINF2 && x < 0)
      return -INFTY;

   return (1.0 / x);
}

/* HTS_PStream_calc_wuw_and_wum: calcurate W'U^{-1}W and W'U^{-1}M */
static void HTS_PStream_calc_wuw_and_wum(HTS_PStream * pst, size_t m)
{
   size_t t, i, j;
   int shift;
   float wu;

   for (t = 0; t < pst->length; t++) {
      /* initialize */
      pst->sm[t].wum = 0.0;
      for (i = 0; i < pst->width; i++)
         pst->sm[t].wuw[i] = 0.0;

      /* calc WUW & WUM */
      for (i = 0; i < pst->win_size; i++)
         for (shift = pst->win_l_width[i]; shift <= pst->win_r_width[i]; shift++)
            if (((int) t + shift >= 0) && ((int) t + shift < pst->length) && (pst->win_coefficient[i][-shift] != 0.0)) {
               wu = pst->win_coefficient[i][-shift] * pst->sm[t + shift].ivar[i * pst->vector_length + m];
               pst->sm[t].wum += wu * pst->sm[t + shift].mean[i * pst->vector_length + m];
               for (j = 0; (j < pst->width) && (t + j < pst->length); j++)
                  if (((int) j <= pst->win_r_width[i] + shift) && (pst->win_coefficient[i][j - shift] != 0.0))
                     pst->sm[t].wuw[j] += wu * pst->win_coefficient[i][j - shift];
            }
   }
}


/* HTS_PStream_ldl_factorization: Factorize W'*U^{-1}*W to L*D*L' (L: lower triangular, D: diagonal) */
static void HTS_PStream_ldl_factorization(HTS_PStream * pst)
{
   size_t t, i, j;

   for (t = 0; t < pst->length; t++) {
      for (i = 1; (i < pst->width) && (t >= i); i++)
         pst->sm[t].wuw[0] -= pst->sm[t - i].wuw[i] * pst->sm[t - i].wuw[i] * pst->sm[t - i].wuw[0];

      for (i = 1; i < pst->width; i++) {
         for (j = 1; (i + j < pst->width) && (t >= j); j++)
            pst->sm[t].wuw[i] -= pst->sm[t - j].wuw[j] * pst->sm[t - j].wuw[i + j] * pst->sm[t - j].wuw[0];
         pst->sm[t].wuw[i] /= pst->sm[t].wuw[0];
      }
   }
}

/* HTS_PStream_forward_substitution: forward subtitution for mlpg */
static void HTS_PStream_forward_substitution(HTS_PStream * pst)
{
   size_t t, i;

   for (t = 0; t < pst->length; t++) {
      pst->sm[t].g = pst->sm[t].wum;
      for (i = 1; (i < pst->width) && (t >= i); i++)
         pst->sm[t].g -= pst->sm[t - i].wuw[i] * pst->sm[t - i].g;
   }
}

/* HTS_PStream_backward_substitution: backward subtitution for mlpg */
static void HTS_PStream_backward_substitution(HTS_PStream * pst, size_t m)
{
   size_t rev, t, i;

   for (rev = 0; rev < pst->length; rev++) {
      t = pst->length - 1 - rev;
      pst->par[t][m] = pst->sm[t].g / pst->sm[t].wuw[0];
      for (i = 1; (i < pst->width) && (t + i < pst->length); i++)
         pst->par[t][m] -= pst->sm[t].wuw[i] * pst->par[t + i][m];
   }
}

/* HTS_PStream_calc_gv: subfunction for mlpg using GV */
static void HTS_PStream_calc_gv(HTS_PStream * pst, size_t m, float *mean, float *vari)
{
   size_t t;

   *mean = 0.0;
   for (t = 0; t < pst->length; t++)
      if (pst->gv_switch[t])
         *mean += pst->par[t][m];
   *mean /= pst->gv_length;
   *vari = 0.0;
   for (t = 0; t < pst->length; t++)
      if (pst->gv_switch[t])
         *vari += (pst->par[t][m] - *mean) * (pst->par[t][m] - *mean);
   *vari /= pst->gv_length;
}

/* HTS_PStream_conv_gv: subfunction for mlpg using GV */
static void HTS_PStream_conv_gv(HTS_PStream * pst, size_t m)
{
   size_t t;
   float ratio;
   float mean;
   float vari;

   HTS_PStream_calc_gv(pst, m, &mean, &vari);
   ratio = sqrt(pst->gv_mean[m] / vari);
   for (t = 0; t < pst->length; t++)
      if (pst->gv_switch[t])
         pst->par[t][m] = ratio * (pst->par[t][m] - mean) + mean;
}

/* HTS_PStream_calc_derivative: subfunction for mlpg using GV */
static float HTS_PStream_calc_derivative(HTS_PStream * pst, size_t m)
{
   size_t t, i;
   float mean;
   float vari;
   float dv;
   float h;
   float gvobj;
   float hmmobj;
   float w = 1.0 / (pst->win_size * pst->length);

   HTS_PStream_calc_gv(pst, m, &mean, &vari);
   gvobj = -0.5 * W2 * vari * pst->gv_vari[m] * (vari - 2.0 * pst->gv_mean[m]);
   dv = -2.0 * pst->gv_vari[m] * (vari - pst->gv_mean[m]) / pst->length;

   for (t = 0; t < pst->length; t++) {
      pst->sm[t].g = pst->sm[t].wuw[0] * pst->par[t][m];
      for (i = 1; i < pst->width; i++) {
         if (t + i < pst->length)
            pst->sm[t].g += pst->sm[t].wuw[i] * pst->par[t + i][m];
         if (t + 1 > i)
            pst->sm[t].g += pst->sm[t - i].wuw[i] * pst->par[t - i][m];
      }
   }

   for (t = 0, hmmobj = 0.0; t < pst->length; t++) {
      hmmobj += W1 * w * pst->par[t][m] * (pst->sm[t].wum - 0.5 * pst->sm[t].g);
      h = -W1 * w * pst->sm[t].wuw[1 - 1] - W2 * 2.0 / (pst->length * pst->length) * ((pst->length - 1) * pst->gv_vari[m] * (vari - pst->gv_mean[m]) + 2.0 * pst->gv_vari[m] * (pst->par[t][m] - mean) * (pst->par[t][m] - mean));
      if (pst->gv_switch[t])
         pst->sm[t].g = 1.0 / h * (W1 * w * (-pst->sm[t].g + pst->sm[t].wum) + W2 * dv * (pst->par[t][m] - mean));
      else
         pst->sm[t].g = 1.0 / h * (W1 * w * (-pst->sm[t].g + pst->sm[t].wum));
   }

   return (-(hmmobj + gvobj));
}

/* HTS_PStream_gv_parmgen: function for mlpg using GV */
static void HTS_PStream_gv_parmgen(HTS_PStream * pst, size_t m)
{
   size_t t, i;
   float step = STEPINIT;
   float prev = 0.0;
   float obj;

   if (pst->gv_length == 0)
      return;

   HTS_PStream_conv_gv(pst, m);
   if (GV_MAX_ITERATION > 0) {
      HTS_PStream_calc_wuw_and_wum(pst, m);
      for (i = 1; i <= GV_MAX_ITERATION; i++) {
         obj = HTS_PStream_calc_derivative(pst, m);
         if (i > 1) {
            if (obj > prev)
               step *= STEPDEC;
            if (obj < prev)
               step *= STEPINC;
         }
         for (t = 0; t < pst->length; t++) {
            if (pst->gv_switch[t])
               pst->par[t][m] += step * pst->sm[t].g;
         }
         prev = obj;
      }
   }
}

/* HTS_PStream_mlpg: generate sequence of speech parameter vector maximizing its output probability for given pdf sequence */
static void HTS_PStream_mlpg(HTS_PStream * pst)
{
   size_t m;

   if (pst->length == 0)
      return;

   for (m = 0; m < pst->vector_length; m++) {
      HTS_PStream_calc_wuw_and_wum(pst, m);
      HTS_PStream_ldl_factorization(pst);       /* LDL factorization */
      HTS_PStream_forward_substitution(pst);    /* forward substitution   */
      HTS_PStream_backward_substitution(pst, m);        /* backward substitution  */
      if (pst->gv_length > 0)
         HTS_PStream_gv_parmgen(pst, m);
   }
}

/* HTS_PStreamSet_initialize: initialize parameter stream set */
void HTS_PStreamSet_initialize(HTS_PStreamSet * pss)
{
   pss->pstream = NULL;
   pss->nstream = 0;
   pss->total_frame = 0;
}

/* HTS_PStreamSet_create: parameter generation using GV weight */
HTS_Boolean HTS_PStreamSet_create(HTS_PStreamSet * pss, HTS_SStreamSet * sss, float *msd_threshold, float *gv_weight)
{
   size_t i, j, k, l, m;
   int shift;
   size_t frame, msd_frame, state;

   HTS_PStream *pst;
   HTS_Boolean not_bound;

   if (pss->nstream != 0) {
      HTS_error(1, "HTS_PstreamSet_create: HTS_PStreamSet should be clear.\n");
      return FALSE;
   }

   /* initialize */
   pss->nstream = HTS_SStreamSet_get_nstream(sss);
   pss->pstream = (HTS_PStream *) HTS_calloc(pss->nstream, sizeof(HTS_PStream));
   pss->total_frame = HTS_SStreamSet_get_total_frame(sss);

   /* create */
   for (i = 0; i < pss->nstream; i++) {
      pst = &pss->pstream[i];
      if (HTS_SStreamSet_is_msd(sss, i) == TRUE) {      /* for MSD */
         pst->length = 0;
         for (state = 0; state < HTS_SStreamSet_get_total_state(sss); state++)
            if (HTS_SStreamSet_get_msd(sss, i, state) > msd_threshold[i])
               pst->length += HTS_SStreamSet_get_duration(sss, state);
         pst->msd_flag = (HTS_Boolean *) HTS_calloc(pss->total_frame, sizeof(HTS_Boolean));
         for (state = 0, frame = 0; state < HTS_SStreamSet_get_total_state(sss); state++) {
            if (HTS_SStreamSet_get_msd(sss, i, state) > msd_threshold[i]) {
               for (j = 0; j < HTS_SStreamSet_get_duration(sss, state); j++) {
                  pst->msd_flag[frame] = TRUE;
                  frame++;
               }
            } else {
               for (j = 0; j < HTS_SStreamSet_get_duration(sss, state); j++) {
                  pst->msd_flag[frame] = FALSE;
                  frame++;
               }
            }
         }
      } else {                  /* for non MSD */
         pst->length = pss->total_frame;
         pst->msd_flag = NULL;
      }
      pst->vector_length = HTS_SStreamSet_get_vector_length(sss, i);
      pst->width = HTS_SStreamSet_get_window_max_width(sss, i) * 2 + 1; /* band width of R */
      pst->win_size = HTS_SStreamSet_get_window_size(sss, i);
      if (pst->length > 0) {
         pst->sm = (HTS_SMatrices *)HTS_calloc(pst->length, sizeof(HTS_SMatrices));
         for (j = 0; j < pst->length; j++) {
            HTS_SMatrices *sm = &pst->sm[j];
            sm->mean = HTS_calloc(pst->vector_length * pst->win_size, sizeof(float));
            sm->ivar = HTS_calloc(pst->vector_length * pst->win_size, sizeof(float));
            sm->wum = 0.0;
            sm->wuw = HTS_calloc(pst->width, sizeof(float));
            sm->g = 0.0;
         }
         pst->par = HTS_alloc_matrix(pst->length, pst->vector_length);
      }
      /* copy dynamic window */
      pst->win_l_width = (int *) HTS_calloc(pst->win_size, sizeof(int));
      pst->win_r_width = (int *) HTS_calloc(pst->win_size, sizeof(int));
      pst->win_coefficient = (float **) HTS_calloc(pst->win_size, sizeof(float));
      for (j = 0; j < pst->win_size; j++) {
         pst->win_l_width[j] = HTS_SStreamSet_get_window_left_width(sss, i, j);
         pst->win_r_width[j] = HTS_SStreamSet_get_window_right_width(sss, i, j);
         if (pst->win_l_width[j] + pst->win_r_width[j] == 0)
            pst->win_coefficient[j] = (float *)
                HTS_calloc(-2 * pst->win_l_width[j] + 1, sizeof(float));
         else
            pst->win_coefficient[j] = (float *)
                HTS_calloc(-2 * pst->win_l_width[j], sizeof(float));
         pst->win_coefficient[j] -= pst->win_l_width[j];
         for (shift = pst->win_l_width[j]; shift <= pst->win_r_width[j]; shift++)
            pst->win_coefficient[j][shift] = HTS_SStreamSet_get_window_coefficient(sss, i, j, shift);
      }
      /* copy GV */
      if (HTS_SStreamSet_use_gv(sss, i)) {
         pst->gv_mean = (float *) HTS_calloc(pst->vector_length, sizeof(float));
         pst->gv_vari = (float *) HTS_calloc(pst->vector_length, sizeof(float));
         for (j = 0; j < pst->vector_length; j++) {
            pst->gv_mean[j] = HTS_SStreamSet_get_gv_mean(sss, i, j) * gv_weight[i];
            pst->gv_vari[j] = HTS_SStreamSet_get_gv_vari(sss, i, j);
         }
         pst->gv_switch = (HTS_Boolean *) HTS_calloc(pst->length, sizeof(HTS_Boolean));
         if (HTS_SStreamSet_is_msd(sss, i) == TRUE) {   /* for MSD */
            for (state = 0, frame = 0, msd_frame = 0; state < HTS_SStreamSet_get_total_state(sss); state++)
               for (j = 0; j < HTS_SStreamSet_get_duration(sss, state); j++, frame++)
                  if (pst->msd_flag[frame] == TRUE)
                     pst->gv_switch[msd_frame++] = HTS_SStreamSet_get_gv_switch(sss, i, state);
         } else {               /* for non MSD */
            for (state = 0, frame = 0; state < HTS_SStreamSet_get_total_state(sss); state++)
               for (j = 0; j < HTS_SStreamSet_get_duration(sss, state); j++)
                  pst->gv_switch[frame++] = HTS_SStreamSet_get_gv_switch(sss, i, state);
         }
         for (j = 0, pst->gv_length = 0; j < pst->length; j++)
            if (pst->gv_switch[j])
               pst->gv_length++;
      } else {
         pst->gv_switch = NULL;
         pst->gv_length = 0;
         pst->gv_mean = NULL;
         pst->gv_vari = NULL;
      }
      /* copy pdfs */
      if (HTS_SStreamSet_is_msd(sss, i) == TRUE) {      /* for MSD */
         for (state = 0, frame = 0, msd_frame = 0; state < HTS_SStreamSet_get_total_state(sss); state++) {
            for (j = 0; j < HTS_SStreamSet_get_duration(sss, state); j++) {
               if (pst->msd_flag[frame] == TRUE) {
                  /* check current frame is MSD boundary or not */
                  for (k = 0; k < pst->win_size; k++) {
                     not_bound = TRUE;
                     for (shift = pst->win_l_width[k]; shift <= pst->win_r_width[k]; shift++)
                        if ((int) frame + shift < 0 || (int) pss->total_frame <= (int) frame + shift || pst->msd_flag[frame + shift] != TRUE) {
                           not_bound = FALSE;
                           break;
                        }
                     for (l = 0; l < pst->vector_length; l++) {
                        m = pst->vector_length * k + l;
                        pst->sm[msd_frame].mean[m] = HTS_SStreamSet_get_mean(sss, i, state, m);
                        if (not_bound || k == 0)
                           pst->sm[msd_frame].ivar[m] = HTS_finv(HTS_SStreamSet_get_vari(sss, i, state, m));
                        else
                           pst->sm[msd_frame].ivar[m] = 0.0;
                     }
                  }
                  msd_frame++;
               }
               frame++;
            }
         }
      } else {                  /* for non MSD */
         for (state = 0, frame = 0; state < HTS_SStreamSet_get_total_state(sss); state++) {
            for (j = 0; j < HTS_SStreamSet_get_duration(sss, state); j++) {
               for (k = 0; k < pst->win_size; k++) {
                  not_bound = TRUE;
                  for (shift = pst->win_l_width[k]; shift <= pst->win_r_width[k]; shift++)
                     if ((int) frame + shift < 0 || (int) pss->total_frame <= (int) frame + shift) {
                        not_bound = FALSE;
                        break;
                     }
                  for (l = 0; l < pst->vector_length; l++) {
                     m = pst->vector_length * k + l;
                     pst->sm[frame].mean[m] = HTS_SStreamSet_get_mean(sss, i, state, m);
                     if (not_bound || k == 0)
                        pst->sm[frame].ivar[m] = HTS_finv(HTS_SStreamSet_get_vari(sss, i, state, m));
                     else
                        pst->sm[frame].ivar[m] = 0.0;
                  }
               }
               frame++;
            }
         }
      }
      /* parameter generation */
      HTS_PStream_mlpg(pst);
   }

   return TRUE;
}

/* HTS_PStreamSet_get_nstream: get number of stream */
size_t HTS_PStreamSet_get_nstream(HTS_PStreamSet * pss)
{
   return pss->nstream;
}

/* HTS_PStreamSet_get_vector_length: get feature length */
size_t HTS_PStreamSet_get_vector_length(HTS_PStreamSet * pss, size_t stream_index)
{
   return pss->pstream[stream_index].vector_length;
}

/* HTS_PStreamSet_get_total_frame: get total number of frame */
size_t HTS_PStreamSet_get_total_frame(HTS_PStreamSet * pss)
{
   return pss->total_frame;
}

/* HTS_PStreamSet_get_parameter: get parameter */
float HTS_PStreamSet_get_parameter(HTS_PStreamSet * pss, size_t stream_index, size_t frame_index, size_t vector_index)
{
   return pss->pstream[stream_index].par[frame_index][vector_index];
}

/* HTS_PStreamSet_get_parameter_vector: get parameter vector*/
float *HTS_PStreamSet_get_parameter_vector(HTS_PStreamSet * pss, size_t stream_index, size_t frame_index)
{
   return pss->pstream[stream_index].par[frame_index];
}

/* HTS_PStreamSet_get_msd_flag: get generated MSD flag per frame */
HTS_Boolean HTS_PStreamSet_get_msd_flag(HTS_PStreamSet * pss, size_t stream_index, size_t frame_index)
{
   return pss->pstream[stream_index].msd_flag[frame_index];
}

/* HTS_PStreamSet_is_msd: get MSD flag */
HTS_Boolean HTS_PStreamSet_is_msd(HTS_PStreamSet * pss, size_t stream_index)
{
   return pss->pstream[stream_index].msd_flag ? TRUE : FALSE;
}

/* HTS_PStreamSet_clear: free parameter stream set */
void HTS_PStreamSet_clear(HTS_PStreamSet * pss)
{
   size_t i, j;
   HTS_PStream *pstream;

   if (pss->pstream) {
      for (i = 0; i < pss->nstream; i++) {
         pstream = &pss->pstream[i];
         if (pstream->sm) {
             for (j = 0; j < pstream->length; j++) {
                 HTS_SMatrices *sm = &pstream->sm[j];
                 HTS_free(sm->mean);
                 HTS_free(sm->ivar);
                 HTS_free(sm->wuw);
             }
             HTS_free(pstream->sm);
         }
         if (pstream->par)
            HTS_free_matrix(pstream->par, pstream->length);
         if (pstream->msd_flag)
            HTS_free(pstream->msd_flag);
         if (pstream->win_coefficient) {
            for (j = 0; j < pstream->win_size; j++) {
               pstream->win_coefficient[j] += pstream->win_l_width[j];
               HTS_free(pstream->win_coefficient[j]);
            }
         }
         if (pstream->gv_mean)
            HTS_free(pstream->gv_mean);
         if (pstream->gv_vari)
            HTS_free(pstream->gv_vari);
         if (pstream->win_coefficient)
            HTS_free(pstream->win_coefficient);
         if (pstream->win_l_width)
            HTS_free(pstream->win_l_width);
         if (pstream->win_r_width)
            HTS_free(pstream->win_r_width);
         if (pstream->gv_switch)
            HTS_free(pstream->gv_switch);
      }
      HTS_free(pss->pstream);
   }
   HTS_PStreamSet_initialize(pss);
}

HTS_PSTREAM_C_END;

#endif                          /* !HTS_PSTREAM_C */
