
// gemm -- general double precision dense matrix-matrix multiplication.
//
// implement: C = alpha * A x B + beta * C, for matrices A, B, C
// Matrix C is M x N  (M rows, N columns)
// Matrix A is M x K
// Matrix B is K x N
//
// Your implementation should make no assumptions about the values contained in any input parameters.
#include <immintrin.h>
#include <omp.h>
#define A(i,j) A[(i)*k+(j)]
#define B(i,j) B[(i)*n+(j)]
#define C(i,j) C[(i)*n+(j)]
// sub matrix A 1*tile_size, B tile_size*tile_size, C 1*tile_size

void gemm_naive(int m, int n, int k, double *A, double *B, double *C,
                  double alpha, double beta) {
  int i, j, kk;
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) {
      double inner_prod = 0;
      for (kk = 0; kk < k; kk++) inner_prod += A[i * k + kk] * B[kk * n + j];
      C[i * n + j] = alpha * inner_prod + beta * C[i * n + j];
    }
  }
}

void block_gemm(int m, int n, int k, double *A, double *B, double *C, double alpha, double beta){
    //scale C by beta
    
    if(beta!=1){
        #pragma omp parallel for
        for (int i = 0; i < m; i++){
            for (int j = 0; j < n; j++){
                C(i,j) *= beta;
            }
        }
    }

    //tiling , https://csapp.cs.cmu.edu/public/waside/waside-blocking.pdf
    int tile_size = 4;
    int i,p,q,pp, qq;
    // #pragma omp parallel for num_threads(8)
    for (pp = 0; pp < k; pp += tile_size){
        for (qq = 0; qq < n; qq += tile_size){
            for (i = 0; i < m; i++){
                for (p = pp; p < pp + tile_size; p++){
                    double inner_prod = 0; // C[i][p]
                    for (q = qq; q < qq + tile_size; q++){
                        inner_prod += A(i,q) * B(q,p); //A[i][q] * B[q][p]
                    }
                    C(i,p) += alpha * inner_prod;
                }
            }
        }
    }
}
// register_tile_size 2 gemm
void reg_2_gemm(int m, int n, int k, double *A, double *B, double *C, double alpha, double beta){
    
    if(beta!=1){
        // #pragma omp parallel for
        for (int i = 0; i < m; i++){
            for (int j = 0; j < n; j++){
                C(i,j) *= beta;
            }
        }
    }

    int i, j, kk;
    int register_tile_size = 2;
    for (i = 0; i < m; i+= register_tile_size) {
        for (j = 0; j < n; j+= register_tile_size) {
            double c00=C(i,j);
            double c01=C(i,j+1);
            double c10=C(i+1,j);
            double c11=C(i+1,j+1);
            for(kk=0; kk<k; kk++){
                double a0 = alpha*A(i,kk);
                double a1 = alpha*A(i+1,kk);
                double b0 = B(kk,j);
                double b1 = B(kk,j+1);
                c00 += a0*b0;
                c01 += a0*b1;
                c10 += a1*b0;
                c11 += a1*b1;
                // loop kk+= register_tile_size;
                // double a00 = alpha*A(i,kk);
                // double a01 = alpha*A(i,kk+1);
                // double a10 = alpha*A(i+1,kk);
                // double a11 = alpha*A(i+1,kk+1);
                // double b00 = B(kk,j);
                // double b01 = B(kk,j+1);
                // double b10 = B(kk+1,j);
                // double b11 = B(kk+1,j+1);
                // c00 += a00*b00 + a01*b10;
                // c01 += a00*b01 + a01*b11;
                // c10 += a10*b00 + a11*b10;
                // c11 += a10*b01 + a11*b11;
            }
            C(i,j) = c00;
            C(i,j+1) = c01;
            C(i+1,j) = c10;
            C(i+1,j+1) = c11; 
        }
    }
}

