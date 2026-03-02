/**
 * @file giac_cabi.h
 * @brief Pure C-ABI interface bridging g++ (GIAC) and clang++ (CxxWrap) on macOS/FreeBSD
 *
 * On macOS/FreeBSD, GIAC_jll is built with g++/libstdc++ while
 * libcxxwrap_julia_jll uses clang++/libc++. The std::string types are
 * ABI-incompatible between the two. This header defines an extern "C"
 * boundary using only C-compatible types (const char*, void*, int, etc.)
 * so that code compiled by either compiler can safely call across.
 *
 * All returned char* are malloc-allocated; caller must free with
 * giac_cabi_free_string(). All GenHandle/GiacContextHandle are
 * heap-allocated; caller must free with the corresponding destroy function.
 */

#ifndef GIAC_CABI_H
#define GIAC_CABI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque handles */
typedef void* GenHandle;
typedef void* GiacContextHandle;

/* === String management === */
void giac_cabi_free_string(char* s);

/* === Error handling === */
/* Returns 0 if no error, 1 if error. Copies message into buf. */
int  giac_cabi_get_last_error(char* buf, int buf_size);
void giac_cabi_clear_error(void);

/* === Version Functions === */
char* giac_cabi_get_giac_version(void);
char* giac_cabi_get_wrapper_version(void);
int   giac_cabi_check_giac_available(void);

/* === Configuration === */
void  giac_cabi_set_xcasroot(const char* path);
char* giac_cabi_get_xcasroot(void);
int   giac_cabi_init_help(const char* aide_cas_path);
char* giac_cabi_list_commands(void);
int   giac_cabi_help_count(void);

/* === Expression Evaluation === */
GenHandle giac_cabi_giac_eval(const char* expr);

/* === Generic Dispatch === */
GenHandle giac_cabi_apply_func0(const char* name);
GenHandle giac_cabi_apply_func1(const char* name, GenHandle arg);
GenHandle giac_cabi_apply_func2(const char* name, GenHandle arg1, GenHandle arg2);
GenHandle giac_cabi_apply_func3(const char* name, GenHandle arg1, GenHandle arg2, GenHandle arg3);
GenHandle giac_cabi_apply_funcN(const char* name, GenHandle* args, int32_t count);

/* === Function Listing === */
char* giac_cabi_list_builtin_functions(void);
int   giac_cabi_builtin_function_count(void);
char* giac_cabi_list_all_functions(void);

/* === Tier 1 Direct Wrappers === */
/* Single-argument */
GenHandle giac_cabi_sin(GenHandle arg);
GenHandle giac_cabi_cos(GenHandle arg);
GenHandle giac_cabi_tan(GenHandle arg);
GenHandle giac_cabi_asin(GenHandle arg);
GenHandle giac_cabi_acos(GenHandle arg);
GenHandle giac_cabi_atan(GenHandle arg);
GenHandle giac_cabi_exp(GenHandle arg);
GenHandle giac_cabi_ln(GenHandle arg);
GenHandle giac_cabi_log10(GenHandle arg);
GenHandle giac_cabi_sqrt(GenHandle arg);
GenHandle giac_cabi_abs(GenHandle arg);
GenHandle giac_cabi_sign(GenHandle arg);
GenHandle giac_cabi_floor(GenHandle arg);
GenHandle giac_cabi_ceil(GenHandle arg);
GenHandle giac_cabi_re(GenHandle arg);
GenHandle giac_cabi_im(GenHandle arg);
GenHandle giac_cabi_conj(GenHandle arg);
GenHandle giac_cabi_normal(GenHandle arg);
GenHandle giac_cabi_evalf(GenHandle arg);
/* Two-argument */
GenHandle giac_cabi_diff(GenHandle expr, GenHandle var);
GenHandle giac_cabi_integrate(GenHandle expr, GenHandle var);
GenHandle giac_cabi_solve(GenHandle expr, GenHandle var);
GenHandle giac_cabi_gcd(GenHandle a, GenHandle b);
GenHandle giac_cabi_lcm(GenHandle a, GenHandle b);
GenHandle giac_cabi_pow(GenHandle base, GenHandle exponent);
/* Three-argument */
GenHandle giac_cabi_subst(GenHandle expr, GenHandle var, GenHandle val);
GenHandle giac_cabi_limit(GenHandle expr, GenHandle var, GenHandle val);
GenHandle giac_cabi_series(GenHandle expr, GenHandle var, GenHandle order);

/* === Gen Lifecycle === */
GenHandle giac_cabi_gen_create_default(void);
GenHandle giac_cabi_gen_create_from_string(const char* expr);
GenHandle giac_cabi_gen_create_from_int64(int64_t value);
GenHandle giac_cabi_gen_create_from_double(double value);
GenHandle giac_cabi_gen_copy(GenHandle src);
void      giac_cabi_gen_destroy(GenHandle handle);

/* === Gen Accessors === */
char*   giac_cabi_gen_to_string(GenHandle handle);
int     giac_cabi_gen_type(GenHandle handle);
int32_t giac_cabi_gen_subtype(GenHandle handle);
char*   giac_cabi_gen_type_name(GenHandle handle);

