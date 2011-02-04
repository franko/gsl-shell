/* This is a generated file. DO NOT EDIT! */

static const FoldFunc fold_func[] = {
  fold_kfold_numarith,
  fold_kfold_fpmath,
  fold_kfold_powi,
  fold_kfold_intarith,
  fold_kfold_bnot,
  fold_kfold_bswap,
  fold_kfold_tonum,
  fold_kfold_tobit,
  fold_kfold_toint,
  fold_kfold_tostr_knum,
  fold_kfold_tostr_kint,
  fold_kfold_strto,
  fold_kfold_snew_kptr,
  fold_kfold_snew_empty,
  fold_kfold_strref,
  fold_kfold_strref_snew,
  fold_kfold_numcomp,
  fold_kfold_intcomp,
  fold_kfold_strcmp,
  lj_opt_cse,
  fold_kfold_kref,
  fold_shortcut_round,
  fold_shortcut_left,
  fold_shortcut_dropleft,
  fold_shortcut_leftleft,
  fold_shortcut_leftleft_toint,
  fold_shortcut_leftleft_across_phi,
  fold_simplify_numadd_negx,
  fold_simplify_numadd_xneg,
  fold_simplify_numsub_k,
  fold_simplify_numsub_negk,
  fold_simplify_numsub_xneg,
  fold_simplify_nummuldiv_k,
  fold_simplify_nummuldiv_negk,
  fold_simplify_nummuldiv_negneg,
  fold_simplify_powi_xk,
  fold_simplify_powi_kx,
  fold_narrow_convert,
  fold_cse_toint,
  fold_simplify_intadd_k,
  fold_simplify_intsub_k,
  fold_simplify_intsub,
  fold_simplify_intsubadd_leftcancel,
  fold_simplify_intsubsub_leftcancel,
  fold_simplify_intsubsub_rightcancel,
  fold_simplify_intsubadd_rightcancel,
  fold_simplify_intsubaddadd_cancel,
  fold_simplify_band_k,
  fold_simplify_bor_k,
  fold_simplify_bxor_k,
  fold_simplify_shift_ik,
  fold_simplify_shift_andk,
  fold_simplify_shift1_ki,
  fold_simplify_shift2_ki,
  fold_reassoc_intarith_k,
  fold_reassoc_dup,
  fold_reassoc_bxor,
  fold_reassoc_shift,
  fold_reassoc_minmax_k,
  fold_reassoc_minmax_left,
  fold_reassoc_minmax_right,
  fold_abc_fwd,
  fold_abc_k,
  fold_abc_invar,
  fold_comm_swap,
  fold_comm_equal,
  fold_comm_comp,
  fold_comm_dup,
  fold_comm_bxor,
  fold_merge_eqne_snew_kgc,
  lj_opt_fwd_aload,
  fold_kfold_hload_kptr,
  lj_opt_fwd_hload,
  lj_opt_fwd_uload,
  lj_opt_fwd_tab_len,
  fold_cse_uref,
  fold_fwd_href_tnew,
  fold_fwd_href_tdup,
  fold_cse_href,
  fold_fload_tab_tnew_asize,
  fold_fload_tab_tnew_hmask,
  fold_fload_tab_tdup_asize,
  fold_fload_tab_tdup_hmask,
  fold_fload_tab_ah,
  fold_fload_str_len_kgc,
  fold_fload_str_len_snew,
  lj_opt_cse,
  lj_opt_fwd_fload,
  fold_fwd_sload,
  fold_xload_kptr,
  fold_fwd_xload,
  fold_barrier_tab,
  fold_barrier_tnew_tdup,
  lj_opt_dse_ahstore,
  lj_opt_dse_ustore,
  lj_opt_dse_fstore,
  lj_ir_emit
};