// register_tile_size 4 gemm
void reg_4_gemm(int m, int n, int k, double *A, double *B, double *C, double alpha, double beta){
    
    if(beta!=1){
        // #pragma omp parallel for
        for (int i = 0; i < m; i++){
            for (int j = 0; j < n; j++){
                C(i,j) *= beta;
            }
        }
    }

    int i, j, kk;
    int register_tile_size = 4;
    for (i = 0; i < m; i+= register_tile_size) {
        for (j = 0; j < n; j+= register_tile_size) {
            double c00=C(i,j);
            double c01=C(i,j+1);
            double c02=C(i,j+2);
            double c03=C(i,j+3);
            double c10=C(i+1,j);
            double c11=C(i+1,j+1);
            double c12=C(i+1,j+2);
            double c13=C(i+1,j+3);
            double c20=C(i+2,j);
            double c21=C(i+2,j+1);
            double c22=C(i+2,j+2);
            double c23=C(i+2,j+3);
            double c30=C(i+3,j);
            double c31=C(i+3,j+1);
            double c32=C(i+3,j+2);
            double c33=C(i+3,j+3);
            for (kk=0;kk<k;kk++){
                double a0 = alpha*A(i,kk);
                double a1 = alpha*A(i+1,kk);
                double a2 = alpha*A(i+2,kk);
                double a3 = alpha*A(i+3,kk);
                double b0 = B(kk,j);
                double b1 = B(kk,j+1);
                double b2 = B(kk,j+2);
                double b3 = B(kk,j+3);
                c00 += a0*b0;
                c01 += a0*b1;
                c02 += a0*b2;
                c03 += a0*b3;
                c10 += a1*b0;
                c11 += a1*b1;
                c12 += a1*b2;
                c13 += a1*b3;
                c20 += a2*b0;
                c21 += a2*b1;
                c22 += a2*b2;
                c23 += a2*b3;
                c30 += a3*b0;
                c31 += a3*b1;
                c32 += a3*b2;
                c33 += a3*b3;
            }
            C(i,j) = c00;
            C(i,j+1) = c01;
            C(i,j+2) = c02;
            C(i,j+3) = c03;
            C(i+1,j) = c10;
            C(i+1,j+1) = c11;
            C(i+1,j+2) = c12;
            C(i+1,j+3) = c13;
            C(i+2,j) = c20;
            C(i+2,j+1) = c21;
            C(i+2,j+2) = c22;
            C(i+2,j+3) = c23;
            C(i+3,j) = c30;
            C(i+3,j+1) = c31;
            C(i+3,j+2) = c32;
            C(i+3,j+3) = c33;
        }
    }
}

//openmp + avx2 gemm 4*4 register tile size
void avx2_4_4_gemm(int m, int n, int k, double *A, double *B, double *C, double alpha, double beta){
    //scale C by beta
    
    if(beta!=1){
        #pragma omp parallel for
        for (int i = 0; i < m; i++){
            for (int j = 0; j < n; j++){
                C(i,j) *= beta;
            }
        }
    }
    // int i,j,kk;
    int register_tile_size = 4;
    __m256d valpha = _mm256_set1_pd(alpha);
    #pragma omp parallel for num_threads(8) collapse(2)
    for (int i = 0; i < m; i += register_tile_size){
        for (int j = 0 ;j < n; j += register_tile_size){
            __m256d c0 = _mm256_setzero_pd();
            __m256d c1 = _mm256_setzero_pd();
            __m256d c2 = _mm256_setzero_pd();
            __m256d c3 = _mm256_setzero_pd();
            for (int kk = 0 ;kk < k; kk++){
                __m256d a0 = _mm256_broadcast_sd(&A(i,kk));
                __m256d a1 = _mm256_broadcast_sd(&A(i+1,kk));
                __m256d a2 = _mm256_broadcast_sd(&A(i+2,kk));
                __m256d a3 = _mm256_broadcast_sd(&A(i+3,kk));
                __m256d b = _mm256_mul_pd(valpha, _mm256_load_pd(&B(kk,j)));
                c0 = _mm256_fmadd_pd(a0,b,c0);
                c1 = _mm256_fmadd_pd(a1,b,c1);
                c2 = _mm256_fmadd_pd(a2,b,c2);
                c3 = _mm256_fmadd_pd(a3,b,c3);
            }
            _mm256_store_pd(&C(i,j), _mm256_add_pd(c0,_mm256_load_pd(&C(i,j))));
            _mm256_store_pd(&C(i+1,j), _mm256_add_pd(c1,_mm256_load_pd(&C(i+1,j))));
            _mm256_store_pd(&C(i+2,j), _mm256_add_pd(c2,_mm256_load_pd(&C(i+2,j))));
            _mm256_store_pd(&C(i+3,j), _mm256_add_pd(c3,_mm256_load_pd(&C(i+3,j))));
        }
    }
}

