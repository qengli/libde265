/*
 * H.265 video codec.
 * Copyright (c) 2013-2014 struktur AG, Dirk Farin <farin@struktur.de>
 *
 * Authors: struktur AG, Dirk Farin <farin@struktur.de>
 *
 * This file is part of libde265.
 *
 * libde265 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * libde265 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libde265.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "libde265/encoder/algo/cb-predmode.h"
#include "libde265/encoder/algo/coding-options.h"
#include "libde265/encoder/encoder-context.h"
#include <assert.h>
#include <limits>
#include <math.h>




enc_cb* Algo_CB_PredMode_BruteForce::analyze(encoder_context* ectx,
                                             context_model_table& ctxModel,
                                             enc_cb* cb)
{
  assert(cb->pcm_flag==0);

  bool try_intra = true;
  bool try_inter = (ectx->shdr->slice_type != SLICE_TYPE_I);

  //try_intra = !try_inter; // TODO HACK: no intra in inter frames

  if (ectx->imgdata->frame_number > 0) {
    //printf("%d\n",ectx->imgdata->frame_number);
  }

  // 0: intra
  // 1: inter

  CodingOptions options(ectx,cb,ctxModel);

  CodingOption option_intra = options.new_option(try_intra);
  CodingOption option_inter = options.new_option(try_inter);

  options.start();

  enc_cb* cb_inter = NULL;
  enc_cb* cb_intra = NULL;

  const int log2CbSize = cb->log2Size;
  const int x = cb->x;
  const int y = cb->y;

  // try encoding with inter

  if (option_inter) {
    option_inter.begin();
    cb_inter = option_inter.get_cb();

    cb_inter->PredMode = MODE_INTER;
    ectx->img->set_pred_mode(x,y, log2CbSize, MODE_INTER);

    enc_cb* cb_result;
    option_inter.set_cb( cb_result=mInterAlgo->analyze(ectx, option_inter.get_context(), cb_inter));

    option_inter.end();
  }


  // try intra

  if (option_intra) {
    option_intra.begin();
    cb_intra = option_intra.get_cb();

    cb_intra->PredMode = MODE_INTRA;
    ectx->img->set_pred_mode(x,y, log2CbSize, MODE_INTRA);

    enc_cb* cb_result;
    option_intra.set_cb( cb_result=mIntraAlgo->analyze(ectx, option_intra.get_context(), cb_intra));

    option_intra.end();
  }


  options.compute_rdo_costs();
  return options.return_best_rdo();
}