static const uint32_t fold_hash[412] = {
0xffffffff,
0xffffffff,
0x10170a0a,
0x11150606,
0x361c1c06,
0x351f06ff,
0x4a4dff08,
0x140c0707,
0xffffffff,
0x4d324405,
0x2f1aff06,
0xffffffff,
0x04180600,
0x4024ffff,
0x3a2d2d0a,
0x10160a0a,
0x361b1b06,
0x11140606,
0x341e06ff,
0xffffffff,
0x6043ffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x15262602,
0x2923ffff,
0xffffffff,
0xffffffff,
0x10150a0a,
0x11130606,
0xffffffff,
0x341d06ff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x4022ffff,
0xffffffff,
0xffffffff,
0x10140a0a,
0x11120606,
0x4d32440a,
0xffffffff,
0xffffffff,
0x5f41ffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x10130a0a,
0x11110606,
0x16264701,
0x0f3742ff,
0x3a2c2c0a,
0x361a1a06,
0x5e40ffff,
0xffffffff,
0xffffffff,
0x2d23ff22,
0x372d2dff,
0xffffffff,
0xffffffff,
0x10120a0a,
0x11100606,
0xffffffff,
0xffffffff,
0xffffffff,
0x5d3fffff,
0x563bff00,
0x533bff05,
0xffffffff,
0x3b2c2dff,
0xffffffff,
0xffffffff,
0xffffffff,
0x10110a0a,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x5d3effff,
0x493aff00,
0xffffffff,
0x19474800,
0xffffffff,
0x4c3243ff,
0xffffffff,
0xffffffff,
0x10100a0a,
0x110e0606,
0x3b2d2cff,
0x1a49470a,
0xffffffff,
0x5a3dffff,
0x4b3507ff,
0x4839ff00,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x15262601,
0x140d0606,
0x372c2cff,
0xffffffff,
0xffffffff,
0x583cffff,
0x4b3407ff,
0x4638ff00,
0xffffffff,
0xffffffff,
0x441cffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x140c0606,
0xffffffff,
0x02270a06,
0xffffffff,
0x1629290a,
0x573bffff,
0x094a0a00,
0x002d0a0a,
0xffffffff,
0x431bffff,
0xffffffff,
0xffffffff,
0x100d0a0a,
0xffffffff,
0xffffffff,
0xffffffff,
0x381c1cff,
0x16264700,
0xffffffff,
0xffffffff,
0x002c0a0a,
0xffffffff,
0x431affff,
0xffffffff,
0xffffffff,
0x100c0a0a,
0xffffffff,
0x371b1bff,
0x272fff06,
0xffffffff,
0xffffffff,
0x18191900,
0xffffffff,
0x002b0a0a,
0xffffffff,
0x1729280a,
0x533bff04,
0x1a4847ff,
0xffffffff,
0xffffffff,
0xffffffff,
0x272eff06,
0x18181800,
0xffffffff,
0xffffffff,
0xffffffff,
0x3e0eff06,
0x002a0a0a,
0x1828280a,
0xffffffff,
0xffffffff,
0xffffffff,
0x5c454400,
0x0b4b0700,
0x07490a0a,
0xffffffff,
0xffffffff,
0xffffffff,
0x0c420806,
0x140d09ff,
0x00290a0a,
0x4217ffff,
0xffffffff,
0xffffffff,
0x0a4a0600,
0xffffffff,
0xffffffff,
0xffffffff,
0x15262600,
0xffffffff,
0xffffffff,
0x371a1aff,
0x140c09ff,
0x00280a0a,
0x4216ffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x5c454300,
0xffffffff,
0xffffffff,
0x2549230a,
0x4215ffff,
0x2125280a,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x22252828,
0xffffffff,
0x5b34ffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x4214ffff,
0x2124280a,
0x523b4406,
0x06470600,
0xffffffff,
0xffffffff,
0x22242828,
0xffffffff,
0x6033ffff,
0xffffffff,
0xffffffff,
0x00250a0a,
0x03230606,
0x4213ffff,
0x1e23280a,
0xffffffff,
0x08480aff,
0x2549220a,
0x533bff03,
0xffffffff,
0xffffffff,
0x4e32ffff,
0xffffffff,
0xffffffff,
0x00240a0a,
0x03220606,
0x4212ffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x36222206,
0xffffffff,
0x2327ff06,
0x503b4306,
0x24270aff,
0xffffffff,
0xffffffff,
0x00230a0a,
0x03210606,
0x4211ffff,
0xffffffff,
0x39212106,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x01260aff,
0x254823ff,
0xffffffff,
0x00220a0a,
0x03200606,
0x4210ffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x3c2cff2d,
0xffffffff,
0xffffffff,
0x292fffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x031f0606,
0x600fffff,
0xffffffff,
0xffffffff,
0x604effff,
0x4d324407,
0xffffffff,
0xffffffff,
0xffffffff,
0x402effff,
0xffffffff,
0x1b2228ff,
0xffffffff,
0x031e0606,
0x3f0effff,
0x254822ff,
0xffffffff,
0x604dffff,
0xffffffff,
0x513b4405,
0x2823ff06,
0x39202006,
0x432dffff,
0xffffffff,
0xffffffff,
0x130d3b09,
0x031d0606,
0x410dffff,
0x0d42ff06,
0xffffffff,
0xffffffff,
0xffffffff,
0x391f1f06,
0x2722ff06,
0xffffffff,
0x432cffff,
0xffffffff,
0xffffffff,
0x130c3b09,
0x031c0606,
0x410cffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x3221ff06,
0x553b4200,
0xffffffff,
0xffffffff,
0x3321ff1a,
0x4f3b4305,
0x3d0eff22,
0x031b0606,
0x2b2323ff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x3220ff06,
0x140dff09,
0xffffffff,
0x3c2dff2c,
0x3320ff1a,
0xffffffff,
0x031a0606,
0x124c4f00,
0xffffffff,
0x0e370706,
0xffffffff,
0x5b45ff00,
0xffffffff,
0x321fff06,
0x140cff09,
0xffffffff,
0xffffffff,
0x391e1e06,
0x331fff1a,
0x2025ff0a,
0xffffffff,
0xffffffff,
0xffffffff,
0x2648ffff,
0x6044ff00,
0xffffffff,
0x321eff06,
0x2a2322ff,
0x391d1d06,
0xffffffff,
0x331eff1a,
0x2024ff0a,
0xffffffff,
0x47390800,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x543b0700,
0x321dff06,
0xffffffff,
0x16264702,
0xffffffff,
0x331dff1a,
0x1d23ff0a,
0x450d4207,
0x11170606,
0x352106ff,
0xffffffff,
0x2c23ff23,
0x5b46ffff,
0x1f23ff28,
0x311cff06,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x450c4207,
0x11160606,
0x352006ff,
0x533bff06,
0x140d0707,
0x1c22ff28,
0x2e232222,
0x301bff06,
0x593d08ff,
0x05190600,
0xffffffff
};

#define fold_hashkey(k)	(lj_rol(lj_rol((k),13)-(k),23)%411)