//openmp + avx2 gemm 4*8 register tile size
void gemm(int m, int n, int k, double *A, double *B, double *C, double alpha, double beta){
    //scale C by beta
    // #pragma prefetch A
    // #pragma prefetch B
    // #pragma prefetch C
    if(beta!=1){
        #pragma omp parallel for
        for (int i = 0; i < m; i++){
            for (int j = 0; j < n; j++){
                C(i,j) *= beta;
            }
        }
    }
    
    // int register_m_tile_size = 4;
    // int register_n_tile_size = 8;
    __m256d valpha = _mm256_set1_pd(alpha);
    #pragma omp parallel for num_threads(8) collapse(2)
    for (int i = 0; i < m; i += 4){
        for (int j = 0 ;j < n; j += 8){
            __m256d c00 = _mm256_setzero_pd();
            __m256d c01 = _mm256_setzero_pd();
            __m256d c10 = _mm256_setzero_pd();
            __m256d c11 = _mm256_setzero_pd();
            __m256d c20 = _mm256_setzero_pd();
            __m256d c21 = _mm256_setzero_pd();
            __m256d c30 = _mm256_setzero_pd();
            __m256d c31 = _mm256_setzero_pd();
            for (int kk = 0 ;kk < k;kk++){
                __m256d a0 = _mm256_broadcast_sd(&A(i,kk));
                __m256d a1 = _mm256_broadcast_sd(&A(i+1,kk));
                __m256d a2 = _mm256_broadcast_sd(&A(i+2,kk));
                __m256d a3 = _mm256_broadcast_sd(&A(i+3,kk));
                __m256d b0 = _mm256_mul_pd(valpha, _mm256_load_pd(&B(kk,j)));
                __m256d b1 = _mm256_mul_pd(valpha, _mm256_load_pd(&B(kk,j+4)));
                c00 = _mm256_fmadd_pd(a0,b0,c00);
                c01 = _mm256_fmadd_pd(a0,b1,c01);
                c10 = _mm256_fmadd_pd(a1,b0,c10);
                c11 = _mm256_fmadd_pd(a1,b1,c11);
                c20 = _mm256_fmadd_pd(a2,b0,c20);
                c21 = _mm256_fmadd_pd(a2,b1,c21);
                c30 = _mm256_fmadd_pd(a3,b0,c30);
                c31 = _mm256_fmadd_pd(a3,b1,c31);
            }
            _mm256_store_pd(&C(i,j), _mm256_add_pd(c00,_mm256_load_pd(&C(i,j))));
            _mm256_store_pd(&C(i,j+4), _mm256_add_pd(c01,_mm256_load_pd(&C(i,j+4))));
            _mm256_store_pd(&C(i+1,j), _mm256_add_pd(c10,_mm256_load_pd(&C(i+1,j))));
            _mm256_store_pd(&C(i+1,j+4), _mm256_add_pd(c11,_mm256_load_pd(&C(i+1,j+4))));
            _mm256_store_pd(&C(i+2,j), _mm256_add_pd(c20,_mm256_load_pd(&C(i+2,j))));
            _mm256_store_pd(&C(i+2,j+4), _mm256_add_pd(c21,_mm256_load_pd(&C(i+2,j+4))));
            _mm256_store_pd(&C(i+3,j), _mm256_add_pd(c30,_mm256_load_pd(&C(i+3,j))));
            _mm256_store_pd(&C(i+3,j+4), _mm256_add_pd(c31,_mm256_load_pd(&C(i+3,j+4))));
        }
    }
}

//cache block gemm
void cache_gemm(int m, int n, int k, double *A, double *B, double *C, double alpha, double beta){
    
    if(beta!=1){
        #pragma omp parallel for
        for (int i = 0; i < m; i++){
            for (int j = 0; j < n; j++){
                C(i,j) *= beta;
            }
        }
    }
    int m_tile_size = 256;
    int n_tile_size = 256;
    int k_tile_size = 256;
    for( int i=0; i<m; i+=m_tile_size){
        for( int j=0; j<n; j+=n_tile_size){
            for( int kk=0; kk<k; kk+=k_tile_size){
                avx2_4_4_gemm(m_tile_size, n_tile_size, k_tile_size, &A(i,kk), &B(kk,j), &C(i,j), alpha, beta);
            }
        }
    }
}