/* Typed accessors */
int64_t giac_cabi_gen_to_int64(GenHandle handle);
int32_t giac_cabi_gen_to_int32(GenHandle handle);
double  giac_cabi_gen_to_double(GenHandle handle);
char*   giac_cabi_gen_zint_to_string(GenHandle handle);
int     giac_cabi_gen_zint_sign(GenHandle handle);
/* Two-call pattern: call with buf=NULL to get size, then with buffer */
int32_t giac_cabi_gen_zint_to_bytes(GenHandle handle, uint8_t* buf, int32_t buf_size);

/* Complex */
GenHandle giac_cabi_gen_cplx_re(GenHandle handle);
GenHandle giac_cabi_gen_cplx_im(GenHandle handle);

/* Fraction */
GenHandle giac_cabi_gen_frac_num(GenHandle handle);
GenHandle giac_cabi_gen_frac_den(GenHandle handle);

/* Vector */
int32_t   giac_cabi_gen_vect_size(GenHandle handle);
GenHandle giac_cabi_gen_vect_at(GenHandle handle, int32_t i);

/* Symbolic */
char*     giac_cabi_gen_symb_sommet_name(GenHandle handle);
GenHandle giac_cabi_gen_symb_feuille(GenHandle handle);

/* Identifier */
char* giac_cabi_gen_idnt_name(GenHandle handle);

/* String */
char* giac_cabi_gen_strng_value(GenHandle handle);

/* Map */
int32_t   giac_cabi_gen_map_size(GenHandle handle);
GenHandle giac_cabi_gen_map_keys(GenHandle handle);
GenHandle giac_cabi_gen_map_values(GenHandle handle);

/* === Gen Predicates === */
int giac_cabi_gen_is_zero(GenHandle handle);
int giac_cabi_gen_is_one(GenHandle handle);
int giac_cabi_gen_is_integer(GenHandle handle);
int giac_cabi_gen_is_approx(GenHandle handle);
int giac_cabi_gen_is_numeric(GenHandle handle);
int giac_cabi_gen_is_vector(GenHandle handle);
int giac_cabi_gen_is_symbolic(GenHandle handle);
int giac_cabi_gen_is_identifier(GenHandle handle);
int giac_cabi_gen_is_fraction(GenHandle handle);
int giac_cabi_gen_is_complex(GenHandle handle);
int giac_cabi_gen_is_string(GenHandle handle);

/* === Gen Operations === */
GenHandle giac_cabi_gen_eval(GenHandle handle);
GenHandle giac_cabi_gen_simplify(GenHandle handle);
GenHandle giac_cabi_gen_expand(GenHandle handle);
GenHandle giac_cabi_gen_factor(GenHandle handle);

/* === Gen Arithmetic === */
GenHandle giac_cabi_gen_add(GenHandle a, GenHandle b);
GenHandle giac_cabi_gen_sub(GenHandle a, GenHandle b);
GenHandle giac_cabi_gen_mul(GenHandle a, GenHandle b);
GenHandle giac_cabi_gen_div(GenHandle a, GenHandle b);
GenHandle giac_cabi_gen_neg(GenHandle a);
int giac_cabi_gen_eq(GenHandle a, GenHandle b);
int giac_cabi_gen_neq(GenHandle a, GenHandle b);

/* === Gen Internal === */
void*     giac_cabi_gen_get_impl(GenHandle handle);
GenHandle giac_cabi_gen_from_impl(void* impl);

/* === Gen Construction (Feature 051) === */
GenHandle giac_cabi_make_identifier(const char* name);
GenHandle giac_cabi_make_zint_from_bytes(const uint8_t* bytes, int32_t byte_count, int32_t sign);
GenHandle giac_cabi_make_symbolic_unevaluated(const char* op_name, GenHandle* args, int32_t count);
GenHandle giac_cabi_make_complex(GenHandle re, GenHandle im);
GenHandle giac_cabi_make_fraction(GenHandle num, GenHandle den);
GenHandle giac_cabi_make_vect(GenHandle* elements, int32_t count, int32_t subtype);

/* === Gen Pointer Management (Feature 051/052) === */
void*     giac_cabi_gen_to_heap_ptr(GenHandle gen);
void      giac_cabi_free_gen_ptr(void* ptr);
char*     giac_cabi_gen_ptr_to_string(void* ptr);
int       giac_cabi_gen_ptr_type(void* ptr);
GenHandle giac_cabi_gen_from_heap_ptr(void* ptr);

/* === GiacContext === */
GiacContextHandle giac_cabi_context_create(void);
void   giac_cabi_context_destroy(GiacContextHandle handle);
char*  giac_cabi_context_eval(GiacContextHandle handle, const char* input);
void   giac_cabi_context_set_variable(GiacContextHandle handle, const char* name, const char* value);
char*  giac_cabi_context_get_variable(GiacContextHandle handle, const char* name);
void   giac_cabi_context_set_timeout(GiacContextHandle handle, double seconds);
double giac_cabi_context_get_timeout(GiacContextHandle handle);
void   giac_cabi_context_set_precision(GiacContextHandle handle, int digits);
int    giac_cabi_context_get_precision(GiacContextHandle handle);
int    giac_cabi_context_is_complex_mode(GiacContextHandle handle);
void   giac_cabi_context_set_complex_mode(GiacContextHandle handle, int enable);

#ifdef __cplusplus
}
#endif

#endif /* GIAC_CABI_H */
